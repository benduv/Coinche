#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <QDateTime>
#include <QAbstractSocket>
#include <QSettings>
#include <QSslConfiguration>
#include <QNetworkRequest>
#include "GameModel.h"

class NetworkManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString matchmakingStatus READ matchmakingStatus NOTIFY matchmakingStatusChanged)
    Q_PROPERTY(int playersInQueue READ playersInQueue NOTIFY playersInQueueChanged)
    Q_PROPERTY(int matchmakingCountdown READ matchmakingCountdown NOTIFY matchmakingCountdownChanged)
    Q_PROPERTY(QJsonArray myCards READ myCards NOTIFY gameDataChanged)
    Q_PROPERTY(int myPosition READ myPosition NOTIFY gameDataChanged)
    Q_PROPERTY(QJsonArray opponents READ opponents NOTIFY gameDataChanged)
    Q_PROPERTY(QString playerAvatar READ playerAvatar NOTIFY playerAvatarChanged)
    Q_PROPERTY(QString playerPseudo READ playerPseudo NOTIFY playerPseudoChanged)
    Q_PROPERTY(QVariantList lobbyPlayers READ lobbyPlayers NOTIFY lobbyPlayersChanged)
    Q_PROPERTY(QString pendingBotReplacement READ pendingBotReplacement NOTIFY pendingBotReplacementChanged)
    Q_PROPERTY(bool hasStoredCredentials READ hasStoredCredentials NOTIFY storedCredentialsChanged)
    Q_PROPERTY(QString storedEmail READ storedEmail NOTIFY storedCredentialsChanged)

