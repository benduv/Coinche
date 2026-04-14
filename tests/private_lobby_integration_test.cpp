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
#include <QDir>
#include <QFile>
#include <iostream>
#include "../server/GameServer.h"
#include "../server/DatabaseManager.h"

// Variable globale pour QCoreApplication
static int argc_lobby = 1;
static char* argv_lobby[] = {(char*)"test_private_lobby_integration", nullptr};
static QCoreApplication* appLobby = nullptr;

// ========================================
// Client WebSocket pour les tests de lobby
// ========================================
class MockLobbyClient : public QObject {
    Q_OBJECT

public:
    explicit MockLobbyClient(const QString& url, const QString& playerName, QObject* parent = nullptr)
        : QObject(parent)
        , m_socket(new QWebSocket)
        , m_playerName(playerName)
        , m_connected(false)
        , m_playerPosition(-1)
        , m_lobbyPlayerCount(0)
        , m_retourneeSuit(-1)
        , m_retourneeValue(-1)
        , m_isBeloteMode(false)
        , m_isReady(false)
        , m_isHost(false)
    {
        connect(m_socket, &QWebSocket::connected, this, [this]() {
            m_connected = true;
            emit connected();
        });

        connect(m_socket, &QWebSocket::textMessageReceived, this, [this](const QString& msg) {
            QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
            QJsonObject obj = doc.object();
            QString type = obj["type"].toString();

            m_allMessages.append(obj);

            if (type == "registered") {
                m_connectionId = obj["connectionId"].toString();
                emit registered();
            } else if (type == "lobbyCreated") {
                m_lobbyCode = obj["code"].toString();
                emit lobbyCreated(m_lobbyCode);
            } else if (type == "lobbyJoined") {
                m_lobbyCode = obj["code"].toString();
                emit lobbyJoined(m_lobbyCode);
            } else if (type == "lobbyUpdate") {
                m_lobbyGameMode = obj["gameMode"].toString("coinche");
                QJsonArray players = obj["players"].toArray();
                m_lobbyPlayerCount = players.size();
                for (const QJsonValue& v : players) {
                    QJsonObject p = v.toObject();
                    if (p["name"].toString() == m_playerName) {
                        m_isReady = p["ready"].toBool();
                        m_isHost = p["isHost"].toBool();
                    }
                }
                emit lobbyUpdated(obj);
            } else if (type == "lobbyGameStart") {
                emit lobbyGameStarted();
            } else if (type == "gameFound") {
                m_playerPosition = obj["playerPosition"].toInt();
                m_myCards = obj["myCards"].toArray();
                m_isBeloteMode = (obj["gameMode"].toString() == "belote");
                if (obj.contains("retournee")) {
                    QJsonObject ret = obj["retournee"].toObject();
                    m_retourneeSuit  = ret["suit"].toInt(-1);
                    m_retourneeValue = ret["value"].toInt(-1);
                }
                emit gameFound(m_playerPosition, m_myCards);
            } else if (type == "lobbyError") {
                m_lastError = obj["message"].toString();
                emit lobbyError(m_lastError);
            }
        });

        m_socket->open(QUrl(url));
    }

    ~MockLobbyClient() {
        if (m_socket) {
            m_socket->close();
            delete m_socket;
        }
    }

    // ---- Actions ----

    void sendRegister() {
        QJsonObject msg;
        msg["type"] = "register";
        msg["playerName"] = m_playerName;
        msg["avatar"] = "avataaars1.svg";
        msg["wasInGame"] = false;
        msg["version"] = GameServer::MIN_CLIENT_VERSION;
        sendMsg(msg);
    }

    void sendCreateLobby() {
        QJsonObject msg;
        msg["type"] = "createPrivateLobby";
        sendMsg(msg);
    }

    void sendJoinLobby(const QString& code) {
        QJsonObject msg;
        msg["type"] = "joinPrivateLobby";
        msg["code"] = code;
        sendMsg(msg);
    }

    void sendLobbyReady(bool ready) {
        QJsonObject msg;
        msg["type"] = "lobbyReady";
        msg["ready"] = ready;
        sendMsg(msg);
    }

    void sendSetGameMode(const QString& mode) {
        QJsonObject msg;
        msg["type"] = "setLobbyGameMode";
        msg["gameMode"] = mode;
        sendMsg(msg);
    }

    void sendStartLobbyGame() {
        QJsonObject msg;
        msg["type"] = "startLobbyGame";
        sendMsg(msg);
    }

    void sendLeaveLobby() {
        QJsonObject msg;
        msg["type"] = "leaveLobby";
        sendMsg(msg);
    }

    void sendJoinMatchmaking(const QString& gameMode = "coinche") {
        QJsonObject msg;
        msg["type"] = "joinMatchmaking";
        msg["gameMode"] = gameMode;
        sendMsg(msg);
    }

