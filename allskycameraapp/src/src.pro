
TEMPLATE = app
TARGET = allskycameraapp
INCLUDEPATH += . ../libs/libsunrise/src

CONFIG += c++11 no_keywords

HEADERS += libs/libsunrise/src/colours.h libs/libsunrise/src/sunrise.h
LIBS += -L../libs/libsunrise/src -lsunrise

CONFIG += link_pkgconfig
PKGCONFIG += geoclue-2.0 glib-2.0

INCLUDEPATH += /usr/include/libgeoclue-2.0/
LIBS += -lgeoclue-2

INCLUDEPATH += /usr/include/libmindcommon /usr/include/libmindaibo /usr/include/libmindeye
LIBS += -lmindcommon -lmindaibo_core -lmindeye

SOURCES += main.cpp
