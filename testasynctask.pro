#-------------------------------------------------
#
# Project created by QtCreator 2014-11-22T17:00:23
#
#-------------------------------------------------

QT += core testlib concurrent
QT -= gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = testasynctask
TEMPLATE = app

CONFIG   += console
CONFIG   -= app_bundle
CONFIG += C++11


SOURCES +=\
    asynctask.cpp \
    test.cpp \
    asyncactiontask.cpp

HEADERS  += \
    asynctask.h \
    asyncactiontask.h

FORMS    += mainwindow.ui

DEFINES += SRCDIR=\\\"$$PWD/\\\"
