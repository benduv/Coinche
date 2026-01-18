#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include "../server/DatabaseManager.h"

// Variable globale pour QCoreApplication (nécessaire pour Qt SQL)
static int argc = 1;
static char* argv[] = {(char*)"test_databasemanager", nullptr};
static QCoreApplication* app = nullptr;

// ========================================
// Test Fixture pour DatabaseManager
// ========================================
class DatabaseManagerTest : public ::testing::Test {
protected:
    DatabaseManager* dbManager;
    QString testDbPath;
    static int testCounter;

    static void SetUpTestSuite() {
        // Créer QCoreApplication une seule fois pour tous les tests
        if (!app) {
            app = new QCoreApplication(argc, argv);
        }
    }

    void SetUp() override {
        // Utiliser un nom unique pour éviter les conflits de connexion
        testDbPath = QDir::temp().filePath(QString("test_coinche_%1.db").arg(++testCounter));

        // Supprimer la base de test si elle existe déjà
        QFile::remove(testDbPath);

        dbManager = new DatabaseManager();

        // Initialiser la base de données
        ASSERT_TRUE(dbManager->initialize(testDbPath));
    }

    void TearDown() override {
        delete dbManager;
        dbManager = nullptr;

        // Nettoyer la connexion de la base de données
        QString connectionName = "coinche_connection";
        if (QSqlDatabase::contains(connectionName)) {
            QSqlDatabase::removeDatabase(connectionName);
        }

        // Nettoyer la base de test
        QFile::remove(testDbPath);
    }
};

int DatabaseManagerTest::testCounter = 0;

// ========================================
// Tests pour la création de compte
// ========================================

TEST_F(DatabaseManagerTest, CreateAccount_Success) {
    QString errorMsg;
    bool result = dbManager->createAccount("testUser", "test@test.com", "password123", "avatar1.svg", errorMsg);

    EXPECT_TRUE(result) << "Création de compte devrait réussir. Erreur: " << errorMsg.toStdString();
    EXPECT_TRUE(errorMsg.isEmpty()) << "Pas d'erreur attendue";
}

TEST_F(DatabaseManagerTest, CreateAccount_DuplicatePseudo) {
    QString errorMsg;

    // Créer un premier compte
    dbManager->createAccount("duplicateUser", "first@test.com", "password123", "avatar1.svg", errorMsg);

    // Essayer de créer un compte avec le même pseudo
    bool result = dbManager->createAccount("duplicateUser", "second@test.com", "password456", "avatar2.svg", errorMsg);

    EXPECT_FALSE(result) << "Création avec pseudo dupliqué devrait échouer";
}

TEST_F(DatabaseManagerTest, CreateAccount_DuplicateEmail) {
    QString errorMsg;

    // Créer un premier compte
    dbManager->createAccount("user1", "duplicate@test.com", "password123", "avatar1.svg", errorMsg);

    // Essayer de créer un compte avec le même email
    bool result = dbManager->createAccount("user2", "duplicate@test.com", "password456", "avatar2.svg", errorMsg);

    EXPECT_FALSE(result) << "Création avec email dupliqué devrait échouer";
}

// ========================================
// Tests pour l'authentification
// ========================================

TEST_F(DatabaseManagerTest, AuthenticateUser_Success) {
    QString errorMsg;
    QString pseudo, avatar;

    // Créer un compte
    dbManager->createAccount("authUser", "auth@test.com", "myPassword", "avatar1.svg", errorMsg);

    // Authentifier
    bool result = dbManager->authenticateUser("auth@test.com", "myPassword", pseudo, avatar, errorMsg);

    EXPECT_TRUE(result) << "Authentification devrait réussir. Erreur: " << errorMsg.toStdString();
    EXPECT_EQ(pseudo, "authUser");
    EXPECT_EQ(avatar, "avatar1.svg");
}

TEST_F(DatabaseManagerTest, AuthenticateUser_WrongPassword) {
    QString errorMsg;
    QString pseudo, avatar;

    // Créer un compte
    dbManager->createAccount("authUser2", "auth2@test.com", "correctPassword", "avatar1.svg", errorMsg);

    // Essayer de s'authentifier avec un mauvais mot de passe
    bool result = dbManager->authenticateUser("auth2@test.com", "wrongPassword", pseudo, avatar, errorMsg);

    EXPECT_FALSE(result) << "Authentification avec mauvais mot de passe devrait échouer";
}