public:
    explicit NetworkManager(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QWebSocket)
        , m_connected(false)
        , m_playersInQueue(0)
        , m_matchmakingCountdown(0)
        , m_myPosition(-1)
        , m_gameModel(nullptr)
        , m_playerAvatar("avataaars1.svg")
        , m_heartbeatTimer(new QTimer(this))
        , m_wasInGame(false)
        , m_lastPongReceived(0)
    {
        setupSocketConnections();

        // Timer pour le heartbeat (détection de connexion morte)
        connect(m_heartbeatTimer, &QTimer::timeout, this, &NetworkManager::sendHeartbeat);
        m_heartbeatTimer->setInterval(5000); // Envoyer un ping toutes les 5 secondes
    }

    ~NetworkManager() {
        if (m_socket) {
            m_socket->close();
            delete m_socket;
            m_socket = nullptr;
        }
        if (m_gameModel) {
            delete m_gameModel;
        }
    }

    bool connected() const { return m_connected; }
    QString matchmakingStatus() const { return m_matchmakingStatus; }
    int playersInQueue() const { return m_playersInQueue; }
    int matchmakingCountdown() const { return m_matchmakingCountdown; }
    QJsonArray myCards() const { return m_myCards; }
    int myPosition() const { return m_myPosition; }
    QJsonArray opponents() const { return m_opponents; }
    GameModel* gameModel() const { return m_gameModel; }
    QString playerAvatar() const { return m_playerAvatar; }
    QString playerPseudo() const { return m_playerPseudo; }
    QVariantList lobbyPlayers() const { return m_lobbyPlayers; }
    QString pendingBotReplacement() const { return m_pendingBotReplacement; }

    // Auto-login - vérifier si des credentials sont stockés
    bool hasStoredCredentials() const {
        QSettings settings("Nebuludik", "CoincheDelEspace");
        return settings.contains("auth/email") && settings.contains("auth/password");
    }

    QString storedEmail() const {
        QSettings settings("Nebuludik", "CoincheDelEspace");
        return settings.value("auth/email", "").toString();
    }

    // Méthode pour consommer le message de remplacement par bot en attente
    Q_INVOKABLE QString consumePendingBotReplacement() {
        QString msg = m_pendingBotReplacement;
        m_pendingBotReplacement.clear();
        return msg;
    }

    // Nouvelle méthode pour créer le GameModel
    Q_INVOKABLE void createGameModel(int position, const QJsonArray& cards, const QJsonArray& opps, bool isReconnection = false) {
        qDebug() << "Creation du GameModel en C++ - Reconnexion:" << isReconnection;

        // Supprimer l'ancien si existant
        if (m_gameModel) {
            delete m_gameModel;
        }

        // Créer le nouveau GameModel
        m_gameModel = new GameModel(this);

        // Connecter les signaux
        connect(m_gameModel, &GameModel::cardPlayedLocally, this, [this](int cardIndex) {
            qDebug() << "Envoi playCard au serveur:" << cardIndex;
            playCard(cardIndex);
        });

        connect(m_gameModel, &GameModel::bidMadeLocally, this, [this](int bidValue, int suitValue) {
            qDebug() << "Envoi makeBid au serveur:" << bidValue << suitValue;
            makeBid(bidValue, suitValue);
        });

        connect(m_gameModel, &GameModel::forfeitLocally, this, [this]() {
            qDebug() << "Envoi forfeit au serveur";
            forfeitGame();
        });

        // Initialiser avec les données en passant le pseudo du joueur et le flag de reconnexion
        m_gameModel->initOnlineGame(position, cards, opps, m_playerPseudo, isReconnection);

        // Définir l'avatar du joueur local
        m_gameModel->setPlayerAvatar(position, m_playerAvatar);

        qDebug() << "GameModel cree et initialise - Pseudo:" << m_playerPseudo << "Avatar:" << m_playerAvatar;

        // Émettre signal pour que QML navigue vers CoincheView
        emit gameModelReady();
    }

    Q_INVOKABLE void connectToServer(const QString &url) {
        qDebug() << "Connexion au serveur:" << url;
        m_serverUrl = url;  // Sauvegarder l'URL pour les reconnexions

        // Avec certificat Let's Encrypt valide via nginx, pas besoin d'ignorer les erreurs SSL
        if (url.startsWith("wss://")) {
            qDebug() << "Connexion WSS détectée - Certificat valide attendu";
        }

        openSocket(url);
    }

    Q_INVOKABLE void registerPlayer(const QString &playerName, const QString &avatar = "avataaars1.svg") {
        QJsonObject msg;
        msg["type"] = "register";
        msg["playerName"] = playerName;
        msg["avatar"] = avatar;
        // Indiquer si on avait un GameModel actif (pour détecter les parties terminées)
        msg["wasInGame"] = (m_gameModel != nullptr);
        sendMessage(msg);
    }

    // Reporter un crash au serveur (méthode publique pour le crash handler)
    Q_INVOKABLE void reportCrash(const QString &error, const QString &stackTrace) {
        QJsonObject msg;
        msg["type"] = "reportCrash";
        msg["error"] = error;
        msg["stackTrace"] = stackTrace;
        msg["playerName"] = m_playerPseudo.isEmpty() ? "Unknown" : m_playerPseudo;
        sendMessage(msg);
    }

    // Getter pour le nom du joueur (pour le crash reporter)
    QString currentPlayerName() const { return m_playerPseudo.isEmpty() ? "Unknown" : m_playerPseudo; }

    Q_INVOKABLE void joinMatchmaking() {
        QJsonObject msg;
        msg["type"] = "joinMatchmaking";
        sendMessage(msg);
    }

    Q_INVOKABLE void leaveMatchmaking() {
        QJsonObject msg;
        msg["type"] = "leaveMatchmaking";
        sendMessage(msg);
    }

    Q_INVOKABLE void playCard(int cardIndex) {
        QJsonObject msg;
        msg["type"] = "playCard";
        msg["cardIndex"] = cardIndex;
        sendMessage(msg);
    }

    Q_INVOKABLE void makeBid(int bidValue, int suit) {
        QJsonObject msg;
        msg["type"] = "makeBid";
        msg["bidValue"] = bidValue;
        msg["suit"] = suit;
        sendMessage(msg);
    }

    Q_INVOKABLE void registerAccount(const QString &pseudo, const QString &email, const QString &password, const QString &avatar = "avataaars1.svg") {
        QJsonObject msg;
        msg["type"] = "registerAccount";
        msg["pseudo"] = pseudo;
        msg["email"] = email;
        msg["password"] = password;
        msg["avatar"] = avatar;
        sendMessage(msg);
    }

    Q_INVOKABLE void loginAccount(const QString &email, const QString &password) {
        QJsonObject msg;
        msg["type"] = "loginAccount";
        msg["email"] = email;
        msg["password"] = password;
        sendMessage(msg);
    }

    Q_INVOKABLE void deleteAccount(const QString &pseudo) {
        qDebug() << "Demande de suppression du compte:" << pseudo;
        QJsonObject msg;
        msg["type"] = "deleteAccount";
        msg["pseudo"] = pseudo;
        sendMessage(msg);
    }

    Q_INVOKABLE void requestStats(const QString &pseudo) {
        QJsonObject msg;
        msg["type"] = "getStats";
        msg["pseudo"] = pseudo;
        sendMessage(msg);
    }

    Q_INVOKABLE void sendContactMessage(const QString &senderName, const QString &subject, const QString &message) {
        qDebug() << "Envoi message de contact - Sujet:" << subject;
        QJsonObject msg;
        msg["type"] = "sendContactMessage";
        msg["senderName"] = senderName;
        msg["subject"] = subject;
        msg["message"] = message;
        sendMessage(msg);
    }

    Q_INVOKABLE void forfeitGame() {
        QJsonObject msg;
        msg["type"] = "forfeit";
        sendMessage(msg);
    }

    Q_INVOKABLE void clearGameModel() {
        if (m_gameModel) {
            qDebug() << "Nettoyage du gameModel";
            // IMPORTANT: Arrêter les timers AVANT de supprimer pour éviter accès mémoire invalide
            m_gameModel->pauseTimers();
            m_gameModel->deleteLater();
            m_gameModel = nullptr;
        }
    }

    Q_INVOKABLE void updateAvatar(const QString &avatar) {
        qDebug() << "Mise à jour de l'avatar:" << avatar;
        m_playerAvatar = avatar;
        emit playerAvatarChanged();

        // Envoyer la mise à jour au serveur
        QJsonObject msg;
        msg["type"] = "updateAvatar";
        msg["avatar"] = avatar;
        sendMessage(msg);
    }

    // Demander à redevenir humain après avoir été remplacé par un bot
    Q_INVOKABLE void requestRehumanize() {
        qDebug() << "Demande de réhumanisation";
        QJsonObject msg;
        msg["type"] = "rehumanize";
        sendMessage(msg);
    }

    // Méthodes pour les lobbies privés
    Q_INVOKABLE void createPrivateLobby() {
        QJsonObject msg;
        msg["type"] = "createPrivateLobby";
        sendMessage(msg);
        qDebug() << "Demande de creation de lobby prive envoyee";
    }

    Q_INVOKABLE void joinPrivateLobby(const QString& code) {
        QJsonObject msg;
        msg["type"] = "joinPrivateLobby";
        msg["code"] = code;
        sendMessage(msg);
        qDebug() << "Demande de rejoindre le lobby" << code;
    }

    Q_INVOKABLE void toggleLobbyReady(bool ready) {
        QJsonObject msg;
        msg["type"] = "lobbyReady";
        msg["ready"] = ready;
        sendMessage(msg);
        qDebug() << "Changement de statut pret:" << ready;
    }

    Q_INVOKABLE void startLobbyGame() {
        QJsonObject msg;
        msg["type"] = "startLobbyGame";
        sendMessage(msg);
        qDebug() << "Demande de demarrage de la partie depuis le lobby";
    }

    Q_INVOKABLE void leaveLobby() {
        QJsonObject msg;
        msg["type"] = "leaveLobby";
        sendMessage(msg);
        qDebug() << "Quitte le lobby";
    }

    // Sauvegarder les credentials pour l'auto-login
    Q_INVOKABLE void saveCredentials(const QString &email, const QString &password) {
        QSettings settings("Nebuludik", "CoincheDelEspace");
        settings.setValue("auth/email", email);
        settings.setValue("auth/password", password);
        qDebug() << "Credentials sauvegardés pour:" << email;
        emit storedCredentialsChanged();
    }

    // Supprimer les credentials stockés (déconnexion)
    Q_INVOKABLE void clearCredentials() {
        QSettings settings("Nebuludik", "CoincheDelEspace");
        settings.remove("auth/email");
        settings.remove("auth/password");

        // Vider aussi le pseudo et l'avatar du joueur
        if (!m_playerPseudo.isEmpty()) {
            m_playerPseudo = "";
            emit playerPseudoChanged();
        }
        if (!m_playerAvatar.isEmpty()) {
            m_playerAvatar = "";
            emit playerAvatarChanged();
        }

        qDebug() << "Credentials et données joueur supprimés";
        emit storedCredentialsChanged();
    }

    // Tenter l'auto-login avec les credentials stockés
    Q_INVOKABLE bool tryAutoLogin() {
        QSettings settings("Nebuludik", "CoincheDelEspace");
        QString email = settings.value("auth/email", "").toString();
        QString password = settings.value("auth/password", "").toString();

        if (email.isEmpty() || password.isEmpty()) {
            qDebug() << "Pas de credentials stockés pour auto-login";
            return false;
        }

        qDebug() << "Tentative d'auto-login pour:" << email;
        loginAccount(email, password);
        return true;
    }

