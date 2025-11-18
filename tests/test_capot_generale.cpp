#include <QCoreApplication>
#include "GameServerTest.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    std::cout << "==========================================" << std::endl;
    std::cout << "Lancement des tests fonctionnels CAPOT/GENERALE" << std::endl;
    std::cout << "==========================================" << std::endl << std::endl;

    try {
        GameServerTest::runAllTests();

        std::cout << "\n==========================================";
        std::cout << std::endl << "SUCCES: Tous les tests sont passes!";
        std::cout << std::endl << "==========================================\n" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cout << "\n==========================================";
        std::cout << std::endl << "ERREUR: Un test a echoue!";
        std::cout << std::endl << "Message: " << e.what();
        std::cout << std::endl << "==========================================\n" << std::endl;

        return 1;
    }
}
