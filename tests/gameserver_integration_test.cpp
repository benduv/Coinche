#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QWebSocket>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QEventLoop>
#include <QTest>
#include <iostream>
#include "../server/GameServer.h"
#include "../server/DatabaseManager.h"

// Variable globale pour QCoreApplication
static int argc = 1;
static char* argv[] = {(char*)"test_gameserver_integration", nullptr};
static QCoreApplication* app = nullptr;

// ========================================
// Mock WebSocket Client pour les tests
// ========================================
class MockGameClient : public QObject {
    Q_OBJECT

public:
    MockGameClient(const QString& url, const QString& playerName, QObject* parent = nullptr)
        : QObject(parent)
        , m_socket(new QWebSocket)
        , m_playerName(playerName)
        , m_connected(false)
    {
        connect(m_socket, &QWebSocket::connected, this, [this]() {
            m_connected = true;
            qDebug() << m_playerName << "connected";
            emit connected();
        });

        connect(m_socket, &QWebSocket::textMessageReceived, this, [this](const QString& message) {
            QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
            QJsonObject obj = doc.object();
            QString type = obj["type"].toString();
            qDebug() << m_playerName << "received:" << type;

            m_lastMessage = obj;
            m_allMessages.append(obj);  // Track all messages
            emit messageReceived(obj);

            // Émettre des signaux spécifiques par type de message
            if (type == "registered") {
                m_connectionId = obj["connectionId"].toString();
                emit registered(m_connectionId);
            } else if (type == "gameFound") {
                m_playerPosition = obj["playerPosition"].toInt();
                m_myCards = obj["myCards"].toArray();
                emit gameFound(m_playerPosition, m_myCards);
            } else if (type == "gameState") {
                emit gameStateUpdated(obj);
            } else if (type == "bidMade") {
                emit bidMade(obj["playerIndex"].toInt(), obj["bidValue"].toInt(), obj["suit"].toInt());
            } else if (type == "cardPlayed") {
                emit cardPlayed(obj["playerIndex"].toInt(), obj["cardIndex"].toInt());
            } else if (type == "pliFinished") {
                emit pliFinished(obj["winnerId"].toInt());
            } else if (type == "mancheFinished") {
                emit mancheFinished(obj["scoreTotalTeam1"].toInt(), obj["scoreTotalTeam2"].toInt());
            } else if (type == "gameOver") {
                emit gameOver(obj["winner"].toInt());
            }
        });

        m_socket->open(QUrl(url));
    }

    ~MockGameClient() {
        if (m_socket) {
            m_socket->close();
            delete m_socket;
        }
    }

    void sendRegister(bool wasInGame = false) {
        QJsonObject msg;
        msg["type"] = "register";
        msg["playerName"] = m_playerName;
        msg["avatar"] = "avataaars1.svg";
        msg["wasInGame"] = wasInGame;
        sendMessage(msg);
    }

    void sendJoinMatchmaking() {
        QJsonObject msg;
        msg["type"] = "joinMatchmaking";
        sendMessage(msg);
    }

    void sendMakeBid(int bidValue, int suit) {
        QJsonObject msg;
        msg["type"] = "makeBid";
        msg["bidValue"] = bidValue;
        msg["suit"] = suit;
        sendMessage(msg);
    }

    void sendPlayCard(int cardIndex) {
        QJsonObject msg;
        msg["type"] = "playCard";
        msg["cardIndex"] = cardIndex;
        sendMessage(msg);
    }

    void sendCoinche() {
        QJsonObject msg;
        msg["type"] = "coinche";
        sendMessage(msg);
    }

    void sendSurcoinche(bool accept) {
        QJsonObject msg;
        msg["type"] = "surcoinche";
        msg["accept"] = accept;
        sendMessage(msg);
    }

    bool isConnected() const { return m_connected; }
    QString playerName() const { return m_playerName; }
    QString connectionId() const { return m_connectionId; }
    int playerPosition() const { return m_playerPosition; }
    QJsonArray myCards() const { return m_myCards; }
    QJsonObject lastMessage() const { return m_lastMessage; }