signals:
    void connectedChanged();
    void matchmakingStatusChanged();
    void playersInQueueChanged();
    void matchmakingCountdownChanged();
    void gameDataChanged();
    void gameFound(int playerPosition, QJsonArray opponents, bool isReconnection = false);
    void gameModelReady();
    void cardPlayed(QString playerId, int cardIndex);
    void bidMade(QString playerId, int bidValue, int suit);
    void playerDisconnected(QString playerId);
    void playerForfeited(int playerIndex, QString playerName);
    void errorOccurred(QString error);
    void registerSuccess(QString playerName, QString avatar);
    void registerFailed(QString error);
    void loginSuccess(QString playerName, QString avatar);
    void loginFailed(QString error);
    void deleteAccountSuccess();
    void deleteAccountFailed(QString error);
    void messageReceived(QString message);  // Pour que QML puisse écouter tous les messages
    void playerAvatarChanged();
    void playerPseudoChanged();

    // Signaux pour les lobbies privés
    void lobbyCreated(QString code);
    void lobbyJoined(QString code);
    void lobbyError(QString message);
    void lobbyPlayersChanged();
    void lobbyGameStarting();

    // Signaux pour le remplacement par bot
    void botReplacement(QString message);
    void rehumanizeSuccess();
    void pendingBotReplacementChanged();

    // Signal pour l'animation de nouvelle manche
    void newMancheAnimation();

    // Signal pour les credentials stockés
    void storedCredentialsChanged();

    // Signaux pour les messages de contact
    void contactMessageSuccess();
    void contactMessageFailed(QString error);

    // Signal pour retourner au menu après reconnexion (partie terminée)
    void returnToMainMenu();

