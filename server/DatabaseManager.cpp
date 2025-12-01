#include "DatabaseManager.h"
#include <QDateTime>
#include <QRandomGenerator>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::initialize(const QString &dbPath)
{
    // Créer ou ouvrir la base de données SQLite
    m_db = QSqlDatabase::addDatabase("QSQLITE", "coinche_connection");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qCritical() << "Erreur ouverture base de donnees:" << m_db.lastError().text();
        return false;
    }

    qDebug() << "Base de donnees ouverte:" << dbPath;

    // Créer les tables si nécessaire
    if (!createTables()) {
        qCritical() << "Erreur creation des tables";
        return false;
    }

    qDebug() << "Base de donnees initialisee avec succes";
    return true;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_db);

    // Table des utilisateurs
    QString createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            pseudo TEXT UNIQUE NOT NULL,
            email TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            salt TEXT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            last_login TIMESTAMP
        )
    )";

    if (!query.exec(createUsersTable)) {
        qCritical() << "Erreur creation table users:" << query.lastError().text();
        return false;
    }

    qDebug() << "Table 'users' creee/verifiee";

    // Table des statistiques
    QString createStatsTable = R"(
        CREATE TABLE IF NOT EXISTS stats (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            games_played INTEGER DEFAULT 0,
            games_won INTEGER DEFAULT 0,
            coinche_attempts INTEGER DEFAULT 0,
            coinche_success INTEGER DEFAULT 0,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        )
    )";

    if (!query.exec(createStatsTable)) {
        qCritical() << "Erreur creation table stats:" << query.lastError().text();
        return false;
    }

    qDebug() << "Table 'stats' creee/verifiee";
    return true;
}

QString DatabaseManager::generateSalt()
{
    // Générer un salt aléatoire de 32 caractères
    QString salt;
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    for (int i = 0; i < 32; i++) {
        salt += chars[QRandomGenerator::global()->bounded(chars.length())];
    }

    return salt;
}