TEST_F(DatabaseManagerTest, AuthenticateUser_NonExistentEmail) {
    QString errorMsg;
    QString pseudo, avatar;

    bool result = dbManager->authenticateUser("nonexistent@test.com", "password", pseudo, avatar, errorMsg);

    EXPECT_FALSE(result) << "Authentification avec email inexistant devrait échouer";
}

// ========================================
// Tests pour les vérifications d'existence
// ========================================

TEST_F(DatabaseManagerTest, EmailExists_True) {
    QString errorMsg;
    dbManager->createAccount("existUser", "exists@test.com", "password", "avatar.svg", errorMsg);

    EXPECT_TRUE(dbManager->emailExists("exists@test.com"));
}

TEST_F(DatabaseManagerTest, EmailExists_False) {
    EXPECT_FALSE(dbManager->emailExists("notexists@test.com"));
}

TEST_F(DatabaseManagerTest, PseudoExists_True) {
    QString errorMsg;
    dbManager->createAccount("existingPseudo", "email@test.com", "password", "avatar.svg", errorMsg);

    EXPECT_TRUE(dbManager->pseudoExists("existingPseudo"));
}

TEST_F(DatabaseManagerTest, PseudoExists_False) {
    EXPECT_FALSE(dbManager->pseudoExists("nonExistingPseudo"));
}

// ========================================
// Tests pour la suppression de compte
// ========================================

TEST_F(DatabaseManagerTest, DeleteAccount_Success) {
    QString errorMsg;

    // Créer un compte
    dbManager->createAccount("toDelete", "delete@test.com", "password", "avatar.svg", errorMsg);

    // Vérifier que le compte existe
    EXPECT_TRUE(dbManager->pseudoExists("toDelete"));

    // Supprimer le compte
    bool result = dbManager->deleteAccount("toDelete", errorMsg);

    EXPECT_TRUE(result) << "Suppression devrait réussir. Erreur: " << errorMsg.toStdString();

    // Vérifier que le compte n'existe plus
    EXPECT_FALSE(dbManager->pseudoExists("toDelete"));
    EXPECT_FALSE(dbManager->emailExists("delete@test.com"));
}

TEST_F(DatabaseManagerTest, DeleteAccount_NonExistent) {
    QString errorMsg;

    bool result = dbManager->deleteAccount("nonExistentUser", errorMsg);

    EXPECT_FALSE(result) << "Suppression d'un compte inexistant devrait échouer";
    EXPECT_FALSE(errorMsg.isEmpty()) << "Un message d'erreur devrait être retourné";
}

TEST_F(DatabaseManagerTest, DeleteAccount_EmptyPseudo) {
    QString errorMsg;

    bool result = dbManager->deleteAccount("", errorMsg);

    EXPECT_FALSE(result) << "Suppression avec pseudo vide devrait échouer";
    EXPECT_FALSE(errorMsg.isEmpty()) << "Un message d'erreur devrait être retourné";
}

TEST_F(DatabaseManagerTest, DeleteAccount_StatsAlsoDeleted) {
    QString errorMsg;

    // Créer un compte
    dbManager->createAccount("statsUser", "stats@test.com", "password", "avatar.svg", errorMsg);

    // Mettre à jour quelques statistiques
    dbManager->updateGameStats("statsUser", true);  // Victoire
    dbManager->updateGameStats("statsUser", false); // Défaite

    // Vérifier que les stats existent
    DatabaseManager::PlayerStats stats = dbManager->getPlayerStats("statsUser");
    EXPECT_EQ(stats.gamesPlayed, 2);
    EXPECT_EQ(stats.gamesWon, 1);

    // Supprimer le compte
    bool result = dbManager->deleteAccount("statsUser", errorMsg);
    EXPECT_TRUE(result);

    // Vérifier que les stats sont aussi supprimées
    stats = dbManager->getPlayerStats("statsUser");
    EXPECT_EQ(stats.gamesPlayed, 0);
    EXPECT_EQ(stats.gamesWon, 0);
}

// ========================================
// Tests pour les statistiques
// ========================================