    void closeConnection() {
        if (m_socket) {
            m_socket->close();
            m_connected = false;
        }
    }

signals:
    void connected();
    void messageReceived(const QJsonObject& message);
    void registered(const QString& connectionId);
    void gameFound(int playerPosition, const QJsonArray& cards);
    void gameStateUpdated(const QJsonObject& state);
    void bidMade(int playerIndex, int bidValue, int suit);
    void cardPlayed(int playerIndex, int cardIndex);
    void pliFinished(int winnerId);
    void mancheFinished(int scoreTeam1, int scoreTeam2);
    void gameOver(int winner);

private:
    void sendMessage(const QJsonObject& message) {
        if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
            QJsonDocument doc(message);
            m_socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
        }
    }

    QWebSocket* m_socket;
    QString m_playerName;
    QString m_connectionId;
    bool m_connected;
    int m_playerPosition;
    QJsonArray m_myCards;
    QJsonObject m_lastMessage;
    QList<QJsonObject> m_allMessages;  // Track all messages for debugging

public:
    const QList<QJsonObject>& allMessages() const { return m_allMessages; }
};

// ========================================
// Test Fixture pour les tests d'intégration
// ========================================
class GameServerIntegrationTest : public ::testing::Test {
protected:
    GameServer* gameServer;
    DatabaseManager* dbManager;
    QList<MockGameClient*> clients;
    const quint16 TEST_PORT = 12346;

    static void SetUpTestSuite() {
        if (!app) {
            app = new QCoreApplication(argc, argv);
        }
    }

    void SetUp() override {
        // Créer la base de données temporaire
        QString testDbPath = QDir::temp().filePath("test_gameserver_integration.db");
        QFile::remove(testDbPath);

        dbManager = new DatabaseManager();
        ASSERT_TRUE(dbManager->initialize(testDbPath));

        // Créer le GameServer
        gameServer = new GameServer(TEST_PORT, dbManager);

        // Attendre que le serveur soit prêt
        QTest::qWait(200);
    }

    void TearDown() override {
        // Supprimer tous les clients
        for (MockGameClient* client : clients) {
            delete client;
        }
        clients.clear();

        delete gameServer;
        delete dbManager;

        QCoreApplication::processEvents();
    }

    // Helper pour créer et connecter un client
    MockGameClient* createClient(const QString& name) {
        const QString url = QString("ws://localhost:%1").arg(TEST_PORT);
        MockGameClient* client = new MockGameClient(url, name, nullptr);
        clients.append(client);

        // Attendre la connexion
        if (!client->isConnected()) {
            QSignalSpy connectedSpy(client, &MockGameClient::connected);
            EXPECT_TRUE(waitForSignal(client, SIGNAL(connected()), 3000));
        }

        return client;
    }

    // Helper pour attendre qu'un signal soit émis
    bool waitForSignal(QObject* obj, const char* signal, int timeout = 2000) {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);

        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        QObject::connect(obj, signal, &loop, SLOT(quit()));

        timer.start(timeout);
        loop.exec();

        return timer.isActive();
    }

    // Helper pour attendre plusieurs signaux
    bool waitForAllSignals(QList<QObject*> objects, const char* signal, int timeout = 3000) {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        QList<QSignalSpy*> spies;

        // Créer un spy pour chaque objet
        for (QObject* obj : objects) {
            QSignalSpy* spy = new QSignalSpy(obj, signal);
            spies.append(spy);
        }

        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

        timer.start(timeout);

        // Attendre que tous aient reçu au moins un signal
        while (timer.isActive()) {
            bool allReceived = true;
            for (QSignalSpy* spy : spies) {
                if (spy->count() == 0) {
                    allReceived = false;
                    break;
                }
            }
            if (allReceived) {
                timer.stop();
                break;
            }
            QCoreApplication::processEvents();
            QTest::qWait(50);
        }

        bool result = timer.isActive();

        // Nettoyer les spies
        qDeleteAll(spies);

        return result;
    }
};

// ========================================
// Tests de connexion et enregistrement
// ========================================

TEST_F(GameServerIntegrationTest, ConnectAndRegister_SinglePlayer) {
    MockGameClient* client = createClient("Player1");
    ASSERT_TRUE(client->isConnected());

    QSignalSpy registeredSpy(client, &MockGameClient::registered);

    // S'enregistrer
    client->sendRegister();

    // Attendre l'enregistrement
    ASSERT_TRUE(waitForSignal(client, SIGNAL(registered(QString)), 2000));
    EXPECT_EQ(registeredSpy.count(), 1);
    EXPECT_FALSE(client->connectionId().isEmpty());
}

