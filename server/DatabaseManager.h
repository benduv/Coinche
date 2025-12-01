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
    bool createAccount(const QString &pseudo, const QString &email, const QString &password, QString &errorMsg);

    // Vérifier les identifiants de connexion
    bool authenticateUser(const QString &email, const QString &password, QString &pseudo, QString &errorMsg);

    // Vérifier si un email existe déjà
    bool emailExists(const QString &email);

    // Vérifier si un pseudo existe déjà
    bool pseudoExists(const QString &pseudo);

    // Récupérer l'ID d'un utilisateur par son pseudo
    int getUserIdByPseudo(const QString &pseudo);

    // Mettre à jour les statistiques de jeu après une partie
    bool updateGameStats(const QString &pseudo, bool won);

    // Mettre à jour les statistiques de coinche
    bool updateCoincheStats(const QString &pseudo, bool attempt, bool success);

    // Récupérer les statistiques d'un joueur
    struct PlayerStats {
        int gamesPlayed;
        int gamesWon;
        double winRatio;
        int coincheAttempts;
        int coincheSuccess;
    };
    PlayerStats getPlayerStats(const QString &pseudo);

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
