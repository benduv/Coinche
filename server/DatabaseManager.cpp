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

    // Optimisations SQLite essentielles
    QSqlQuery pragmaQuery(m_db);

    // 1. WAL Mode (Write-Ahead Logging) - permet lectures pendant écritures
    if (!pragmaQuery.exec("PRAGMA journal_mode = WAL")) {
        qWarning() << "Impossible d'activer WAL mode:" << pragmaQuery.lastError().text();
    } else {
        qInfo() << "✅ SQLite WAL mode activé (lectures non bloquantes)";
    }

    // 2. Synchronous = NORMAL (bon compromis perf/sécurité)
    if (!pragmaQuery.exec("PRAGMA synchronous = NORMAL")) {
        qWarning() << "Impossible de définir synchronous:" << pragmaQuery.lastError().text();
    }

    // 3. Cache size = 10MB (au lieu de 2MB par défaut)
    if (!pragmaQuery.exec("PRAGMA cache_size = -10000")) {  // -10000 = 10MB
        qWarning() << "Impossible de définir cache_size:" << pragmaQuery.lastError().text();
    } else {
        qInfo() << "✅ SQLite cache: 10 MB";
    }

    // 4. Temp store en RAM (plus rapide)
    if (!pragmaQuery.exec("PRAGMA temp_store = MEMORY")) {
        qWarning() << "Impossible de définir temp_store:" << pragmaQuery.lastError().text();
    }

    // 5. Mmap pour meilleure performance sur gros fichiers
    if (!pragmaQuery.exec("PRAGMA mmap_size = 30000000000")) {  // 30GB max mmap
        qWarning() << "Impossible de définir mmap_size:" << pragmaQuery.lastError().text();
    }

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
    bool hasTempPasswordHash = false;
    bool hasTempPasswordCreated = false;
    bool hasProcessingRestricted = false;
    bool hasRestrictionReason = false;
    while (checkUsersQuery.next()) {
        QString columnName = checkUsersQuery.value(1).toString();
        if (columnName == "avatar") hasAvatar = true;
        if (columnName == "temp_password_hash") hasTempPasswordHash = true;
        if (columnName == "temp_password_created") hasTempPasswordCreated = true;
        if (columnName == "processing_restricted") hasProcessingRestricted = true;
        if (columnName == "restriction_reason") hasRestrictionReason = true;
    }

    if (!hasAvatar) {
        qDebug() << "Ajout de la colonne avatar dans la table users";
        if (!query.exec("ALTER TABLE users ADD COLUMN avatar TEXT DEFAULT 'avataaars1.svg'")) {
            qWarning() << "Erreur ajout colonne avatar:" << query.lastError().text();
        }
    }

    if (!hasTempPasswordHash) {
        qDebug() << "Ajout de la colonne temp_password_hash dans la table users";
        if (!query.exec("ALTER TABLE users ADD COLUMN temp_password_hash TEXT")) {
            qWarning() << "Erreur ajout colonne temp_password_hash:" << query.lastError().text();
        }
    }

    if (!hasTempPasswordCreated) {
        qDebug() << "Ajout de la colonne temp_password_created dans la table users";
        if (!query.exec("ALTER TABLE users ADD COLUMN temp_password_created TIMESTAMP")) {
            qWarning() << "Erreur ajout colonne temp_password_created:" << query.lastError().text();
        }
    }

    if (!hasProcessingRestricted) {
        qDebug() << "Ajout de la colonne processing_restricted dans la table users";
        if (!query.exec("ALTER TABLE users ADD COLUMN processing_restricted BOOLEAN DEFAULT 0")) {
            qWarning() << "Erreur ajout colonne processing_restricted:" << query.lastError().text();
        }
    }

    if (!hasRestrictionReason) {
        qDebug() << "Ajout de la colonne restriction_reason dans la table users";
        if (!query.exec("ALTER TABLE users ADD COLUMN restriction_reason TEXT DEFAULT ''")) {
            qWarning() << "Erreur ajout colonne restriction_reason:" << query.lastError().text();
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

    // Vérifier et ajouter les colonnes pour surcoinche subies et séries de victoires
    bool hasAnnoncesSurcoinchees = false;
    bool hasAnnoncesSurcoincheesGagnees = false;
    bool hasMaxWinStreak = false;
    bool hasCurrentWinStreak = false;

    query.exec("PRAGMA table_info(stats)");
    while (query.next()) {
        QString columnName = query.value(1).toString();
        if (columnName == "annonces_surcoinchees") hasAnnoncesSurcoinchees = true;
        if (columnName == "annonces_surcoinchees_gagnees") hasAnnoncesSurcoincheesGagnees = true;
        if (columnName == "max_win_streak") hasMaxWinStreak = true;
        if (columnName == "current_win_streak") hasCurrentWinStreak = true;
    }

    if (!hasAnnoncesSurcoinchees) {
        qDebug() << "Ajout de la colonne annonces_surcoinchees";
        if (!query.exec("ALTER TABLE stats ADD COLUMN annonces_surcoinchees INTEGER DEFAULT 0")) {
            qWarning() << "Erreur ajout colonne annonces_surcoinchees:" << query.lastError().text();
        }
    }

    if (!hasAnnoncesSurcoincheesGagnees) {
        qDebug() << "Ajout de la colonne annonces_surcoinchees_gagnees";
        if (!query.exec("ALTER TABLE stats ADD COLUMN annonces_surcoinchees_gagnees INTEGER DEFAULT 0")) {
            qWarning() << "Erreur ajout colonne annonces_surcoinchees_gagnees:" << query.lastError().text();
        }
    }

    if (!hasMaxWinStreak) {
        qDebug() << "Ajout de la colonne max_win_streak";
        if (!query.exec("ALTER TABLE stats ADD COLUMN max_win_streak INTEGER DEFAULT 0")) {
            qWarning() << "Erreur ajout colonne max_win_streak:" << query.lastError().text();
        }
    }

    if (!hasCurrentWinStreak) {
        qDebug() << "Ajout de la colonne current_win_streak";
        if (!query.exec("ALTER TABLE stats ADD COLUMN current_win_streak INTEGER DEFAULT 0")) {
            qWarning() << "Erreur ajout colonne current_win_streak:" << query.lastError().text();
        }
    }

    // Table des statistiques quotidiennes
    QString createDailyStatsTable = R"(
        CREATE TABLE IF NOT EXISTS daily_stats (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            date TEXT UNIQUE NOT NULL,
            logins INTEGER DEFAULT 0,
            game_rooms_created INTEGER DEFAULT 0,
            new_accounts INTEGER DEFAULT 0,
            player_quits INTEGER DEFAULT 0,
            crashes INTEGER DEFAULT 0,
            total_session_time INTEGER DEFAULT 0,
            session_count INTEGER DEFAULT 0,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(createDailyStatsTable)) {
        qCritical() << "Erreur creation table daily_stats:" << query.lastError().text();
        return false;
    }

    qDebug() << "Table 'daily_stats' creee/verifiee";

    // Migration: Ajouter les nouvelles colonnes si elles n'existent pas
    if (!query.exec("PRAGMA table_info(daily_stats)")) {
        qWarning() << "Erreur lecture schema daily_stats:" << query.lastError().text();
    } else {
        bool hasCrashes = false, hasTotalSessionTime = false, hasSessionCount = false;
        while (query.next()) {
            QString colName = query.value(1).toString();
            if (colName == "crashes") hasCrashes = true;
            if (colName == "total_session_time") hasTotalSessionTime = true;
            if (colName == "session_count") hasSessionCount = true;
        }

        if (!hasCrashes) {
            if (!query.exec("ALTER TABLE daily_stats ADD COLUMN crashes INTEGER DEFAULT 0")) {
                qWarning() << "Erreur ajout colonne crashes:" << query.lastError().text();
            }
        }
        if (!hasTotalSessionTime) {
            if (!query.exec("ALTER TABLE daily_stats ADD COLUMN total_session_time INTEGER DEFAULT 0")) {
                qWarning() << "Erreur ajout colonne total_session_time:" << query.lastError().text();
            }
        }
        if (!hasSessionCount) {
            if (!query.exec("ALTER TABLE daily_stats ADD COLUMN session_count INTEGER DEFAULT 0")) {
                qWarning() << "Erreur ajout colonne session_count:" << query.lastError().text();
            }
        }
    }

    // Table des sessions utilisateurs (pour calcul de retention)
    QString createUserSessionsTable = R"(
        CREATE TABLE IF NOT EXISTS user_sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            pseudo TEXT NOT NULL,
            login_time TIMESTAMP NOT NULL,
            logout_time TIMESTAMP,
            session_duration INTEGER DEFAULT 0,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(createUserSessionsTable)) {
        qCritical() << "Erreur creation table user_sessions:" << query.lastError().text();
        return false;
    }

    qDebug() << "Table 'user_sessions' creee/verifiee";

    // Table d'audit RGPD (traçabilité des actions sur les données personnelles)
    QString createGdprAuditLog = R"(
        CREATE TABLE IF NOT EXISTS gdpr_audit_log (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            user_pseudo TEXT NOT NULL,
            user_email TEXT NOT NULL,
            action TEXT NOT NULL,
            reason TEXT,
            performed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            performed_by TEXT DEFAULT 'system'
        )
    )";

    if (!query.exec(createGdprAuditLog)) {
        qCritical() << "Erreur creation table gdpr_audit_log:" << query.lastError().text();
        return false;
    }

    qDebug() << "Table 'gdpr_audit_log' creee/verifiee";

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
        errorMsg = "Le pseudonyme doit contenir au moins 3 caractères";
        return false;
    }

    if (password.length() < 6) {
        errorMsg = "Le mot de passe doit contenir au moins 6 caractères";
        return false;
    }

    if (!email.contains("@") || !email.contains(".")) {
        errorMsg = "Adresse email invalide";
        return false;
    }

    // Vérifier si l'email existe déjà
    if (emailExists(email)) {
        errorMsg = "Cette adresse email est dejà utilisée";
        return false;
    }

    // Vérifier si le pseudo existe déjà
    if (pseudoExists(pseudo)) {
        errorMsg = "Ce pseudonyme est déjà utilisé";
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

bool DatabaseManager::authenticateUser(const QString &email, const QString &password, QString &pseudo, QString &avatar, QString &errorMsg, bool &usingTempPassword)
{
    // Initialiser le flag
    usingTempPassword = false;

    if (email.isEmpty() || password.isEmpty()) {
        errorMsg = "Email et mot de passe requis";
        return false;
    }

    // Récupérer le salt, les hash (permanent et temporaire) et l'avatar pour cet email
    QSqlQuery query(m_db);
    query.prepare("SELECT pseudo, password_hash, salt, avatar, temp_password_hash, processing_restricted, restriction_reason FROM users WHERE email = :email");
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
    QString tempPasswordHash = query.value(4).toString();

    // Hasher le mot de passe fourni avec le salt
    QString providedHash = hashPassword(password, salt);

    // Vérifier d'abord le mot de passe permanent
    if (providedHash == storedHash) {
        // Mot de passe permanent correct
        usingTempPassword = false;
    }
    // Si le mot de passe permanent ne correspond pas, vérifier le mot de passe temporaire
    else if (!tempPasswordHash.isEmpty() && providedHash == tempPasswordHash) {
        // Mot de passe temporaire correct
        usingTempPassword = true;
        qDebug() << "Authentification avec mot de passe temporaire pour:" << email;
    }
    else {
        // Aucun des deux mots de passe ne correspond
        errorMsg = "Email ou mot de passe incorrect";
        return false;
    }

    // Vérifier si le compte est restreint (RGPD - droit à la limitation du traitement)
    bool isRestricted = query.value(5).toBool();
    if (isRestricted) {
        QString reason = query.value(6).toString();
        errorMsg = "Votre compte est temporairement restreint. Raison : " + reason + ". Contactez-nous pour plus d'informations.";
        qDebug() << "Connexion bloquée - compte restreint:" << email << "Raison:" << reason;
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
        // Victoire : incrémenter games_played, games_won, current_win_streak
        // et mettre à jour max_win_streak si nécessaire
        query.prepare("UPDATE stats SET "
                     "games_played = games_played + 1, "
                     "games_won = games_won + 1, "
                     "current_win_streak = current_win_streak + 1, "
                     "max_win_streak = CASE WHEN (current_win_streak + 1) > max_win_streak THEN (current_win_streak + 1) ELSE max_win_streak END "
                     "WHERE user_id = :user_id");
    } else {
        // Défaite : incrémenter games_played et réinitialiser current_win_streak
        query.prepare("UPDATE stats SET "
                     "games_played = games_played + 1, "
                     "current_win_streak = 0 "
                     "WHERE user_id = :user_id");
    }
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qCritical() << "Erreur mise a jour stats de jeu:" << query.lastError().text();
        return false;
    }

    qDebug() << "Stats de jeu mises a jour pour:" << pseudo << "Won:" << won;
    return true;
}

