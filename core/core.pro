#-------------------------------------------------
#
# Project created by QtCreator 2019-12-25T17:01:29
#
#-------------------------------------------------
TARGET = onvifcore
CONFIG +=  c++11
TEMPLATE = lib
CONFIG -= app_bundle
CONFIG -= qt
# disables the lib prefix
#CONFIG += no_plugin_name_prefix
# disable symlinks & versioning
#CONFIG += staticlib
CONFIG += plugin

CONFIG(release, debug|release) {
    DESTDIR = release
}
CONFIG(debug, debug|release) {
    DESTDIR = debug
}

DEFINES += ONVIFCORE_LIBRARY
DEFINES += WITH_OPENSSL
DEFINES += WITH_DOM
DEFINES += WITH_ZLIB
DEFINES += WITH_COOKIES

INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/include/gsoap
INCLUDEPATH += $$PWD/include/gsoap/plugin
#INCLUDEPATH += $$PWD/include/gsoap/import
#INCLUDEPATH += $$PWD/gsoap/custom


HEADERS += \
    include/gsoap/plugin/calcrest.h \
    include/gsoap/plugin/curlapi.h \
    include/gsoap/plugin/httpda.h \
    include/gsoap/plugin/httpform.h \
    include/gsoap/plugin/httpget.h \
    include/gsoap/plugin/httpmd5.h \
    include/gsoap/plugin/httppipe.h \
    include/gsoap/plugin/httppost.h \
    include/gsoap/plugin/logging.h \
    include/gsoap/plugin/md5evp.h \
    include/gsoap/plugin/mecevp.h \
    include/gsoap/plugin/mq.h \
    include/gsoap/plugin/plugin.h \
    include/gsoap/plugin/sessions.h \
    include/gsoap/plugin/smdevp.h \
    include/gsoap/plugin/threads.h \
    include/gsoap/plugin/wsaapi.h \
#    include/gsoap/plugin/wsddapi.h \
    include/gsoap/plugin/wsseapi.h \
    include/gsoap/plugin/wstapi.h \
    include/gsoap/stdsoap2.h \
    include/soapAdvancedSecurityServiceBindingProxy.h \
    include/soapDeviceBindingProxy.h \
    include/soapDeviceIOBindingProxy.h \
    include/soapH.h \
    include/soapImagingBindingProxy.h \
    include/soapMediaBindingProxy.h \
    include/soapPTZBindingProxy.h \
    include/soapPullPointSubscriptionBindingProxy.h \
    include/soapRemoteDiscoveryBindingProxy.h \
    include/soapStub.h \
    include/wsdd.nsmap

SOURCES += \
    src/gsoap/dom.cpp \
    src/gsoap/plugin/curlapi.cpp \
    src/gsoap/plugin/httpda.cpp \
    src/gsoap/plugin/httpform.cpp \
    src/gsoap/plugin/httpget.cpp \
    src/gsoap/plugin/httpmd5.cpp \
#    src/gsoap/plugin/httpmd5test.cpp \
    src/gsoap/plugin/httppipe.cpp \
    src/gsoap/plugin/httppost.cpp \
    src/gsoap/plugin/logging.cpp \
    src/gsoap/plugin/md5evp.cpp \
    src/gsoap/plugin/mecevp.cpp \
    src/gsoap/plugin/mq.cpp \
    src/gsoap/plugin/plugin.cpp \
    src/gsoap/plugin/sessions.cpp \
    src/gsoap/plugin/smdevp.cpp \
    src/gsoap/plugin/threads.cpp \
    src/gsoap/plugin/wsaapi.cpp \
#    src/gsoap/plugin/wsddapi.cpp \
    src/gsoap/plugin/wsseapi.cpp \
    src/gsoap/stdsoap2.cpp \
    src/soapAdvancedSecurityServiceBindingProxy.cpp \
    src/soapC.cpp \
    src/soapDeviceBindingProxy.cpp \
    src/soapDeviceIOBindingProxy.cpp \
    src/soapImagingBindingProxy.cpp \
    src/soapMediaBindingProxy.cpp \
    src/soapPTZBindingProxy.cpp \
    src/soapPullPointSubscriptionBindingProxy.cpp \
    src/soapRemoteDiscoveryBindingProxy.cpp

LIBS += -lcrypto -lssl -lz

DISTFILES += \
    include/gsoap/import/README.txt