    void sendReorderPlayers(const QStringList& orderedNames) {
        QJsonArray arr;
        for (const QString& n : orderedNames) arr.append(n);
        QJsonObject msg;
        msg["type"] = "reorderLobbyPlayers";
        msg["order"] = arr;
        sendMsg(msg);
    }

    // ---- Accessors ----

    bool isConnected() const { return m_connected; }
    QString playerName() const { return m_playerName; }
    QString connectionId() const { return m_connectionId; }
    QString lobbyCode() const { return m_lobbyCode; }
    QString lobbyGameMode() const { return m_lobbyGameMode; }
    int lobbyPlayerCount() const { return m_lobbyPlayerCount; }
    bool isReady() const { return m_isReady; }
    bool isHost() const { return m_isHost; }
    int playerPosition() const { return m_playerPosition; }
    QJsonArray myCards() const { return m_myCards; }
    bool isBeloteMode() const { return m_isBeloteMode; }
    int retourneeSuit() const { return m_retourneeSuit; }
    int retourneeValue() const { return m_retourneeValue; }
    QString lastError() const { return m_lastError; }
    const QList<QJsonObject>& allMessages() const { return m_allMessages; }

    int countMessagesOfType(const QString& type) const {
        int count = 0;
        for (const QJsonObject& msg : m_allMessages) {
            if (msg["type"].toString() == type) count++;
        }
        return count;
    }

signals:
    void connected();
    void registered();
    void lobbyCreated(const QString& code);
    void lobbyJoined(const QString& code);
    void lobbyUpdated(const QJsonObject& update);
    void lobbyGameStarted();
    void gameFound(int position, const QJsonArray& cards);
    void lobbyError(const QString& message);

private:
    void sendMsg(const QJsonObject& message) {
        if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
            QJsonDocument doc(message);
            m_socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
        }
    }

    QWebSocket* m_socket;
    QString m_playerName;
    QString m_connectionId;
    QString m_lobbyCode;
    QString m_lobbyGameMode = "coinche";
    QString m_lastError;
    bool m_connected;
    bool m_isReady;
    bool m_isHost;
    int m_lobbyPlayerCount;
    int m_playerPosition;
    QJsonArray m_myCards;
    bool m_isBeloteMode;
    int m_retourneeSuit;
    int m_retourneeValue;
    QList<QJsonObject> m_allMessages;
};

// ========================================
// Test Fixture
// ========================================
class PrivateLobbyIntegrationTest : public ::testing::Test {
protected:
    GameServer* gameServer = nullptr;
    QList<MockLobbyClient*> clients;
    quint16 testPort;
    static int testCounter;

    static void SetUpTestSuite() {
        if (!appLobby) {
            appLobby = new QCoreApplication(argc_lobby, argv_lobby);
        }
    }

    void SetUp() override {
        testPort = 12600 + (++testCounter);
        QFile::remove("coinche.db");
        gameServer = new GameServer(testPort);
        QTest::qWait(200);
    }

    void TearDown() override {
        for (MockLobbyClient* c : clients) delete c;
        clients.clear();
        delete gameServer;
        gameServer = nullptr;
        QCoreApplication::processEvents();
    }

    // Crée un client et attend la connexion WebSocket
    MockLobbyClient* createClient(const QString& name) {
        QString url = QString("ws://localhost:%1").arg(testPort);
        auto* c = new MockLobbyClient(url, name);
        clients.append(c);
        if (!c->isConnected()) {
            EXPECT_TRUE(waitForSignal(c, SIGNAL(connected()), 3000));
        }
        return c;
    }

    // Crée un client, connecte, enregistre et attend registered
    MockLobbyClient* createRegisteredClient(const QString& name) {
        MockLobbyClient* c = createClient(name);
        c->sendRegister();
        EXPECT_TRUE(waitForSignal(c, SIGNAL(registered()), 2000));
        return c;
    }

    // Attend qu'un signal soit émis (retourne true si reçu avant timeout)
    bool waitForSignal(QObject* obj, const char* signal, int timeout = 3000) {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        QObject::connect(obj, signal, &loop, SLOT(quit()));
        timer.start(timeout);
        loop.exec();
        return timer.isActive();
    }

    // Attend que tous les clients aient reçu au moins un message "gameFound"
    // Utilise countMessagesOfType (ne rate pas les messages déjà reçus avant l'appel)
    bool waitAllGameFound(const QList<MockLobbyClient*>& players, int timeout = 10000) {
        int elapsed = 0;
        while (elapsed < timeout) {
            QCoreApplication::processEvents();
            QTest::qWait(100);
            elapsed += 100;
            bool allDone = true;
            for (auto* p : players) {
                if (p->countMessagesOfType("gameFound") == 0) { allDone = false; break; }
            }
            if (allDone) return true;
        }
        return false;
    }