TEST_F(DatabaseManagerTest, UpdateGameStats_Win) {
    QString errorMsg;
    dbManager->createAccount("winUser", "win@test.com", "password", "avatar.svg", errorMsg);

    dbManager->updateGameStats("winUser", true);

    DatabaseManager::PlayerStats stats = dbManager->getPlayerStats("winUser");
    EXPECT_EQ(stats.gamesPlayed, 1);
    EXPECT_EQ(stats.gamesWon, 1);
}

TEST_F(DatabaseManagerTest, UpdateGameStats_Loss) {
    QString errorMsg;
    dbManager->createAccount("lossUser", "loss@test.com", "password", "avatar.svg", errorMsg);

    dbManager->updateGameStats("lossUser", false);

    DatabaseManager::PlayerStats stats = dbManager->getPlayerStats("lossUser");
    EXPECT_EQ(stats.gamesPlayed, 1);
    EXPECT_EQ(stats.gamesWon, 0);
}

TEST_F(DatabaseManagerTest, UpdateGameStats_MultipleGames) {
    QString errorMsg;
    dbManager->createAccount("multiUser", "multi@test.com", "password", "avatar.svg", errorMsg);

    dbManager->updateGameStats("multiUser", true);  // W
    dbManager->updateGameStats("multiUser", true);  // W
    dbManager->updateGameStats("multiUser", false); // L
    dbManager->updateGameStats("multiUser", true);  // W
    dbManager->updateGameStats("multiUser", false); // L

    DatabaseManager::PlayerStats stats = dbManager->getPlayerStats("multiUser");
    EXPECT_EQ(stats.gamesPlayed, 5);
    EXPECT_EQ(stats.gamesWon, 3);
    EXPECT_DOUBLE_EQ(stats.winRatio, 0.6);
}

TEST_F(DatabaseManagerTest, UpdateCoincheStats_Success) {
    QString errorMsg;
    dbManager->createAccount("coincheUser", "coinche@test.com", "password", "avatar.svg", errorMsg);

    dbManager->updateCoincheStats("coincheUser", true, true);  // Tentative réussie
    dbManager->updateCoincheStats("coincheUser", true, false); // Tentative échouée

    DatabaseManager::PlayerStats stats = dbManager->getPlayerStats("coincheUser");
    EXPECT_EQ(stats.coincheAttempts, 2);
    EXPECT_EQ(stats.coincheSuccess, 1);
}

TEST_F(DatabaseManagerTest, UpdateCapotStats) {
    QString errorMsg;
    dbManager->createAccount("capotUser", "capot@test.com", "password", "avatar.svg", errorMsg);

    dbManager->updateCapotStats("capotUser", false); // Capot non annoncé
    dbManager->updateCapotStats("capotUser", true);  // Capot annoncé

    DatabaseManager::PlayerStats stats = dbManager->getPlayerStats("capotUser");
    EXPECT_EQ(stats.capotRealises, 2);
    EXPECT_EQ(stats.capotAnnoncesRealises, 1);
}

TEST_F(DatabaseManagerTest, UpdateGeneraleStats) {
    QString errorMsg;
    dbManager->createAccount("generaleUser", "generale@test.com", "password", "avatar.svg", errorMsg);

    dbManager->updateGeneraleStats("generaleUser", true);  // Générale réussie
    dbManager->updateGeneraleStats("generaleUser", false); // Générale échouée

    DatabaseManager::PlayerStats stats = dbManager->getPlayerStats("generaleUser");
    EXPECT_EQ(stats.generaleAttempts, 2);
    EXPECT_EQ(stats.generaleSuccess, 1);
}

TEST_F(DatabaseManagerTest, GetUserIdByPseudo_Exists) {
    QString errorMsg;
    dbManager->createAccount("idUser", "id@test.com", "password", "avatar.svg", errorMsg);

    int userId = dbManager->getUserIdByPseudo("idUser");

    EXPECT_GT(userId, 0) << "User ID devrait être positif";
}

TEST_F(DatabaseManagerTest, GetUserIdByPseudo_NotExists) {
    int userId = dbManager->getUserIdByPseudo("nonExistentUser");

    EXPECT_EQ(userId, -1) << "User ID devrait être -1 pour un utilisateur inexistant";
}
