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
    bool usingTempPassword = false;
    bool isAnonymous = false;

    // Créer un compte
    dbManager->createAccount("authUser", "auth@test.com", "myPassword", "avatar1.svg", errorMsg);

    // Authentifier
    bool result = dbManager->authenticateUser("auth@test.com", "myPassword", pseudo, avatar, errorMsg, usingTempPassword, isAnonymous);

    EXPECT_TRUE(result) << "Authentification devrait réussir. Erreur: " << errorMsg.toStdString();
    EXPECT_EQ(pseudo, "authUser");
    EXPECT_EQ(avatar, "avatar1.svg");
    EXPECT_FALSE(usingTempPassword) << "Should not be using temp password for normal login";
    EXPECT_FALSE(isAnonymous) << "Should not be anonymous by default";
}

TEST_F(DatabaseManagerTest, AuthenticateUser_WrongPassword) {
    QString errorMsg;
    QString pseudo, avatar;
    bool usingTempPassword = false;
    bool isAnonymous = false;

    // Créer un compte
    dbManager->createAccount("authUser2", "auth2@test.com", "correctPassword", "avatar1.svg", errorMsg);

    // Essayer de s'authentifier avec un mauvais mot de passe
    bool result = dbManager->authenticateUser("auth2@test.com", "wrongPassword", pseudo, avatar, errorMsg, usingTempPassword, isAnonymous);

    EXPECT_FALSE(result) << "Authentification avec mauvais mot de passe devrait échouer";
}

