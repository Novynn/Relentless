#-------------------------------------------------
#
# Project created by QtCreator 2011-06-19T11:06:47
#
#-------------------------------------------------

QT += core network

win32 {
    QT += widgets
}

CONFIG += c++11

LIBS += -lz

TARGET = relentless

CONFIG += console

DESTDIR = ../bin

TEMPLATE = app

INCLUDEPATH += shared

SOURCES += main.cpp \
    core.cpp \
    client/clientcore.cpp \
    game/gamecore.cpp \
    shared/qbytearraybuilder.cpp \
    game/map/mpqfile.cpp \
    game/map/mpqarchive.cpp \
    game/map/sha1.cpp \
    game/map/map.cpp \
    game/map/crc32.cpp \
    game/game.cpp \
    game/instance.cpp \
    game/lobby.cpp \
    game/player.cpp \
    client/clientprotocol.cpp \
    client/client.cpp \
    shared/packet/packet.cpp \
    logger.cpp \
    plugins/plugincore.cpp \
    game/gameprotocol.cpp \
    game/map/slotmap.cpp \
    client/clientproxy.cpp \
    client/bnlsprotocol.cpp

HEADERS += \
    core.h \
    client/clientcore.h \
    game/gamecore.h \
    shared/qbytearraybuilder.h \
    shared/packet/packet.h \
    game/map/mpqfile.h \
    game/map/mpqarchive.h \
    game/map/sha1.h \
    game/map/map.h \
    game/map/crc32.h \
    shared/functions.h \
    game/game.h \
    game/instance.h \
    game/lobby.h \
    game/player.h \
    coreobject.h \
    client/clientprotocol.h \
    client/client.h \
    game/gamelistener.h \
    game/map/slotmap.h \
    shared/packet/bncspacket.h \
    shared/packet/bnlspacket.h \
    logger.h \
    plugins/plugincore.h \
    shared/MessageOrigin.h \
    shared/packet/w3gspacket.h \
    game/gameprotocol.h \
    shared/plugin/plugin.h \
    shared/MessageType.h \
    client/friend.h \
    client/clientproxy.h \
    client/bnlsprotocol.h \
    client/protocol.h





