    // Setup complet d'un lobby à N joueurs (tous enregistrés, host a créé, guests ont rejoint)
    // Retourne [host, guest1, guest2, ...] dans l'ordre d'entrée
    QList<MockLobbyClient*> setupLobby(int numPlayers, const QString& prefix = "LP") {
        QList<MockLobbyClient*> lobbyClients;

        MockLobbyClient* host = createRegisteredClient(prefix + "1");
        lobbyClients.append(host);

        host->sendCreateLobby();
        if (!waitForSignal(host, SIGNAL(lobbyCreated(QString)), 2000)) {
            ADD_FAILURE() << "Host did not receive lobbyCreated";
            return {};
        }
        QString code = host->lobbyCode();

        for (int i = 2; i <= numPlayers; i++) {
            MockLobbyClient* guest = createRegisteredClient(prefix + QString::number(i));
            lobbyClients.append(guest);
            guest->sendJoinLobby(code);
            if (!waitForSignal(guest, SIGNAL(lobbyJoined(QString)), 2000)) {
                ADD_FAILURE() << "Guest did not receive lobbyJoined";
                return {};
            }
            QTest::qWait(50);
            QCoreApplication::processEvents();
        }

        return lobbyClients;
    }

    // Envoie lobbyReady(true) pour tous les joueurs et attend que les updates soient traitées
    void makeAllReady(const QList<MockLobbyClient*>& players) {
        for (auto* p : players) {
            p->sendLobbyReady(true);
            QTest::qWait(100);
            QCoreApplication::processEvents();
        }
        QTest::qWait(200);
        QCoreApplication::processEvents();
    }
};

int PrivateLobbyIntegrationTest::testCounter = 0;

// ========================================
// Création de lobby
// ========================================

TEST_F(PrivateLobbyIntegrationTest, CreateLobby_HostReceives4CharCode) {
    MockLobbyClient* host = createRegisteredClient("Host1");
    QSignalSpy spy(host, SIGNAL(lobbyCreated(QString)));

    host->sendCreateLobby();

    ASSERT_TRUE(waitForSignal(host, SIGNAL(lobbyCreated(QString)), 2000));
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(host->lobbyCode().length(), 4) << "Le code doit avoir exactement 4 caractères";
}

TEST_F(PrivateLobbyIntegrationTest, CreateLobby_HostReceivesLobbyUpdate_1Player_CoinchByDefault) {
    MockLobbyClient* host = createRegisteredClient("Host2");

    host->sendCreateLobby();
    QTest::qWait(500);
    QCoreApplication::processEvents();

    EXPECT_GE(host->countMessagesOfType("lobbyUpdate"), 1);
    EXPECT_EQ(host->lobbyPlayerCount(), 1);
    EXPECT_EQ(host->lobbyGameMode(), "coinche");
    EXPECT_TRUE(host->isHost());
}

// ========================================
// Rejoindre un lobby
// ========================================

TEST_F(PrivateLobbyIntegrationTest, JoinLobby_ValidCode_BothSee2Players) {
    MockLobbyClient* host = createRegisteredClient("Host3");
    host->sendCreateLobby();
    ASSERT_TRUE(waitForSignal(host, SIGNAL(lobbyCreated(QString)), 2000));

    MockLobbyClient* guest = createRegisteredClient("Guest3");
    QSignalSpy joinSpy(guest, SIGNAL(lobbyJoined(QString)));

    guest->sendJoinLobby(host->lobbyCode());
    ASSERT_TRUE(waitForSignal(guest, SIGNAL(lobbyJoined(QString)), 2000));

    EXPECT_EQ(joinSpy.count(), 1);
    EXPECT_EQ(guest->lobbyCode(), host->lobbyCode());

    QTest::qWait(200);
    QCoreApplication::processEvents();

    EXPECT_EQ(host->lobbyPlayerCount(), 2);
    EXPECT_EQ(guest->lobbyPlayerCount(), 2);
}

TEST_F(PrivateLobbyIntegrationTest, JoinLobby_InvalidCode_ReturnsError) {
    MockLobbyClient* guest = createRegisteredClient("Guest4");
    QSignalSpy errorSpy(guest, SIGNAL(lobbyError(QString)));

    guest->sendJoinLobby("ZZZZ");

    ASSERT_TRUE(waitForSignal(guest, SIGNAL(lobbyError(QString)), 2000));
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_FALSE(guest->lastError().isEmpty());
}

