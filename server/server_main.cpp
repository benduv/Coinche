// server_main.cpp
#include <QCoreApplication>
#include "GameServer.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    GameServer server(1234); // port du serveur
    qDebug() << "Serveur de jeu demarre...";
    return app.exec();
}
