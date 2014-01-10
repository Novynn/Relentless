#-------------------------------------------------
#
# Project created by QtCreator 2012-04-21T19:37:25
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = SBNLSP
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    qsslserver.cpp \
    bnlsproxy.cpp

HEADERS += \
    qsslserver.h \
    bnlsproxy.h
