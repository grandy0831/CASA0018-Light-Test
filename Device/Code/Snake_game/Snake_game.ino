/* Edge Impulse ingestion SDK
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// If your target is limited in memory remove this macro to save 10K RAM
#define EIDSP_QUANTIZE_FILTERBANK   0

/**
 * Define the number of slices per model window. E.g. a model window of 1000 ms
 * with slices per model window set to 4. Results in a slice size of 250 ms.
 * For more info: https://docs.edgeimpulse.com/docs/continuous-audio-sampling
 */
#define EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW 4

/*
 ** NOTE: If you run into TFLite arena allocation issue.
 **
 ** This may be due to may dynamic memory fragmentation.
 ** Try defining "-DEI_CLASSIFIER_ALLOCATION_STATIC" in boards.local.txt (create
 ** if it doesn't exist) and copy this file to
 ** `<ARDUINO_CORE_INSTALL_PATH>/arduino/hardware/<mbed_core>/<core_version>/`.
 **
 ** See
 ** (https://support.arduino.cc/hc/en-us/articles/360012076960-Where-are-the-installed-cores-located-)
 ** to find where Arduino installs cores on your machine.
 **
 ** If the problem persists then there's not enough memory for this model and application.
 */

/* Includes ---------------------------------------------------------------- */
#include <PDM.h>
#include <Voice-Controlled_Snake_Game_inferencing.h>
#include <LedControl.h> // MAX7219 library


// Parameters for MAX7219
#define DATA_IN   12
#define CLK       10
#define LOAD      11
#define MAX_DEVICES 1


#define MAX_SNAKE_LENGTH 64  // Maximum length of the snake, more than needed

struct Snake {
    int x[MAX_SNAKE_LENGTH]; // x coordinates of snake segments
    int y[MAX_SNAKE_LENGTH]; // y coordinates of snake segments
    int length;              // length of the snake
    String direction;        // current movement direction of the snake head
} snake;

struct Apple {
    int x;  // x coordinate of the apple
    int y;  // y coordinate of the apple
} apple;


LedControl lc = LedControl(DATA_IN, CLK, LOAD, MAX_DEVICES);

/** Audio buffers, pointers and selectors */
typedef struct {
    signed short *buffers[2];
    unsigned char buf_select;
    unsigned char buf_ready;
    unsigned int buf_count;
    unsigned int n_samples;
} inference_t;

static inference_t inference;
static bool record_ready = false;
static signed short *sampleBuffer;
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static int print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);

/**
 * @brief      Arduino setup function
 */
void setup()
{
    Serial.begin(115200);
    while (!Serial);

    randomSeed(analogRead(0));  

    lc.shutdown(0, false);
    lc.setIntensity(0, 8);
    lc.clearDisplay(0);


    // Initialize snake
    snake.x[0] = 4;  // Start in the middle of the display
    snake.y[0] = 4;
    snake.x[1] = 4;  // Second segment
    snake.y[1] = 5;
    snake.x[2] = 4;  // Third segment
    snake.y[2] = 6;
    snake.length = 3;
    snake.direction = "up";  // Initial direction

    place_apple();  // Place the initial apple


    // summary of inferencing settings (from model_metadata.h)
    ei_printf("Inferencing settings:\n");
    ei_printf("\tInterval: %.2f ms.\n", (float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) /
                                            sizeof(ei_classifier_inferencing_categories[0]));

    run_classifier_init();
    if (microphone_inference_start(EI_CLASSIFIER_SLICE_SIZE) == false) {
        ei_printf("ERR: Could not allocate audio buffer (size %d), this could be due to the window length of your model\r\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
        return;
    }
}

int dot_x = 3, dot_y = 3; // Start in the middle of an 8x8 matrix

/**
 * @brief      Arduino main function. Runs the inferencing loop.
 */
void loop()
{
    bool m = microphone_inference_record();
    if (!m) {
        ei_printf("ERR: Failed to record audio...\n");
        return;
    }

    signal_t signal;
    signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
    signal.get_data = &microphone_audio_signal_get_data;
    ei_impulse_result_t result = {0};

    EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, debug_nn);
    if (r != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", r);
        return;
    }

    // Evaluate the classifier results
    handle_classifier_result(result);

    // Delay slightly before next inferencing
    delay(10);
}

/**
 * @brief      PDM buffer full callback
 *             Get data and call audio thread callback
 */
static void pdm_data_ready_inference_callback(void)
{
    int bytesAvailable = PDM.available();

    // read into the sample buffer
    int bytesRead = PDM.read((char *)&sampleBuffer[0], bytesAvailable);

    if (record_ready == true) {
        for (int i = 0; i<bytesRead>> 1; i++) {
            inference.buffers[inference.buf_select][inference.buf_count++] = sampleBuffer[i];

            if (inference.buf_count >= inference.n_samples) {
                inference.buf_select ^= 1;
                inference.buf_count = 0;
                inference.buf_ready = 1;
            }
        }
    }
}

/**
 * @brief      Init inferencing struct and setup/start PDM
 *
 * @param[in]  n_samples  The n samples
 *
 * @return     { description_of_the_return_value }
 */
