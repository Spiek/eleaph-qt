#-------------------------------------------------
#
# Project created by QtCreator 2014-02-25T23:11:57
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = eleaphrpc
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# Include Eleaph
include("../../eleaph-qt.pri")

SOURCES += main.cpp

HEADERS += server.h \
           client.h

