win32:LIBS += -lws2_32 -lIphlpapi

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += ZMQ_STATIC
LIBS += -L$$PWD/../lib -lzmq
INCLUDEPATH += $$PWD/../include

SOURCES += main.cpp

unix:!macx: LIBS += -L/usr/local/lib -lzmq
