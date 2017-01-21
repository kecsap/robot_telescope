#!/usr/bin/python

import Adafruit_DHT as dht

h,t = dht.read_retry(dht.DHT22, 4)
print 'outTemp={0:0.1f}'.format(t)
print 'outHumidity={0:0.1f}'.format(h)
