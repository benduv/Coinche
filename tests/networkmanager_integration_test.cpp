#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QEventLoop>
#include <QTest>
#include "../server/NetworkManager.h"

// Variable globale pour QCoreApplication (nécessaire pour Qt)
static int argc = 1;
static char* argv[] = {(char*)"test_networkmanager_integration", nullptr};
static QCoreApplication* app = nullptr;

// ========================================
// Mock WebSocket Server pour les tests
// ========================================
class MockGameServer : public QObject {
    Q_OBJECT

public:
    MockGameServer(quint16 port, QObject* parent = nullptr)
        : QObject(parent)
        , m_server(new QWebSocketServer("MockGameServer", QWebSocketServer::NonSecureMode, this))
    {
        if (!m_server->listen(QHostAddress::LocalHost, port)) {
            qWarning() << "Failed to start mock server on port" << port;
        } else {
            qDebug() << "Mock server started on port" << port;
            connect(m_server, &QWebSocketServer::newConnection, this, &MockGameServer::onNewConnection);
        }
    }

    ~MockGameServer() {
        m_server->close();
        qDeleteAll(m_clients);
    }

    // Envoyer un message à tous les clients connectés
    void sendToAll(const QJsonObject& message) {
        QJsonDocument doc(message);
        QString jsonString = doc.toJson(QJsonDocument::Compact);

        for (QWebSocket* client : m_clients) {
            if (client->state() == QAbstractSocket::ConnectedState) {
                client->sendTextMessage(jsonString);
            }
        }
    }

    // Envoyer un message à un client spécifique
    void sendToClient(QWebSocket* client, const QJsonObject& message) {
        if (client && client->state() == QAbstractSocket::ConnectedState) {
            QJsonDocument doc(message);
            QString jsonString = doc.toJson(QJsonDocument::Compact);
            client->sendTextMessage(jsonString);
        }
    }

    QList<QWebSocket*> clients() const { return m_clients; }
    QWebSocket* lastClient() const { return m_clients.isEmpty() ? nullptr : m_clients.last(); }
    bool isListening() const { return m_server->isListening(); }
    quint16 serverPort() const { return m_server->serverPort(); }

signals:
    void clientConnected(QWebSocket* client);
    void messageReceived(QWebSocket* client, const QJsonObject& message);

private slots:
    void onNewConnection() {
        QWebSocket* client = m_server->nextPendingConnection();
        if (client) {
            qDebug() << "Mock server: New client connected";
            m_clients.append(client);

            connect(client, &QWebSocket::textMessageReceived, this, [this, client](const QString& message) {
                QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
                QJsonObject obj = doc.object();
                qDebug() << "Mock server received:" << obj["type"].toString();
                emit messageReceived(client, obj);
            });

            connect(client, &QWebSocket::disconnected, this, [this, client]() {
                qDebug() << "Mock server: Client disconnected";
                m_clients.removeAll(client);
                client->deleteLater();
            });

            emit clientConnected(client);
        }
    }

private:
    QWebSocketServer* m_server;
    QList<QWebSocket*> m_clients;
};

// ========================================
// Test Fixture pour les tests d'intégration
// ========================================
class NetworkManagerIntegrationTest : public ::testing::Test {
protected:
    MockGameServer* mockServer;
    NetworkManager* netManager;
    const quint16 TEST_PORT = 12345;

    static void SetUpTestSuite() {
        if (!app) {
            app = new QCoreApplication(argc, argv);
        }
    }

    void SetUp() override {
        // Créer le serveur mock
        mockServer = new MockGameServer(TEST_PORT);

        // Attendre que le serveur soit prêt
        QCoreApplication::processEvents();
        QTest::qWait(100);

        // Vérifier que le serveur écoute
        ASSERT_TRUE(mockServer->isListening()) << "Mock server failed to start";
        qDebug() << "Mock server listening on port" << mockServer->serverPort();

        // Créer le NetworkManager
        netManager = new NetworkManager();
    }