TEST_F(PrivateLobbyIntegrationTest, JoinLobby_FullLobby4Players_ReturnsError) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(4, "Full");
    ASSERT_EQ(lobbyClients.size(), 4);

    MockLobbyClient* extra = createRegisteredClient("FullExtra");
    QSignalSpy errorSpy(extra, SIGNAL(lobbyError(QString)));
    extra->sendJoinLobby(lobbyClients[0]->lobbyCode());

    ASSERT_TRUE(waitForSignal(extra, SIGNAL(lobbyError(QString)), 2000));
    EXPECT_EQ(errorSpy.count(), 1);
}

TEST_F(PrivateLobbyIntegrationTest, JoinLobby_AlreadyInLobby_ReturnsError) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "Dup");
    ASSERT_EQ(lobbyClients.size(), 2);

    MockLobbyClient* guest = lobbyClients[1];
    QSignalSpy errorSpy(guest, SIGNAL(lobbyError(QString)));

    // Tenter de rejoindre le même lobby une deuxième fois
    guest->sendJoinLobby(lobbyClients[0]->lobbyCode());

    ASSERT_TRUE(waitForSignal(guest, SIGNAL(lobbyError(QString)), 2000));
    EXPECT_EQ(errorSpy.count(), 1);
}

// ========================================
// Quitter un lobby
// ========================================

TEST_F(PrivateLobbyIntegrationTest, LeaveLobby_GuestLeaves_HostSeesOnePlayer) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "Leave");
    ASSERT_EQ(lobbyClients.size(), 2);

    MockLobbyClient* host = lobbyClients[0];
    MockLobbyClient* guest = lobbyClients[1];

    QSignalSpy hostUpdateSpy(host, SIGNAL(lobbyUpdated(QJsonObject)));
    guest->sendLeaveLobby();

    ASSERT_TRUE(waitForSignal(host, SIGNAL(lobbyUpdated(QJsonObject)), 2000));
    EXPECT_EQ(host->lobbyPlayerCount(), 1);
}

TEST_F(PrivateLobbyIntegrationTest, LeaveLobby_HostLeaves_GuestBecomesHost) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "HostLeave");
    ASSERT_EQ(lobbyClients.size(), 2);

    MockLobbyClient* host = lobbyClients[0];
    MockLobbyClient* guest = lobbyClients[1];

    EXPECT_TRUE(host->isHost());
    EXPECT_FALSE(guest->isHost());

    QSignalSpy guestUpdateSpy(guest, SIGNAL(lobbyUpdated(QJsonObject)));
    host->sendLeaveLobby();

    ASSERT_TRUE(waitForSignal(guest, SIGNAL(lobbyUpdated(QJsonObject)), 2000));
    QTest::qWait(100);
    QCoreApplication::processEvents();

    EXPECT_EQ(guest->lobbyPlayerCount(), 1);
    EXPECT_TRUE(guest->isHost()) << "L'invité doit devenir hôte après le départ de l'hôte";
}

// ========================================
// Mode de jeu
// ========================================

TEST_F(PrivateLobbyIntegrationTest, SetGameMode_HostSwitchesToBelote_AllSeeUpdate) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "Mode");
    ASSERT_EQ(lobbyClients.size(), 2);

    MockLobbyClient* host = lobbyClients[0];
    MockLobbyClient* guest = lobbyClients[1];

    EXPECT_EQ(host->lobbyGameMode(), "coinche");

    host->sendSetGameMode("belote");

    ASSERT_TRUE(waitForSignal(host, SIGNAL(lobbyUpdated(QJsonObject)), 2000));
    QTest::qWait(200);
    QCoreApplication::processEvents();

    EXPECT_EQ(host->lobbyGameMode(), "belote");
    EXPECT_EQ(guest->lobbyGameMode(), "belote");
}

TEST_F(PrivateLobbyIntegrationTest, SetGameMode_SwitchBackToCoinche_AllSeeUpdate) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "ModeBack");
    ASSERT_EQ(lobbyClients.size(), 2);

    MockLobbyClient* host = lobbyClients[0];
    MockLobbyClient* guest = lobbyClients[1];

    host->sendSetGameMode("belote");
    ASSERT_TRUE(waitForSignal(host, SIGNAL(lobbyUpdated(QJsonObject)), 2000));

    host->sendSetGameMode("coinche");
    ASSERT_TRUE(waitForSignal(host, SIGNAL(lobbyUpdated(QJsonObject)), 2000));
    QTest::qWait(100);
    QCoreApplication::processEvents();

    EXPECT_EQ(host->lobbyGameMode(), "coinche");
    EXPECT_EQ(guest->lobbyGameMode(), "coinche");
}

