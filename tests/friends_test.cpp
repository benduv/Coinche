#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QJsonArray>
#include <QJsonObject>
#include "../server/DatabaseManager.h"

// Variable globale pour QCoreApplication (nécessaire pour Qt SQL)
static int argc = 1;
static char* argv[] = {(char*)"test_friends", nullptr};
static QCoreApplication* app = nullptr;

// ========================================
// Test Fixture pour le système d'amis
// ========================================
class FriendsTest : public ::testing::Test {
protected:
    DatabaseManager* dbManager;
    QString testDbPath;
    static int testCounter;

    static void SetUpTestSuite() {
        if (!app) {
            app = new QCoreApplication(argc, argv);
        }
    }

    void SetUp() override {
        testDbPath = QDir::temp().filePath(QString("test_friends_%1.db").arg(++testCounter));
        QFile::remove(testDbPath);

        dbManager = new DatabaseManager();
        ASSERT_TRUE(dbManager->initialize(testDbPath));

        // Créer des comptes de test
        QString errorMsg;
        ASSERT_TRUE(dbManager->createAccount("Alice", "alice@test.com", "password", "avataaars1.svg", errorMsg));
        ASSERT_TRUE(dbManager->createAccount("Bob", "bob@test.com", "password", "avataaars2.svg", errorMsg));
        ASSERT_TRUE(dbManager->createAccount("Charlie", "charlie@test.com", "password", "avataaars3.svg", errorMsg));
    }

    void TearDown() override {
        delete dbManager;
        dbManager = nullptr;

        QString connectionName = "coinche_connection";
        if (QSqlDatabase::contains(connectionName)) {
            QSqlDatabase::removeDatabase(connectionName);
        }

        QFile::remove(testDbPath);
    }
};

int FriendsTest::testCounter = 0;

// ========================================
// Tests pour sendFriendRequest
// ========================================

TEST_F(FriendsTest, SendFriendRequest_Success) {
    QString errorMsg;
    bool result = dbManager->sendFriendRequest("Alice", "Bob", errorMsg);

    EXPECT_TRUE(result) << "Envoi de demande d'ami devrait réussir. Erreur: " << errorMsg.toStdString();
}

TEST_F(FriendsTest, SendFriendRequest_ToSelf) {
    QString errorMsg;
    bool result = dbManager->sendFriendRequest("Alice", "Alice", errorMsg);

    EXPECT_FALSE(result) << "On ne peut pas s'ajouter soi-même";
    EXPECT_FALSE(errorMsg.isEmpty());
}

TEST_F(FriendsTest, SendFriendRequest_EmptyPseudo) {
    QString errorMsg;

    EXPECT_FALSE(dbManager->sendFriendRequest("", "Bob", errorMsg));
    EXPECT_FALSE(dbManager->sendFriendRequest("Alice", "", errorMsg));
}

TEST_F(FriendsTest, SendFriendRequest_NonExistentTarget) {
    QString errorMsg;
    bool result = dbManager->sendFriendRequest("Alice", "NonExistent", errorMsg);

    EXPECT_FALSE(result) << "Demande vers un joueur inexistant devrait échouer";
    EXPECT_FALSE(errorMsg.isEmpty());
}

TEST_F(FriendsTest, SendFriendRequest_Duplicate) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);

    bool result = dbManager->sendFriendRequest("Alice", "Bob", errorMsg);

    EXPECT_FALSE(result) << "Double demande devrait échouer";
}

TEST_F(FriendsTest, SendFriendRequest_AlreadyFriends) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);
    dbManager->acceptFriendRequest("Alice", "Bob", errorMsg);

    bool result = dbManager->sendFriendRequest("Alice", "Bob", errorMsg);

    EXPECT_FALSE(result) << "Demande quand déjà amis devrait échouer";
}

