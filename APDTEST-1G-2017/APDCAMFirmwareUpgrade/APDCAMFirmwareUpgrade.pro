#-------------------------------------------------
#
# Project created by QtCreator 2015-09-10T10:08:38
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = APDCAMFirmwareUpgrade
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

INCLUDEPATH += ../APDLib

LIBS += ../build-APDLib-Desktop_Qt_5_4_1_GCC_64bit-Debug/libAPDLib.a
LIBS += ../build-APDLib-Desktop_Qt_5_4_1_GCC_64bit-Release/libAPDLib.a
LIBS += -lcap
