#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <QRandomGenerator>
#include "server/NetworkManager.h"
#include "GameModel.h"
#include "WindowPositioner.h"

// Pointeur global vers NetworkManager pour le crash reporter
static NetworkManager* g_networkManager = nullptr;

// Handler de messages Qt personnalisé pour capturer et reporter les crashes
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Formatter le message normalement
    QString formattedMessage = qFormatLogMessage(type, context, msg);

    // Afficher dans la console (comportement par défaut)
    fprintf(stderr, "%s\n", formattedMessage.toLocal8Bit().constData());
    fflush(stderr);

    // Reporter seulement les erreurs critiques au serveur
    if ((type == QtCriticalMsg || type == QtFatalMsg) && g_networkManager) {
        QString errorMsg = QString("%1").arg(msg);
        QString stackTrace = QString("File: %1:%2, Function: %3")
                                .arg(context.file ? context.file : "unknown")
                                .arg(context.line)
                                .arg(context.function ? context.function : "unknown");

        // Envoyer au serveur via la méthode publique
        g_networkManager->reportCrash(errorMsg, stackTrace);

        qWarning() << "🔴 Crash reporté au serveur:" << errorMsg;
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Installer le handler de messages personnalisé pour le crash reporting
    qInstallMessageHandler(customMessageHandler);
    qInfo() << "✅ Crash reporter initialisé";

    // Configuration pour QSettings (utilisé par QtCore.Settings en QML)
    app.setOrganizationName("Nebuludik");
    app.setApplicationName("CoincheDelEspace");

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

    // Initialiser le pointeur global pour le crash reporter
    g_networkManager = &networkManager;

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

    // Capturer les erreurs/warnings QML spécifiques
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, [&networkManager](const QList<QQmlError> &warnings) {
        for (const QQmlError &warning : warnings) {
            if (!warning.isValid()) continue;

            QString msg = warning.toString();

            // Reporter seulement les vraies erreurs (pas les warnings mineurs)
            if (msg.contains("Error", Qt::CaseInsensitive) ||
                msg.contains("TypeError", Qt::CaseInsensitive) ||
                msg.contains("ReferenceError", Qt::CaseInsensitive)) {

                QString errorMsg = QString("QML Error: %1").arg(warning.description());
                QString stackTrace = QString("URL: %1, Line: %2, Column: %3")
                                        .arg(warning.url().toString())
                                        .arg(warning.line())
                                        .arg(warning.column());

                networkManager.reportCrash(errorMsg, stackTrace);
                qWarning() << "🔴 QML crash reporté au serveur:" << msg;
            }
        }
    });

    const QUrl url(QStringLiteral("qrc:/qml/MainMenu.qml"));
    engine.load(url);
    
    return app.exec();
}