TEST_F(FriendsTest, SendFriendRequest_AutoAcceptInverse) {
    QString errorMsg;

    // Alice envoie une demande à Bob
    ASSERT_TRUE(dbManager->sendFriendRequest("Alice", "Bob", errorMsg));

    // Bob envoie une demande à Alice → auto-accept
    bool result = dbManager->sendFriendRequest("Bob", "Alice", errorMsg);
    EXPECT_TRUE(result) << "Demande inverse devrait auto-accepter. Erreur: " << errorMsg.toStdString();

    // Vérifier qu'ils sont amis
    QJsonArray aliceFriends = dbManager->getFriendsList("Alice");
    EXPECT_EQ(aliceFriends.size(), 1);
    EXPECT_EQ(aliceFriends[0].toObject()["pseudo"].toString(), "Bob");

    QJsonArray bobFriends = dbManager->getFriendsList("Bob");
    EXPECT_EQ(bobFriends.size(), 1);
    EXPECT_EQ(bobFriends[0].toObject()["pseudo"].toString(), "Alice");
}

// ========================================
// Tests pour getPendingFriendRequests
// ========================================

TEST_F(FriendsTest, GetPendingRequests_Empty) {
    QJsonArray pending = dbManager->getPendingFriendRequests("Alice");
    EXPECT_EQ(pending.size(), 0);
}

TEST_F(FriendsTest, GetPendingRequests_OneRequest) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);

    // Bob devrait voir la demande
    QJsonArray pending = dbManager->getPendingFriendRequests("Bob");
    EXPECT_EQ(pending.size(), 1);
    EXPECT_EQ(pending[0].toObject()["pseudo"].toString(), "Alice");
    EXPECT_EQ(pending[0].toObject()["avatar"].toString(), "avataaars1.svg");

    // Alice ne devrait PAS voir de demande en attente (elle est l'envoyeur)
    QJsonArray alicePending = dbManager->getPendingFriendRequests("Alice");
    EXPECT_EQ(alicePending.size(), 0);
}

TEST_F(FriendsTest, GetPendingRequests_MultipleRequests) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Charlie", errorMsg);
    dbManager->sendFriendRequest("Bob", "Charlie", errorMsg);

    QJsonArray pending = dbManager->getPendingFriendRequests("Charlie");
    EXPECT_EQ(pending.size(), 2);
}

// ========================================
// Tests pour acceptFriendRequest
// ========================================

TEST_F(FriendsTest, AcceptFriendRequest_Success) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);

    bool result = dbManager->acceptFriendRequest("Alice", "Bob", errorMsg);
    EXPECT_TRUE(result) << "Acceptation devrait réussir. Erreur: " << errorMsg.toStdString();

    // Vérifier que la demande n'est plus en attente
    QJsonArray pending = dbManager->getPendingFriendRequests("Bob");
    EXPECT_EQ(pending.size(), 0);

    // Vérifier qu'ils sont amis
    QJsonArray aliceFriends = dbManager->getFriendsList("Alice");
    EXPECT_EQ(aliceFriends.size(), 1);
}

TEST_F(FriendsTest, AcceptFriendRequest_NonExistent) {
    QString errorMsg;
    bool result = dbManager->acceptFriendRequest("Alice", "Bob", errorMsg);

    EXPECT_FALSE(result) << "Acceptation sans demande devrait échouer";
    EXPECT_FALSE(errorMsg.isEmpty());
}

TEST_F(FriendsTest, AcceptFriendRequest_WrongDirection) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);

    // Bob ne peut pas accepter en tant que requester (les paramètres sont inversés)
    bool result = dbManager->acceptFriendRequest("Bob", "Alice", errorMsg);
    EXPECT_FALSE(result) << "Acceptation dans le mauvais sens devrait échouer";
}

// ========================================
// Tests pour rejectFriendRequest
// ========================================

TEST_F(FriendsTest, RejectFriendRequest_Success) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);

    bool result = dbManager->rejectFriendRequest("Alice", "Bob", errorMsg);
    EXPECT_TRUE(result) << "Rejet devrait réussir. Erreur: " << errorMsg.toStdString();

    // Vérifier que la demande a été supprimée
    QJsonArray pending = dbManager->getPendingFriendRequests("Bob");
    EXPECT_EQ(pending.size(), 0);

    // Vérifier qu'ils ne sont PAS amis
    QJsonArray aliceFriends = dbManager->getFriendsList("Alice");
    EXPECT_EQ(aliceFriends.size(), 0);
}

