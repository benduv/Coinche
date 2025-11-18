#include <QCoreApplication>
#include "server/GameServerTest.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Lancement des tests fonctionnels CAPOT/GENERALE...";

    GameServerTest::runAllTests();

    qDebug() << "\nTests termines avec succes!";

    return 0;
}