    void TearDown() override {
        delete netManager;
        delete mockServer;

        // Traiter les événements en attente
        QCoreApplication::processEvents();
    }

    // Helper pour attendre qu'un signal soit émis
    bool waitForSignal(QObject* obj, const char* signal, int timeout = 1000) {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);

        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        QObject::connect(obj, signal, &loop, SLOT(quit()));

        timer.start(timeout);
        loop.exec();

        return timer.isActive();
    }
};

// ========================================
// Tests de connexion
// ========================================

TEST_F(NetworkManagerIntegrationTest, ConnectToServer_Success) {
    // Vérifier que le serveur écoute
    qDebug() << "Server is listening:" << mockServer->clients().isEmpty();

    // Spy sur le signal connectedChanged
    QSignalSpy connectedSpy(netManager, &NetworkManager::connectedChanged);

    // Connecter au serveur mock
    QString url = QString("ws://localhost:%1").arg(TEST_PORT);
    qDebug() << "Connecting to:" << url;
    netManager->connectToServer(url);

    // Traiter les événements
    QCoreApplication::processEvents();

    // Attendre la connexion
    bool connected = waitForSignal(netManager, SIGNAL(connectedChanged()), 3000);
    qDebug() << "Connected:" << connected << "Spy count:" << connectedSpy.count();
    ASSERT_TRUE(connected);

    // Vérifier que le signal a été émis
    EXPECT_EQ(connectedSpy.count(), 1);
    EXPECT_TRUE(netManager->connected());
}

TEST_F(NetworkManagerIntegrationTest, RegisterPlayer_ReceivesResponse) {
    // Connecter d'abord
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    // Spy sur le signal de message reçu par le serveur
    QSignalSpy serverMessageSpy(mockServer, &MockGameServer::messageReceived);

    // S'enregistrer
    netManager->registerPlayer("TestPlayer", "avataaars1.svg");

    // Attendre que le serveur reçoive le message
    ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(messageReceived(QWebSocket*, const QJsonObject&)), 2000));

    // Vérifier que le serveur a reçu le message
    EXPECT_EQ(serverMessageSpy.count(), 1);

    QJsonObject receivedMsg = serverMessageSpy.at(0).at(1).value<QJsonObject>();
    EXPECT_EQ(receivedMsg["type"].toString(), "register");
    EXPECT_EQ(receivedMsg["playerName"].toString(), "TestPlayer");
    EXPECT_EQ(receivedMsg["avatar"].toString(), "avataaars1.svg");
}

TEST_F(NetworkManagerIntegrationTest, RegisterPlayer_ServerResponse) {
    // Connecter et s'enregistrer
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    QSignalSpy pseudoSpy(netManager, &NetworkManager::playerPseudoChanged);
    QSignalSpy avatarSpy(netManager, &NetworkManager::playerAvatarChanged);

    // Attendre que le serveur ait un client
    if (mockServer->clients().isEmpty()) {
        ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(clientConnected(QWebSocket*)), 2000));
    }

    // Simuler la réponse du serveur
    QJsonObject response;
    response["type"] = "registered";
    response["connectionId"] = "test-connection-123";
    response["playerName"] = "TestPlayer";
    response["avatar"] = "avataaars2.svg";

    mockServer->sendToClient(mockServer->lastClient(), response);

    // Attendre que le client traite le message
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(playerPseudoChanged()), 2000));

    // Vérifier que les propriétés ont été mises à jour
    EXPECT_EQ(netManager->playerPseudo(), "TestPlayer");
    EXPECT_EQ(netManager->playerAvatar(), "avataaars2.svg");
}

// ========================================
// Tests de matchmaking
// ========================================

TEST_F(NetworkManagerIntegrationTest, JoinMatchmaking_SendsMessage) {
    // Connecter
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    QSignalSpy serverMessageSpy(mockServer, &MockGameServer::messageReceived);

    // Rejoindre le matchmaking
    netManager->joinMatchmaking();

    // Attendre le message
    ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(messageReceived(QWebSocket*, const QJsonObject&)), 2000));

    QJsonObject receivedMsg = serverMessageSpy.at(0).at(1).value<QJsonObject>();
    EXPECT_EQ(receivedMsg["type"].toString(), "joinMatchmaking");
}