TEST_F(FriendsTest, RejectFriendRequest_NonExistent) {
    QString errorMsg;
    bool result = dbManager->rejectFriendRequest("Alice", "Bob", errorMsg);

    EXPECT_FALSE(result) << "Rejet sans demande devrait échouer";
}

TEST_F(FriendsTest, RejectFriendRequest_ThenResend) {
    QString errorMsg;

    // Alice envoie, Bob rejette
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);
    dbManager->rejectFriendRequest("Alice", "Bob", errorMsg);

    // Alice peut renvoyer une demande
    bool result = dbManager->sendFriendRequest("Alice", "Bob", errorMsg);
    EXPECT_TRUE(result) << "Renvoi après rejet devrait réussir. Erreur: " << errorMsg.toStdString();
}

// ========================================
// Tests pour getFriendsList
// ========================================

TEST_F(FriendsTest, GetFriendsList_Empty) {
    QJsonArray friends = dbManager->getFriendsList("Alice");
    EXPECT_EQ(friends.size(), 0);
}

TEST_F(FriendsTest, GetFriendsList_OneFriend) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);
    dbManager->acceptFriendRequest("Alice", "Bob", errorMsg);

    QJsonArray aliceFriends = dbManager->getFriendsList("Alice");
    ASSERT_EQ(aliceFriends.size(), 1);
    EXPECT_EQ(aliceFriends[0].toObject()["pseudo"].toString(), "Bob");
    EXPECT_EQ(aliceFriends[0].toObject()["avatar"].toString(), "avataaars2.svg");

    // Vérifier la bidirectionnalité
    QJsonArray bobFriends = dbManager->getFriendsList("Bob");
    ASSERT_EQ(bobFriends.size(), 1);
    EXPECT_EQ(bobFriends[0].toObject()["pseudo"].toString(), "Alice");
    EXPECT_EQ(bobFriends[0].toObject()["avatar"].toString(), "avataaars1.svg");
}

TEST_F(FriendsTest, GetFriendsList_MultipleFriends) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);
    dbManager->acceptFriendRequest("Alice", "Bob", errorMsg);
    dbManager->sendFriendRequest("Alice", "Charlie", errorMsg);
    dbManager->acceptFriendRequest("Alice", "Charlie", errorMsg);

    QJsonArray aliceFriends = dbManager->getFriendsList("Alice");
    EXPECT_EQ(aliceFriends.size(), 2);

    // Bob et Charlie n'ont qu'Alice
    EXPECT_EQ(dbManager->getFriendsList("Bob").size(), 1);
    EXPECT_EQ(dbManager->getFriendsList("Charlie").size(), 1);
}

TEST_F(FriendsTest, GetFriendsList_PendingNotIncluded) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);

    // La demande est en attente, pas encore acceptée
    QJsonArray aliceFriends = dbManager->getFriendsList("Alice");
    EXPECT_EQ(aliceFriends.size(), 0) << "Les demandes pending ne doivent pas apparaître dans la liste d'amis";

    QJsonArray bobFriends = dbManager->getFriendsList("Bob");
    EXPECT_EQ(bobFriends.size(), 0);
}

// ========================================
// Tests pour removeFriend
// ========================================

TEST_F(FriendsTest, RemoveFriend_Success) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);
    dbManager->acceptFriendRequest("Alice", "Bob", errorMsg);

    // Vérifier qu'ils sont amis
    ASSERT_EQ(dbManager->getFriendsList("Alice").size(), 1);

    bool result = dbManager->removeFriend("Alice", "Bob", errorMsg);
    EXPECT_TRUE(result) << "Suppression d'ami devrait réussir. Erreur: " << errorMsg.toStdString();

    // Vérifier que la relation est supprimée des deux côtés
    EXPECT_EQ(dbManager->getFriendsList("Alice").size(), 0);
    EXPECT_EQ(dbManager->getFriendsList("Bob").size(), 0);
}

