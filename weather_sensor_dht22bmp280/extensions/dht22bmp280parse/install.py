# installer for the dht22bmp280parse driver

from setup import ExtensionInstaller

def loader():
    return Dht22Bmp280ParseInstaller()

class Dht22Bmp280ParseInstaller(ExtensionInstaller):
    def __init__(self):
        super(Dht22Bmp280ParseInstaller, self).__init__(
            version="0.1",
            name='dht22bmp280parse',
            description='File parser driver for DHT22/BMP280 sensor file in weewx.',
            author="Csaba Kertesz",
            author_email="csaba.kertesz@gmail.com",
            config={
                'Station': {
                    'station_type': 'Dht22Bmp280Parse'},
                'Dht22Bmp280Parse': {
                    'poll_interval': '20',
                    'path': '/tmp/dht22bmp280_sensor',
                    'driver': 'user.dht22bmp280parse'}},
            files=[('bin/user', ['bin/user/dht22bmp280parse.py'])]
            )
