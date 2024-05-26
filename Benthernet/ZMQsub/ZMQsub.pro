TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += ZMQ_STATIC
win32: LIBS += -lws2_32 -lIphlpapi
LIBS += -L$$PWD/../lib -lzmq

INCLUDEPATH += $$PWD/../include

SOURCES += main.cpp \
    BaseCommunication.cpp \
    randomsentenceservice.cpp \
    spellcheckerservice.cpp

HEADERS += \
    BaseCommunication.h \
    randomsentenceservice.h \
    spellcheckerservice.h
