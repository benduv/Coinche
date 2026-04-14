#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <QRandomGenerator>
#include <QQuickWindow>
#include <QFontDatabase>
#include <QFont>
#include <QScreen>
#include <QTimer>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QCoreApplication>
#endif
#include "server/NetworkManager.h"
#include "GameModel.h"
#include "WindowPositioner.h"
#include "OrientationHelper.h"

int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    // Utiliser le backend audio natif Android au lieu de FFmpeg
    // Évite les problèmes de compatibilité 16KB page size sur Android 15+
    qputenv("QT_MEDIA_BACKEND", "android");
#endif

    QGuiApplication app(argc, argv);

#ifdef Q_OS_ANDROID
    // Désactiver le clavier plein écran en mode paysage
    QJniObject activity = QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative",
        "activity",
        "()Landroid/app/Activity;");
    if (activity.isValid()) {
        QJniObject::callStaticMethod<void>(
            "fr/nebuludik/coinche/KeyboardHelper",
            "install",
            "(Landroid/app/Activity;)V",
            activity.object<jobject>());
    }
#endif

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

    // qDebug() << "Nom du joueur:" << playerName;
    if (!autoLoginEmail.isEmpty()) {
        // qDebug() << "Auto-login activé pour:" << autoLoginEmail << "avec avatar:" << autoLoginAvatar;
    }
    if (disableAutoLogin) {
        // qDebug() << "Auto-login depuis QSettings désactivé";
    }

    QQuickStyle::setStyle("Material");

    // Sauvegarder la police système avant de la changer
    QString systemFontFamily = app.font().family();

    // Police Orbitron globale (identique sur Windows et Android)
    int fontId = QFontDatabase::addApplicationFont(":/resources/fonts/Orbitron.ttf");
    if (fontId != -1) {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont appFont(family);
        app.setFont(appFont);
    }

    QQmlApplicationEngine engine;

    // NetworkManager global
    NetworkManager networkManager;

    engine.rootContext()->setContextProperty("networkManager", &networkManager);
    engine.rootContext()->setContextProperty("systemFontFamily", systemFontFamily);
    engine.rootContext()->setContextProperty("defaultPlayerName", playerName);
    engine.rootContext()->setContextProperty("autoLoginEmail", autoLoginEmail);
    engine.rootContext()->setContextProperty("autoLoginPassword", autoLoginPassword);
    engine.rootContext()->setContextProperty("autoLoginAvatar", autoLoginAvatar);
    engine.rootContext()->setContextProperty("disableAutoLogin", disableAutoLogin);

    // WindowPositioner pour positionner automatiquement les fenêtres
    WindowPositioner windowPositioner;
    engine.rootContext()->setContextProperty("windowPositioner", &windowPositioner);

    // OrientationHelper pour contrôler l'orientation sur Android via JNI
    OrientationHelper orientationHelper;
    engine.rootContext()->setContextProperty("orientationHelper", &orientationHelper);

    engine.rootContext()->setContextProperty("gameModel", QVariant::fromValue<QObject*>(nullptr));
    
    // Mettre à jour quand gameModel est créé
    QObject::connect(&networkManager, &NetworkManager::gameModelReady, [&]() {
        // qDebug() << "Exposition de gameModel au contexte QML";
        engine.rootContext()->setContextProperty("gameModel", networkManager.gameModel());
    });

    const QUrl url(QStringLiteral("qrc:/qml/MainMenu.qml"));

#if defined(Q_OS_ANDROID) && !(defined(__arm__) && !defined(__aarch64__))
    // Verrouiller l'orientation courante pendant le chargement pour éviter le bug
    // demi-écran si l'utilisateur tourne le téléphone dans la première seconde.
    // On détecte l'orientation initiale et on la verrouille, puis on déverrouille après 1s.
    {
        QSize screenSize = app.primaryScreen()->size();
        if (screenSize.width() > screenSize.height())
            orientationHelper.setLandscape();
        else
            orientationHelper.setPortrait();
    }
    QTimer::singleShot(1000, &app, [&orientationHelper]() {
        orientationHelper.setAllOrientations();
    });

    // showFullScreen via objectCreated pour être sûr d'activer le plein écran
    // dès que la fenêtre QML est construite (pendant engine.load())
    QQuickWindow *mainQmlWindow = nullptr;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [&mainQmlWindow](QObject *obj, const QUrl &) {
        if (!mainQmlWindow) {
            QQuickWindow *w = qobject_cast<QQuickWindow*>(obj);
            if (w) {
                mainQmlWindow = w;
                mainQmlWindow->showFullScreen();
                qDebug() << "Mode plein écran activé";
            }
        }
    }, Qt::DirectConnection);
#endif

    engine.load(url);

    // Empêcher la mise en veille et FLAG_KEEP_SCREEN_ON (Android uniquement)
    if (!engine.rootObjects().isEmpty()) {
        QObject *rootObject = engine.rootObjects().first();
        if (rootObject) {
            QQuickWindow *window = qobject_cast<QQuickWindow*>(rootObject);
            if (window) {
#if defined(Q_OS_ANDROID) && !(defined(__arm__) && !defined(__aarch64__))
                // (showFullScreen déjà appelé via objectCreated ci-dessus)
#endif
#ifdef Q_OS_ANDROID
                // Empêcher la mise en veille de l'écran pendant l'utilisation de l'app
                QJniObject androidActivity = QJniObject::callStaticObjectMethod(
                    "org/qtproject/qt/android/QtNative",
                    "activity",
                    "()Landroid/app/Activity;");
                if (androidActivity.isValid()) {
                    QJniObject androidWindow = androidActivity.callObjectMethod(
                        "getWindow", "()Landroid/view/Window;");
                    if (androidWindow.isValid()) {
                        const int FLAG_KEEP_SCREEN_ON = 0x00000080;
                        androidWindow.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                        qDebug() << "FLAG_KEEP_SCREEN_ON activé";
                    }
                }
#endif
            }
        }
    }

    return app.exec();
}