QT += core websockets sql
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = coinche_server

# Fichiers sources du serveur
SOURCES += \
    server_main.cpp

# Headers
HEADERS += \
    GameServer.h \
    DatabaseManager.h \
    ../Player.h \
    ../Deck.h \
    ../Carte.h \
    ../GameModel.h

# Fichiers sources des classes partagées
SOURCES += \
    ../Player.cpp \
    ../Deck.cpp \
    ../Carte.cpp \
    ../GameModel.cpp \
    DatabaseManager.cpp

# Définir le répertoire de sortie
DESTDIR = $$PWD

# Inclure les chemins pour les headers
INCLUDEPATH += $$PWD \
               $$PWD/..

# Installation
target.path = /usr/local/bin
INSTALLS += target