TEST_F(NetworkManagerIntegrationTest, MatchmakingStatus_UpdatesCorrectly) {
    // Connecter
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    if (mockServer->clients().isEmpty()) {
        ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(clientConnected(QWebSocket*)), 2000));
    }

    QSignalSpy statusSpy(netManager, &NetworkManager::matchmakingStatusChanged);
    QSignalSpy queueSpy(netManager, &NetworkManager::playersInQueueChanged);

    // Simuler une mise à jour du statut
    QJsonObject statusMsg;
    statusMsg["type"] = "matchmakingStatus";
    statusMsg["status"] = "En attente de joueurs";
    statusMsg["playersInQueue"] = 3;

    mockServer->sendToClient(mockServer->lastClient(), statusMsg);

    // Attendre la mise à jour
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(matchmakingStatusChanged()), 2000));

    EXPECT_EQ(netManager->matchmakingStatus(), "En attente de joueurs");
    EXPECT_EQ(netManager->playersInQueue(), 3);
}

TEST_F(NetworkManagerIntegrationTest, MatchmakingCountdown_UpdatesCorrectly) {
    // Connecter
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    if (mockServer->clients().isEmpty()) {
        ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(clientConnected(QWebSocket*)), 2000));
    }

    QSignalSpy countdownSpy(netManager, &NetworkManager::matchmakingCountdownChanged);

    // Simuler un countdown
    QJsonObject countdownMsg;
    countdownMsg["type"] = "matchmakingCountdown";
    countdownMsg["seconds"] = 5;

    mockServer->sendToClient(mockServer->lastClient(), countdownMsg);

    // Attendre la mise à jour
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(matchmakingCountdownChanged()), 2000));

    EXPECT_EQ(netManager->matchmakingCountdown(), 5);
}

// ========================================
// Tests d'authentification
// ========================================

TEST_F(NetworkManagerIntegrationTest, LoginAccount_Success) {
    // Connecter
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    QSignalSpy serverMessageSpy(mockServer, &MockGameServer::messageReceived);

    // Login
    netManager->loginAccount("test@example.com", "password123");

    // Attendre le message
    ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(messageReceived(QWebSocket*, const QJsonObject&)), 2000));

    QJsonObject receivedMsg = serverMessageSpy.at(0).at(1).value<QJsonObject>();
    EXPECT_EQ(receivedMsg["type"].toString(), "loginAccount");
    EXPECT_EQ(receivedMsg["email"].toString(), "test@example.com");
    EXPECT_EQ(receivedMsg["password"].toString(), "password123");
}

TEST_F(NetworkManagerIntegrationTest, LoginAccount_SuccessResponse) {
    // Connecter
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    if (mockServer->clients().isEmpty()) {
        ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(clientConnected(QWebSocket*)), 2000));
    }

    QSignalSpy loginSuccessSpy(netManager, &NetworkManager::loginSuccess);

    // Simuler une réponse de succès
    QJsonObject response;
    response["type"] = "loginAccountSuccess";
    response["playerName"] = "TestPlayer";
    response["avatar"] = "avataaars3.svg";
    response["connectionId"] = "conn-456";
    response["usingTempPassword"] = false;

    mockServer->sendToClient(mockServer->lastClient(), response);

    // Attendre le signal
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(loginSuccess(QString, QString, bool)), 2000));

    EXPECT_EQ(loginSuccessSpy.count(), 1);
    EXPECT_EQ(netManager->playerPseudo(), "TestPlayer");
    EXPECT_EQ(netManager->playerAvatar(), "avataaars3.svg");
}