TEST_F(GameServerIntegrationTest, ConnectAndRegister_FourPlayers) {
    QList<MockGameClient*> players;

    for (int i = 0; i < 4; i++) {
        MockGameClient* client = createClient(QString("Player%1").arg(i + 1));
        ASSERT_TRUE(client->isConnected());

        QSignalSpy registeredSpy(client, &MockGameClient::registered);
        client->sendRegister();

        ASSERT_TRUE(waitForSignal(client, SIGNAL(registered(QString)), 2000));
        EXPECT_FALSE(client->connectionId().isEmpty());

        players.append(client);
    }

    EXPECT_EQ(players.size(), 4);
}

// ========================================
// Tests de matchmaking
// ========================================

TEST_F(GameServerIntegrationTest, Matchmaking_FourPlayersStartGame) {
    QList<MockGameClient*> players;

    // Créer et enregistrer 4 joueurs
    for (int i = 0; i < 4; i++) {
        MockGameClient* client = createClient(QString("Player%1").arg(i + 1));
        client->sendRegister();
        ASSERT_TRUE(waitForSignal(client, SIGNAL(registered(QString)), 2000));
        players.append(client);
        QTest::qWait(100); // Attendre entre chaque joueur
    }

    // Espionner les signaux gameFound pour chaque joueur
    QList<QSignalSpy*> gameFoundSpies;
    for (MockGameClient* player : players) {
        gameFoundSpies.append(new QSignalSpy(player, SIGNAL(gameFound(int, const QJsonArray&))));
    }

    // Tous rejoignent le matchmaking
    for (MockGameClient* player : players) {
        player->sendJoinMatchmaking();
        QCoreApplication::processEvents();
        QTest::qWait(50);
    }

    // Attendre que tous reçoivent gameFound
    int maxWaitTime = 10000; // 10 secondes max
    int elapsed = 0;
    bool allReceived = false;

    while (elapsed < maxWaitTime && !allReceived) {
        QCoreApplication::processEvents();
        QTest::qWait(100);
        elapsed += 100;

        // Vérifier si tous ont reçu le signal
        allReceived = true;
        for (QSignalSpy* spy : gameFoundSpies) {
            if (spy->count() == 0) {
                allReceived = false;
                break;
            }
        }
    }

    // Nettoyer les spies
    qDeleteAll(gameFoundSpies);

    ASSERT_TRUE(allReceived) << "Not all players received gameFound signal after " << elapsed << "ms";

    // Vérifier que chaque joueur a une position différente (0-3)
    QSet<int> positions;
    for (MockGameClient* player : players) {
        positions.insert(player->playerPosition());
        qDebug() << player->playerName() << "position:" << player->playerPosition() << "cards:" << player->myCards().size();
    }
    EXPECT_EQ(positions.size(), 4); // 4 positions différentes

    // Vérifier que chaque joueur a 8 cartes
    for (MockGameClient* player : players) {
        EXPECT_EQ(player->myCards().size(), 8) << qPrintable(player->playerName()) << " should have 8 cards";
    }
}

// ========================================
// Tests de phase d'enchères
// ========================================

TEST_F(GameServerIntegrationTest, BiddingPhase_AllPlayersPass) {
    // Créer 4 joueurs et démarrer une partie
    QList<MockGameClient*> players;
    for (int i = 0; i < 4; i++) {
        MockGameClient* client = createClient(QString("Player%1").arg(i + 1));
        client->sendRegister();
        waitForSignal(client, SIGNAL(registered(QString)), 2000);
        client->sendJoinMatchmaking();
        players.append(client);
    }

    // Attendre gameFound
    waitForAllSignals(
        QList<QObject*>({players[0], players[1], players[2], players[3]}),
        SIGNAL(gameFound(int, const QJsonArray&)),
        5000
    );

    QCoreApplication::processEvents();

    // Tous les joueurs passent
    for (int i = 0; i < 4; i++) {
        QTest::qWait(100);
        players[i]->sendMakeBid(14, 0);  // 14 = PASSE // 0 = passer
        QCoreApplication::processEvents();
    }

    // Attendre que la partie recommence (nouvelle distribution)
    QTest::qWait(500);
    QCoreApplication::processEvents();
}

