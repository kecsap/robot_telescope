# WeeWX integration for DHT22/BMP280 sensor on Raspberry Pi

Setup sensor polling:

1. Install Python DHT library for Raspberry Pi according to the instuctions

   https://github.com/adafruit/Adafruit_Python_DHT

2. Install the DHT sensor reader script in this directory

   cd sensor_polling
   sudo ./install_dht22bmp280_reader.sh

3. Reboot the Pi or start the polling manually

   /etc/init.d/dht22bmp280_sensor_weewx_startup.sh

4. Install the WeeWX on Raspberry Pi

   http://www.weewx.com/docs/apt-get.htm

5. Install the WeeWX extension

   sudo wee_extension --install extensions/dht22bmp280parse

6. Select the DHT22/BMP280 WeeWX driver

   sudo wee_config --reconfigure

7. Start WeeWX

   I have some strange locale problem on my Raspberry Pi, if this command does not work to start WeeWx:

   sudo /etc/init.d/weewx start

   ...try to start manually:

   sudo weewxd /etc/weewx/weewx.conf

   If the locale problem happens you can try this command:

   LANGUAGE=en_US.UTF-8 LC_ALL=en_US.UTF-8 weewxd /etc/weewx/weewx.conf

   If it works, you can enable in sensor_polling/dht22bmp280_sensor_weewx_startup.sh script then repeat the Step 2 and 3.
