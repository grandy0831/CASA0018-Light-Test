# Voice-Controlled Gluttonous Snake

**Author:** Guandi Chen

**GitHub Repository:** [https://github.com/grandy0831/DL4SN-Voice-Controlled-Gluttonous-snake](https://github.com/grandy0831/DL4SN-Voice-Controlled-Gluttonous-snake)

**Edge Impulse Project:** [https://studio.edgeimpulse.com/studio/380405](https://studio.edgeimpulse.com/studio/380405)


## Introduction

This project aims to develop a voice-controlled system for the classic game Snake. With the rapid development of the Internet of Things (IoT) and smart home technologies, voice interaction is becoming an integral and efficient mode of everyday interaction. Inspired by trends in modern interactive technologies, this project explores the potential of voice recognition in game control by enabling directional commands such as "up," "down," "left," and "right" to manipulate game movements.

The game control system captures the player's voice commands through an integrated microphone, processes and recognizes them in real-time, and translates them into movement controls within the game. This interaction allows the player to seamlessly control the game using voice alone, without the need for physical controllers, creating a new gaming experience.

### Key Inspiration:
1. **Smart Assistants**: The everyday use of devices like Alexa and Google Home reflects the reliability and user-friendliness of voice-controlled systems.
2. **Voice Recognition in Apps**: Mobile applications utilizing voice commands are becoming commonplace, demonstrating the technology's versatility.
3. **Accessible Gaming**: The concept of making gaming accessible to everyone, regardless of physical capability, showcases the profound impact of inclusive design principles.

Through this project, I hope to explore the wider use of speech technology in entertainment and open up new possibilities for accessible game design.


## Research Question

The core question of this project is:

**"How can voice recognition technology be effectively applied to control the game Snake, enabling hands-free operation with response times and accuracy that meet the demands of real-time gameplay?"**

In addition, the project will investigate the feasibility and performance optimization strategies for using deep learning models on resource-constrained embedded systems to process voice commands.


## Application Overview

This project develops a voice-controlled Snake game that consists of several key components:

- **Voice Recognition Module:** This module collects audio through a microphone, capturing short commands such as "up," "down," "left," and "right," along with background noise and non-command sounds.
- **Data Preprocessing and Feature Extraction:** This involves processing the audio data to clip effective commands and extracting sound features using techniques like Mel Frequency Cepstral Coefficients (MFCC).
- **Deep Learning Model Training and Evaluation:** Utilizes various network architectures, such as 1D-CNN, on the Edge Impulse platform to train and evaluate models, aiming to improve recognition accuracy and response speed.
- **Command Execution Interface:** Deploys the trained model onto an embedded device (e.g., Arduino) and performs real-time recognition of user commands to control the direction of the Snake game.

The design focuses on model light-weighting and inference speed optimization to adapt to the computational capabilities of embedded devices while ensuring the smoothness and interactivity of the game experience.

<br><br>![Architecture Diagram](https://github.com/grandy0831/DL4SN-Voice-Controlled-Gluttonous-snake/assets/140076679/aa66579e-b67a-47b2-8369-aced552f9521)




## Data

### Data Collection

The dataset for this project comprises six types of audio samples: command instructions ("up," "down," "left," "right"), "unknown" (random words), and "noise" (various background noises). Command samples were collected using smartphone microphones from 15 participants of diverse nationalities, genders, ages, and accents to enhance dataset diversity and improve the model's generalization capabilities. "Unknown" and "noise" samples were sourced from Edge Impulse's keyword spotting audio sample library (Edge Impulse, 2023a).

### Data Preprocessing

The preprocessing steps included:

1. **Sample Clipping:** Cutting out segments containing a keyword from longer audio files to ensure each clip contains only one clear command or noise.
<br><br>![Sample Clipping](https://github.com/grandy0831/DL4SN-Voice-Controlled-Gluttonous-snake/assets/140076679/2c1f3581-3f2e-4ab9-afcc-e55bacbca24e)

2. **Audio Cleaning:** Manually removing audio clips with poor quality or recording errors.
<br><br>![Audio Cleaning](https://github.com/grandy0831/DL4SN-Voice-Controlled-Gluttonous-snake/assets/140076679/66cb1939-6b8c-4593-b812-964fbef1c9e2)

3. **Duration Standardization:** All audio clips were processed to uniform length to maintain consistency during feature extraction.
4. **Label Annotation:** Each audio clip was accurately labelled to ensure precise correspondence between training data and labels.

### Dataset Construction

Approximately 300 samples per command were collected as raw data. Using the Edge Impulse platform, this data was automatically divided into an 80% training set and a 20% testing set. This division ratio helps the model learn sufficient features while retaining enough data to validate the model's generalization ability.
<br><br>![Dataset Construction](https://github.com/grandy0831/DL4SN-Voice-Controlled-Gluttonous-snake/assets/140076679/4267c6ea-d3b3-4813-8818-55cffe2246ae)


### Feature Extraction

Mel Frequency Cepstral Coefficients (MFCC) techniques were employed to extract features from the audio. MFCC is extensively used in audio signal processing, particularly suitable for capturing qualitative characteristics of speech and music. It simulates the auditory perception of human ears, effectively capturing the short-term energy variations and spectral structure of audio signals, making it the preferred feature for sound recognition tasks(M. Rammo and N. Al-Hamdani, 2022).


## Model

### Overview of the Model

This project employs deep learning techniques for the classification and recognition of audio signals. To enhance the model's performance and adapt it to different application scenarios, three distinct network architectures were experimented with:

1. **One-Dimensional Convolutional Neural Network (1D-CNN):** Designed for one-dimensional temporal data, this architecture effectively extracts temporal features from audio signals by applying filters along the time axis, making it suitable for real-time processing of lengthy audio streams.
2. **Two-Dimensional Convolutional Neural Network (2D-CNN):** Primarily used for processing two-dimensional spatial data, such as images and spectrograms. In this project, audio signals are transformed into spectrograms, which are then processed by 2D-CNNs to capture the frequency domain features of the audio.
3. **Transfer Learning (MobileNetV2):** Utilizes the pre-trained MobileNetV2 model to leverage knowledge learned on large-scale datasets, enhancing the model's generalization ability and performance.

### Model Comparison and Selection

- **Data Characteristics:** Given the temporal features of audio data, 1D-CNNs naturally excel in processing time-series data.
- **Computational Efficiency:** Experimental results show that 1D-CNNs have much faster inference times compared to 2D-CNNs and MobileNetV2, which is crucial for scenarios requiring quick responses.
- **Resource Utilization:** 1D-CNNs consume less peak RAM compared to 2D-CNNs and MobileNetV2, demonstrating higher applicability on resource-constrained devices.
- **Performance Comparison:** Although 1D-CNNs and 2D-CNNs are similar in accuracy, 1D-CNNs offer significant advantages in inference speed and resource usage. While MobileNetV2 offers higher accuracy, its longer inference times and greater resource demands limit its application in latency-sensitive and resource-constrained environments.

![Model Comparison Chart](https://github.com/grandy0831/DL4SN-Voice-Controlled-Gluttonous-snake/assets/140076679/b0a0d3fa-78e8-4f21-acac-23be5878bd6c)


Considering all these factors, especially the real-time requirements and resource constraints of the project, the 1D-CNN was chosen as the final model architecture for efficiently handling real-time audio processing tasks.


## Experiments

### Overview

To assess the model's performance under different configurations, we adjusted the following parameters: number of epochs, learning rate, dropout, and the neural network architecture. Specific experimental setups included:

#### 1. Epoch Adjustment
- **Objective:** Analyze the impact of training duration on model accuracy and overfitting.
- **Methodology:** Trained the model from 20 to 400 epochs, monitoring accuracy and loss to find the optimal number to prevent overfitting and maximize performance.

#### 2. Learning Rate Adjustment
- **Objective:** Evaluate how different learning rates affect training efficiency and final model performance.
- **Methodology:** Varied learning rates from 0.001 to 0.02 to find the rate that provided the highest test accuracy while maintaining process stability.

#### 3. Dropout Ratio Adjustment
- **Objective:** Determine how different dropout settings impact the model’s overfitting prevention and accuracy.
- **Methodology:** Tested dropout ratios from 0.10 to 0.45 to assess their effects on model performance and identify the optimal ratio.

#### 4. Neural Network Architecture Comparison
- **Objective:** Compare the performance impacts of various neural network configurations, especially changes in neuron counts and layers.
- **Methodology:** Evaluated everything from simple single-layer architectures to complex multi-layer setups on accuracy, loss, inference time, and resource usage.

### Experimental Results

- **Epoch Effects:** Increasing epochs to 70 significantly improved training and test accuracies, indicating better learning. Training beyond 70 epochs provided diminishing returns, suggesting potential overfitting.
  ![Epoch Effects](https://github.com/grandy0831/DL4SN-Voice-Controlled-Gluttonous-snake/assets/140076679/b5636559-7a80-470f-bd98-f45f48bb2a2c)


- **Learning Rate Effects:** A learning rate of 0.005 optimized performance, achieving up to 87.88% accuracy, enhancing both learning efficiency and model performance.
  ![Learning Rate Effects](https://github.com/grandy0831/DL4SN-Voice-Controlled-Gluttonous-snake/assets/140076679/4ce1ab75-8796-469b-a6d1-f8a7a86ded56)


- **Dropout Effects:** A moderate dropout rate of 0.16 greatly enhanced test accuracy to 90.36%, effectively preventing overfitting.
  ![Dropout Effects](https://github.com/grandy0831/DL4SN-Voice-Controlled-Gluttonous-snake/assets/140076679/f75c9fcf-008a-4895-906a-1b410c435e22)

- **Neural Network Architecture Effects:** Multi-layer configurations (1D conv (16-16-24 neurons)) excelled in increasing model complexity and capability, with the highest accuracy reaching 94.02%.

  ![Architecture Effects](https://github.com/grandy0831/DL4SN-Voice-Controlled-Gluttonous-snake/assets/140076679/cc4ffc0d-d798-4c63-a04e-a6d656a7af32)


### Final Model Configuration

Considering both performance and resource efficiency, the final model configuration was determined as follows:

<br><br>![Model](https://github.com/grandy0831/DL4SN-Voice-Controlled-Gluttonous-snake/assets/140076679/c203b690-1860-4c62-aa55-77e277749a49)


This configuration ensured high accuracy (Test Accuracy: 94.02%) while maintaining low resource consumption and fast inference times, making it highly suitable for the real-time audio processing needs of my voice-controlled Gluttonous Snake game. Extensive testing of model performance under different parameter configurations showed that this setup offered the best performance, demonstrating excellent generalisation ability and efficiency.



## Results and Observations



## Bibliography
*If you added any references then add them in here using this format:*

1. Last name, First initial. (Year published). Title. Edition. (Only include the edition if it is not the first edition) City published: Publisher, Page(s). http://google.com

2. Last name, First initial. (Year published). Title. Edition. (Only include the edition if it is not the first edition) City published: Publisher, Page(s). http://google.com

*Tip: we use [https://www.citethisforme.com](https://www.citethisforme.com) to make this task even easier.* 

----

## Declaration of Authorship

I, AUTHORS NAME HERE, confirm that the work presented in this assessment is my own. Where information has been derived from other sources, I confirm that this has been indicated in the work.


*Digitally Sign by typing your name here*

ASSESSMENT DATE

Word count: 