TEST_F(DatabaseManagerTest, AuthenticateUser_NonExistentEmail) {
    QString errorMsg;
    QString pseudo, avatar;
    bool usingTempPassword = false;
    bool isAnonymous = false;

    bool result = dbManager->authenticateUser("nonexistent@test.com", "password", pseudo, avatar, errorMsg, usingTempPassword, isAnonymous);

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

// ========================================
// Tests pour les statistiques de joueur manquantes
// ========================================

TEST_F(DatabaseManagerTest, CancelDefeat_Success) {
    QString errorMsg;
    dbManager->createAccount("defeatUser", "defeat@test.com", "password", "avatar.svg", errorMsg);

    // Simuler des défaites
    dbManager->updateGameStats("defeatUser", false);
    dbManager->updateGameStats("defeatUser", false);
    dbManager->updateGameStats("defeatUser", true);

    DatabaseManager::PlayerStats statsBefore = dbManager->getPlayerStats("defeatUser");
    EXPECT_EQ(statsBefore.gamesPlayed, 3);
    EXPECT_EQ(statsBefore.gamesWon, 1);

    // Annuler une défaite
    dbManager->cancelDefeat("defeatUser");

    DatabaseManager::PlayerStats statsAfter = dbManager->getPlayerStats("defeatUser");
    EXPECT_EQ(statsAfter.gamesPlayed, 2) << "Une partie devrait être annulée";
    EXPECT_EQ(statsAfter.gamesWon, 1) << "Les victoires ne devraient pas changer";
}

TEST_F(DatabaseManagerTest, UpdateCapotAnnonceTente_Success) {
    QString errorMsg;
    dbManager->createAccount("capotTenteUser", "capottente@test.com", "password", "avatar.svg", errorMsg);

    // Tenter plusieurs capots annoncés
    dbManager->updateCapotAnnonceTente("capotTenteUser");
    dbManager->updateCapotAnnonceTente("capotTenteUser");
    dbManager->updateCapotAnnonceTente("capotTenteUser");

    DatabaseManager::PlayerStats stats = dbManager->getPlayerStats("capotTenteUser");
    EXPECT_EQ(stats.capotAnnoncesTentes, 3) << "3 tentatives de capot annoncé";
}

TEST_F(DatabaseManagerTest, UpdateAnnonceCoinchee_Won) {
    QString errorMsg;
    dbManager->createAccount("coincheeUser", "coinchee@test.com", "password", "avatar.svg", errorMsg);

    // Annonces coinchées gagnées et perdues
    dbManager->updateAnnonceCoinchee("coincheeUser", true);
    dbManager->updateAnnonceCoinchee("coincheeUser", true);
    dbManager->updateAnnonceCoinchee("coincheeUser", false);

    DatabaseManager::PlayerStats stats = dbManager->getPlayerStats("coincheeUser");
    EXPECT_EQ(stats.annoncesCoinchees, 3) << "3 annonces coinchées au total";
    EXPECT_EQ(stats.annoncesCoincheesgagnees, 2) << "2 annonces coinchées gagnées";
}

TEST_F(DatabaseManagerTest, UpdateSurcoincheStats_Success) {
    QString errorMsg;
    dbManager->createAccount("surcoincheUser", "surcoinche@test.com", "password", "avatar.svg", errorMsg);

    // Surcoincherries réussies et échouées
    dbManager->updateSurcoincheStats("surcoincheUser", true, true);   // Tentative réussie
    dbManager->updateSurcoincheStats("surcoincheUser", true, false);  // Tentative échouée
    dbManager->updateSurcoincheStats("surcoincheUser", true, true);   // Tentative réussie

    DatabaseManager::PlayerStats stats = dbManager->getPlayerStats("surcoincheUser");
    EXPECT_EQ(stats.surcoincheAttempts, 3) << "3 tentatives de surcoinche";
    EXPECT_EQ(stats.surcoincheSuccess, 2) << "2 surcoincherries réussies";
}

TEST_F(DatabaseManagerTest, UpdateAnnonceSurcoinchee_Won) {
    QString errorMsg;
    dbManager->createAccount("surcoincheAnnonceUser", "surcoincheannonce@test.com", "password", "avatar.svg", errorMsg);

    // Annonces surcoinchées gagnées et perdues
    dbManager->updateAnnonceSurcoinchee("surcoincheAnnonceUser", true);
    dbManager->updateAnnonceSurcoinchee("surcoincheAnnonceUser", false);
    dbManager->updateAnnonceSurcoinchee("surcoincheAnnonceUser", true);
    dbManager->updateAnnonceSurcoinchee("surcoincheAnnonceUser", true);

    DatabaseManager::PlayerStats stats = dbManager->getPlayerStats("surcoincheAnnonceUser");
    EXPECT_EQ(stats.annoncesSurcoinchees, 4) << "4 annonces surcoinchées au total";
    EXPECT_EQ(stats.annoncesSurcoincheesGagnees, 3) << "3 annonces surcoinchées gagnées";
}

// ========================================
// Tests pour les statistiques quotidiennes
// ========================================

TEST_F(DatabaseManagerTest, RecordLogin_Success) {
    // Enregistrer plusieurs logins
    dbManager->recordLogin("user1");
    dbManager->recordLogin("user2");
    dbManager->recordLogin("user1"); // Même utilisateur se reconnecte

    // Vérifier via getDailyStats
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    DatabaseManager::DailyStats stats = dbManager->getDailyStats(today);

    EXPECT_GE(stats.logins, 2) << "Au moins 2 logins uniques devraient être enregistrés";
}

TEST_F(DatabaseManagerTest, RecordGameRoomCreated_Success) {
    // Créer plusieurs parties
    dbManager->recordGameRoomCreated();
    dbManager->recordGameRoomCreated();
    dbManager->recordGameRoomCreated();

    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    DatabaseManager::DailyStats stats = dbManager->getDailyStats(today);

    EXPECT_GE(stats.gameRoomsCreated, 3) << "3 parties devraient être enregistrées";
}

TEST_F(DatabaseManagerTest, RecordNewAccount_Success) {
    // Enregistrer de nouveaux comptes
    dbManager->recordNewAccount();
    dbManager->recordNewAccount();

    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    DatabaseManager::DailyStats stats = dbManager->getDailyStats(today);

    EXPECT_GE(stats.newAccounts, 2) << "2 nouveaux comptes devraient être enregistrés";
}

TEST_F(DatabaseManagerTest, RecordPlayerQuit_Success) {
    // Enregistrer des abandons
    dbManager->recordPlayerQuit();
    dbManager->recordPlayerQuit();
    dbManager->recordPlayerQuit();

    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    DatabaseManager::DailyStats stats = dbManager->getDailyStats(today);

    EXPECT_GE(stats.playerQuits, 3) << "3 abandons devraient être enregistrés";
}

TEST_F(DatabaseManagerTest, GetDailyStats_Today) {
    // Enregistrer diverses statistiques pour aujourd'hui
    dbManager->recordLogin("testUser");
    dbManager->recordGameRoomCreated();
    dbManager->recordNewAccount();
    dbManager->recordPlayerQuit();

    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    DatabaseManager::DailyStats stats = dbManager->getDailyStats(today);

    EXPECT_EQ(stats.date, today);
    EXPECT_GE(stats.logins, 1);
    EXPECT_GE(stats.gameRoomsCreated, 1);
    EXPECT_GE(stats.newAccounts, 1);
    EXPECT_GE(stats.playerQuits, 1);
}

TEST_F(DatabaseManagerTest, GetDailyStats_Default) {
    // Sans paramètre, devrait retourner les stats d'aujourd'hui
    dbManager->recordLogin("defaultUser");

    DatabaseManager::DailyStats stats = dbManager->getDailyStats();
    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    EXPECT_EQ(stats.date, today);
}

TEST_F(DatabaseManagerTest, GetYesterdayStats_Success) {
    // Note: Ce test vérifie juste que la méthode ne crash pas
    // Les stats d'hier peuvent être vides en test
    DatabaseManager::DailyStats yesterdayStats = dbManager->getYesterdayStats();

    QString yesterday = QDate::currentDate().addDays(-1).toString("yyyy-MM-dd");
    EXPECT_EQ(yesterdayStats.date, yesterday);
}

TEST_F(DatabaseManagerTest, RecordSessionStartEnd_Success) {
    QString errorMsg;
    dbManager->createAccount("sessionUser", "session@test.com", "password", "avatar.svg", errorMsg);

    // Démarrer une session
    dbManager->recordSessionStart("sessionUser");

    // Simuler un délai (en réalité instantané en test)
    dbManager->recordSessionEnd("sessionUser");

    // Vérifier qu'aucune erreur ne se produit
    // Les temps de session sont calculés dans getRetentionStats
    SUCCEED() << "Session start/end enregistrée sans erreur";
}

TEST_F(DatabaseManagerTest, RecordCrash_Success) {
    // Enregistrer plusieurs crashes
    dbManager->recordCrash();
    dbManager->recordCrash();

    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    DatabaseManager::DailyStats stats = dbManager->getDailyStats(today);

    EXPECT_GE(stats.crashes, 2) << "2 crashes devraient être enregistrés";
}

// ========================================
// Tests pour les analytics
// ========================================

TEST_F(DatabaseManagerTest, GetRetentionStats_Success) {
    QString errorMsg;

    // Créer quelques comptes
    dbManager->createAccount("retentionUser1", "ret1@test.com", "password", "avatar.svg", errorMsg);
    dbManager->createAccount("retentionUser2", "ret2@test.com", "password", "avatar.svg", errorMsg);

    // Simuler des logins
    dbManager->recordLogin("retentionUser1");
    dbManager->recordLogin("retentionUser2");

    DatabaseManager::RetentionStats stats = dbManager->getRetentionStats();

    // Vérifier que les stats sont retournées (valeurs exactes dépendent de l'état de la DB)
    EXPECT_GE(stats.d1Retention, 0.0);
    EXPECT_GE(stats.d7Retention, 0.0);
    EXPECT_GE(stats.d30Retention, 0.0);
}

TEST_F(DatabaseManagerTest, GetTrendStats_7Days) {
    // Enregistrer des statistiques pour créer une tendance
    dbManager->recordLogin("trendUser");
    dbManager->recordGameRoomCreated();

    QList<DatabaseManager::DailyStats> trends = dbManager->getTrendStats(7);

    EXPECT_LE(trends.size(), 7) << "Ne devrait pas retourner plus de 7 jours";
    EXPECT_GE(trends.size(), 1) << "Devrait retourner au moins 1 jour (aujourd'hui)";

    // Vérifier que les dates sont dans l'ordre si on a plusieurs résultats
    if (trends.size() > 1) {
        for (int i = 0; i < trends.size() - 1; i++) {
            QDate date1 = QDate::fromString(trends[i].date, "yyyy-MM-dd");
            QDate date2 = QDate::fromString(trends[i + 1].date, "yyyy-MM-dd");
            EXPECT_TRUE(date1 < date2) << "Les dates devraient être en ordre croissant";
        }
    }
}

TEST_F(DatabaseManagerTest, GetTrendStats_30Days) {
    // Enregistrer quelques stats
    dbManager->recordLogin("trendUser30");
    dbManager->recordGameRoomCreated();

    QList<DatabaseManager::DailyStats> trends = dbManager->getTrendStats(30);

    EXPECT_LE(trends.size(), 30) << "Ne devrait pas retourner plus de 30 jours";
    EXPECT_GE(trends.size(), 0) << "Devrait retourner au moins 0 jour (base vide possible)";
}

// ========================================
// Tests pour le mot de passe temporaire
// ========================================

TEST_F(DatabaseManagerTest, IsUsingTempPassword_True) {
    QString errorMsg;
    QString tempPassword;

    dbManager->createAccount("tempPwdUser", "temppwd@test.com", "password", "avatar.svg", errorMsg);
    dbManager->setTempPassword("temppwd@test.com", tempPassword, errorMsg);

    bool isUsingTemp = dbManager->isUsingTempPassword("temppwd@test.com");
    EXPECT_TRUE(isUsingTemp) << "L'utilisateur devrait avoir un mot de passe temporaire";
}

TEST_F(DatabaseManagerTest, IsUsingTempPassword_False) {
    QString errorMsg;

    dbManager->createAccount("normalPwdUser", "normalpwd@test.com", "password", "avatar.svg", errorMsg);

    bool isUsingTemp = dbManager->isUsingTempPassword("normalpwd@test.com");
    EXPECT_FALSE(isUsingTemp) << "L'utilisateur ne devrait pas avoir de mot de passe temporaire";
}

TEST_F(DatabaseManagerTest, IsUsingTempPassword_AfterUpdate) {
    QString errorMsg;
    QString tempPassword;

    dbManager->createAccount("updatePwdUser", "updatepwd@test.com", "password", "avatar.svg", errorMsg);
    dbManager->setTempPassword("updatepwd@test.com", tempPassword, errorMsg);

    EXPECT_TRUE(dbManager->isUsingTempPassword("updatepwd@test.com"));

    // Mettre à jour avec un mot de passe permanent
    dbManager->updatePassword("updatepwd@test.com", "newPassword", errorMsg);

    EXPECT_FALSE(dbManager->isUsingTempPassword("updatepwd@test.com"))
        << "Le mot de passe temporaire devrait être effacé après mise à jour";
}