bool DatabaseManager::cancelDefeat(const QString &pseudo)
{
    int userId = getUserIdByPseudo(pseudo);
    if (userId == -1) {
        qWarning() << "Utilisateur non trouve pour annulation defaite:" << pseudo;
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE stats SET games_played = games_played - 1 WHERE user_id = :user_id AND games_played > 0");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qCritical() << "Erreur annulation defaite:" << query.lastError().text();
        return false;
    }

    qDebug() << "Defaite annulee pour:" << pseudo;
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
    PlayerStats stats = {0, 0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    int userId = getUserIdByPseudo(pseudo);
    if (userId == -1) {
        qWarning() << "Utilisateur non trouve pour recuperation stats:" << pseudo;
        return stats;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT games_played, games_won, coinche_attempts, coinche_success, capot_realises, capot_annonces_realises, capot_annonces_tentes, generale_attempts, generale_success, annonces_coinchees, annonces_coinchees_gagnees, surcoinche_attempts, surcoinche_success, annonces_surcoinchees, annonces_surcoinchees_gagnees, max_win_streak FROM stats WHERE user_id = :user_id");
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
        stats.annoncesSurcoinchees = query.value(13).toInt();
        stats.annoncesSurcoincheesGagnees = query.value(14).toInt();
        stats.maxWinStreak = query.value(15).toInt();

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

bool DatabaseManager::updateAnnonceSurcoinchee(const QString &pseudo, bool won)
{
    int userId = getUserIdByPseudo(pseudo);
    if (userId == -1) {
        qWarning() << "Utilisateur non trouve pour mise a jour annonce surcoinchee:" << pseudo;
        return false;
    }

    QSqlQuery query(m_db);
    if (won) {
        // L'annonce a été surcoinchée mais le joueur a quand même gagné la manche
        query.prepare("UPDATE stats SET annonces_surcoinchees = annonces_surcoinchees + 1, annonces_surcoinchees_gagnees = annonces_surcoinchees_gagnees + 1 WHERE user_id = :user_id");
    } else {
        // L'annonce a été surcoinchée et le joueur a perdu la manche
        query.prepare("UPDATE stats SET annonces_surcoinchees = annonces_surcoinchees + 1 WHERE user_id = :user_id");
    }
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qCritical() << "Erreur mise a jour annonce surcoinchee:" << query.lastError().text();
        return false;
    }

    qDebug() << "Stats annonce surcoinchee mises a jour pour:" << pseudo << "Gagnee:" << won;
    return true;
}

bool DatabaseManager::deleteAccount(const QString &pseudo, QString &errorMsg)
{
    if (pseudo.isEmpty()) {
        errorMsg = "Pseudo invalide";
        return false;
    }

    int userId = getUserIdByPseudo(pseudo);
    if (userId == -1) {
        errorMsg = "Compte non trouve";
        return false;
    }

    // Audit RGPD : enregistrer la suppression AVANT de supprimer les données
    QSqlQuery auditQuery(m_db);
    auditQuery.prepare("INSERT INTO gdpr_audit_log (user_id, user_pseudo, user_email, action, reason, performed_by) "
                        "SELECT id, pseudo, email, 'deletion', 'Suppression demandée par le joueur', 'system' "
                        "FROM users WHERE id = :user_id");
    auditQuery.bindValue(":user_id", userId);
    if (!auditQuery.exec()) {
        qWarning() << "Erreur audit RGPD (suppression):" << auditQuery.lastError().text();
    }

    // Supprimer d'abord les statistiques (table stats)
    QSqlQuery deleteStatsQuery(m_db);
    deleteStatsQuery.prepare("DELETE FROM stats WHERE user_id = :user_id");
    deleteStatsQuery.bindValue(":user_id", userId);

    if (!deleteStatsQuery.exec()) {
        errorMsg = "Erreur lors de la suppression des statistiques: " + deleteStatsQuery.lastError().text();
        qCritical() << "Erreur suppression stats:" << deleteStatsQuery.lastError().text();
        return false;
    }

    qDebug() << "Statistiques supprimees pour user_id:" << userId;

    // Supprimer ensuite le compte utilisateur (table users)
    QSqlQuery deleteUserQuery(m_db);
    deleteUserQuery.prepare("DELETE FROM users WHERE id = :user_id");
    deleteUserQuery.bindValue(":user_id", userId);

    if (!deleteUserQuery.exec()) {
        errorMsg = "Erreur lors de la suppression du compte: " + deleteUserQuery.lastError().text();
        qCritical() << "Erreur suppression utilisateur:" << deleteUserQuery.lastError().text();
        return false;
    }

    qDebug() << "Compte supprime avec succes pour:" << pseudo;
    return true;
}

// ==================== PASSWORD RECOVERY METHODS ====================

QString DatabaseManager::generateTempPassword()
{
    // Générer un mot de passe temporaire de 8 caractères (majuscules, minuscules, chiffres)
    QString tempPassword;
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    for (int i = 0; i < 8; i++) {
        tempPassword += chars[QRandomGenerator::global()->bounded(chars.length())];
    }

    return tempPassword;
}

bool DatabaseManager::setTempPassword(const QString &email, QString &tempPassword, QString &errorMsg)
{
    if (email.isEmpty()) {
        errorMsg = "Email requis";
        return false;
    }

    // Récupérer le salt existant pour cet email
    QSqlQuery getSaltQuery(m_db);
    getSaltQuery.prepare("SELECT salt FROM users WHERE email = :email");
    getSaltQuery.bindValue(":email", email);

    if (!getSaltQuery.exec()) {
        errorMsg = "Erreur lors de la vérification de l'email: " + getSaltQuery.lastError().text();
        qCritical() << "Erreur getSalt:" << getSaltQuery.lastError().text();
        return false;
    }

    if (!getSaltQuery.next()) {
        errorMsg = "Cette adresse mail ne correspond a aucun compte";
        return false;
    }

    QString salt = getSaltQuery.value(0).toString();

    // Générer le mot de passe temporaire
    tempPassword = generateTempPassword();

    // Hasher le mot de passe temporaire avec le salt existant
    QString tempPasswordHash = hashPassword(tempPassword, salt);

    // Mettre à jour la base de données
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET temp_password_hash = :temp_hash, temp_password_created = CURRENT_TIMESTAMP WHERE email = :email");
    query.bindValue(":temp_hash", tempPasswordHash);
    query.bindValue(":email", email);

    if (!query.exec()) {
        errorMsg = "Erreur lors de la génération du mot de passe temporaire: " + query.lastError().text();
        qCritical() << "Erreur setTempPassword:" << query.lastError().text();
        return false;
    }

    qDebug() << "Mot de passe temporaire généré pour:" << email;
    return true;
}

bool DatabaseManager::isUsingTempPassword(const QString &email)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT temp_password_hash FROM users WHERE email = :email");
    query.bindValue(":email", email);

    if (!query.exec()) {
        qWarning() << "Erreur isUsingTempPassword:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return !query.value(0).isNull();
    }

    return false;
}

bool DatabaseManager::updatePassword(const QString &email, const QString &newPassword, QString &errorMsg)
{
    if (email.isEmpty() || newPassword.isEmpty()) {
        errorMsg = "Email et nouveau mot de passe requis";
        return false;
    }

    // Valider le nouveau mot de passe
    if (newPassword.length() < 6) {
        errorMsg = "Le mot de passe doit contenir au moins 6 caractères";
        return false;
    }

    // Vérifier si l'email existe
    if (!emailExists(email)) {
        errorMsg = "Compte non trouvé";
        return false;
    }

    // Générer un nouveau salt et hasher le nouveau mot de passe
    QString salt = generateSalt();
    QString passwordHash = hashPassword(newPassword, salt);

    // Mettre à jour le mot de passe permanent et effacer le mot de passe temporaire
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET password_hash = :password_hash, salt = :salt, temp_password_hash = NULL, temp_password_created = NULL WHERE email = :email");
    query.bindValue(":password_hash", passwordHash);
    query.bindValue(":salt", salt);
    query.bindValue(":email", email);

    if (!query.exec()) {
        errorMsg = "Erreur lors de la mise à jour du mot de passe: " + query.lastError().text();
        qCritical() << "Erreur updatePassword:" << query.lastError().text();
        return false;
    }

    qDebug() << "Mot de passe mis à jour avec succès pour:" << email;
    return true;
}

bool DatabaseManager::updatePseudo(const QString &currentPseudo, const QString &newPseudo, QString &errorMsg)
{
    if (currentPseudo.isEmpty() || newPseudo.isEmpty()) {
        errorMsg = "Pseudo actuel et nouveau pseudo requis";
        return false;
    }

    if (currentPseudo == newPseudo) {
        errorMsg = "Le nouveau pseudo est identique à l'actuel";
        return false;
    }

    if (!pseudoExists(currentPseudo)) {
        errorMsg = "Compte non trouvé";
        return false;
    }

    if (pseudoExists(newPseudo)) {
        errorMsg = "Ce pseudo est déjà utilisé";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET pseudo = :newPseudo WHERE pseudo = :currentPseudo");
    query.bindValue(":newPseudo", newPseudo);
    query.bindValue(":currentPseudo", currentPseudo);

    if (!query.exec()) {
        errorMsg = "Erreur lors de la mise à jour du pseudo: " + query.lastError().text();
        qCritical() << "Erreur updatePseudo:" << query.lastError().text();
        return false;
    }

    // Audit RGPD : enregistrer le changement de pseudo
    QSqlQuery auditQuery(m_db);
    auditQuery.prepare("INSERT INTO gdpr_audit_log (user_id, user_pseudo, user_email, action, reason, performed_by) "
                        "SELECT id, :newPseudo, email, 'pseudo_change', :reason, 'system' "
                        "FROM users WHERE pseudo = :pseudo");
    auditQuery.bindValue(":newPseudo", newPseudo);
    auditQuery.bindValue(":reason", "Changement de pseudo: " + currentPseudo + " -> " + newPseudo);
    auditQuery.bindValue(":pseudo", newPseudo);
    if (!auditQuery.exec()) {
        qWarning() << "Erreur audit RGPD (pseudo_change):" << auditQuery.lastError().text();
    }

    qDebug() << "Pseudo mis à jour avec succès:" << currentPseudo << "->" << newPseudo;
    return true;
}

bool DatabaseManager::updateEmail(const QString &pseudo, const QString &newEmail, QString &errorMsg)
{
    if (pseudo.isEmpty() || newEmail.isEmpty()) {
        errorMsg = "Pseudo et nouvel email requis";
        return false;
    }

    if (!pseudoExists(pseudo)) {
        errorMsg = "Compte non trouvé";
        return false;
    }

    if (emailExists(newEmail)) {
        errorMsg = "Cet email est déjà utilisé";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET email = :newEmail WHERE pseudo = :pseudo");
    query.bindValue(":newEmail", newEmail);
    query.bindValue(":pseudo", pseudo);

    if (!query.exec()) {
        errorMsg = "Erreur lors de la mise à jour de l'email: " + query.lastError().text();
        qCritical() << "Erreur updateEmail:" << query.lastError().text();
        return false;
    }

    // Audit RGPD : enregistrer le changement d'email
    QSqlQuery auditQuery(m_db);
    auditQuery.prepare("INSERT INTO gdpr_audit_log (user_id, user_pseudo, user_email, action, reason, performed_by) "
                        "SELECT id, pseudo, :newEmail, 'email_change', :reason, 'system' "
                        "FROM users WHERE pseudo = :pseudo");
    auditQuery.bindValue(":newEmail", newEmail);
    auditQuery.bindValue(":reason", "Changement d'email pour " + pseudo);
    auditQuery.bindValue(":pseudo", pseudo);
    if (!auditQuery.exec()) {
        qWarning() << "Erreur audit RGPD (email_change):" << auditQuery.lastError().text();
    }

    qDebug() << "Email mis à jour avec succès pour:" << pseudo;
    return true;
}

// ==================== STATISTIQUES QUOTIDIENNES ====================

bool DatabaseManager::recordLogin(const QString &pseudo)
{
    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    // S'assurer que l'entrée du jour existe
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT id FROM daily_stats WHERE date = :date");
    checkQuery.bindValue(":date", today);

    if (!checkQuery.exec()) {
        qWarning() << "Erreur verif daily_stats:" << checkQuery.lastError().text();
        return false;
    }

    if (!checkQuery.next()) {
        // Créer l'entrée du jour
        QSqlQuery insertQuery(m_db);
        insertQuery.prepare("INSERT INTO daily_stats (date, logins) VALUES (:date, 1)");
        insertQuery.bindValue(":date", today);

        if (!insertQuery.exec()) {
            qWarning() << "Erreur creation daily_stats:" << insertQuery.lastError().text();
            return false;
        }
    } else {
        // Incrémenter le compteur de logins
        QSqlQuery updateQuery(m_db);
        updateQuery.prepare("UPDATE daily_stats SET logins = logins + 1 WHERE date = :date");
        updateQuery.bindValue(":date", today);

        if (!updateQuery.exec()) {
            qWarning() << "Erreur update logins:" << updateQuery.lastError().text();
            return false;
        }
    }

    qDebug() << "Login enregistré pour:" << pseudo << "à la date:" << today;
    return true;
}

bool DatabaseManager::recordGameRoomCreated()
{
    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO daily_stats (date, game_rooms_created) VALUES (:date, 1)
        ON CONFLICT(date) DO UPDATE SET game_rooms_created = game_rooms_created + 1
    )");
    query.bindValue(":date", today);

    if (!query.exec()) {
        qWarning() << "Erreur recordGameRoomCreated:" << query.lastError().text();
        return false;
    }

    qDebug() << "GameRoom créée enregistrée pour:" << today;
    return true;
}

bool DatabaseManager::recordNewAccount()
{
    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO daily_stats (date, new_accounts) VALUES (:date, 1)
        ON CONFLICT(date) DO UPDATE SET new_accounts = new_accounts + 1
    )");
    query.bindValue(":date", today);

    if (!query.exec()) {
        qWarning() << "Erreur recordNewAccount:" << query.lastError().text();
        return false;
    }

    qDebug() << "Nouveau compte enregistré pour:" << today;
    return true;
}

bool DatabaseManager::recordPlayerQuit()
{
    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO daily_stats (date, player_quits) VALUES (:date, 1)
        ON CONFLICT(date) DO UPDATE SET player_quits = player_quits + 1
    )");
    query.bindValue(":date", today);

    if (!query.exec()) {
        qWarning() << "Erreur recordPlayerQuit:" << query.lastError().text();
        return false;
    }

    qDebug() << "Abandon de partie enregistré pour:" << today;
    return true;
}

