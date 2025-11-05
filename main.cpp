#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <QRandomGenerator>
#include "NetworkManager.h"
#include "GameModel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // Parser les arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Coinche Game Client");
    parser.addHelpOption();
    
    QCommandLineOption playerNameOption(
        QStringList() << "n" << "name",
        "Nom du joueur",
        "playerName",
        "Joueur" + QString::number(QRandomGenerator::global()->bounded(1000))
    );
    parser.addOption(playerNameOption);
    
    parser.process(app);
    
    QString playerName = parser.value(playerNameOption);
    qDebug() << "Nom du joueur:" << playerName;

    QQuickStyle::setStyle("Material");
    
    QQmlApplicationEngine engine;

    // Initialize game components
    // Deck deck;
    // deck.shuffleDeck();

    // std::vector<Carte*> main1, main2, main3, main4;
    // deck.distribute(main1, main2, main3, main4);

    // std::vector<std::unique_ptr<Player>> players;
    // players.push_back(std::make_unique<Player>("Joueur1", main1, 0));
    // players.push_back(std::make_unique<Player>("Joueur2", main2, 1));
    // players.push_back(std::make_unique<Player>("Joueur3", main3, 2));
    // players.push_back(std::make_unique<Player>("Joueur4", main4, 3));

    // std::vector<std::reference_wrapper<std::unique_ptr<Player>>> playerRefs;
    // for (auto& player : players) {
    //     player->sortHand();
    //     playerRefs.push_back(std::ref(player));
    // }
    
    // GameModel gameModel(playerRefs, deck);
    // engine.rootContext()->setContextProperty("gameModel", &gameModel);
    
    NetworkManager networkManager;
    engine.rootContext()->setContextProperty("networkManager", &networkManager);
    engine.rootContext()->setContextProperty("defaultPlayerName", playerName);
    
    const QUrl url(QStringLiteral("qrc:/qml/MainMenu.qml"));
    engine.load(url);
    
    return app.exec();
}