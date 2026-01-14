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

    QCommandLineOption emailOption(
        QStringList() << "e" << "email",
        "Email pour connexion automatique",
        "email",
        ""
    );
    parser.addOption(emailOption);

    QCommandLineOption passwordOption(
        QStringList() << "p" << "password",
        "Mot de passe pour connexion automatique",
        "password",
        ""
    );
    parser.addOption(passwordOption);

    QCommandLineOption avatarOption(
        QStringList() << "a" << "avatar",
        "Avatar pour connexion automatique",
        "avatar",
        "avataaars1.svg"
    );
    parser.addOption(avatarOption);

    QCommandLineOption noAutoLoginOption(
        "no-autologin",
        "Désactiver l'auto-login avec les credentials stockés"
    );
    parser.addOption(noAutoLoginOption);

    parser.process(app);

    QString playerName = parser.value(playerNameOption);
    QString autoLoginEmail = parser.value(emailOption);
    QString autoLoginPassword = parser.value(passwordOption);
    QString autoLoginAvatar = parser.value(avatarOption);
    bool disableAutoLogin = parser.isSet(noAutoLoginOption);

    qDebug() << "Nom du joueur:" << playerName;
    if (!autoLoginEmail.isEmpty()) {
        qDebug() << "Auto-login activé pour:" << autoLoginEmail << "avec avatar:" << autoLoginAvatar;
    }
    if (disableAutoLogin) {
        qDebug() << "Auto-login depuis QSettings désactivé";
    }

    QQuickStyle::setStyle("Material");
    
    QQmlApplicationEngine engine;
        
    // NetworkManager global
    NetworkManager networkManager;
    engine.rootContext()->setContextProperty("networkManager", &networkManager);
    engine.rootContext()->setContextProperty("defaultPlayerName", playerName);
    engine.rootContext()->setContextProperty("autoLoginEmail", autoLoginEmail);
    engine.rootContext()->setContextProperty("autoLoginPassword", autoLoginPassword);
    engine.rootContext()->setContextProperty("autoLoginAvatar", autoLoginAvatar);
    engine.rootContext()->setContextProperty("disableAutoLogin", disableAutoLogin);

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