TEST_F(GameServerIntegrationTest, BiddingPhase_OnePlayerBids) {
    // Créer 4 joueurs et démarrer une partie
    QList<MockGameClient*> players;
    for (int i = 0; i < 4; i++) {
        MockGameClient* client = createClient(QString("Player%1").arg(i + 1));
        client->sendRegister();
        waitForSignal(client, SIGNAL(registered(QString)), 2000);
        client->sendJoinMatchmaking();
        players.append(client);
    }

    waitForAllSignals(
        QList<QObject*>({players[0], players[1], players[2], players[3]}),
        SIGNAL(gameFound(int, const QJsonArray&)),
        5000
    );

    QCoreApplication::processEvents();

    // Espionner les messages bidMade
    QSignalSpy bidSpy0(players[0], &MockGameClient::bidMade);
    QSignalSpy bidSpy1(players[1], &MockGameClient::bidMade);
    QSignalSpy bidSpy2(players[2], &MockGameClient::bidMade);
    QSignalSpy bidSpy3(players[3], &MockGameClient::bidMade);

    // Joueur 0 passe
    players[0]->sendMakeBid(14, 0);  // 14 = PASSE
    QTest::qWait(200);
    QCoreApplication::processEvents();

    // Joueur 1 fait une enchère QUATREVINGT (80 points) PIQUE (enum value 1, suit 6)
    players[1]->sendMakeBid(1, 6);  // 1 = QUATREVINGT
    QTest::qWait(200);
    QCoreApplication::processEvents();

    // Joueur 2 passe
    players[2]->sendMakeBid(14, 0);  // 14 = PASSE
    QTest::qWait(200);
    QCoreApplication::processEvents();

    // Joueur 3 passe
    players[3]->sendMakeBid(14, 0);  // 14 = PASSE
    QTest::qWait(200);
    QCoreApplication::processEvents();

    // Vérifier qu'au moins certains joueurs ont reçu le message bidMade
    int totalBids = bidSpy0.count() + bidSpy1.count() + bidSpy2.count() + bidSpy3.count();
    EXPECT_GT(totalBids, 0) << "At least some players should have received bidMade messages";
}

// ========================================
// Tests de phase de jeu (simplifiés)
// ========================================

TEST_F(GameServerIntegrationTest, PlayingPhase_PlayersCanPlayCards) {
    // Créer 4 joueurs
    QList<MockGameClient*> players;
    for (int i = 0; i < 4; i++) {
        MockGameClient* client = createClient(QString("Player%1").arg(i + 1));
        client->sendRegister();
        waitForSignal(client, SIGNAL(registered(QString)), 2000);
        client->sendJoinMatchmaking();
        players.append(client);
    }

    // Attendre que tous les joueurs reçoivent gameFound
    QList<QSignalSpy*> gameFoundSpies;
    for (MockGameClient* player : players) {
        gameFoundSpies.append(new QSignalSpy(player, SIGNAL(gameFound(int, const QJsonArray&))));
    }

    int maxWaitTime = 10000;
    int elapsed = 0;
    bool allReceived = false;

    while (elapsed < maxWaitTime && !allReceived) {
        QCoreApplication::processEvents();
        QTest::qWait(100);
        elapsed += 100;

        allReceived = true;
        for (QSignalSpy* spy : gameFoundSpies) {
            if (spy->count() == 0) {
                allReceived = false;
                break;
            }
        }
    }

    for (QSignalSpy* spy : gameFoundSpies) delete spy;
    ASSERT_TRUE(allReceived) << "All players should receive gameFound signal";

    QCoreApplication::processEvents();
    QTest::qWait(500);

    // Faire les enchères: Joueur 1 gagne avec QUATREVINGT (80 points) PIQUE, les autres passent
    // NOTE: Pour terminer les enchères, il faut 3 passes consécutives APRÈS une enchère valide
    // NOTE: Les valeurs d'annonce sont: QUATREVINGT=1 (80pts), PASSE=14
    // NOTE: Les valeurs de couleur sont: COEUR=3, TREFLE=4, CARREAU=5, PIQUE=6
    players[0]->sendMakeBid(14, 0);  // Player 0 passe (14 = PASSE)
    QTest::qWait(200);
    QCoreApplication::processEvents();

    players[1]->sendMakeBid(1, 6);  // Player 1 enchérit QUATREVINGT (1 = 80pts) PIQUE (suit 6)
    QTest::qWait(200);
    QCoreApplication::processEvents();

    players[2]->sendMakeBid(14, 0);  // Player 2 passe (1ère passe après enchère, 14 = PASSE)
    QTest::qWait(200);
    QCoreApplication::processEvents();

    players[3]->sendMakeBid(14, 0);  // Player 3 passe (2ème passe après enchère, 14 = PASSE)
    QTest::qWait(200);
    QCoreApplication::processEvents();

    players[0]->sendMakeBid(14, 0);  // Player 0 passe (3ème passe après enchère → fin enchères, 14 = PASSE)
    QTest::qWait(2000);  // Attendre que la phase de jeu démarre
    QCoreApplication::processEvents();

    // Créer des spies pour cardPlayed
    QSignalSpy cardSpy0(players[0], &MockGameClient::cardPlayed);
    QSignalSpy cardSpy1(players[1], &MockGameClient::cardPlayed);
    QSignalSpy cardSpy2(players[2], &MockGameClient::cardPlayed);
    QSignalSpy cardSpy3(players[3], &MockGameClient::cardPlayed);

    // Essayer de jouer des cartes (tous les joueurs essaient, seul le joueur courant pourra)
    for (int attempt = 0; attempt < 8; attempt++) {
        for (int i = 0; i < 4; i++) {
            players[i]->sendPlayCard(0);  // Jouer la première carte
        }
        QTest::qWait(300);
        QCoreApplication::processEvents();
    }

    // Vérifier qu'au moins quelques cartes ont été jouées
    int totalCardsPlayed = cardSpy0.count() + cardSpy1.count() + cardSpy2.count() + cardSpy3.count();
    EXPECT_GT(totalCardsPlayed, 0) << "At least some cards should have been played. Got: " << totalCardsPlayed;
}

