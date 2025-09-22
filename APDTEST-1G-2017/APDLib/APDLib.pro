#-------------------------------------------------
#
# Project created by QtCreator 2015-01-22T10:42:16
#
#-------------------------------------------------

QT       -= core gui

TARGET = APDLib
TEMPLATE = lib


CONFIG(release, debug|release) {
    #This is a release build
    DEFINES += APD_VERSION
} else {
    #This is a debug build
    DEFINES += APD_VERSION
}

CONFIG += staticlib

SOURCES += HighlevelFunctions.cpp \
    DataEvaluation.cpp \
    Helpers.cpp \
    LnxClasses.cpp \
    InternalFunctions.cpp \
    LowlevelFunctions.cpp \
    CamClient.cpp \
    CamServer.cpp \
    GECClient.cpp \
    GECCommands.cpp \
    SysLnxClasses.cpp \
    UDPClient.cpp \
    UDPServer.cpp \
    helper.cpp

HEADERS += APDLib.h \
    DataEvaluation.h \
    Helpers.h \
    LnxClasses.h \
    TypeDefs.h \
    InternalFunctions.h \
    LowlevelFunctions.h \
    CamClient.h \
    CamServer.h \
    GECClient.h \
    GECCommands.h \
    InterfaceDefs.h \
    SysLnxClasses.h \
    UDPClient.h \
    UDPServer.h \
    helper.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