private slots:
    void onConnected() {
        qDebug() << "Connecte au serveur";
        m_connected = true;

        // Démarrer le heartbeat pour détecter les connexions mortes
        m_lastPongReceived = QDateTime::currentMSecsSinceEpoch();
        m_heartbeatTimer->start();
        qDebug() << "Heartbeat démarré (ping toutes les 5 secondes)";

        // Si on se reconnecte et qu'on avait un pseudo et avatar, se réenregistrer
        if (!m_playerPseudo.isEmpty()) {
            qDebug() << "Reconnexion - Réenregistrement avec pseudo:" << m_playerPseudo;
            registerPlayer(m_playerPseudo, m_playerAvatar);
        }

        emit connectedChanged();
    }

    void onDisconnected() {
        qDebug() << "====================== onDisconnected() appelé ===============================";
        qDebug() << "  m_connected:" << m_connected;
        qDebug() << "  m_playerPseudo:" << m_playerPseudo;
        qDebug() << "  m_gameModel:" << (m_gameModel != nullptr);

        // Capturer la raison de la déconnexion pour le diagnostic
        if (m_socket) {
            qDebug() << "  closeCode:" << m_socket->closeCode();
            qDebug() << "  closeReason:" << m_socket->closeReason();
            qDebug() << "  errorString:" << m_socket->errorString();
        }

        bool wasConnected = m_connected;
        //m_connected = false;

        // Arrêter le heartbeat
        m_heartbeatTimer->stop();
        qDebug() << "Heartbeat arrêté";

        // Si on était en partie ou connecté, activer la reconnexion automatique
        if (wasConnected && (!m_playerPseudo.isEmpty() || m_gameModel != nullptr)) {
            qDebug() << "Perte de connexion detectee - Demarrage tentatives de reconnexion";
            m_wasInGame = (m_gameModel != nullptr);  // Sauvegarder si on était en partie

            qDebug() << "Reconnexion à" << m_serverUrl;
            openSocket(m_serverUrl);
        } else {
            qDebug() << "Pas de reconnexion automatique - conditions non remplies";
            qDebug() << "  wasConnected:" << wasConnected;
        }

        emit connectedChanged();
    }

    void onMessageReceived(const QString &message) {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();

        qDebug() << "NetWorkManager - Message recu:" << type;

        // Émettre le message pour que QML puisse l'écouter (ex: StatsView)
        emit messageReceived(message);

        if (type == "registered") {
            m_playerId = obj["connectionId"].toString();
            QString playerName = obj["playerName"].toString();
            QString avatar = obj["avatar"].toString();

            // Mettre à jour le pseudo (le serveur peut l'avoir modifié pour les invités)
            if (!playerName.isEmpty()) {
                m_playerPseudo = playerName;
                emit playerPseudoChanged();
            }

            if (!avatar.isEmpty()) {
                m_playerAvatar = avatar;
                emit playerAvatarChanged();
                qDebug() << "Enregistre avec ID:" << m_playerId << "Pseudo:" << m_playerPseudo << "Avatar:" << avatar;
            } else {
                qDebug() << "Enregistre avec ID:" << m_playerId << "Pseudo:" << m_playerPseudo;
            }
        }
        else if (type == "matchmakingStatus") {
            m_matchmakingStatus = obj["status"].toString();
            m_playersInQueue = obj["playersInQueue"].toInt();
            // Réinitialiser le countdown quand on reçoit un status (nouveau joueur rejoint)
            if (m_matchmakingCountdown > 0) {
                m_matchmakingCountdown = 0;
                emit matchmakingCountdownChanged();
            }
            emit matchmakingStatusChanged();
            emit playersInQueueChanged();
        }
        else if (type == "matchmakingCountdown") {
            m_matchmakingCountdown = obj["seconds"].toInt();
            qDebug() << "Countdown reçu:" << m_matchmakingCountdown << "secondes";
            emit matchmakingCountdownChanged();
        }
        else if (type == "gameFound") {
            qDebug() << "=============== GAME FOUND ===============";

            bool isReconnection = obj["reconnection"].toBool(false);
            m_myPosition = obj["playerPosition"].toInt();
            m_myCards = obj["myCards"].toArray();
            m_opponents = obj["opponents"].toArray();

            qDebug() << "Position:" << m_myPosition;
            qDebug() << "Nombre de cartes:" << m_myCards.size();
            qDebug() << "Nombre d'adversaires:" << m_opponents.size();
            qDebug() << "Reconnexion:" << isReconnection;

            // Si c'est une NOUVELLE partie (pas une reconnexion), effacer le message de bot replacement en attente
            // car il provient de la partie précédente
            if (!isReconnection && !m_pendingBotReplacement.isEmpty()) {
                qInfo() << "Nouvelle partie - Nettoyage du message bot replacement en attente de la partie précédente";
                m_pendingBotReplacement.clear();
                emit pendingBotReplacementChanged();
            }

            // Si c'est une reconnexion et qu'on a déjà un GameModel, on met à jour les cartes
            if (isReconnection && m_gameModel != nullptr) {
                qInfo() << "Reconnexion detectee - GameModel existe deja, mise a jour des cartes et adversaires";
                // Mettre à jour les cartes du joueur avec celles envoyées par le serveur
                m_gameModel->resyncCards(m_myCards);
                qInfo() << "Cartes resynchronisees:" << m_myCards.size();

                // Mettre à jour les mains des adversaires avec le cardCount correct
                m_gameModel->resyncOpponents(m_opponents);
                qInfo() << "Adversaires resynchronises:" << m_opponents.size();

                // Reprendre les timers (ils seront redémarrés par updateGameState si nécessaire)
                m_gameModel->resumeTimers();
            } else {
                // Nouvelle partie (ou relance d'app) - émettre le signal pour créer un nouveau GameModel
                emit gameDataChanged();
                emit gameFound(m_myPosition, m_opponents, isReconnection);
                qDebug() << "Signal gameFound emis - Reconnexion:" << isReconnection;
            }
        }
        else if (type == "cardPlayed") {
            int playerIndex = obj["playerIndex"].toInt();
            int cardIndex = obj["cardIndex"].toInt();
            int cardValue = obj["cardValue"].toInt();
            int cardSuit = obj["cardSuit"].toInt();

            // Créer un objet avec toutes les infos de la carte
            QJsonObject cardData;
            cardData["index"] = cardIndex;
            cardData["value"] = cardValue;
            cardData["suit"] = cardSuit;

            // Transmettre au GameModel
            if (m_gameModel) {
                m_gameModel->receivePlayerAction(playerIndex, "playCard", cardData);
            }
        }
        else if (type == "bidMade") {
            int playerIndex = obj["playerIndex"].toInt();
            int bidValue = obj["bidValue"].toInt();
            int suit = obj["suit"].toInt();

            // Transmettre au GameModel
            if (m_gameModel) {
                QJsonObject bidData;
                bidData["value"] = bidValue;
                bidData["suit"] = suit;
                m_gameModel->receivePlayerAction(playerIndex, "makeBid", bidData);
            }
        }
        else if (type == "gameState") {
            // Transmettre l'état du jeu au GameModel
            if (m_gameModel) {
                m_gameModel->updateGameState(obj);
                qDebug() << "NetworkManager - Etat du jeu mis à jour";
            }
        }
        else if (type == "pliFinished") {
            int winnerId = obj["winnerId"].toInt();
            int scoreMancheTeam1 = obj["scoreMancheTeam1"].toInt();
            int scoreMancheTeam2 = obj["scoreMancheTeam2"].toInt();

            qDebug() << "NetworkManager - Pli termine, gagnant:" << winnerId;
            qDebug() << "  Scores de manche: Team1 =" << scoreMancheTeam1 << ", Team2 =" << scoreMancheTeam2;

            // Transmettre au GameModel pour nettoyer le pli et mettre à jour les scores de manche
            if (m_gameModel) {
                QJsonObject pliData;
                pliData["winnerId"] = winnerId;
                pliData["scoreMancheTeam1"] = scoreMancheTeam1;
                pliData["scoreMancheTeam2"] = scoreMancheTeam2;
                m_gameModel->receivePlayerAction(winnerId, "pliFinished", pliData);
            }
        }
        else if (type == "mancheFinished") {
            int scoreTotalTeam1 = obj["scoreTotalTeam1"].toInt();
            int scoreTotalTeam2 = obj["scoreTotalTeam2"].toInt();
            int scoreMancheTeam1 = obj["scoreMancheTeam1"].toInt();
            int scoreMancheTeam2 = obj["scoreMancheTeam2"].toInt();
            int capotTeam = obj["capotTeam"].toInt(0);

            qDebug() << "NetworkManager - Manche terminee!";
            qDebug() << "  Scores de manche finaux: Team1 =" << scoreMancheTeam1 << ", Team2 =" << scoreMancheTeam2;
            qDebug() << "  Scores totaux: Team1 =" << scoreTotalTeam1 << ", Team2 =" << scoreTotalTeam2;

            // Transmettre au GameModel
            if (m_gameModel) {
                QJsonObject scoreData;
                scoreData["scoreTotalTeam1"] = scoreTotalTeam1;
                scoreData["scoreTotalTeam2"] = scoreTotalTeam2;
                scoreData["scoreMancheTeam1"] = scoreMancheTeam1;
                scoreData["scoreMancheTeam2"] = scoreMancheTeam2;
                scoreData["capotTeam"] = capotTeam;
                m_gameModel->receivePlayerAction(-1, "mancheFinished", scoreData);
            }
        }
        else if (type == "gameOver") {
            int winner = obj["winner"].toInt();
            int scoreTeam1 = obj["scoreTeam1"].toInt();
            int scoreTeam2 = obj["scoreTeam2"].toInt();

            qDebug() << "NetworkManager - Partie terminee! Gagnant: Equipe" << winner;
            qDebug() << "  Scores finaux: Team1 =" << scoreTeam1 << ", Team2 =" << scoreTeam2;

            // Transmettre au GameModel
            if (m_gameModel) {
                QJsonObject gameOverData;
                gameOverData["winner"] = winner;
                gameOverData["scoreTeam1"] = scoreTeam1;
                gameOverData["scoreTeam2"] = scoreTeam2;
                m_gameModel->receivePlayerAction(-1, "gameOver", gameOverData);
            }
        }
        else if (type == "newMancheAnimation") {
            qDebug() << "NetworkManager - Animation nouvelle manche!";
            emit newMancheAnimation();
        }
        else if (type == "newManche") {
            int playerPosition = obj["playerPosition"].toInt();
            int biddingPlayer = obj["biddingPlayer"].toInt();
            int currentPlayer = obj["currentPlayer"].toInt();
            QJsonArray myCards = obj["myCards"].toArray();

            qDebug() << "NetworkManager - Nouvelle manche!";
            qDebug() << "  Position:" << playerPosition;
            qDebug() << "  Joueur qui commence les enchères:" << biddingPlayer;
            qDebug() << "  Nombre de cartes:" << myCards.size();

            // Transmettre au GameModel
            if (m_gameModel) {
                QJsonObject newMancheData;
                newMancheData["playerPosition"] = playerPosition;
                newMancheData["biddingPlayer"] = biddingPlayer;
                newMancheData["currentPlayer"] = currentPlayer;
                newMancheData["myCards"] = myCards;
                m_gameModel->receivePlayerAction(-1, "newManche", newMancheData);
            }
        }
        else if (type == "surcoincheOffer") {
            int timeLeft = obj["timeLeft"].toInt();

            qDebug() << "NetworkManager - Offre de surcoinche reçue, temps restant:" << timeLeft;

            // Transmettre au GameModel
            if (m_gameModel) {
                QJsonObject surcoincheData;
                surcoincheData["timeLeft"] = timeLeft;
                m_gameModel->receivePlayerAction(-1, "surcoincheOffer", surcoincheData);
            }
        }
        else if (type == "surcoincheTimeout") {
            qDebug() << "NetworkManager - Timeout de la surcoinche";

            // Transmettre au GameModel
            if (m_gameModel) {
                m_gameModel->receivePlayerAction(-1, "surcoincheTimeout", QJsonObject());
            }
        }
        else if (type == "surcoincheTimeUpdate") {
            int timeLeft = obj["timeLeft"].toInt();

            qDebug() << "NetworkManager - Mise à jour temps surcoinche:" << timeLeft;

            // Transmettre au GameModel
            if (m_gameModel) {
                QJsonObject timeData;
                timeData["timeLeft"] = timeLeft;
                m_gameModel->receivePlayerAction(-1, "surcoincheTimeUpdate", timeData);
            }
        }
        else if (type == "surcoincheWaiting") {
            int timeLeft = obj["timeLeft"].toInt();

            qDebug() << "NetworkManager - Attente surcoinche adverse, temps restant:" << timeLeft;

            // Transmettre au GameModel
            if (m_gameModel) {
                QJsonObject waitingData;
                waitingData["timeLeft"] = timeLeft;
                m_gameModel->receivePlayerAction(-1, "surcoincheWaiting", waitingData);
            }
        }
        else if (type == "surcoincheWaitingUpdate") {
            int timeLeft = obj["timeLeft"].toInt();

            qDebug() << "NetworkManager - Mise à jour attente surcoinche:" << timeLeft;

            // Transmettre au GameModel
            if (m_gameModel) {
                QJsonObject waitingData;
                waitingData["timeLeft"] = timeLeft;
                m_gameModel->receivePlayerAction(-1, "surcoincheWaitingUpdate", waitingData);
            }
        }
        else if (type == "belote") {
            int playerIndex = obj["playerIndex"].toInt();
            qDebug() << "NetworkManager - BELOTE annoncee par joueur" << playerIndex;

            if (m_gameModel) {
                m_gameModel->receivePlayerAction(playerIndex, "belote", QJsonObject());
            }
        }
        else if (type == "rebelote") {
            int playerIndex = obj["playerIndex"].toInt();
            qDebug() << "NetworkManager - REBELOTE annoncee par joueur" << playerIndex;

            if (m_gameModel) {
                m_gameModel->receivePlayerAction(playerIndex, "rebelote", QJsonObject());
            }
        }
        else if (type == "registerAccountSuccess") {
            QString playerName = obj["playerName"].toString();
            QString avatar = obj["avatar"].toString();
            if (avatar.isEmpty()) avatar = "avataaars1.svg";
            m_playerPseudo = playerName;
            m_playerAvatar = avatar;
            qDebug() << "NetworkManager - Compte cree avec succès:" << playerName << "Avatar:" << avatar;
            emit playerAvatarChanged();
            emit playerPseudoChanged();
            emit registerSuccess(playerName, avatar);
        }
        else if (type == "registerAccountFailed") {
            QString error = obj["error"].toString();
            qDebug() << "NetworkManager - Echec creation compte:" << error;
            emit registerFailed(error);
        }
        else if (type == "loginAccountSuccess") {
            QString playerName = obj["playerName"].toString();
            QString avatar = obj["avatar"].toString();
            QString connectionId = obj["connectionId"].toString();
            if (avatar.isEmpty()) avatar = "avataaars1.svg";
            m_playerPseudo = playerName;
            m_playerAvatar = avatar;
            if (!connectionId.isEmpty()) {
                m_playerId = connectionId;
                qDebug() << "NetworkManager - ConnectionId enregistre:" << connectionId;
            }
            qDebug() << "NetworkManager - Connexion reussie:" << playerName << "Avatar:" << avatar;
            emit playerAvatarChanged();
            emit playerPseudoChanged();
            emit loginSuccess(playerName, avatar);
        }
        else if (type == "loginAccountFailed") {
            QString error = obj["error"].toString();
            qDebug() << "NetworkManager - Echec connexion:" << error;
            emit loginFailed(error);
        }
        else if (type == "deleteAccountSuccess") {
            qDebug() << "NetworkManager - Compte supprime avec succes";
            emit deleteAccountSuccess();
        }
        else if (type == "deleteAccountFailed") {
            QString error = obj["error"].toString();
            qDebug() << "NetworkManager - Echec suppression compte:" << error;
            emit deleteAccountFailed(error);
        }
        else if (type == "contactMessageSuccess") {
            qDebug() << "NetworkManager - Message de contact envoye avec succes";
            emit contactMessageSuccess();
        }
        else if (type == "contactMessageFailed") {
            QString error = obj["error"].toString();
            qDebug() << "NetworkManager - Echec envoi message de contact:" << error;
            emit contactMessageFailed(error);
        }
        else if (type == "error") {
            QString errorMsg = obj["message"].toString();
            qDebug() << "NetworkManager - Erreur recue:" << errorMsg;
            emit errorOccurred(errorMsg);
        }
        else if (type == "playerDisconnected") {
            QString playerId = obj["playerId"].toString();
            emit playerDisconnected(playerId);
        }
        else if (type == "playerForfeited") {
            int playerIndex = obj["playerIndex"].toInt();
            QString playerName = obj["playerName"].toString();
            qDebug() << "NetworkManager - Joueur" << playerName << "(index:" << playerIndex << ") a abandonné la partie";
            emit playerForfeited(playerIndex, playerName);
        }
        else if (type == "lobbyCreated") {
            QString code = obj["code"].toString();
            qDebug() << "NetworkManager - Lobby créé avec le code:" << code;
            emit lobbyCreated(code);
        }
        else if (type == "lobbyJoined") {
            QString code = obj["code"].toString();
            qDebug() << "NetworkManager - Lobby rejoint:" << code;
            emit lobbyJoined(code);
        }
        else if (type == "lobbyUpdate") {
            QJsonArray players = obj["players"].toArray();
            qDebug() << "NetworkManager - Réception lobbyUpdate avec" << players.size() << "joueurs";
            qDebug() << "NetworkManager - Contenu:" << players;

            // Créer une nouvelle liste au lieu de modifier l'ancienne
            // pour que QML détecte le changement
            QVariantList newList;

            for (const QJsonValue &playerVal : players) {
                QJsonObject playerObj = playerVal.toObject();
                QVariantMap playerMap;
                playerMap["name"] = playerObj["name"].toString();
                playerMap["avatar"] = playerObj["avatar"].toString();
                playerMap["ready"] = playerObj["ready"].toBool();
                playerMap["isHost"] = playerObj["isHost"].toBool();
                newList.append(playerMap);
            }

            m_lobbyPlayers = newList;
            qDebug() << "NetworkManager - Lobby mis à jour:" << m_lobbyPlayers.length() << "joueurs";
            emit lobbyPlayersChanged();
        }
        else if (type == "lobbyError") {
            QString errorMsg = obj["message"].toString();
            qDebug() << "NetworkManager - Erreur lobby:" << errorMsg;
            emit lobbyError(errorMsg);
        }
        else if (type == "lobbyGameStart") {
            qDebug() << "NetworkManager - La partie du lobby démarre!";
            emit lobbyGameStarting();
        }
        else if (type == "cardsDealt") {
            qDebug() << "NetworkManager - Réception des cartes!";
            QJsonArray cards = obj["cards"].toArray();
            qDebug() << "  Nombre de cartes reçues:" << cards.size();

            // Transmettre les cartes au GameModel
            if (m_gameModel) {
                m_gameModel->receiveCardsDealt(cards);
            }
        }
        else if (type == "botReplacement") {
            QString message = obj["message"].toString();
            qDebug() << "NetworkManager - Remplace par un bot:" << message;
            // Stocker le message pour que CoincheView puisse le récupérer quand il est prêt
            m_pendingBotReplacement = message;
            emit pendingBotReplacementChanged();
            // Émettre aussi le signal pour les cas où CoincheView est déjà chargé
            emit botReplacement(message);
        }
        else if (type == "rehumanizeSuccess") {
            qDebug() << "NetworkManager - Rehumanisation reussie";
            emit rehumanizeSuccess();
        }
        else if (type == "gameNoLongerExists") {
            QString message = obj["message"].toString();
            qInfo() << "NetworkManager - La partie n'existe plus:" << message;

            // Nettoyer le GameModel
            clearGameModel();

            // Émettre un signal dédié pour que QML retourne au menu principal
            emit returnToMainMenu();
        }
    }

