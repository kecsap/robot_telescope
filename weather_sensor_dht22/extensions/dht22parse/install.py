# installer for the dht22parse driver

from setup import ExtensionInstaller

def loader():
    return Dht22ParseInstaller()

class Dht22ParseInstaller(ExtensionInstaller):
    def __init__(self):
        super(Dht22ParseInstaller, self).__init__(
            version="0.1",
            name='dht22parse',
            description='File parser driver for DHT22 sensor file in weewx.',
            author="Csaba Kertesz",
            author_email="csaba.kertesz@gmail.com",
            config={
                'Station': {
                    'station_type': 'Dht22Parse'},
                'Dht22Parse': {
                    'poll_interval': '20',
                    'path': '/tmp/dht22_sensor',
                    'driver': 'user.dht22parse'}},
            files=[('bin/user', ['bin/user/dht22parse.py'])]
            )