TEST_F(NetworkManagerIntegrationTest, LoginAccount_FailedResponse) {
    // Connecter
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    if (mockServer->clients().isEmpty()) {
        ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(clientConnected(QWebSocket*)), 2000));
    }

    QSignalSpy loginFailedSpy(netManager, &NetworkManager::loginFailed);

    // Simuler une réponse d'échec
    QJsonObject response;
    response["type"] = "loginAccountFailed";
    response["error"] = "Email ou mot de passe incorrect";

    mockServer->sendToClient(mockServer->lastClient(), response);

    // Attendre le signal
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(loginFailed(QString)), 2000));

    EXPECT_EQ(loginFailedSpy.count(), 1);
    QList<QVariant> arguments = loginFailedSpy.takeFirst();
    EXPECT_EQ(arguments.at(0).toString(), "Email ou mot de passe incorrect");
}

TEST_F(NetworkManagerIntegrationTest, RegisterAccount_Success) {
    // Connecter
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    QSignalSpy serverMessageSpy(mockServer, &MockGameServer::messageReceived);

    // S'inscrire
    netManager->registerAccount("NewPlayer", "new@example.com", "newpass123", "avataaars4.svg");

    // Attendre le message
    ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(messageReceived(QWebSocket*, const QJsonObject&)), 2000));

    QJsonObject receivedMsg = serverMessageSpy.at(0).at(1).value<QJsonObject>();
    EXPECT_EQ(receivedMsg["type"].toString(), "registerAccount");
    EXPECT_EQ(receivedMsg["pseudo"].toString(), "NewPlayer");
    EXPECT_EQ(receivedMsg["email"].toString(), "new@example.com");
    EXPECT_EQ(receivedMsg["password"].toString(), "newpass123");
    EXPECT_EQ(receivedMsg["avatar"].toString(), "avataaars4.svg");
}

// ========================================
// Tests de lobby privé
// ========================================

TEST_F(NetworkManagerIntegrationTest, CreatePrivateLobby_SendsMessage) {
    // Connecter
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    QSignalSpy serverMessageSpy(mockServer, &MockGameServer::messageReceived);

    // Créer un lobby
    netManager->createPrivateLobby();

    // Attendre le message
    ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(messageReceived(QWebSocket*, const QJsonObject&)), 2000));

    QJsonObject receivedMsg = serverMessageSpy.at(0).at(1).value<QJsonObject>();
    EXPECT_EQ(receivedMsg["type"].toString(), "createPrivateLobby");
}

TEST_F(NetworkManagerIntegrationTest, LobbyCreated_EmitsSignal) {
    // Connecter
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    if (mockServer->clients().isEmpty()) {
        ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(clientConnected(QWebSocket*)), 2000));
    }

    QSignalSpy lobbyCreatedSpy(netManager, &NetworkManager::lobbyCreated);

    // Simuler la création d'un lobby
    QJsonObject response;
    response["type"] = "lobbyCreated";
    response["code"] = "ABC123";

    mockServer->sendToClient(mockServer->lastClient(), response);

    // Attendre le signal
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(lobbyCreated(QString)), 2000));

    EXPECT_EQ(lobbyCreatedSpy.count(), 1);
    QList<QVariant> arguments = lobbyCreatedSpy.takeFirst();
    EXPECT_EQ(arguments.at(0).toString(), "ABC123");
}

TEST_F(NetworkManagerIntegrationTest, JoinPrivateLobby_SendsCode) {
    // Connecter
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    QSignalSpy serverMessageSpy(mockServer, &MockGameServer::messageReceived);

    // Rejoindre un lobby
    netManager->joinPrivateLobby("XYZ789");

    // Attendre le message
    ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(messageReceived(QWebSocket*, const QJsonObject&)), 2000));

    QJsonObject receivedMsg = serverMessageSpy.at(0).at(1).value<QJsonObject>();
    EXPECT_EQ(receivedMsg["type"].toString(), "joinPrivateLobby");
    EXPECT_EQ(receivedMsg["code"].toString(), "XYZ789");
}

