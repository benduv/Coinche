#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <QRandomGenerator>
#include "server/NetworkManager.h"
#include "GameModel.h"
#include "WindowPositioner.h"

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
        
    // NetworkManager global
    NetworkManager networkManager;
    engine.rootContext()->setContextProperty("networkManager", &networkManager);
    engine.rootContext()->setContextProperty("defaultPlayerName", playerName);

    // WindowPositioner pour positionner automatiquement les fenêtres
    WindowPositioner windowPositioner;
    engine.rootContext()->setContextProperty("windowPositioner", &windowPositioner);

    engine.rootContext()->setContextProperty("gameModel", QVariant::fromValue<QObject*>(nullptr));
    
    // Mettre à jour quand gameModel est créé
    QObject::connect(&networkManager, &NetworkManager::gameModelReady, [&]() {
        qDebug() << "Exposition de gameModel au contexte QML";
        engine.rootContext()->setContextProperty("gameModel", networkManager.gameModel());
    });
    
    const QUrl url(QStringLiteral("qrc:/qml/MainMenu.qml"));
    engine.load(url);
    
    return app.exec();
}