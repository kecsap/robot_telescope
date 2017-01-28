# All-sky camera application on Raspberry Pi

This small C++ application captures images with the Raspberry Pi camera,
detects clear sky in the night and uploads the images as a webcam for Wunderground service.
This program is written for my own purpose, but it can be adapted with minimal source changes into a
new environment.

## Features

* Automatic GPS coordinate determination with geoclue based on the IP address.
* Automatic sunset/sunrise calculation based on GPS coordinates.
* Automatic shutter time based on the average image brightness.
* Using mask image to filter out the environment silhouette for clear sky detection.
* Detecting clear sky based on the dark/light areas of the sky.
* Upload the captured image to Wunderground service.

## How to compile on Raspbian

1. Add my Ubuntu PPA for RaspPi to the package sources:

   sudo nano /etc/apt/sources.list.d/aiboplus-rpi.list
   ```
   deb http://ppa.launchpad.net/csaba-kertesz/aiboplus-rpi/ubuntu trusty main
   deb-src http://ppa.launchpad.net/csaba-kertesz/aiboplus-rpi/ubuntu trusty main
   ```

2. Update package repos

   - sudo apt-get update
   - sudo apt-get upgrade
   
3. Install Qt5, geoclue and other dependencies:

   - sudo apt-get install qt5-default qt5-qmake qtbase5-dev qtbase5-dev-tools
   - sudo apt-get install libgeoclue-2-dev geoclue-2.0 gir1.2-geoclue-2.0
   - sudo apt-get install libmindcommon-dev libmindaibo-dev libmindeye-dev

4. After cloning this repo, compile the application with

   - cd allskycameraapp
   - qmake
   - make -j3

5. Run the application:

   - cd src
   - ./allskycameraapp Wunderground_camera_ID Wunderground_password

Optional 1: Create an RGB image (mask.png) which contains white (255, 255, 255) areas over the environment silhouette.
Place this file in allskycameraapp/src before running the application. This image is used for the clear sky detection.

Optional 2: Create an RGB image (info_layer.jpg) to composite any informative graphics or text to the image in night mode.
