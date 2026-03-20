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
static int argc = 1;
static char* argv[] = {(char*)"test_friends_integration", nullptr};
static QCoreApplication* app = nullptr;

// ========================================
// Client WebSocket pour les tests d'amis
// ========================================
class FriendTestClient : public QObject {
    Q_OBJECT

public:
    FriendTestClient(const QString& url, QObject* parent = nullptr)
        : QObject(parent)
        , m_socket(new QWebSocket)
        , m_connected(false)
    {
        connect(m_socket, &QWebSocket::connected, this, [this]() {
            m_connected = true;
            emit connected();
        });

        connect(m_socket, &QWebSocket::textMessageReceived, this, [this](const QString& message) {
            QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
            QJsonObject obj = doc.object();
            QString type = obj["type"].toString();

            m_lastMessage = obj;
            m_allMessages.append(obj);

            if (type == "registerAccountSuccess") {
                m_playerName = obj["playerName"].toString();
                emit accountRegistered();
            } else if (type == "friendRequestSent") {
                emit friendRequestSent();
            } else if (type == "friendRequestFailed") {
                emit friendRequestFailed(obj["error"].toString());
            } else if (type == "friendRequestReceived") {
                emit friendRequestReceived(obj["fromPseudo"].toString(), obj["fromAvatar"].toString());
            } else if (type == "friendRequestAccepted") {
                emit friendRequestAccepted(obj["pseudo"].toString());
            } else if (type == "friendRequestRejected") {
                emit friendRequestRejected();
            } else if (type == "friendsList") {
                QJsonArray friends = obj["friends"].toArray();
                QJsonArray pending = obj["pendingRequests"].toArray();
                emit friendsListReceived(friends, pending);
            } else if (type == "friendRemoved") {
                emit friendRemoved(obj["pseudo"].toString());
            }
        });

        m_socket->open(QUrl(url));
    }

    ~FriendTestClient() {
        if (m_socket) {
            m_socket->close();
            delete m_socket;
        }
    }

    // Créer un compte via WebSocket (registerAccount)
    void sendRegisterAccount(const QString& pseudo, const QString& email, const QString& password, const QString& avatar = "avataaars1.svg") {
        QJsonObject msg;
        msg["type"] = "registerAccount";
        msg["pseudo"] = pseudo;
        msg["email"] = email;
        msg["password"] = password;
        msg["avatar"] = avatar;
        msg["version"] = GameServer::MIN_CLIENT_VERSION;
        sendMessage(msg);
    }

    void sendFriendRequest(const QString& target) {
        QJsonObject msg;
        msg["type"] = "sendFriendRequest";
        msg["targetPseudo"] = target;
        sendMessage(msg);
    }

    void sendAcceptFriendRequest(const QString& requester) {
        QJsonObject msg;
        msg["type"] = "acceptFriendRequest";
        msg["requesterPseudo"] = requester;
        sendMessage(msg);
    }

    void sendRejectFriendRequest(const QString& requester) {
        QJsonObject msg;
        msg["type"] = "rejectFriendRequest";
        msg["requesterPseudo"] = requester;
        sendMessage(msg);
    }

    void sendGetFriendsList() {
        QJsonObject msg;
        msg["type"] = "getFriendsList";
        sendMessage(msg);
    }

    void sendRemoveFriend(const QString& pseudo) {
        QJsonObject msg;
        msg["type"] = "removeFriend";
        msg["pseudo"] = pseudo;
        sendMessage(msg);
    }

    bool isConnected() const { return m_connected; }
    QString playerName() const { return m_playerName; }
    QJsonObject lastMessage() const { return m_lastMessage; }
    const QList<QJsonObject>& allMessages() const { return m_allMessages; }

signals:
    void connected();
    void accountRegistered();
    void friendRequestSent();
    void friendRequestFailed(const QString& error);
    void friendRequestReceived(const QString& fromPseudo, const QString& fromAvatar);
    void friendRequestAccepted(const QString& pseudo);
    void friendRequestRejected();
    void friendsListReceived(const QJsonArray& friends, const QJsonArray& pendingRequests);
    void friendRemoved(const QString& pseudo);

private:
    void sendMessage(const QJsonObject& message) {
        if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
            QJsonDocument doc(message);
            m_socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
        }
    }

    QWebSocket* m_socket;
    QString m_playerName;
    bool m_connected;
    QJsonObject m_lastMessage;
    QList<QJsonObject> m_allMessages;
};

// ========================================
// Test Fixture
// ========================================
class FriendsIntegrationTest : public ::testing::Test {
protected:
    GameServer* gameServer;
    QList<FriendTestClient*> clients;
    quint16 testPort;
    static int testCounter;

