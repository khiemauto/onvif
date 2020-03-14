#-------------------------------------------------
#
# Project created by QtCreator 2019-12-25T17:01:29
#
#-------------------------------------------------

TARGET = ipcamera
TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

target.path = /media/card
INSTALLS += target

CONFIG(release, debug|release) {
    DESTDIR = release
    LIBS += -L../core/release -lonvifcore

}
CONFIG(debug, debug|release) {
    DESTDIR = debug
    LIBS += -L../core/debug -lonvifcore

}

#DEFINES += WITH_OPENSSL
#DEFINES += WITH_DOM
#DEFINES += WITH_ZLIB
#DEFINES += WITH_COOKIES

INCLUDEPATH += ../core/include
INCLUDEPATH += ../core/include/gsoap
INCLUDEPATH += ../core/include/gsoap/plugin

HEADERS +=

SOURCES += \
    main.cpp

LIBS += -lonvifcore
LIBS += -lcurl
LIBS += -lpthread
