#!/usr/bin/python
#
# weewx driver that reads data from /tmp/dht22_sensor
#
# Temperature and humidity values are in metric form:
#
# outTemp=12.2
# outHumidity=73.2
#

from __future__ import with_statement
import syslog
import time

import weewx.drivers

DRIVER_NAME = 'Dht22Parse'
DRIVER_VERSION = "0.1"

def logmsg(dst, msg):
    syslog.syslog(dst, 'dht22parse: %s' % msg)

def logdbg(msg):
    logmsg(syslog.LOG_DEBUG, msg)

def loginf(msg):
    logmsg(syslog.LOG_INFO, msg)

def logerr(msg):
    logmsg(syslog.LOG_ERR, msg)

def _get_as_float(d, s):
    v = None
    if s in d:
        try:
            v = float(d[s])
        except ValueError, e:
            logerr("cannot read value for '%s': %s" % (s, e))
    return v

def loader(config_dict, engine):
    return Dht22FileParseDriver(**config_dict[DRIVER_NAME])

class Dht22FileParseDriver(weewx.drivers.AbstractDevice):
    """weewx driver that reads dht22 data from /tmp/dht22_sensor"""

    def __init__(self, **stn_dict):
        self.path = stn_dict.get('path', '/tmp/dht22_sensor')
        self.poll_interval = float(stn_dict.get('poll_interval', 30))
        self.label_map = stn_dict.get('label_map', {})

        loginf("data file is %s" % self.path)
        loginf("polling interval is %s" % self.poll_interval)
        loginf('label map is %s' % self.label_map)

    def genLoopPackets(self):
        while True:
            # read whatever values we can get from the file
            data = {}
            try:
                with open(self.path) as f:
                    for line in f:
                        eq_index = line.find('=')
                        name = line[:eq_index].strip()
                        value = line[eq_index + 1:].strip()
                        data[name] = value
            except Exception, e:
                logerr("read failed: %s" % e)

            # map the data into a weewx loop packet
            _packet = {'dateTime': int(time.time() + 0.5),
                       'usUnits': weewx.METRIC}
            for vname in data:
                _packet[self.label_map.get(vname, vname)] = _get_as_float(data, vname)

            yield _packet
            time.sleep(self.poll_interval)

    @property
    def hardware_name(self):
        return "Dht22Parse"

if __name__ == "__main__":
    import weeutil.weeutil
    driver = Dht22FileParseDriver()
    for packet in driver.genLoopPackets():
        print weeutil.weeutil.timestamp_to_string(packet['dateTime']), packet