TEST_F(PrivateLobbyIntegrationTest, SetGameMode_NonHostSendsRequest_IgnoredByServer) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "ModeNonHost");
    ASSERT_EQ(lobbyClients.size(), 2);

    MockLobbyClient* host = lobbyClients[0];
    MockLobbyClient* guest = lobbyClients[1];

    int hostUpdatesBefore = host->countMessagesOfType("lobbyUpdate");
    guest->sendSetGameMode("belote");
    QTest::qWait(500);
    QCoreApplication::processEvents();

    // Aucun lobbyUpdate supplémentaire pour l'hôte
    EXPECT_EQ(host->countMessagesOfType("lobbyUpdate"), hostUpdatesBefore);
    EXPECT_EQ(host->lobbyGameMode(), "coinche") << "Le mode ne doit pas avoir changé";
}

TEST_F(PrivateLobbyIntegrationTest, SetGameMode_InvalidModeDefaultsToCoinche) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(1, "ModeInvalid");
    ASSERT_EQ(lobbyClients.size(), 1);

    MockLobbyClient* host = lobbyClients[0];

    host->sendSetGameMode("mahjong");  // mode invalide
    ASSERT_TRUE(waitForSignal(host, SIGNAL(lobbyUpdated(QJsonObject)), 2000));
    QTest::qWait(100);
    QCoreApplication::processEvents();

    EXPECT_EQ(host->lobbyGameMode(), "coinche") << "Un mode invalide doit fallback à coinche";
}

// ========================================
// Réorganisation des joueurs
// ========================================

TEST_F(PrivateLobbyIntegrationTest, ReorderPlayers_HostInvertsOrder_LobbyUpdated) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "Reorder");
    ASSERT_EQ(lobbyClients.size(), 2);

    MockLobbyClient* host = lobbyClients[0];
    MockLobbyClient* guest = lobbyClients[1];

    int updatesBefore = host->countMessagesOfType("lobbyUpdate");

    // Inverser l'ordre (guest d'abord, puis host)
    host->sendReorderPlayers({guest->playerName(), host->playerName()});

    ASSERT_TRUE(waitForSignal(host, SIGNAL(lobbyUpdated(QJsonObject)), 2000));
    EXPECT_GT(host->countMessagesOfType("lobbyUpdate"), updatesBefore)
        << "Un lobbyUpdate doit être envoyé après la réorganisation";
    EXPECT_EQ(host->lobbyPlayerCount(), 2);
}

TEST_F(PrivateLobbyIntegrationTest, ReorderPlayers_NonHostIgnored) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "ReorderNH");
    ASSERT_EQ(lobbyClients.size(), 2);

    MockLobbyClient* host = lobbyClients[0];
    MockLobbyClient* guest = lobbyClients[1];

    int hostUpdatesBefore = host->countMessagesOfType("lobbyUpdate");

    // Le guest tente de réorganiser
    guest->sendReorderPlayers({host->playerName(), guest->playerName()});
    QTest::qWait(400);
    QCoreApplication::processEvents();

    EXPECT_EQ(host->countMessagesOfType("lobbyUpdate"), hostUpdatesBefore)
        << "Un non-hôte ne peut pas réorganiser les joueurs";
}

// ========================================
// Statut prêt
// ========================================

TEST_F(PrivateLobbyIntegrationTest, ReadyStatus_PlayerTogglesReady_OthersSeeUpdate) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "Rdy");
    ASSERT_EQ(lobbyClients.size(), 2);

    MockLobbyClient* host = lobbyClients[0];
    MockLobbyClient* guest = lobbyClients[1];

    QSignalSpy guestSpy(guest, SIGNAL(lobbyUpdated(QJsonObject)));
    host->sendLobbyReady(true);

    // Attendre que le guest et l'hôte aient tous les deux traité le lobbyUpdate
    ASSERT_TRUE(waitForSignal(guest, SIGNAL(lobbyUpdated(QJsonObject)), 2000));
    // Laisser le temps à l'hôte de traiter son propre lobbyUpdate
    QTest::qWait(200);
    QCoreApplication::processEvents();

    EXPECT_GE(guestSpy.count(), 1);
    EXPECT_TRUE(host->isReady()) << "L'hôte doit voir son propre statut prêt après lobbyUpdate";
}

// ========================================
// Erreurs de démarrage
// ========================================

TEST_F(PrivateLobbyIntegrationTest, StartLobby_NotAllReady_ReturnsError) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "StartErr");
    ASSERT_EQ(lobbyClients.size(), 2);

    MockLobbyClient* host = lobbyClients[0];
    // Aucun joueur n'est prêt

    QSignalSpy errorSpy(host, SIGNAL(lobbyError(QString)));
    host->sendStartLobbyGame();

    ASSERT_TRUE(waitForSignal(host, SIGNAL(lobbyError(QString)), 2000));
    EXPECT_EQ(errorSpy.count(), 1);
}

