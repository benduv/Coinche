#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "Deck.h"
#include "Carte.h"
#include "Player.h"
#include "GameManager.h"
#include "CardModel.h"
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

    // Create game manager and card model
    GameManager gameManager(playerRefs, Carte::CARREAU, 0);
    CardModel *cardsModel = new CardModel(&app);

    // Create QML engine
    QQmlApplicationEngine engine;

    // Expose C++ objects to QML
    engine.rootContext()->setContextProperty("gameManager", &gameManager);
    engine.rootContext()->setContextProperty("cardsModel", cardsModel);

    // Set initial cards in the model
    cardsModel->setCards(players[0]->getCartes());  // Show first player's cards

    // Load main QML file
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}