TEST_F(NetworkManagerIntegrationTest, LobbyUpdate_UpdatesPlayers) {
    // Connecter
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    if (mockServer->clients().isEmpty()) {
        ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(clientConnected(QWebSocket*)), 2000));
    }

    QSignalSpy lobbyPlayersSpy(netManager, &NetworkManager::lobbyPlayersChanged);

    // Simuler une mise à jour du lobby
    QJsonArray players;
    QJsonObject player1;
    player1["name"] = "Player1";
    player1["avatar"] = "avataaars1.svg";
    player1["ready"] = true;
    player1["isHost"] = true;
    players.append(player1);

    QJsonObject player2;
    player2["name"] = "Player2";
    player2["avatar"] = "avataaars2.svg";
    player2["ready"] = false;
    player2["isHost"] = false;
    players.append(player2);

    QJsonObject response;
    response["type"] = "lobbyUpdate";
    response["players"] = players;

    mockServer->sendToClient(mockServer->lastClient(), response);

    // Attendre le signal
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(lobbyPlayersChanged()), 2000));

    EXPECT_EQ(lobbyPlayersSpy.count(), 1);
    EXPECT_EQ(netManager->lobbyPlayers().length(), 2);
}

// ========================================
// Tests de mot de passe oublié
// ========================================

TEST_F(NetworkManagerIntegrationTest, ForgotPassword_SendsMessage) {
    // Connecter
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    QSignalSpy serverMessageSpy(mockServer, &MockGameServer::messageReceived);

    // Demander un nouveau mot de passe
    netManager->forgotPassword("forgot@example.com");

    // Attendre le message
    ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(messageReceived(QWebSocket*, const QJsonObject&)), 2000));

    QJsonObject receivedMsg = serverMessageSpy.at(0).at(1).value<QJsonObject>();
    EXPECT_EQ(receivedMsg["type"].toString(), "forgotPassword");
    EXPECT_EQ(receivedMsg["email"].toString(), "forgot@example.com");
}

TEST_F(NetworkManagerIntegrationTest, ChangePassword_SendsMessage) {
    // Connecter
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    QSignalSpy serverMessageSpy(mockServer, &MockGameServer::messageReceived);

    // Changer le mot de passe
    netManager->changePassword("user@example.com", "newPassword456");

    // Attendre le message
    ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(messageReceived(QWebSocket*, const QJsonObject&)), 2000));

    QJsonObject receivedMsg = serverMessageSpy.at(0).at(1).value<QJsonObject>();
    EXPECT_EQ(receivedMsg["type"].toString(), "changePassword");
    EXPECT_EQ(receivedMsg["email"].toString(), "user@example.com");
    EXPECT_EQ(receivedMsg["newPassword"].toString(), "newPassword456");
}

// ========================================
// Tests de message de contact
// ========================================

TEST_F(NetworkManagerIntegrationTest, SendContactMessage_SendsAllFields) {
    // Connecter
    netManager->connectToServer(QString("ws://localhost:%1").arg(TEST_PORT));
    QCoreApplication::processEvents();
    ASSERT_TRUE(waitForSignal(netManager, SIGNAL(connectedChanged()), 3000));

    QSignalSpy serverMessageSpy(mockServer, &MockGameServer::messageReceived);

    // Envoyer un message de contact
    netManager->sendContactMessage("TestUser", "test@example.com", "Bug Report", "J'ai trouvé un bug");

    // Attendre le message
    ASSERT_TRUE(waitForSignal(mockServer, SIGNAL(messageReceived(QWebSocket*, const QJsonObject&)), 2000));

    QJsonObject receivedMsg = serverMessageSpy.at(0).at(1).value<QJsonObject>();
    EXPECT_EQ(receivedMsg["type"].toString(), "sendContactMessage");
    EXPECT_EQ(receivedMsg["senderName"].toString(), "TestUser");
    EXPECT_EQ(receivedMsg["senderEmail"].toString(), "test@example.com");
    EXPECT_EQ(receivedMsg["subject"].toString(), "Bug Report");
    EXPECT_EQ(receivedMsg["message"].toString(), "J'ai trouvé un bug");
}

// ========================================
// Main function
// ========================================
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// Inclusion du MOC pour MockGameServer
#include "networkmanager_integration_test.moc"