TEST_F(PrivateLobbyIntegrationTest, StartLobby_3Players_WrongCount_ReturnsError) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(3, "Start3");
    ASSERT_EQ(lobbyClients.size(), 3);

    makeAllReady(lobbyClients);

    MockLobbyClient* host = lobbyClients[0];
    QSignalSpy errorSpy(host, SIGNAL(lobbyError(QString)));
    host->sendStartLobbyGame();

    ASSERT_TRUE(waitForSignal(host, SIGNAL(lobbyError(QString)), 2000));
    EXPECT_EQ(errorSpy.count(), 1);
}

TEST_F(PrivateLobbyIntegrationTest, StartLobby_NonHostTriesToStart_ReturnsError) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "StartNH");
    ASSERT_EQ(lobbyClients.size(), 2);

    makeAllReady(lobbyClients);

    MockLobbyClient* guest = lobbyClients[1];
    QSignalSpy errorSpy(guest, SIGNAL(lobbyError(QString)));
    guest->sendStartLobbyGame();

    ASSERT_TRUE(waitForSignal(guest, SIGNAL(lobbyError(QString)), 2000));
    EXPECT_EQ(errorSpy.count(), 1);
}

// ========================================
// Partie à 4 joueurs — Mode Coinche
// ========================================

TEST_F(PrivateLobbyIntegrationTest, StartLobby_4Players_Coinche_AllReceiveGameFound_8Cards) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(4, "C4");
    ASSERT_EQ(lobbyClients.size(), 4);

    makeAllReady(lobbyClients);

    // L'hôte lance la partie
    lobbyClients[0]->sendStartLobbyGame();

    // Tous doivent recevoir lobbyGameStart
    QTest::qWait(300);
    QCoreApplication::processEvents();
    for (auto* p : lobbyClients) {
        EXPECT_GE(p->countMessagesOfType("lobbyGameStart"), 1)
            << qPrintable(p->playerName()) << " doit recevoir lobbyGameStart";
    }

    // Tous doivent recevoir gameFound
    ASSERT_TRUE(waitAllGameFound(lobbyClients, 10000))
        << "Tous les joueurs doivent recevoir gameFound";

    QSet<int> positions;
    for (auto* p : lobbyClients) {
        EXPECT_EQ(p->myCards().size(), 8)
            << qPrintable(p->playerName()) << " doit avoir 8 cartes en Coinche";
        EXPECT_FALSE(p->isBeloteMode()) << "isBeloteMode doit être false en Coinche";
        positions.insert(p->playerPosition());
    }
    EXPECT_EQ(positions.size(), 4) << "Les 4 positions (0-3) doivent être uniques";
}

TEST_F(PrivateLobbyIntegrationTest, StartLobby_4Players_Coinche_TeamsRespected) {
    // Lobby indices 0,1 = équipe 1 ; 2,3 = équipe 2
    // En jeu : positions 0,2 = partenaires (équipe 1 lobby), 1,3 = adversaires (équipe 2 lobby)
    QList<MockLobbyClient*> lobbyClients = setupLobby(4, "C4Teams");
    ASSERT_EQ(lobbyClients.size(), 4);

    makeAllReady(lobbyClients);
    lobbyClients[0]->sendStartLobbyGame();

    ASSERT_TRUE(waitAllGameFound(lobbyClients, 10000));

    // lobby[0] et lobby[1] → équipe 1 → positions 0 et 2
    int pos0 = lobbyClients[0]->playerPosition();
    int pos1 = lobbyClients[1]->playerPosition();
    EXPECT_TRUE((pos0 == 0 && pos1 == 2) || (pos0 == 2 && pos1 == 0))
        << "lobby[0] et lobby[1] (équipe 1) doivent être aux positions 0 et 2. Got: "
        << pos0 << " et " << pos1;

    // lobby[2] et lobby[3] → équipe 2 → positions 1 et 3
    int pos2 = lobbyClients[2]->playerPosition();
    int pos3 = lobbyClients[3]->playerPosition();
    EXPECT_TRUE((pos2 == 1 && pos3 == 3) || (pos2 == 3 && pos3 == 1))
        << "lobby[2] et lobby[3] (équipe 2) doivent être aux positions 1 et 3. Got: "
        << pos2 << " et " << pos3;
}

// ========================================
// Partie à 4 joueurs — Mode Belote
// ========================================

