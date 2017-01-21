#!/bin/bash

killall dht22_sensor_weewx.sh
dht22_sensor_weewx.sh &

# This hack is needed for me to start weewx on Raspberry Pi
#LANGUAGE=en_US.UTF-8 LC_ALL=en_US.UTF-8 weewxd /etc/weewx/weewx.conf
