TEMPLATE = app
CONFIG += c++11 console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += ZMQ_STATIC

INCLUDEPATH += $$PWD/../include

unix:!macx: LIBS += -lzmq
win32: LIBS += -lws2_32 -lIphlpapi

SOURCES += main.cpp \
    mainwindow.cpp \
    mainwindow.cpp \
    client.cpp

HEADERS += mainwindow.h \
    client.h \
    mainwindow.h

FORMS += mainwindow.ui
