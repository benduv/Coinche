#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QCryptographicHash>
#include <QDebug>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    // Initialiser la base de données
    bool initialize(const QString &dbPath = "coinche.db");

    // Créer un compte utilisateur
    bool createAccount(const QString &pseudo, const QString &email, const QString &password, const QString &avatar, QString &errorMsg);

    // Vérifier les identifiants de connexion
    bool authenticateUser(const QString &email, const QString &password, QString &pseudo, QString &avatar, QString &errorMsg, bool &usingTempPassword, bool &isAnonymous);

    // Vérifier si un email existe déjà
    bool emailExists(const QString &email);

    // Vérifier si un pseudo existe déjà
    bool pseudoExists(const QString &pseudo);

    // Récupérer l'ID d'un utilisateur par son pseudo
    int getUserIdByPseudo(const QString &pseudo);

    // Mettre à jour les statistiques de jeu après une partie
    bool updateGameStats(const QString &pseudo, bool won);

    // Annuler une défaite (décrémenter gamesPlayed)
    bool cancelDefeat(const QString &pseudo);

    // Mettre à jour les statistiques de coinche
    bool updateCoincheStats(const QString &pseudo, bool attempt, bool success);

    // Récupérer les statistiques d'un joueur
    struct PlayerStats {
        int gamesPlayed;
        int gamesWon;
        double winRatio;
        int coincheAttempts;
        int coincheSuccess;
        int capotRealises;
        int capotAnnoncesRealises;
        int capotAnnoncesTentes;
        int generaleAttempts;
        int generaleSuccess;
        int annoncesCoinchees;
        int annoncesCoincheesgagnees;
        int surcoincheAttempts;
        int surcoincheSuccess;
        int annoncesSurcoinchees;
        int annoncesSurcoincheesGagnees;
        int maxWinStreak;
    };
    PlayerStats getPlayerStats(const QString &pseudo);

    // Mettre à jour les statistiques de capot
    bool updateCapotStats(const QString &pseudo, bool annonceCapot);

    // Mettre à jour les statistiques de capot annoncé (tenté)
    bool updateCapotAnnonceTente(const QString &pseudo);

    // Mettre à jour les statistiques de générale
    bool updateGeneraleStats(const QString &pseudo, bool success);

    // Mettre à jour les statistiques d'annonces coinchées
    bool updateAnnonceCoinchee(const QString &pseudo, bool won);

    // Mettre à jour les statistiques de surcoinche
    bool updateSurcoincheStats(const QString &pseudo, bool attempt, bool success);

    // Mettre à jour les statistiques de surcoinche subies
    bool updateAnnonceSurcoinchee(const QString &pseudo, bool won);

    // Supprimer un compte utilisateur et toutes ses données
    bool deleteAccount(const QString &pseudo, QString &errorMsg);

    // Tracking des statistiques quotidiennes
    bool recordLogin(const QString &pseudo);
    bool recordGameRoomCreated();
    bool recordNewAccount();
    bool recordPlayerQuit();

    // Récupérer les statistiques du jour
    struct DailyStats {
        QString date;
        int logins;
        int gameRoomsCreated;
        int newAccounts;
        int playerQuits;
        int crashes;
        int totalSessionTime;
        int sessionCount;
    };
    DailyStats getDailyStats(const QString &date = QString());

    // Récupérer les stats de la veille (pour comparaison)
    DailyStats getYesterdayStats();

    // Tracking du temps de session (lightweight: pas de timers)
    bool recordSessionStart(const QString &pseudo);
    bool recordSessionEnd(const QString &pseudo);

    // Tracking des crashes
    bool recordCrash();

    // Calcul des taux de rétention
    struct RetentionStats {
        double d1Retention;  // % de joueurs revenus J+1
        double d7Retention;  // % de joueurs revenus J+7
        double d30Retention; // % de joueurs revenus J+30
    };
    RetentionStats getRetentionStats();

    // Obtenir les tendances sur N jours
    QList<DailyStats> getTrendStats(int days);

    // Password recovery methods
    QString generateTempPassword();
    bool setTempPassword(const QString &email, QString &tempPassword, QString &errorMsg);
    bool isUsingTempPassword(const QString &email);
    bool updatePassword(const QString &email, const QString &newPassword, QString &errorMsg);
    bool updatePseudo(const QString &currentPseudo, const QString &newPseudo, QString &errorMsg);
    bool updateEmail(const QString &pseudo, const QString &newEmail, QString &errorMsg);
    bool setAnonymous(const QString &pseudo, bool anonymous, QString &errorMsg);

private:
    QSqlDatabase m_db;

    // Créer les tables si elles n'existent pas
    bool createTables();

    // Hasher un mot de passe avec SHA-256
    QString hashPassword(const QString &password, const QString &salt);

    // Générer un salt aléatoire
    QString generateSalt();
};

#endif // DATABASEMANAGER_H
