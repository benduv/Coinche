#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "Deck.h"
#include "Carte.h"
#include "Player.h"
#include "GameManager.h"
#include "GameModel.h"
#include <memory>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // Initialize game components
    Deck deck;
    deck.shuffleDeck();

    std::vector<Carte*> main1, main2, main3, main4;
    deck.distribute(main1, main2, main3, main4);

    // Create players
    std::vector<std::unique_ptr<Player>> players;
    players.push_back(std::make_unique<Player>("Joueur1", main1, 0));
    players.push_back(std::make_unique<Player>("Joueur2", main2, 1));
    players.push_back(std::make_unique<Player>("Joueur3", main3, 2));
    players.push_back(std::make_unique<Player>("Joueur4", main4, 3));

    // Create references vector for GameManager
    std::vector<std::reference_wrapper<std::unique_ptr<Player>>> playerRefs;
    for (auto& player : players) {
        playerRefs.push_back(std::ref(player));
    }

        // Créer le GameManager
    GameManager gameManager(playerRefs, Carte::COEUR, 0);
    
    // Créer le modèle QML
    GameModel gameModel(playerRefs, deck);
    
    // Créer et configurer le moteur QML
    QQmlApplicationEngine engine;
    
    // Exposer le modèle à QML
    engine.rootContext()->setContextProperty("gameModel", &gameModel);
    
    // Charger le fichier QML principal
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    engine.load(url);
    
    if (engine.rootObjects().isEmpty())
        return -1;
    
    return app.exec();

}