TEST_F(PrivateLobbyIntegrationTest, StartLobby_4Players_Belote_AllReceiveGameFound_5Cards) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(4, "B4");
    ASSERT_EQ(lobbyClients.size(), 4);

    lobbyClients[0]->sendSetGameMode("belote");
    ASSERT_TRUE(waitForSignal(lobbyClients[0], SIGNAL(lobbyUpdated(QJsonObject)), 2000));
    QTest::qWait(100);

    for (auto* p : lobbyClients) {
        EXPECT_EQ(p->lobbyGameMode(), "belote");
    }

    makeAllReady(lobbyClients);
    lobbyClients[0]->sendStartLobbyGame();

    ASSERT_TRUE(waitAllGameFound(lobbyClients, 10000))
        << "Tous les joueurs doivent recevoir gameFound en Belote";

    for (auto* p : lobbyClients) {
        EXPECT_EQ(p->myCards().size(), 5)
            << qPrintable(p->playerName()) << " doit avoir 5 cartes en Belote";
        EXPECT_TRUE(p->isBeloteMode()) << "isBeloteMode doit être true";
        EXPECT_GE(p->retourneeSuit(), 0) << "retourneeSuit doit être valide (>= 0)";
        EXPECT_GE(p->retourneeValue(), 0) << "retourneeValue doit être valide (>= 0)";
    }
}

TEST_F(PrivateLobbyIntegrationTest, StartLobby_4Players_Belote_SameRetourneeSentToAll) {
    QList<MockLobbyClient*> lobbyClients = setupLobby(4, "B4Ret");
    ASSERT_EQ(lobbyClients.size(), 4);

    lobbyClients[0]->sendSetGameMode("belote");
    ASSERT_TRUE(waitForSignal(lobbyClients[0], SIGNAL(lobbyUpdated(QJsonObject)), 2000));

    makeAllReady(lobbyClients);
    lobbyClients[0]->sendStartLobbyGame();

    ASSERT_TRUE(waitAllGameFound(lobbyClients, 10000));

    // Tous voient la même carte retournée
    int refSuit = lobbyClients[0]->retourneeSuit();
    int refValue = lobbyClients[0]->retourneeValue();
    EXPECT_GE(refSuit, 0);
    EXPECT_GE(refValue, 0);

    for (auto* p : lobbyClients) {
        EXPECT_EQ(p->retourneeSuit(), refSuit);
        EXPECT_EQ(p->retourneeValue(), refValue);
    }
}

TEST_F(PrivateLobbyIntegrationTest, StartLobby_4Players_Belote_32CardsTotal) {
    // Après distribution belote (5×4 = 20 cartes en main + 1 retournée = 21 distribuées)
    // La phase de jeu complète en donne 8 par joueur (32 total) mais ici on vérifie juste les 5
    QList<MockLobbyClient*> lobbyClients = setupLobby(4, "B4Total");
    ASSERT_EQ(lobbyClients.size(), 4);

    lobbyClients[0]->sendSetGameMode("belote");
    ASSERT_TRUE(waitForSignal(lobbyClients[0], SIGNAL(lobbyUpdated(QJsonObject)), 2000));

    makeAllReady(lobbyClients);
    lobbyClients[0]->sendStartLobbyGame();

    ASSERT_TRUE(waitAllGameFound(lobbyClients, 10000));

    // Vérifier que les cartes en main sont bien 5 par joueur (phase initiale Belote)
    int totalCards = 0;
    for (auto* p : lobbyClients) {
        totalCards += p->myCards().size();
    }
    EXPECT_EQ(totalCards, 20) << "5 cartes × 4 joueurs = 20 cartes au total en phase initiale Belote";
}

// ========================================
// Partie à 2 joueurs (matchmaking avec partenaires)
// ========================================

TEST_F(PrivateLobbyIntegrationTest, StartLobby_2Players_Coinche_PlusTwoMatchmaking_GameFound) {
    // 2 joueurs forment un lobby Coinche
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "MM2C");
    ASSERT_EQ(lobbyClients.size(), 2);

    // 2 joueurs extérieurs rejoignent le matchmaking Coinche
    MockLobbyClient* extra1 = createRegisteredClient("MM2CExtra1");
    MockLobbyClient* extra2 = createRegisteredClient("MM2CExtra2");

    extra1->sendJoinMatchmaking("coinche");
    QTest::qWait(100);
    extra2->sendJoinMatchmaking("coinche");
    QTest::qWait(100);
    QCoreApplication::processEvents();

    makeAllReady(lobbyClients);
    lobbyClients[0]->sendStartLobbyGame();

    // Les 4 joueurs doivent recevoir gameFound
    QList<MockLobbyClient*> allPlayers = {lobbyClients[0], lobbyClients[1], extra1, extra2};
    ASSERT_TRUE(waitAllGameFound(allPlayers, 10000))
        << "Les 4 joueurs (2 lobby + 2 matchmaking) doivent recevoir gameFound";

    for (auto* p : allPlayers) {
        EXPECT_EQ(p->myCards().size(), 8)
            << qPrintable(p->playerName()) << " doit avoir 8 cartes en Coinche";
        EXPECT_FALSE(p->isBeloteMode());
    }
}

