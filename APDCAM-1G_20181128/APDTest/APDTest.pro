#-------------------------------------------------
#
# Project created by QtCreator 2015-01-19T09:41:28
#
#-------------------------------------------------

QT       -= core

QT       -= gui

TARGET = APDTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    APDTest.cpp


INCLUDEPATH += ../APDLib

LIBS += ../build-APDLib-Desktop_Qt_5_10_0_GCC_64bit-Debug/libAPDLib.a
LIBS += ../build-APDLib-Desktop_Qt_5_10_0_GCC_64bit-Release/libAPDLib.a
LIBS += -lcap