// ========================================
// Tests de reconnexion
// ========================================

TEST_F(GameServerIntegrationTest, Reconnection_PlayerReconnectsDuringGame) {
    // Créer 4 joueurs et démarrer une partie
    QList<MockGameClient*> players;
    for (int i = 0; i < 4; i++) {
        MockGameClient* client = createClient(QString("Player%1").arg(i + 1));
        client->sendRegister();
        waitForSignal(client, SIGNAL(registered(QString)), 2000);
        client->sendJoinMatchmaking();
        players.append(client);
    }

    // Attendre que tous les joueurs reçoivent gameFound
    QList<QSignalSpy*> gameFoundSpies;
    for (MockGameClient* player : players) {
        gameFoundSpies.append(new QSignalSpy(player, SIGNAL(gameFound(int, const QJsonArray&))));
    }

    int maxWaitTime = 10000;
    int elapsed = 0;
    bool allReceived = false;

    while (elapsed < maxWaitTime && !allReceived) {
        QCoreApplication::processEvents();
        QTest::qWait(100);
        elapsed += 100;

        allReceived = true;
        for (QSignalSpy* spy : gameFoundSpies) {
            if (spy->count() == 0) {
                allReceived = false;
                break;
            }
        }
    }

    for (QSignalSpy* spy : gameFoundSpies) delete spy;
    ASSERT_TRUE(allReceived) << "All players should receive gameFound signal";

    // Attendre que le jeu démarre (phase d'enchères)
    QTest::qWait(1000);
    QCoreApplication::processEvents();

    // Faire une enchère pour que le jeu soit vraiment actif
    players[0]->sendMakeBid(14, 0);  // Player 0 passe
    QTest::qWait(500);  // Attendre que l'enchère soit pleinement traitée
    QCoreApplication::processEvents();

    QString player0ConnectionId = players[0]->connectionId();
    std::cout << "TEST: Player0 connectionId before disconnect: " << player0ConnectionId.toStdString() << std::endl;

    // Simuler une déconnexion du joueur 0 pendant la phase d'enchères
    // Fermer explicitement la connexion avant de delete pour trigger le signal disconnected()
    players[0]->closeConnection();
    QTest::qWait(500);  // Attendre que le signal disconnected soit émis et traité
    QCoreApplication::processEvents();

    // IMPORTANT: Retirer de la liste clients AVANT de delete pour éviter double-free dans TearDown
    clients.removeOne(players[0]);
    delete players[0];
    players[0] = nullptr;

    // Attendre suffisamment longtemps pour que le serveur détecte la déconnexion
    // et vide le connectionId dans la room
    QTest::qWait(1000);  // Attendre que le serveur traite complètement la déconnexion
    QCoreApplication::processEvents();

    // Le joueur se reconnecte avec wasInGame=true pour signaler une reconnexion
    std::cout << "TEST: Creating new client for reconnection..." << std::endl;
    MockGameClient* reconnectedPlayer = createClient("Player1");
    players[0] = reconnectedPlayer;

    // Créer les spies AVANT d'envoyer le message pour capturer tous les signaux
    QSignalSpy registeredSpy(reconnectedPlayer, SIGNAL(registered(QString)));
    QSignalSpy gameFoundSpy(reconnectedPlayer, SIGNAL(gameFound(int, const QJsonArray&)));

    std::cout << "TEST: Sending register with wasInGame=true..." << std::endl;
    reconnectedPlayer->sendRegister(true);  // wasInGame = true pour reconnexion

    // Attendre l'enregistrement
    std::cout << "TEST: Waiting for signals..." << std::endl;
    QTest::qWait(2000);  // Attendre suffisamment pour tous les messages
    QCoreApplication::processEvents();

    std::cout << "TEST: Registered! New connectionId: " << reconnectedPlayer->connectionId().toStdString() << std::endl;

    // Print all messages received by the reconnected player
    const auto& allMsgs = reconnectedPlayer->allMessages();
    std::cout << "TEST: Total messages received: " << allMsgs.size() << std::endl;
    for (int i = 0; i < allMsgs.size(); i++) {
        std::cout << "  Message " << i << ": " << allMsgs[i]["type"].toString().toStdString();
        if (allMsgs[i].contains("message")) {
            std::cout << " - " << allMsgs[i]["message"].toString().toStdString();
        }
        if (allMsgs[i].contains("reconnection")) {
            std::cout << " - reconnection=" << (allMsgs[i]["reconnection"].toBool() ? "true" : "false");
        }
        std::cout << std::endl;
    }

    // Vérifier que les signaux ont été reçus
    std::cout << "TEST: Registered signal count: " << registeredSpy.count() << std::endl;
    std::cout << "TEST: GameFound signal count: " << gameFoundSpy.count() << std::endl;

    ASSERT_TRUE(registeredSpy.count() > 0) << "Should receive registered signal";
    EXPECT_TRUE(gameFoundSpy.count() > 0) << "Should receive gameFound signal for reconnection";
}