DatabaseManager::DailyStats DatabaseManager::getDailyStats(const QString &date)
{
    QString targetDate = date.isEmpty() ? QDate::currentDate().toString("yyyy-MM-dd") : date;

    DailyStats stats;
    stats.date = targetDate;
    stats.logins = 0;
    stats.gameRoomsCreated = 0;
    stats.newAccounts = 0;
    stats.playerQuits = 0;
    stats.crashes = 0;
    stats.totalSessionTime = 0;
    stats.sessionCount = 0;

    QSqlQuery query(m_db);
    query.prepare("SELECT logins, game_rooms_created, new_accounts, player_quits, crashes, total_session_time, session_count FROM daily_stats WHERE date = :date");
    query.bindValue(":date", targetDate);

    if (!query.exec()) {
        qWarning() << "Erreur getDailyStats:" << query.lastError().text();
        return stats;
    }

    if (query.next()) {
        stats.logins = query.value(0).toInt();
        stats.gameRoomsCreated = query.value(1).toInt();
        stats.newAccounts = query.value(2).toInt();
        stats.playerQuits = query.value(3).toInt();
        stats.crashes = query.value(4).toInt();
        stats.totalSessionTime = query.value(5).toInt();
        stats.sessionCount = query.value(6).toInt();
    }

    return stats;
}

