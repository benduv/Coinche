# Tracking des Crashes - Guide d'Implémentation

## Vue d'ensemble

Le système de tracking des crashes permet de détecter et reporter automatiquement les erreurs côté client vers le serveur pour analyse.

## Architecture

### Côté Serveur (✅ Déjà implémenté)

Le serveur est prêt à recevoir les rapports de crash:
- Handler: `handleReportCrash()` dans [GameServer.h](server/GameServer.h:976)
- Enregistrement: `recordCrash()` dans [DatabaseManager.cpp](server/DatabaseManager.cpp:1056)
- Statistiques incluses dans le rapport quotidien

### Côté Client (⚠️ À implémenter)

Il reste à implémenter la détection et l'envoi des crashes depuis l'application QML.

## Implémentation Côté Client

### 1. Ajouter un gestionnaire d'erreurs QML

Dans votre fichier `main.cpp`, ajoutez un handler pour les erreurs QML:

```cpp
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    // Handler d'erreurs QML
    QObject::connect(&engine, &QQmlApplicationEngine::warnings,
        [](const QList<QQmlError> &warnings) {
            for (const QQmlError &warning : warnings) {
                if (warning.isValid()) {
                    qWarning() << "QML Warning:" << warning.toString();
                    // Envoyer au serveur si critique
                }
            }
        });

    // Votre code existant...
    engine.load(QUrl(QStringLiteral("qrc:/qml/MainMenu.qml")));

    return app.exec();
}
```

### 2. Créer une classe CrashReporter (Recommandé)

Créez un singleton pour gérer les rapports de crash:

**CrashReporter.h:**
```cpp
#ifndef CRASHREPORTER_H
#define CRASHREPORTER_H

#include <QObject>
#include <QString>

class NetworkManager; // Forward declaration

class CrashReporter : public QObject
{
    Q_OBJECT

public:
    static CrashReporter* instance();

    void setNetworkManager(NetworkManager* nm);
    void reportCrash(const QString &errorMsg, const QString &stackTrace = QString());

private:
    explicit CrashReporter(QObject *parent = nullptr);
    NetworkManager* m_networkManager = nullptr;
};

#endif // CRASHREPORTER_H
```

**CrashReporter.cpp:**
```cpp
#include "CrashReporter.h"
#include "NetworkManager.h"
#include <QJsonObject>
#include <QDebug>

CrashReporter* CrashReporter::instance()
{
    static CrashReporter* inst = new CrashReporter();
    return inst;
}

CrashReporter::CrashReporter(QObject *parent)
    : QObject(parent)
{
}

void CrashReporter::setNetworkManager(NetworkManager* nm)
{
    m_networkManager = nm;
}

void CrashReporter::reportCrash(const QString &errorMsg, const QString &stackTrace)
{
    if (!m_networkManager) {
        qWarning() << "CrashReporter: NetworkManager not set";
        return;
    }

    qWarning() << "Reporting crash:" << errorMsg;

    QJsonObject data;
    data["type"] = "reportCrash";
    data["error"] = errorMsg;
    data["stackTrace"] = stackTrace;
    data["playerName"] = m_networkManager->getPlayerName(); // Ajuster selon votre API

    m_networkManager->sendMessage(data);
}
```

### 3. Intégrer dans votre code QML

**Option A: Try-Catch JavaScript dans QML**

```qml
// Dans vos fichiers QML, wrappez le code critique
Button {
    onClicked: {
        try {
            // Votre code ici
            riskyOperation()
        } catch (error) {
            console.error("Error:", error.message)
            CrashReporter.reportCrash("Button click error: " + error.message)
        }
    }
}
```

**Option B: Handler global dans main.qml**

```qml
ApplicationWindow {
    id: root

    // Handler global pour les erreurs non catchées
    Component.onCompleted: {
        // Installer un handler Qt pour les messages
        // (nécessite exposition depuis C++)
    }

    Connections {
        target: gameModel // Votre modèle de jeu

        function onErrorOccurred(errorMsg) {
            console.error("Game error:", errorMsg)
            // Reporter au serveur via NetworkManager
            networkManager.sendMessage({
                type: "reportCrash",
                error: errorMsg,
                stackTrace: "",
                playerName: networkManager.playerName
            })
        }
    }
}
```

