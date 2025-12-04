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
            avatar TEXT DEFAULT 'avataaars1.svg',
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
            capot_realises INTEGER DEFAULT 0,
            capot_annonces_realises INTEGER DEFAULT 0,
            capot_annonces_tentes INTEGER DEFAULT 0,
            generale_attempts INTEGER DEFAULT 0,
            generale_success INTEGER DEFAULT 0,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        )
    )";

    if (!query.exec(createStatsTable)) {
        qCritical() << "Erreur creation table stats:" << query.lastError().text();
        return false;
    }

    qDebug() << "Table 'stats' creee/verifiee";

    // Migration: Ajouter la colonne avatar à la table users si elle n'existe pas
    QSqlQuery checkUsersQuery(m_db);
    checkUsersQuery.exec("PRAGMA table_info(users)");

    bool hasAvatar = false;
    while (checkUsersQuery.next()) {
        QString columnName = checkUsersQuery.value(1).toString();
        if (columnName == "avatar") hasAvatar = true;
    }

    if (!hasAvatar) {
        qDebug() << "Ajout de la colonne avatar dans la table users";
        if (!query.exec("ALTER TABLE users ADD COLUMN avatar TEXT DEFAULT 'avataaars1.svg'")) {
            qWarning() << "Erreur ajout colonne avatar:" << query.lastError().text();
        }
    }

    // Ajouter les colonnes si elles n'existent pas déjà (pour migration)
    QSqlQuery checkQuery(m_db);
    checkQuery.exec("PRAGMA table_info(stats)");

    bool hasCapotRealises = false;
    bool hasCapotAnnoncesRealises = false;
    bool hasCapotAnnoncesTentes = false;
    bool hasGeneraleAttempts = false;
    bool hasGeneraleSuccess = false;
    bool hasAnnoncesCoinchees = false;
    bool hasAnnoncesCoincheesgagnees = false;
    bool hasSurcoincheAttempts = false;
    bool hasSurcoincheSuccess = false;

    while (checkQuery.next()) {
        QString columnName = checkQuery.value(1).toString();
        if (columnName == "capot_realises") hasCapotRealises = true;
        if (columnName == "capot_annonces_realises") hasCapotAnnoncesRealises = true;
        if (columnName == "capot_annonces_tentes") hasCapotAnnoncesTentes = true;
        if (columnName == "generale_attempts") hasGeneraleAttempts = true;
        if (columnName == "generale_success") hasGeneraleSuccess = true;
        if (columnName == "annonces_coinchees") hasAnnoncesCoinchees = true;
        if (columnName == "annonces_coinchees_gagnees") hasAnnoncesCoincheesgagnees = true;
        if (columnName == "surcoinche_attempts") hasSurcoincheAttempts = true;
        if (columnName == "surcoinche_success") hasSurcoincheSuccess = true;
    }

    if (!hasCapotRealises) {
        qDebug() << "Ajout de la colonne capot_realises";
        if (!query.exec("ALTER TABLE stats ADD COLUMN capot_realises INTEGER DEFAULT 0")) {
            qWarning() << "Erreur ajout colonne capot_realises:" << query.lastError().text();
        }
    }

    if (!hasCapotAnnoncesRealises) {
        qDebug() << "Ajout de la colonne capot_annonces_realises";
        if (!query.exec("ALTER TABLE stats ADD COLUMN capot_annonces_realises INTEGER DEFAULT 0")) {
            qWarning() << "Erreur ajout colonne capot_annonces_realises:" << query.lastError().text();
        }
    }

    if (!hasCapotAnnoncesTentes) {
        qDebug() << "Ajout de la colonne capot_annonces_tentes";
        if (!query.exec("ALTER TABLE stats ADD COLUMN capot_annonces_tentes INTEGER DEFAULT 0")) {
            qWarning() << "Erreur ajout colonne capot_annonces_tentes:" << query.lastError().text();
        }
    }

    if (!hasGeneraleAttempts) {
        qDebug() << "Ajout de la colonne generale_attempts";
        if (!query.exec("ALTER TABLE stats ADD COLUMN generale_attempts INTEGER DEFAULT 0")) {
            qWarning() << "Erreur ajout colonne generale_attempts:" << query.lastError().text();
        }
    }

    if (!hasGeneraleSuccess) {
        qDebug() << "Ajout de la colonne generale_success";
        if (!query.exec("ALTER TABLE stats ADD COLUMN generale_success INTEGER DEFAULT 0")) {
            qWarning() << "Erreur ajout colonne generale_success:" << query.lastError().text();
        }
    }

    if (!hasAnnoncesCoinchees) {
        qDebug() << "Ajout de la colonne annonces_coinchees";
        if (!query.exec("ALTER TABLE stats ADD COLUMN annonces_coinchees INTEGER DEFAULT 0")) {
            qWarning() << "Erreur ajout colonne annonces_coinchees:" << query.lastError().text();
        }
    }

    if (!hasAnnoncesCoincheesgagnees) {
        qDebug() << "Ajout de la colonne annonces_coinchees_gagnees";
        if (!query.exec("ALTER TABLE stats ADD COLUMN annonces_coinchees_gagnees INTEGER DEFAULT 0")) {
            qWarning() << "Erreur ajout colonne annonces_coinchees_gagnees:" << query.lastError().text();
        }
    }

    if (!hasSurcoincheAttempts) {
        qDebug() << "Ajout de la colonne surcoinche_attempts";
        if (!query.exec("ALTER TABLE stats ADD COLUMN surcoinche_attempts INTEGER DEFAULT 0")) {
            qWarning() << "Erreur ajout colonne surcoinche_attempts:" << query.lastError().text();
        }
    }

    if (!hasSurcoincheSuccess) {
        qDebug() << "Ajout de la colonne surcoinche_success";
        if (!query.exec("ALTER TABLE stats ADD COLUMN surcoinche_success INTEGER DEFAULT 0")) {
            qWarning() << "Erreur ajout colonne surcoinche_success:" << query.lastError().text();
        }
    }

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

