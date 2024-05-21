win32:LIBS += -lws2_32 -lIphlpapi

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += ZMQ_STATIC
INCLUDEPATH += $$PWD/../include

# Remove the local libzmq.a reference
# LIBS += -L$$PWD/../lib -lzmq

# Use the system-installed libzmq
unix:!macx: LIBS += -lzmq

SOURCES += main.cpp