TEST_F(FriendsTest, RemoveFriend_ReverseDirection) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);
    dbManager->acceptFriendRequest("Alice", "Bob", errorMsg);

    // Bob supprime Alice (sens inverse de la demande initiale)
    bool result = dbManager->removeFriend("Bob", "Alice", errorMsg);
    EXPECT_TRUE(result) << "Suppression dans l'autre sens devrait fonctionner. Erreur: " << errorMsg.toStdString();

    EXPECT_EQ(dbManager->getFriendsList("Alice").size(), 0);
    EXPECT_EQ(dbManager->getFriendsList("Bob").size(), 0);
}

TEST_F(FriendsTest, RemoveFriend_NotFriends) {
    QString errorMsg;
    bool result = dbManager->removeFriend("Alice", "Bob", errorMsg);

    EXPECT_FALSE(result) << "Suppression sans amitié devrait échouer";
    EXPECT_FALSE(errorMsg.isEmpty());
}

TEST_F(FriendsTest, RemoveFriend_ThenResendRequest) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);
    dbManager->acceptFriendRequest("Alice", "Bob", errorMsg);
    dbManager->removeFriend("Alice", "Bob", errorMsg);

    // On peut renvoyer une demande après suppression
    bool result = dbManager->sendFriendRequest("Alice", "Bob", errorMsg);
    EXPECT_TRUE(result) << "Renvoi après suppression devrait réussir. Erreur: " << errorMsg.toStdString();
}

// ========================================
// Tests pour la cascade deleteAccount
// ========================================

TEST_F(FriendsTest, DeleteAccount_RemovesFriendships) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);
    dbManager->acceptFriendRequest("Alice", "Bob", errorMsg);
    dbManager->sendFriendRequest("Charlie", "Alice", errorMsg);

    // Alice a 1 ami (Bob) et 1 demande en attente (de Charlie)
    ASSERT_EQ(dbManager->getFriendsList("Alice").size(), 1);
    ASSERT_EQ(dbManager->getPendingFriendRequests("Alice").size(), 1);

    // Supprimer le compte d'Alice
    bool result = dbManager->deleteAccount("Alice", errorMsg);
    EXPECT_TRUE(result);

    // Bob ne doit plus avoir Alice comme ami
    EXPECT_EQ(dbManager->getFriendsList("Bob").size(), 0);

    // Charlie ne doit plus avoir de demande en attente vers Alice
    // (getPendingFriendRequests retourne les demandes REÇUES, pas envoyées)
    // Mais la demande de Charlie→Alice doit avoir été supprimée
    QJsonArray charlieFriends = dbManager->getFriendsList("Charlie");
    EXPECT_EQ(charlieFriends.size(), 0);
}

TEST_F(FriendsTest, DeleteAccount_RemovesPendingRequests) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);

    // Supprimer Alice (qui a une demande en attente vers Bob)
    dbManager->deleteAccount("Alice", errorMsg);

    // Bob ne doit plus voir la demande
    QJsonArray pending = dbManager->getPendingFriendRequests("Bob");
    EXPECT_EQ(pending.size(), 0);
}

// ========================================
// Tests pour la cascade updatePseudo
// ========================================

TEST_F(FriendsTest, UpdatePseudo_UpdatesFriendships) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);
    dbManager->acceptFriendRequest("Alice", "Bob", errorMsg);

    // Changer le pseudo d'Alice en "AliceNew"
    bool result = dbManager->updatePseudo("Alice", "AliceNew", errorMsg);
    EXPECT_TRUE(result) << "Changement de pseudo devrait réussir. Erreur: " << errorMsg.toStdString();

    // Bob doit voir "AliceNew" dans sa liste d'amis
    QJsonArray bobFriends = dbManager->getFriendsList("Bob");
    ASSERT_EQ(bobFriends.size(), 1);
    EXPECT_EQ(bobFriends[0].toObject()["pseudo"].toString(), "AliceNew");

    // AliceNew doit voir Bob dans sa liste d'amis
    QJsonArray aliceFriends = dbManager->getFriendsList("AliceNew");
    ASSERT_EQ(aliceFriends.size(), 1);
    EXPECT_EQ(aliceFriends[0].toObject()["pseudo"].toString(), "Bob");
}