static bool microphone_inference_start(uint32_t n_samples)
{
    inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));

    if (inference.buffers[0] == NULL) {
        return false;
    }

    inference.buffers[1] = (signed short *)malloc(n_samples * sizeof(signed short));

    if (inference.buffers[1] == NULL) {
        free(inference.buffers[0]);
        return false;
    }

    sampleBuffer = (signed short *)malloc((n_samples >> 1) * sizeof(signed short));

    if (sampleBuffer == NULL) {
        free(inference.buffers[0]);
        free(inference.buffers[1]);
        return false;
    }

    inference.buf_select = 0;
    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;

    // configure the data receive callback
    PDM.onReceive(&pdm_data_ready_inference_callback);

    PDM.setBufferSize((n_samples >> 1) * sizeof(int16_t));

    // initialize PDM with:
    // - one channel (mono mode)
    // - a 16 kHz sample rate
    if (!PDM.begin(1, EI_CLASSIFIER_FREQUENCY)) {
        ei_printf("Failed to start PDM!");
    }

    // set the gain, defaults to 20
    PDM.setGain(127);

    record_ready = true;

    return true;
}

/**
 * @brief      Wait on new data
 *
 * @return     True when finished
 */
static bool microphone_inference_record(void)
{
    bool ret = true;

    if (inference.buf_ready == 1) {
        ei_printf(
            "Error sample buffer overrun. Decrease the number of slices per model window "
            "(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW)\n");
        ret = false;
    }

    while (inference.buf_ready == 0) {
        delay(1);
    }

    inference.buf_ready = 0;

    return ret;
}

/**
 * Get raw audio signal data
 */
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);

    return 0;
}

/**
 * @brief      Stop PDM and release buffers
 */
static void microphone_inference_end(void)
{
    PDM.end();
    free(inference.buffers[0]);
    free(inference.buffers[1]);
    free(sampleBuffer);
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif


void update_snake_position() {
    int new_x = snake.x[0];
    int new_y = snake.y[0];

    if (snake.direction == "up") {
        new_y = (new_y - 1 + 8) % 8;
    } else if (snake.direction == "down") {
        new_y = (new_y + 1) % 8;
    } else if (snake.direction == "left") {
        new_x = (new_x - 1 + 8) % 8;
    } else if (snake.direction == "right") {
        new_x = (new_x + 1) % 8;
    }

    // 检查自身碰撞
    for (int i = 1; i < snake.length; i++) {
        if (snake.x[i] == new_x && snake.y[i] == new_y) {
            Serial.println("Game Over: Collided with itself!");
            setup();  // 重置游戏
            return;
        }
    }

    // 移动蛇身
    for (int i = snake.length - 1; i > 0; i--) {
        snake.x[i] = snake.x[i-1];
        snake.y[i] = snake.y[i-1];
    }

    // 更新蛇头位置
    snake.x[0] = new_x;
    snake.y[0] = new_y;

    // 检查是否吃到苹果
    if (new_x == apple.x && new_y == apple.y) {
        if (snake.length < MAX_SNAKE_LENGTH) {
            // 增加蛇的长度
            snake.length++;
            snake.x[snake.length - 1] = snake.x[snake.length - 2];
            snake.y[snake.length - 1] = snake.y[snake.length - 2];
        }
        place_apple();  // 放置新苹果
    }

    lc.clearDisplay(0);  // 清屏
    for (int i = 0; i < snake.length; i++) {
        lc.setLed(0, snake.y[i], snake.x[i], true);  // 重新绘制蛇
    }
    lc.setLed(0, apple.y, apple.x, true);  // 确保苹果也被重新绘制
}




void place_apple() {
    lc.clearDisplay(0);  // 清除显示，准备放置新的苹果
    bool apple_placed = false;
    while (!apple_placed) {
        apple.x = random(8);  // 在0到7之间生成一个随机x坐标
        apple.y = random(8);  // 在0到7之间生成一个随机y坐标

        apple_placed = true;
        for (int i = 0; i < snake.length; i++) {
            if (snake.x[i] == apple.x && snake.y[i] == apple.y) {
                apple_placed = false;  // 如果苹果生成在蛇身上，重新生成
                break;
            }
        }
    }
    lc.setLed(0, apple.y, apple.x, true);  // 在新位置显示苹果

    // 重新绘制蛇，因为清屏会清除蛇的显示
    for (int i = 0; i < snake.length; i++) {
        lc.setLed(0, snake.y[i], snake.x[i], true);
    }
}




void handle_classifier_result(const ei_impulse_result_t& result) {
    float highest_confidence = 0.0;
    String command = "none";

    // Determine the command with the highest confidence
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        if (result.classification[ix].value > highest_confidence) {
            command = result.classification[ix].label;
            highest_confidence = result.classification[ix].value;
        }
    }

    Serial.print("Command detected: ");
    Serial.print(command);
    Serial.print(" with confidence: ");
    Serial.println(highest_confidence, 5);

    // Update snake direction based on the command
    if (highest_confidence > 0.5) {
        if (command == "up" || command == "down" || command == "left" || command == "right") {
            snake.direction = command;
        }
    }

    update_snake_position();  // Move snake in the new direction
}