bool DatabaseManager::createAccount(const QString &pseudo, const QString &email, const QString &password, const QString &avatar, QString &errorMsg)
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

    // Utiliser un avatar par défaut si non fourni
    QString avatarToUse = avatar.isEmpty() ? "avataaars1.svg" : avatar;

    // Insérer le nouvel utilisateur
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO users (pseudo, email, password_hash, salt, avatar) VALUES (:pseudo, :email, :password_hash, :salt, :avatar)");
    query.bindValue(":pseudo", pseudo);
    query.bindValue(":email", email);
    query.bindValue(":password_hash", passwordHash);
    query.bindValue(":salt", salt);
    query.bindValue(":avatar", avatarToUse);

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

bool DatabaseManager::authenticateUser(const QString &email, const QString &password, QString &pseudo, QString &avatar, QString &errorMsg)
{
    if (email.isEmpty() || password.isEmpty()) {
        errorMsg = "Email et mot de passe requis";
        return false;
    }

    // Récupérer le salt, le hash et l'avatar pour cet email
    QSqlQuery query(m_db);
    query.prepare("SELECT pseudo, password_hash, salt, avatar FROM users WHERE email = :email");
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
    QString storedAvatar = query.value(3).toString();

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
    avatar = storedAvatar;
    qDebug() << "Authentification reussie pour:" << pseudo << "avec avatar:" << avatar;
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
        // Nouvelle tentative réussie
        query.prepare("UPDATE stats SET coinche_attempts = coinche_attempts + 1, coinche_success = coinche_success + 1 WHERE user_id = :user_id");
    } else if (attempt && !success) {
        // Nouvelle tentative échouée
        query.prepare("UPDATE stats SET coinche_attempts = coinche_attempts + 1 WHERE user_id = :user_id");
    } else if (!attempt && success) {
        // Marquer une coinche existante comme réussie (sans incrémenter les tentatives)
        query.prepare("UPDATE stats SET coinche_success = coinche_success + 1 WHERE user_id = :user_id");
    } else {
        // attempt = false, success = false : rien à faire
        return true;
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
    PlayerStats stats = {0, 0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    int userId = getUserIdByPseudo(pseudo);
    if (userId == -1) {
        qWarning() << "Utilisateur non trouve pour recuperation stats:" << pseudo;
        return stats;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT games_played, games_won, coinche_attempts, coinche_success, capot_realises, capot_annonces_realises, capot_annonces_tentes, generale_attempts, generale_success, annonces_coinchees, annonces_coinchees_gagnees, surcoinche_attempts, surcoinche_success FROM stats WHERE user_id = :user_id");
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
        stats.capotRealises = query.value(4).toInt();
        stats.capotAnnoncesRealises = query.value(5).toInt();
        stats.capotAnnoncesTentes = query.value(6).toInt();
        stats.generaleAttempts = query.value(7).toInt();
        stats.generaleSuccess = query.value(8).toInt();
        stats.annoncesCoinchees = query.value(9).toInt();
        stats.annoncesCoincheesgagnees = query.value(10).toInt();
        stats.surcoincheAttempts = query.value(11).toInt();
        stats.surcoincheSuccess = query.value(12).toInt();

        if (stats.gamesPlayed > 0) {
            stats.winRatio = (double)stats.gamesWon / (double)stats.gamesPlayed;
        }
    }

    return stats;
}

bool DatabaseManager::updateCapotStats(const QString &pseudo, bool annonceCapot)
{
    int userId = getUserIdByPseudo(pseudo);
    if (userId == -1) {
        qWarning() << "Utilisateur non trouve pour mise a jour capot stats:" << pseudo;
        return false;
    }

    QSqlQuery query(m_db);
    if (annonceCapot) {
        // Capot annoncé et réalisé
        query.prepare("UPDATE stats SET capot_realises = capot_realises + 1, capot_annonces_realises = capot_annonces_realises + 1 WHERE user_id = :user_id");
    } else {
        // Capot réalisé sans annonce
        query.prepare("UPDATE stats SET capot_realises = capot_realises + 1 WHERE user_id = :user_id");
    }
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qCritical() << "Erreur mise a jour capot stats:" << query.lastError().text();
        return false;
    }

    qDebug() << "Stats de capot mises a jour pour:" << pseudo << "Annonce:" << annonceCapot;
    return true;
}