TEST_F(PrivateLobbyIntegrationTest, StartLobby_2Players_LobbyPartnersAtPositions0And2) {
    // Les partenaires de lobby doivent être aux positions 0 et 2 (partenaires en jeu)
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "MM2Pos");
    ASSERT_EQ(lobbyClients.size(), 2);

    MockLobbyClient* extra1 = createRegisteredClient("MM2PosExtra1");
    MockLobbyClient* extra2 = createRegisteredClient("MM2PosExtra2");

    extra1->sendJoinMatchmaking("coinche");
    QTest::qWait(100);
    extra2->sendJoinMatchmaking("coinche");
    QTest::qWait(100);
    QCoreApplication::processEvents();

    makeAllReady(lobbyClients);
    lobbyClients[0]->sendStartLobbyGame();

    QList<MockLobbyClient*> allPlayers = {lobbyClients[0], lobbyClients[1], extra1, extra2};
    ASSERT_TRUE(waitAllGameFound(allPlayers, 10000));

    int lobbyPos0 = lobbyClients[0]->playerPosition();
    int lobbyPos1 = lobbyClients[1]->playerPosition();
    EXPECT_TRUE((lobbyPos0 == 0 && lobbyPos1 == 2) || (lobbyPos0 == 2 && lobbyPos1 == 0))
        << "Les partenaires de lobby doivent être aux positions 0 et 2. Got: "
        << lobbyPos0 << " et " << lobbyPos1;
}

TEST_F(PrivateLobbyIntegrationTest, StartLobby_2Players_Belote_PlusTwoMatchmaking_GameFound) {
    // 2 joueurs forment un lobby Belote
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "MM2B");
    ASSERT_EQ(lobbyClients.size(), 2);

    lobbyClients[0]->sendSetGameMode("belote");
    ASSERT_TRUE(waitForSignal(lobbyClients[0], SIGNAL(lobbyUpdated(QJsonObject)), 2000));

    // 2 joueurs extérieurs en mode Belote rejoignent le matchmaking
    MockLobbyClient* extra1 = createRegisteredClient("MM2BExtra1");
    MockLobbyClient* extra2 = createRegisteredClient("MM2BExtra2");

    extra1->sendJoinMatchmaking("belote");
    QTest::qWait(100);
    extra2->sendJoinMatchmaking("belote");
    QTest::qWait(100);
    QCoreApplication::processEvents();

    makeAllReady(lobbyClients);
    lobbyClients[0]->sendStartLobbyGame();

    QList<MockLobbyClient*> allPlayers = {lobbyClients[0], lobbyClients[1], extra1, extra2};
    ASSERT_TRUE(waitAllGameFound(allPlayers, 10000))
        << "Les 4 joueurs Belote doivent recevoir gameFound";

    for (auto* p : allPlayers) {
        EXPECT_EQ(p->myCards().size(), 5)
            << qPrintable(p->playerName()) << " doit avoir 5 cartes en Belote";
        EXPECT_TRUE(p->isBeloteMode());
    }
}

TEST_F(PrivateLobbyIntegrationTest, StartLobby_2Players_ModeMismatch_CoincheLobbyDoesNotMatchBeloteQueue) {
    // 2 joueurs en lobby Coinche ne doivent PAS matcher avec 2 joueurs en queue Belote
    QList<MockLobbyClient*> lobbyClients = setupLobby(2, "MMix");
    ASSERT_EQ(lobbyClients.size(), 2);
    // mode par défaut = coinche

    MockLobbyClient* extra1 = createRegisteredClient("MMixExtra1");
    MockLobbyClient* extra2 = createRegisteredClient("MMixExtra2");

    // Ces extras rejoignent la queue Belote (pas Coinche)
    extra1->sendJoinMatchmaking("belote");
    QTest::qWait(100);
    extra2->sendJoinMatchmaking("belote");
    QTest::qWait(100);
    QCoreApplication::processEvents();

    makeAllReady(lobbyClients);
    lobbyClients[0]->sendStartLobbyGame();

    // Après 2s, aucun des 4 ne doit avoir reçu gameFound (files séparées)
    QTest::qWait(2000);
    QCoreApplication::processEvents();

    QList<MockLobbyClient*> lobbyOnly = {lobbyClients[0], lobbyClients[1]};
    for (auto* p : lobbyOnly) {
        EXPECT_EQ(p->countMessagesOfType("gameFound"), 0)
            << qPrintable(p->playerName()) << " ne doit pas avoir reçu gameFound (files séparées)";
    }
}

// ========================================
// Main
// ========================================
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include "private_lobby_integration_test.moc"