DatabaseManager::DailyStats DatabaseManager::getYesterdayStats()
{
    QString yesterday = QDate::currentDate().addDays(-1).toString("yyyy-MM-dd");
    return getDailyStats(yesterday);
}

// Tracking du temps de session - Lightweight (pas de timers)
bool DatabaseManager::recordSessionStart(const QString &pseudo)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO user_sessions (pseudo, login_time) VALUES (:pseudo, datetime('now'))");
    query.bindValue(":pseudo", pseudo);

    if (!query.exec()) {
        qWarning() << "Erreur recordSessionStart:" << query.lastError().text();
        return false;
    }

    qDebug() << "Session démarrée pour:" << pseudo;
    return true;
}

bool DatabaseManager::recordSessionEnd(const QString &pseudo)
{
    QSqlQuery query(m_db);

    // Trouver la session active la plus récente sans logout_time
    query.prepare("SELECT id, login_time FROM user_sessions WHERE pseudo = :pseudo AND logout_time IS NULL ORDER BY login_time DESC LIMIT 1");
    query.bindValue(":pseudo", pseudo);

    if (!query.exec() || !query.next()) {
        qWarning() << "Aucune session active trouvée pour:" << pseudo;
        return false;
    }

    int sessionId = query.value(0).toInt();
    QDateTime loginTime = query.value(1).toDateTime();
    QDateTime logoutTime = QDateTime::currentDateTime();
    int duration = loginTime.secsTo(logoutTime);

    // Mettre à jour la session avec le logout_time et la durée
    query.prepare("UPDATE user_sessions SET logout_time = datetime('now'), session_duration = :duration WHERE id = :id");
    query.bindValue(":duration", duration);
    query.bindValue(":id", sessionId);

    if (!query.exec()) {
        qWarning() << "Erreur recordSessionEnd:" << query.lastError().text();
        return false;
    }

    // Mettre à jour les stats quotidiennes
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    query.prepare(R"(
        INSERT INTO daily_stats (date, total_session_time, session_count)
        VALUES (:date, :duration, 1)
        ON CONFLICT(date) DO UPDATE SET
            total_session_time = total_session_time + :duration,
            session_count = session_count + 1
    )");
    query.bindValue(":date", today);
    query.bindValue(":duration", duration);

    if (!query.exec()) {
        qWarning() << "Erreur mise à jour stats session:" << query.lastError().text();
        return false;
    }

    qDebug() << "Session terminée pour" << pseudo << "- Durée:" << duration << "secondes";
    return true;
}