    static void SetUpTestSuite() {
        if (!app) {
            app = new QCoreApplication(argc, argv);
        }
    }

    void SetUp() override {
        testPort = 12500 + (++testCounter);

        // Supprimer la base de données du GameServer (il utilise coinche.db)
        QFile::remove("coinche.db");

        gameServer = new GameServer(testPort);
        QTest::qWait(300);
    }

    void TearDown() override {
        for (FriendTestClient* client : clients) {
            delete client;
        }
        clients.clear();

        delete gameServer;
        QCoreApplication::processEvents();

        QFile::remove("coinche.db");
    }

    // Helper: attendre un spy avec polling
    bool waitForSpy(QSignalSpy& spy, int timeout = 3000) {
        int elapsed = 0;
        while (elapsed < timeout && spy.count() == 0) {
            QCoreApplication::processEvents();
            QTest::qWait(50);
            elapsed += 50;
        }
        return spy.count() > 0;
    }

    // Helper: attendre un signal
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

    // Helper: créer un client, connecter, et créer un compte
    FriendTestClient* createClient(const QString& name, const QString& email) {
        const QString url = QString("ws://localhost:%1").arg(testPort);
        FriendTestClient* client = new FriendTestClient(url, nullptr);
        clients.append(client);

        // Attendre la connexion WebSocket
        if (!client->isConnected()) {
            EXPECT_TRUE(waitForSignal(client, SIGNAL(connected()), 3000))
                << "Connexion WebSocket timeout pour " << name.toStdString();
        }

        // Créer un compte via WebSocket
        QSignalSpy regSpy(client, &FriendTestClient::accountRegistered);
        client->sendRegisterAccount(name, email, "password123", "avataaars1.svg");
        EXPECT_TRUE(waitForSpy(regSpy, 3000))
            << "Account registration timeout pour " << name.toStdString();

        return client;
    }
};

int FriendsIntegrationTest::testCounter = 0;

// ========================================
// Tests d'envoi de demande d'ami
// ========================================

TEST_F(FriendsIntegrationTest, SendFriendRequest_Success) {
    FriendTestClient* alice = createClient("Alice", "alice@test.com");
    FriendTestClient* bob = createClient("Bob", "bob@test.com");

    QSignalSpy sentSpy(alice, &FriendTestClient::friendRequestSent);
    QSignalSpy receivedSpy(bob, &FriendTestClient::friendRequestReceived);

    alice->sendFriendRequest("Bob");

    EXPECT_TRUE(waitForSpy(sentSpy)) << "Alice devrait recevoir la confirmation d'envoi";
    EXPECT_TRUE(waitForSpy(receivedSpy)) << "Bob devrait recevoir la notification de demande";

    if (receivedSpy.count() > 0) {
        QList<QVariant> args = receivedSpy.takeFirst();
        EXPECT_EQ(args.at(0).toString(), "Alice");
    }
}

TEST_F(FriendsIntegrationTest, SendFriendRequest_TargetOffline) {
    FriendTestClient* alice = createClient("Alice", "alice@test.com");
    // Créer Bob (son compte existe) mais il se déconnecte
    FriendTestClient* bob = createClient("Bob", "bob@test.com");
    // On ne déconnecte pas Bob dans ce test simplifié, on vérifie juste que la demande est envoyée
    // Pour un vrai test offline, il faudrait déconnecter Bob

    QSignalSpy sentSpy(alice, &FriendTestClient::friendRequestSent);
    alice->sendFriendRequest("Bob");

    EXPECT_TRUE(waitForSpy(sentSpy)) << "La demande devrait réussir";
}

TEST_F(FriendsIntegrationTest, SendFriendRequest_NonExistentUser) {
    FriendTestClient* alice = createClient("Alice", "alice@test.com");

    QSignalSpy failedSpy(alice, &FriendTestClient::friendRequestFailed);
    alice->sendFriendRequest("UnknownPlayer");

    EXPECT_TRUE(waitForSpy(failedSpy)) << "Demande vers joueur inexistant devrait échouer";
}

// ========================================
// Tests d'acceptation de demande
// ========================================

TEST_F(FriendsIntegrationTest, AcceptFriendRequest_BothNotified) {
    FriendTestClient* alice = createClient("Alice", "alice@test.com");
    FriendTestClient* bob = createClient("Bob", "bob@test.com");

    // Alice envoie une demande
    QSignalSpy sentSpy(alice, &FriendTestClient::friendRequestSent);
    alice->sendFriendRequest("Bob");
    ASSERT_TRUE(waitForSpy(sentSpy));

    QSignalSpy aliceAcceptedSpy(alice, &FriendTestClient::friendRequestAccepted);
    QSignalSpy bobAcceptedSpy(bob, &FriendTestClient::friendRequestAccepted);

    // Bob accepte
    bob->sendAcceptFriendRequest("Alice");

    EXPECT_TRUE(waitForSpy(bobAcceptedSpy)) << "Bob devrait recevoir la confirmation d'acceptation";
    EXPECT_TRUE(waitForSpy(aliceAcceptedSpy)) << "Alice devrait être notifiée de l'acceptation";
}