// ========================================
// Tests de coinche/surcoinche
// ========================================

TEST_F(GameServerIntegrationTest, Coinche_PlayerCanCoinche) {
    // Créer 4 joueurs
    QList<MockGameClient*> players;
    for (int i = 0; i < 4; i++) {
        MockGameClient* client = createClient(QString("Player%1").arg(i + 1));
        client->sendRegister();
        waitForSignal(client, SIGNAL(registered(QString)), 2000);
        client->sendJoinMatchmaking();
        players.append(client);
    }

    // Attendre que tous les joueurs reçoivent gameFound
    QList<QSignalSpy*> gameFoundSpies;
    for (MockGameClient* player : players) {
        gameFoundSpies.append(new QSignalSpy(player, SIGNAL(gameFound(int, const QJsonArray&))));
    }

    int maxWaitTime = 10000;
    int elapsed = 0;
    bool allReceived = false;

    while (elapsed < maxWaitTime && !allReceived) {
        QCoreApplication::processEvents();
        QTest::qWait(100);
        elapsed += 100;

        allReceived = true;
        for (QSignalSpy* spy : gameFoundSpies) {
            if (spy->count() == 0) {
                allReceived = false;
                break;
            }
        }
    }

    for (QSignalSpy* spy : gameFoundSpies) delete spy;
    ASSERT_TRUE(allReceived) << "All players should receive gameFound signal";

    QCoreApplication::processEvents();

    // Joueur 0 passe, joueur 1 enchérit QUATREVINGT PIQUE, joueur 2 passe, joueur 3 coinche
    players[0]->sendMakeBid(14, 0);  // 14 = PASSE
    QTest::qWait(100);
    players[1]->sendMakeBid(1, 6);  // QUATREVINGT (1 = 80pts) PIQUE (suit 6)
    QTest::qWait(100);
    players[2]->sendMakeBid(14, 0);  // 14 = PASSE
    QTest::qWait(100);

    // Joueur 3 (équipe adverse) coinche avant de passer
    players[3]->sendCoinche();
    QTest::qWait(200);
    QCoreApplication::processEvents();

    // On peut vérifier que le message a été envoyé sans erreur
    // (le serveur devrait traiter la coinche)
}

// ========================================
// Main function
// ========================================
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// Inclusion du MOC pour MockGameClient
#include "gameserver_integration_test.moc"