bool DatabaseManager::updateCapotAnnonceTente(const QString &pseudo)
{
    int userId = getUserIdByPseudo(pseudo);
    if (userId == -1) {
        qWarning() << "Utilisateur non trouve pour mise a jour capot annonce tente:" << pseudo;
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE stats SET capot_annonces_tentes = capot_annonces_tentes + 1 WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qCritical() << "Erreur mise a jour capot annonce tente:" << query.lastError().text();
        return false;
    }

    qDebug() << "Stats de capot annonce tente mises a jour pour:" << pseudo;
    return true;
}

bool DatabaseManager::updateGeneraleStats(const QString &pseudo, bool success)
{
    int userId = getUserIdByPseudo(pseudo);
    if (userId == -1) {
        qWarning() << "Utilisateur non trouve pour mise a jour generale stats:" << pseudo;
        return false;
    }

    QSqlQuery query(m_db);
    if (success) {
        query.prepare("UPDATE stats SET generale_attempts = generale_attempts + 1, generale_success = generale_success + 1 WHERE user_id = :user_id");
    } else {
        query.prepare("UPDATE stats SET generale_attempts = generale_attempts + 1 WHERE user_id = :user_id");
    }
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qCritical() << "Erreur mise a jour generale stats:" << query.lastError().text();
        return false;
    }

    qDebug() << "Stats de generale mises a jour pour:" << pseudo << "Success:" << success;
    return true;
}

bool DatabaseManager::updateAnnonceCoinchee(const QString &pseudo, bool won)
{
    int userId = getUserIdByPseudo(pseudo);
    if (userId == -1) {
        qWarning() << "Utilisateur non trouve pour mise a jour annonce coinchee:" << pseudo;
        return false;
    }

    QSqlQuery query(m_db);
    if (won) {
        // L'annonce a été coinchée mais le joueur a quand même gagné la manche
        query.prepare("UPDATE stats SET annonces_coinchees = annonces_coinchees + 1, annonces_coinchees_gagnees = annonces_coinchees_gagnees + 1 WHERE user_id = :user_id");
    } else {
        // L'annonce a été coinchée et le joueur a perdu la manche
        query.prepare("UPDATE stats SET annonces_coinchees = annonces_coinchees + 1 WHERE user_id = :user_id");
    }
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qCritical() << "Erreur mise a jour annonce coinchee:" << query.lastError().text();
        return false;
    }

    qDebug() << "Stats annonce coinchee mises a jour pour:" << pseudo << "Gagnee:" << won;
    return true;
}

bool DatabaseManager::updateSurcoincheStats(const QString &pseudo, bool attempt, bool success)
{
    int userId = getUserIdByPseudo(pseudo);
    if (userId == -1) {
        qWarning() << "Utilisateur non trouve pour mise a jour surcoinche stats:" << pseudo;
        return false;
    }

    QSqlQuery query(m_db);
    if (attempt && success) {
        // Nouvelle tentative réussie
        query.prepare("UPDATE stats SET surcoinche_attempts = surcoinche_attempts + 1, surcoinche_success = surcoinche_success + 1 WHERE user_id = :user_id");
    } else if (attempt && !success) {
        // Nouvelle tentative échouée
        query.prepare("UPDATE stats SET surcoinche_attempts = surcoinche_attempts + 1 WHERE user_id = :user_id");
    } else if (!attempt && success) {
        // Marquer une surcoinche existante comme réussie (sans incrémenter les tentatives)
        query.prepare("UPDATE stats SET surcoinche_success = surcoinche_success + 1 WHERE user_id = :user_id");
    } else {
        // attempt = false, success = false : rien à faire
        return true;
    }
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qCritical() << "Erreur mise a jour surcoinche stats:" << query.lastError().text();
        return false;
    }

    qDebug() << "Stats de surcoinche mises a jour pour:" << pseudo << "Attempt:" << attempt << "Success:" << success;
    return true;
}