private:
    void setupSocketConnections() {
        connect(m_socket, &QWebSocket::connected, this, &NetworkManager::onConnected);
        connect(m_socket, &QWebSocket::disconnected, this, &NetworkManager::onDisconnected);
        connect(m_socket, &QWebSocket::textMessageReceived,
                this, &NetworkManager::onMessageReceived);

        // Réception du pong pour le heartbeat
        connect(m_socket, &QWebSocket::pong, this, [this](quint64 elapsedTime, const QByteArray &payload) {
            Q_UNUSED(payload);
            m_lastPongReceived = QDateTime::currentMSecsSinceEpoch();
            qDebug() << "Pong reçu - latence:" << elapsedTime << "ms";
        });

        connect(m_socket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors),
                this, [this](const QList<QSslError> &errors) {
            qDebug() << "ERREURS SSL détectées (" << errors.size() << "):";
            for (const auto &error : errors) {
                qDebug() << "  - Type:" << error.error() << "Message:" << error.errorString();
            }
        });

        // Handler d'erreur pour diagnostiquer les déconnexions inattendues
        connect(m_socket, &QWebSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error) {
            qDebug() << "=== ERREUR SOCKET ===" << error << ":" << m_socket->errorString();
            qDebug() << "  État du socket:" << m_socket->state();
        });

        // Log de l'état de la connexion pour le debug
        connect(m_socket, &QWebSocket::stateChanged, this, [](QAbstractSocket::SocketState state) {
            QString stateStr;
            switch (state) {
                case QAbstractSocket::UnconnectedState: stateStr = "Unconnected"; break;
                case QAbstractSocket::HostLookupState: stateStr = "HostLookup"; break;
                case QAbstractSocket::ConnectingState: stateStr = "Connecting"; break;
                case QAbstractSocket::ConnectedState: stateStr = "Connected"; break;
                case QAbstractSocket::ClosingState: stateStr = "Closing"; break;
                default: stateStr = "Unknown"; break;
            }
            qDebug() << "Socket state changed:" << stateStr;
        });
    }

    // Envoie un ping et vérifie si la connexion est toujours active
    void sendHeartbeat() {
        if (!m_connected || !m_socket) {
            return;
        }

        qint64 now = QDateTime::currentMSecsSinceEpoch();

        // Si on n'a pas reçu de pong depuis plus de 15 secondes, la connexion est morte
        if (m_lastPongReceived > 0 && (now - m_lastPongReceived) > 15000) {
            qDebug() << "=== HEARTBEAT TIMEOUT - Connexion morte détectée ===";
            qDebug() << "  Dernier pong reçu il y a" << (now - m_lastPongReceived) / 1000 << "secondes";

            // Arrêter le heartbeat
            m_heartbeatTimer->stop();

            // Forcer la déconnexion et déclencher la reconnexion
            m_socket->abort();  // Fermer brutalement le socket
            onDisconnected();   // Déclencher manuellement le handler de déconnexion
            return;
        }

        // Envoyer un ping
        qDebug() << "Envoi ping heartbeat...";
        m_socket->ping();
    }

    // Ouvre le socket avec une configuration SSL fraîche qui évite le cache de session
    void openSocket(const QString &url) {
        QUrl qurl(url);

        if (url.startsWith("wss://")) {
            QNetworkRequest request(qurl);

            qDebug() << "Ouverture socket";
            m_socket->open(request);
        } else {
            // Connexion non sécurisée (ws://)
            m_socket->open(qurl);
        }
    }

    void sendMessage(const QJsonObject &message) {
        if (!m_connected) {
            qDebug() << "Erreur: non connecte au serveur";
            return;
        }

        QJsonDocument doc(message);
        m_socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    }

    QWebSocket *m_socket;
    bool m_connected;
    QString m_playerId;
    QString m_matchmakingStatus;
    int m_playersInQueue;
    int m_matchmakingCountdown;

    QJsonArray m_myCards;
    int m_myPosition;
    QJsonArray m_opponents;

    GameModel* m_gameModel;  // Le GameModel géré par NetworkManager
    QString m_playerPseudo;
    QString m_playerAvatar;
    QString m_pendingBotReplacement;  // Message de remplacement par bot en attente

    // Lobbies privés
    QVariantList m_lobbyPlayers;

    // Reconnexion automatique
    QString m_serverUrl;
    bool m_wasInGame;

    // Heartbeat pour détecter les connexions mortes
    QTimer* m_heartbeatTimer;
    qint64 m_lastPongReceived;
};

#endif // NETWORKMANAGER_H