QString DatabaseManager::hashPassword(const QString &password, const QString &salt)
{
    // Combiner le mot de passe et le salt, puis hasher avec SHA-256
    QString combined = password + salt;
    QByteArray hash = QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

bool DatabaseManager::emailExists(const QString &email)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM users WHERE email = :email");
    query.bindValue(":email", email);

    if (!query.exec()) {
        qWarning() << "Erreur verification email:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

bool DatabaseManager::pseudoExists(const QString &pseudo)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM users WHERE pseudo = :pseudo");
    query.bindValue(":pseudo", pseudo);

    if (!query.exec()) {
        qWarning() << "Erreur verification pseudo:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

bool DatabaseManager::createAccount(const QString &pseudo, const QString &email, const QString &password, QString &errorMsg)
{
    // Validations
    if (pseudo.isEmpty() || email.isEmpty() || password.isEmpty()) {
        errorMsg = "Tous les champs sont obligatoires";
        return false;
    }

    if (pseudo.length() < 3) {
        errorMsg = "Le pseudonyme doit contenir au moins 3 caracteres";
        return false;
    }

    if (password.length() < 6) {
        errorMsg = "Le mot de passe doit contenir au moins 6 caracteres";
        return false;
    }

    if (!email.contains("@") || !email.contains(".")) {
        errorMsg = "Adresse email invalide";
        return false;
    }

    // Vérifier si l'email existe déjà
    if (emailExists(email)) {
        errorMsg = "Cette adresse email est dejà utilisee";
        return false;
    }

    // Vérifier si le pseudo existe déjà
    if (pseudoExists(pseudo)) {
        errorMsg = "Ce pseudonyme est déjà utilise";
        return false;
    }

    // Générer le salt et hasher le mot de passe
    QString salt = generateSalt();
    QString passwordHash = hashPassword(password, salt);

    // Insérer le nouvel utilisateur
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO users (pseudo, email, password_hash, salt) VALUES (:pseudo, :email, :password_hash, :salt)");
    query.bindValue(":pseudo", pseudo);
    query.bindValue(":email", email);
    query.bindValue(":password_hash", passwordHash);
    query.bindValue(":salt", salt);

    if (!query.exec()) {
        errorMsg = "Erreur lors de la création du compte: " + query.lastError().text();
        qCritical() << "Erreur insertion utilisateur:" << query.lastError().text();
        return false;
    }

    // Créer l'entrée de statistiques pour ce nouvel utilisateur
    int userId = query.lastInsertId().toInt();
    QSqlQuery statsQuery(m_db);
    statsQuery.prepare("INSERT INTO stats (user_id) VALUES (:user_id)");
    statsQuery.bindValue(":user_id", userId);

    if (!statsQuery.exec()) {
        qWarning() << "Erreur creation stats pour utilisateur:" << statsQuery.lastError().text();
    }

    qDebug() << "Compte cree avec succès pour:" << pseudo;
    return true;
}

bool DatabaseManager::authenticateUser(const QString &email, const QString &password, QString &pseudo, QString &errorMsg)
{
    if (email.isEmpty() || password.isEmpty()) {
        errorMsg = "Email et mot de passe requis";
        return false;
    }

    // Récupérer le salt et le hash pour cet email
    QSqlQuery query(m_db);
    query.prepare("SELECT pseudo, password_hash, salt FROM users WHERE email = :email");
    query.bindValue(":email", email);

    if (!query.exec()) {
        errorMsg = "Erreur lors de la verification: " + query.lastError().text();
        qCritical() << "Erreur authentification:" << query.lastError().text();
        return false;
    }

    if (!query.next()) {
        errorMsg = "Email ou mot de passe incorrect";
        return false;
    }

    QString storedPseudo = query.value(0).toString();
    QString storedHash = query.value(1).toString();
    QString salt = query.value(2).toString();

    // Hasher le mot de passe fourni avec le salt
    QString providedHash = hashPassword(password, salt);

    // Comparer les hash
    if (providedHash != storedHash) {
        errorMsg = "Email ou mot de passe incorrect";
        return false;
    }

    // Mettre à jour la date de dernière connexion
    QSqlQuery updateQuery(m_db);
    updateQuery.prepare("UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE email = :email");
    updateQuery.bindValue(":email", email);
    updateQuery.exec();

    pseudo = storedPseudo;
    qDebug() << "Authentification reussie pour:" << pseudo;
    return true;
}

int DatabaseManager::getUserIdByPseudo(const QString &pseudo)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id FROM users WHERE pseudo = :pseudo");
    query.bindValue(":pseudo", pseudo);

    if (!query.exec()) {
        qWarning() << "Erreur recuperation user_id:" << query.lastError().text();
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}

bool DatabaseManager::updateGameStats(const QString &pseudo, bool won)
{
    int userId = getUserIdByPseudo(pseudo);
    if (userId == -1) {
        qWarning() << "Utilisateur non trouve pour mise a jour stats:" << pseudo;
        return false;
    }

    QSqlQuery query(m_db);
    if (won) {
        query.prepare("UPDATE stats SET games_played = games_played + 1, games_won = games_won + 1 WHERE user_id = :user_id");
    } else {
        query.prepare("UPDATE stats SET games_played = games_played + 1 WHERE user_id = :user_id");
    }
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qCritical() << "Erreur mise a jour stats de jeu:" << query.lastError().text();
        return false;
    }

    qDebug() << "Stats de jeu mises a jour pour:" << pseudo << "Won:" << won;
    return true;
}

bool DatabaseManager::updateCoincheStats(const QString &pseudo, bool attempt, bool success)
{
    int userId = getUserIdByPseudo(pseudo);
    if (userId == -1) {
        qWarning() << "Utilisateur non trouve pour mise a jour coinche stats:" << pseudo;
        return false;
    }

    QSqlQuery query(m_db);
    if (attempt && success) {
        query.prepare("UPDATE stats SET coinche_attempts = coinche_attempts + 1, coinche_success = coinche_success + 1 WHERE user_id = :user_id");
    } else if (attempt) {
        query.prepare("UPDATE stats SET coinche_attempts = coinche_attempts + 1 WHERE user_id = :user_id");
    } else {
        return true; // Rien à faire
    }
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qCritical() << "Erreur mise a jour coinche stats:" << query.lastError().text();
        return false;
    }

    qDebug() << "Stats de coinche mises a jour pour:" << pseudo << "Attempt:" << attempt << "Success:" << success;
    return true;
}

DatabaseManager::PlayerStats DatabaseManager::getPlayerStats(const QString &pseudo)
{
    PlayerStats stats = {0, 0, 0.0, 0, 0};

    int userId = getUserIdByPseudo(pseudo);
    if (userId == -1) {
        qWarning() << "Utilisateur non trouve pour recuperation stats:" << pseudo;
        return stats;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT games_played, games_won, coinche_attempts, coinche_success FROM stats WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qCritical() << "Erreur recuperation stats:" << query.lastError().text();
        return stats;
    }

    if (query.next()) {
        stats.gamesPlayed = query.value(0).toInt();
        stats.gamesWon = query.value(1).toInt();
        stats.coincheAttempts = query.value(2).toInt();
        stats.coincheSuccess = query.value(3).toInt();

        if (stats.gamesPlayed > 0) {
            stats.winRatio = (double)stats.gamesWon / (double)stats.gamesPlayed;
        }
    }

    return stats;
}