TEST_F(FriendsTest, UpdatePseudo_UpdatesPendingRequests) {
    QString errorMsg;
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);

    dbManager->updatePseudo("Alice", "AliceNew", errorMsg);

    // Bob doit voir "AliceNew" dans ses demandes en attente
    QJsonArray pending = dbManager->getPendingFriendRequests("Bob");
    ASSERT_EQ(pending.size(), 1);
    EXPECT_EQ(pending[0].toObject()["pseudo"].toString(), "AliceNew");
}

// ========================================
// Tests de scénarios complets
// ========================================

TEST_F(FriendsTest, Scenario_FullFriendshipLifecycle) {
    QString errorMsg;

    // 1. Alice envoie une demande à Bob
    ASSERT_TRUE(dbManager->sendFriendRequest("Alice", "Bob", errorMsg));
    EXPECT_EQ(dbManager->getPendingFriendRequests("Bob").size(), 1);

    // 2. Bob accepte
    ASSERT_TRUE(dbManager->acceptFriendRequest("Alice", "Bob", errorMsg));
    EXPECT_EQ(dbManager->getFriendsList("Alice").size(), 1);
    EXPECT_EQ(dbManager->getFriendsList("Bob").size(), 1);
    EXPECT_EQ(dbManager->getPendingFriendRequests("Bob").size(), 0);

    // 3. Bob supprime Alice
    ASSERT_TRUE(dbManager->removeFriend("Bob", "Alice", errorMsg));
    EXPECT_EQ(dbManager->getFriendsList("Alice").size(), 0);
    EXPECT_EQ(dbManager->getFriendsList("Bob").size(), 0);

    // 4. Bob peut renvoyer une demande
    ASSERT_TRUE(dbManager->sendFriendRequest("Bob", "Alice", errorMsg));
    EXPECT_EQ(dbManager->getPendingFriendRequests("Alice").size(), 1);

    // 5. Alice accepte
    ASSERT_TRUE(dbManager->acceptFriendRequest("Bob", "Alice", errorMsg));
    EXPECT_EQ(dbManager->getFriendsList("Alice").size(), 1);
    EXPECT_EQ(dbManager->getFriendsList("Bob").size(), 1);
}

TEST_F(FriendsTest, Scenario_MultipleUsers) {
    QString errorMsg;

    // Alice est amie avec Bob et Charlie
    dbManager->sendFriendRequest("Alice", "Bob", errorMsg);
    dbManager->acceptFriendRequest("Alice", "Bob", errorMsg);
    dbManager->sendFriendRequest("Alice", "Charlie", errorMsg);
    dbManager->acceptFriendRequest("Alice", "Charlie", errorMsg);

    // Bob est aussi ami avec Charlie
    dbManager->sendFriendRequest("Bob", "Charlie", errorMsg);
    dbManager->acceptFriendRequest("Bob", "Charlie", errorMsg);

    // Vérifier les compteurs
    EXPECT_EQ(dbManager->getFriendsList("Alice").size(), 2);
    EXPECT_EQ(dbManager->getFriendsList("Bob").size(), 2);
    EXPECT_EQ(dbManager->getFriendsList("Charlie").size(), 2);

    // Supprimer Alice→Bob ne touche pas les autres relations
    dbManager->removeFriend("Alice", "Bob", errorMsg);
    EXPECT_EQ(dbManager->getFriendsList("Alice").size(), 1);   // Charlie
    EXPECT_EQ(dbManager->getFriendsList("Bob").size(), 1);     // Charlie
    EXPECT_EQ(dbManager->getFriendsList("Charlie").size(), 2); // Alice + Bob
}
