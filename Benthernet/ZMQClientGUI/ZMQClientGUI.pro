QT += core gui widgets

TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp \
           mainwindow.cpp \
           client.cpp

HEADERS += mainwindow.h \
           client.h

FORMS += mainwindow.ui

# ZeroMQ settings
DEFINES += ZMQ_STATIC
INCLUDEPATH += "C:/Users/dries/OneDrive/Desktop/git/Pxl_Benternet/Benthernet/include"

LIBS += -L"C:/Users/dries/OneDrive/Desktop/git/Pxl_Benternet/Benthernet/lib" -lzmq

win32: LIBS += -lws2_32 -lIphlpapi