### 4. Détecter les crashes critiques (C++)

Dans votre code C++, utilisez un message handler personnalisé:

```cpp
// Dans main.cpp

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString formattedMessage = qFormatLogMessage(type, context, msg);

    // Log normal
    fprintf(stderr, "%s\n", formattedMessage.toLocal8Bit().constData());

    // Reporter les erreurs critiques
    if (type == QtCriticalMsg || type == QtFatalMsg) {
        CrashReporter::instance()->reportCrash(
            QString("Critical error: %1").arg(msg),
            QString("File: %1:%2").arg(context.file).arg(context.line)
        );
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(customMessageHandler);

    // Reste du code...
}
```

## Exemples d'utilisation

### Exemple 1: Crash lors d'un drag & drop de carte

```qml
// Card.qml
MouseArea {
    onReleased: {
        try {
            if (!isValidMove(card)) {
                throw new Error("Invalid card move")
            }
            playCard(card)
        } catch (error) {
            console.error("Card play error:", error.message)
            networkManager.reportCrash("Card play failed: " + error.message)
        }
    }
}
```

### Exemple 2: Erreur de connexion réseau

```cpp
// Dans NetworkManager.cpp
void NetworkManager::onError(QAbstractSocket::SocketError error)
{
    QString errorMsg = m_socket->errorString();
    qWarning() << "Network error:" << errorMsg;

    // Reporter uniquement les erreurs critiques
    if (error == QAbstractSocket::RemoteHostClosedError ||
        error == QAbstractSocket::NetworkError) {
        CrashReporter::instance()->reportCrash(
            QString("Network error: %1").arg(errorMsg)
        );
    }
}
```

### Exemple 3: Erreur QML détectée automatiquement

```cpp
// Dans main.cpp
QObject::connect(&engine, &QQmlApplicationEngine::warnings,
    [networkManager](const QList<QQmlError> &warnings) {
        for (const QQmlError &warning : warnings) {
            if (warning.isValid()) {
                QString msg = warning.toString();

                // Reporter les erreurs (pas les warnings simples)
                if (msg.contains("Error", Qt::CaseInsensitive)) {
                    CrashReporter::instance()->reportCrash(
                        QString("QML Error: %1").arg(msg),
                        QString("Line: %1 in %2")
                            .arg(warning.line())
                            .arg(warning.url().toString())
                    );
                }
            }
        }
    });
```

## Tests

Pour tester le système de crash reporting:

```qml
// Bouton de test dans MainMenu.qml (à retirer en production)
Button {
    text: "Test Crash Report"
    visible: Qt.platform.os !== "android" // Debug seulement
    onClicked: {
        networkManager.sendMessage({
            type: "reportCrash",
            error: "Test crash from QML",
            stackTrace: "Test stacktrace",
            playerName: networkManager.playerName
        })
    }
}
```

## Visualisation dans le Rapport

Les crashes apparaissent dans le rapport quotidien:

```
💥 Crashes détectés: 5
   📉 -20% vs hier
```

## Bonnes Pratiques

1. **Ne pas abuser**: Reporter seulement les vraies erreurs, pas les warnings
2. **Pas de données sensibles**: Ne pas inclure de mots de passe ou données personnelles
3. **Contexte utile**: Inclure l'état du jeu si pertinent (en partie, lobby, etc.)
4. **Rate limiting**: Éviter de spammer le serveur avec des rapports répétitifs
5. **Offline handling**: Gérer le cas où le serveur n'est pas connecté

## Prochaines Améliorations Possibles

- 📧 Email d'alerte immédiat pour les crashes critiques
- 💾 Table dédiée `crash_logs` avec détails complets
- 📊 Dashboard web pour visualiser les crashes
- 🔍 Groupement des crashes similaires
- 📱 Info device (OS, version, mémoire) dans le rapport

---

**Note**: Le système est déjà opérationnel côté serveur. Il suffit d'envoyer un message JSON avec `type: "reportCrash"` pour que le crash soit enregistré et apparaisse dans les statistiques quotidiennes.