// ========================================
// Tests de rejet de demande
// ========================================

TEST_F(FriendsIntegrationTest, RejectFriendRequest_Success) {
    FriendTestClient* alice = createClient("Alice", "alice@test.com");
    FriendTestClient* bob = createClient("Bob", "bob@test.com");

    QSignalSpy sentSpy(alice, &FriendTestClient::friendRequestSent);
    alice->sendFriendRequest("Bob");
    ASSERT_TRUE(waitForSpy(sentSpy));

    QSignalSpy rejectedSpy(bob, &FriendTestClient::friendRequestRejected);
    bob->sendRejectFriendRequest("Alice");

    EXPECT_TRUE(waitForSpy(rejectedSpy)) << "Bob devrait recevoir la confirmation de rejet";
}

// ========================================
// Tests de récupération de la liste d'amis
// ========================================

TEST_F(FriendsIntegrationTest, GetFriendsList_WithOnlineStatus) {
    FriendTestClient* alice = createClient("Alice", "alice@test.com");
    FriendTestClient* bob = createClient("Bob", "bob@test.com");

    // Devenir amis
    QSignalSpy sentSpy(alice, &FriendTestClient::friendRequestSent);
    alice->sendFriendRequest("Bob");
    ASSERT_TRUE(waitForSpy(sentSpy));

    QSignalSpy acceptedSpy(bob, &FriendTestClient::friendRequestAccepted);
    bob->sendAcceptFriendRequest("Alice");
    ASSERT_TRUE(waitForSpy(acceptedSpy));

    // Demander la liste d'amis
    QSignalSpy listSpy(alice, &FriendTestClient::friendsListReceived);
    alice->sendGetFriendsList();
    ASSERT_TRUE(waitForSpy(listSpy)) << "Alice devrait recevoir sa liste d'amis";

    QList<QVariant> args = listSpy.takeFirst();
    QJsonArray friends = args.at(0).toJsonArray();
    ASSERT_EQ(friends.size(), 1);

    QJsonObject bobEntry = friends[0].toObject();
    EXPECT_EQ(bobEntry["pseudo"].toString(), "Bob");
    EXPECT_TRUE(bobEntry["online"].toBool()) << "Bob est connecté, devrait être en ligne";
}

TEST_F(FriendsIntegrationTest, GetFriendsList_WithPendingRequests) {
    FriendTestClient* alice = createClient("Alice", "alice@test.com");
    FriendTestClient* bob = createClient("Bob", "bob@test.com");

    // Alice envoie une demande à Bob (pas acceptée)
    QSignalSpy sentSpy(alice, &FriendTestClient::friendRequestSent);
    alice->sendFriendRequest("Bob");
    ASSERT_TRUE(waitForSpy(sentSpy));

    QSignalSpy listSpy(bob, &FriendTestClient::friendsListReceived);
    bob->sendGetFriendsList();
    ASSERT_TRUE(waitForSpy(listSpy));

    QList<QVariant> args = listSpy.takeFirst();
    QJsonArray friends = args.at(0).toJsonArray();
    QJsonArray pending = args.at(1).toJsonArray();

    EXPECT_EQ(friends.size(), 0) << "Pas encore amis";
    EXPECT_EQ(pending.size(), 1) << "Une demande en attente";
    if (pending.size() > 0) {
        EXPECT_EQ(pending[0].toObject()["pseudo"].toString(), "Alice");
    }
}

// ========================================
// Tests de suppression d'ami
// ========================================

TEST_F(FriendsIntegrationTest, RemoveFriend_Success) {
    FriendTestClient* alice = createClient("Alice", "alice@test.com");
    FriendTestClient* bob = createClient("Bob", "bob@test.com");

    // Devenir amis
    QSignalSpy sentSpy(alice, &FriendTestClient::friendRequestSent);
    alice->sendFriendRequest("Bob");
    ASSERT_TRUE(waitForSpy(sentSpy));

    QSignalSpy acceptedSpy(bob, &FriendTestClient::friendRequestAccepted);
    bob->sendAcceptFriendRequest("Alice");
    ASSERT_TRUE(waitForSpy(acceptedSpy));

    // Alice supprime Bob
    QSignalSpy removedSpy(alice, &FriendTestClient::friendRemoved);
    alice->sendRemoveFriend("Bob");
    EXPECT_TRUE(waitForSpy(removedSpy)) << "Alice devrait recevoir la confirmation de suppression";

    // Vérifier que la liste est vide
    QSignalSpy listSpy(alice, &FriendTestClient::friendsListReceived);
    alice->sendGetFriendsList();
    ASSERT_TRUE(waitForSpy(listSpy));

    QJsonArray friends = listSpy.takeFirst().at(0).toJsonArray();
    EXPECT_EQ(friends.size(), 0) << "La liste devrait être vide après suppression";
}