// Tracking des crashes
bool DatabaseManager::recordCrash()
{
    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO daily_stats (date, crashes)
        VALUES (:date, 1)
        ON CONFLICT(date) DO UPDATE SET crashes = crashes + 1
    )");
    query.bindValue(":date", today);

    if (!query.exec()) {
        qWarning() << "Erreur recordCrash:" << query.lastError().text();
        return false;
    }

    qDebug() << "Crash enregistré pour:" << today;
    return true;
}

// Calcul des taux de rétention
DatabaseManager::RetentionStats DatabaseManager::getRetentionStats()
{
    RetentionStats retention;
    retention.d1Retention = 0.0;
    retention.d7Retention = 0.0;
    retention.d30Retention = 0.0;

    QSqlQuery query(m_db);

    // Calcul D1 : % de joueurs actifs il y a 2 jours qui se sont reconnectés le lendemain
    query.prepare(R"(
        SELECT
            COUNT(DISTINCT s1.pseudo) as total_users,
            COUNT(DISTINCT CASE WHEN s2.pseudo IS NOT NULL THEN s1.pseudo END) as returned_users
        FROM user_sessions s1
        LEFT JOIN user_sessions s2 ON s1.pseudo = s2.pseudo
            AND date(s2.login_time) = date(s1.login_time, '+1 day')
        WHERE date(s1.login_time) = date('now', '-2 days')
    )");

    if (query.exec() && query.next()) {
        int total = query.value(0).toInt();
        int returned = query.value(1).toInt();
        if (total > 0) {
            retention.d1Retention = (returned * 100.0) / total;
        }
    }

    // Calcul D7
    query.prepare(R"(
        SELECT
            COUNT(DISTINCT s1.pseudo) as total_users,
            COUNT(DISTINCT CASE WHEN s2.pseudo IS NOT NULL THEN s1.pseudo END) as returned_users
        FROM user_sessions s1
        LEFT JOIN user_sessions s2 ON s1.pseudo = s2.pseudo
            AND date(s2.login_time) BETWEEN date(s1.login_time, '+6 days') AND date(s1.login_time, '+8 days')
        WHERE date(s1.login_time) = date('now', '-9 days')
    )");

    if (query.exec() && query.next()) {
        int total = query.value(0).toInt();
        int returned = query.value(1).toInt();
        if (total > 0) {
            retention.d7Retention = (returned * 100.0) / total;
        }
    }

    // Calcul D30
    query.prepare(R"(
        SELECT
            COUNT(DISTINCT s1.pseudo) as total_users,
            COUNT(DISTINCT CASE WHEN s2.pseudo IS NOT NULL THEN s1.pseudo END) as returned_users
        FROM user_sessions s1
        LEFT JOIN user_sessions s2 ON s1.pseudo = s2.pseudo
            AND date(s2.login_time) BETWEEN date(s1.login_time, '+29 days') AND date(s1.login_time, '+31 days')
        WHERE date(s1.login_time) = date('now', '-32 days')
    )");

    if (query.exec() && query.next()) {
        int total = query.value(0).toInt();
        int returned = query.value(1).toInt();
        if (total > 0) {
            retention.d30Retention = (returned * 100.0) / total;
        }
    }

    return retention;
}

// Obtenir les tendances sur N jours
QList<DatabaseManager::DailyStats> DatabaseManager::getTrendStats(int days)
{
    QList<DailyStats> trends;

    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT date, logins, game_rooms_created, new_accounts, player_quits, crashes, total_session_time, session_count
        FROM daily_stats
        WHERE date >= date('now', '-' || :days || ' days')
        ORDER BY date ASC
    )");
    query.bindValue(":days", days);

    if (!query.exec()) {
        qWarning() << "Erreur getTrendStats:" << query.lastError().text();
        return trends;
    }

    while (query.next()) {
        DailyStats stats;
        stats.date = query.value(0).toString();
        stats.logins = query.value(1).toInt();
        stats.gameRoomsCreated = query.value(2).toInt();
        stats.newAccounts = query.value(3).toInt();
        stats.playerQuits = query.value(4).toInt();
        stats.crashes = query.value(5).toInt();
        stats.totalSessionTime = query.value(6).toInt();
        stats.sessionCount = query.value(7).toInt();
        trends.append(stats);
    }

    return trends;
}
