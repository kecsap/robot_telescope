#!/bin/bash

cp dht22_sensor_weewx.py /usr/bin/
cp dht22_sensor_weewx.sh /usr/bin/
cp dht22_sensor_weewx_startup.sh /etc/init.d/
update-rc.d dht22_sensor_weewx_startup.sh defaults