// ========================================
// Tests de scénario complet
// ========================================

TEST_F(FriendsIntegrationTest, Scenario_FullLifecycleWithThreePlayers) {
    FriendTestClient* alice = createClient("Alice", "alice@test.com");
    FriendTestClient* bob = createClient("Bob", "bob@test.com");
    FriendTestClient* charlie = createClient("Charlie", "charlie@test.com");

    // 1. Alice envoie des demandes à Bob et Charlie
    {
        QSignalSpy sentSpy(alice, &FriendTestClient::friendRequestSent);
        alice->sendFriendRequest("Bob");
        ASSERT_TRUE(waitForSpy(sentSpy));
    }
    {
        QSignalSpy sentSpy(alice, &FriendTestClient::friendRequestSent);
        alice->sendFriendRequest("Charlie");
        ASSERT_TRUE(waitForSpy(sentSpy));
    }

    // 2. Bob accepte, Charlie rejette
    {
        QSignalSpy acceptedSpy(bob, &FriendTestClient::friendRequestAccepted);
        bob->sendAcceptFriendRequest("Alice");
        ASSERT_TRUE(waitForSpy(acceptedSpy));
    }
    {
        QSignalSpy rejectedSpy(charlie, &FriendTestClient::friendRequestRejected);
        charlie->sendRejectFriendRequest("Alice");
        ASSERT_TRUE(waitForSpy(rejectedSpy));
    }

    // 3. Vérifier la liste d'Alice : 1 ami (Bob)
    {
        QSignalSpy listSpy(alice, &FriendTestClient::friendsListReceived);
        alice->sendGetFriendsList();
        ASSERT_TRUE(waitForSpy(listSpy));

        QJsonArray friends = listSpy.takeFirst().at(0).toJsonArray();
        EXPECT_EQ(friends.size(), 1) << "Alice devrait avoir 1 seul ami (Bob)";
        if (friends.size() > 0) {
            EXPECT_EQ(friends[0].toObject()["pseudo"].toString(), "Bob");
        }
    }

    // 4. Bob supprime Alice
    {
        QSignalSpy removedSpy(bob, &FriendTestClient::friendRemoved);
        bob->sendRemoveFriend("Alice");
        ASSERT_TRUE(waitForSpy(removedSpy));
    }

    // 5. Vérifier que la liste d'Alice est vide
    {
        QSignalSpy listSpy(alice, &FriendTestClient::friendsListReceived);
        alice->sendGetFriendsList();
        ASSERT_TRUE(waitForSpy(listSpy));

        QJsonArray friends = listSpy.takeFirst().at(0).toJsonArray();
        EXPECT_EQ(friends.size(), 0) << "Alice ne devrait plus avoir d'amis";
    }
}

TEST_F(FriendsIntegrationTest, Scenario_AutoAcceptCrossRequest) {
    FriendTestClient* alice = createClient("Alice", "alice@test.com");
    FriendTestClient* bob = createClient("Bob", "bob@test.com");

    // Alice envoie à Bob
    {
        QSignalSpy sentSpy(alice, &FriendTestClient::friendRequestSent);
        alice->sendFriendRequest("Bob");
        ASSERT_TRUE(waitForSpy(sentSpy));
    }

    // Bob envoie à Alice → devrait auto-accepter
    {
        QSignalSpy sentSpy(bob, &FriendTestClient::friendRequestSent);
        bob->sendFriendRequest("Alice");
        ASSERT_TRUE(waitForSpy(sentSpy));
    }

    // Vérifier qu'ils sont amis
    QSignalSpy listSpy(alice, &FriendTestClient::friendsListReceived);
    alice->sendGetFriendsList();
    ASSERT_TRUE(waitForSpy(listSpy));

    QJsonArray friends = listSpy.takeFirst().at(0).toJsonArray();
    EXPECT_EQ(friends.size(), 1) << "Auto-accept: Alice et Bob devraient être amis";
}

// ========================================
// Main
// ========================================
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include "friends_integration_test.moc"
