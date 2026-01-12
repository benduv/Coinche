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
#include "GameModel.h"

class NetworkManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString matchmakingStatus READ matchmakingStatus NOTIFY matchmakingStatusChanged)
    Q_PROPERTY(int playersInQueue READ playersInQueue NOTIFY playersInQueueChanged)
    Q_PROPERTY(QJsonArray myCards READ myCards NOTIFY gameDataChanged)
    Q_PROPERTY(int myPosition READ myPosition NOTIFY gameDataChanged)
    Q_PROPERTY(QJsonArray opponents READ opponents NOTIFY gameDataChanged)
    Q_PROPERTY(QString playerAvatar READ playerAvatar NOTIFY playerAvatarChanged)
    Q_PROPERTY(QVariantList lobbyPlayers READ lobbyPlayers NOTIFY lobbyPlayersChanged)

public:
    explicit NetworkManager(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QWebSocket)
        , m_connected(false)
        , m_playersInQueue(0)
        , m_myPosition(-1)
        , m_gameModel(nullptr)
        , m_playerAvatar("avataaars1.svg")
        , m_pingTimer(new QTimer(this))
        , m_lastPongReceived(QDateTime::currentMSecsSinceEpoch())
        , m_reconnectTimer(new QTimer(this))
        , m_wasInGame(false)
        , m_reconnectAttempts(0)
    {
        connect(m_socket, &QWebSocket::connected, this, &NetworkManager::onConnected);
        connect(m_socket, &QWebSocket::disconnected, this, &NetworkManager::onDisconnected);
        connect(m_socket, &QWebSocket::textMessageReceived,
                this, &NetworkManager::onMessageReceived);
        connect(m_socket, &QWebSocket::pong, this, &NetworkManager::onPongReceived);

        // Timer pour envoyer des pings toutes les 5 secondes
        connect(m_pingTimer, &QTimer::timeout, this, &NetworkManager::sendPing);
        m_pingTimer->setInterval(5000);

        // Timer pour les tentatives de reconnexion
        connect(m_reconnectTimer, &QTimer::timeout, this, &NetworkManager::attemptReconnect);
        m_reconnectTimer->setInterval(3000); // Tenter de se reconnecter toutes les 3 secondes
    }

    ~NetworkManager() {
        m_socket->close();
        delete m_socket;
        if (m_gameModel) {
            delete m_gameModel;
        }
    }

    bool connected() const { return m_connected; }
    QString matchmakingStatus() const { return m_matchmakingStatus; }
    int playersInQueue() const { return m_playersInQueue; }
    QJsonArray myCards() const { return m_myCards; }
    int myPosition() const { return m_myPosition; }
    QJsonArray opponents() const { return m_opponents; }
    GameModel* gameModel() const { return m_gameModel; }
    QString playerAvatar() const { return m_playerAvatar; }
    QVariantList lobbyPlayers() const { return m_lobbyPlayers; }

    // Nouvelle méthode pour créer le GameModel
    Q_INVOKABLE void createGameModel(int position, const QJsonArray& cards, const QJsonArray& opps) {
        qDebug() << "Creation du GameModel en C++";
        
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

        // Initialiser avec les données en passant le pseudo du joueur
        m_gameModel->initOnlineGame(position, cards, opps, m_playerPseudo);

        // Définir l'avatar du joueur local
        m_gameModel->setPlayerAvatar(position, m_playerAvatar);

        qDebug() << "GameModel cree et initialise - Pseudo:" << m_playerPseudo << "Avatar:" << m_playerAvatar;

        // Émettre signal pour que QML navigue vers CoincheView
        emit gameModelReady();
    }

    Q_INVOKABLE void connectToServer(const QString &url) {
        qDebug() << "Connexion au serveur:" << url;
        m_serverUrl = url;  // Sauvegarder l'URL pour les reconnexions
        m_socket->open(QUrl(url));
    }

    Q_INVOKABLE void registerPlayer(const QString &playerName, const QString &avatar = "avataaars1.svg") {
        QJsonObject msg;
        msg["type"] = "register";
        msg["playerName"] = playerName;
        msg["avatar"] = avatar;
        sendMessage(msg);
    }

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

    Q_INVOKABLE void requestStats(const QString &pseudo) {
        QJsonObject msg;
        msg["type"] = "getStats";
        msg["pseudo"] = pseudo;
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

signals:
    void connectedChanged();
    void matchmakingStatusChanged();
    void playersInQueueChanged();
    void gameDataChanged();
    void gameFound(int playerPosition, QJsonArray opponents);
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
    void messageReceived(QString message);  // Pour que QML puisse écouter tous les messages
    void playerAvatarChanged();

    // Signaux pour les lobbies privés
    void lobbyCreated(QString code);
    void lobbyJoined(QString code);
    void lobbyError(QString message);
    void lobbyPlayersChanged();
    void lobbyGameStarting();

    // Signaux pour le remplacement par bot
    void botReplacement(QString message);
    void rehumanizeSuccess();

private slots:
    void onConnected() {
        qDebug() << "Connecte au serveur";
        m_connected = true;
        m_lastPongReceived = QDateTime::currentMSecsSinceEpoch();
        m_pingTimer->start();  // Démarrer le ping timer
        m_reconnectTimer->stop();  // Arrêter les tentatives de reconnexion
        m_reconnectAttempts = 0;  // Réinitialiser le compteur

        // Si on se reconnecte et qu'on avait un pseudo et avatar, se réenregistrer
        if (!m_playerPseudo.isEmpty()) {
            qDebug() << "Reconnexion - Réenregistrement avec pseudo:" << m_playerPseudo;
            registerPlayer(m_playerPseudo, m_playerAvatar);
        }

        emit connectedChanged();
    }

    void onDisconnected() {
        qDebug() << "Deconnecte du serveur";
        bool wasConnected = m_connected;
        m_connected = false;
        m_pingTimer->stop();  // Arrêter le ping timer

        // Si on était en partie ou connecté, activer la reconnexion automatique
        if (wasConnected && (!m_playerPseudo.isEmpty() || m_gameModel != nullptr)) {
            qDebug() << "Perte de connexion detectee - Demarrage tentatives de reconnexion";
            m_wasInGame = (m_gameModel != nullptr);  // Sauvegarder si on était en partie
            m_reconnectAttempts = 0;
            m_reconnectTimer->start();  // Démarrer les tentatives de reconnexion
        }

        emit connectedChanged();
    }

    void onPongReceived(quint64 elapsedTime, const QByteArray &payload) {
        Q_UNUSED(elapsedTime)
        Q_UNUSED(payload)
        m_lastPongReceived = QDateTime::currentMSecsSinceEpoch();
    }

    void sendPing() {
        if (m_socket->state() == QAbstractSocket::ConnectedState) {
            // Vérifier si on a reçu un pong récemment (dans les 15 dernières secondes)
            qint64 timeSinceLastPong = QDateTime::currentMSecsSinceEpoch() - m_lastPongReceived;
            if (timeSinceLastPong > 15000) {
                // Pas de pong depuis 15 secondes, considérer la connexion comme perdue
                qDebug() << "Aucun pong reçu depuis" << timeSinceLastPong << "ms, déconnexion détectée";
                m_socket->close();
                onDisconnected();
            } else {
                // Envoyer un ping
                m_socket->ping();
            }
        } else if (m_connected) {
            // Socket pas connecté mais m_connected est true, forcer la déconnexion
            qDebug() << "Socket déconnecté détecté, mise à jour de l'état";
            onDisconnected();
        }
    }

    void attemptReconnect() {
        // Limiter le nombre de tentatives (par exemple, 20 tentatives = 1 minute)
        const int MAX_ATTEMPTS = 20;

        if (m_reconnectAttempts >= MAX_ATTEMPTS) {
            qDebug() << "Nombre maximum de tentatives de reconnexion atteint (" << MAX_ATTEMPTS << ")";
            m_reconnectTimer->stop();
            m_wasInGame = false;
            return;
        }

        m_reconnectAttempts++;
        qDebug() << "Tentative de reconnexion" << m_reconnectAttempts << "/" << MAX_ATTEMPTS;

        // Si le socket n'est pas en train de se connecter, tenter la connexion
        if (m_socket->state() != QAbstractSocket::ConnectingState &&
            m_socket->state() != QAbstractSocket::ConnectedState) {
            qDebug() << "Reconnexion à" << m_serverUrl;
            m_socket->open(QUrl(m_serverUrl));
        }
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
            emit matchmakingStatusChanged();
            emit playersInQueueChanged();
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

            // Si c'est une reconnexion et qu'on a déjà un GameModel, on le met à jour au lieu d'en créer un nouveau
            if (isReconnection && m_gameModel != nullptr) {
                qDebug() << "Reconnexion detectee - GameModel existe deja, pas besoin de mise a jour";
                qDebug() << "Le message gameState qui suit va tout mettre a jour (currentPlayer, atout, scores, cartes jouees, etc.)";
                // Note: Les cartes seront mises à jour directement par le message gameState du serveur
                // qui contient les playableCards, donc pas besoin de receiveCardsDealt ici
            } else {
                // Nouvelle partie - émettre le signal pour créer un nouveau GameModel
                emit gameDataChanged();
                emit gameFound(m_myPosition, m_opponents);
                qDebug() << "Signal gameFound emis";
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
            emit loginSuccess(playerName, avatar);
        }
        else if (type == "loginAccountFailed") {
            QString error = obj["error"].toString();
            qDebug() << "NetworkManager - Echec connexion:" << error;
            emit loginFailed(error);
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
            emit botReplacement(message);
        }
        else if (type == "rehumanizeSuccess") {
            qDebug() << "NetworkManager - Rehumanisation reussie";
            emit rehumanizeSuccess();
        }
    }

private:
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
    
    QJsonArray m_myCards;
    int m_myPosition;
    QJsonArray m_opponents;

    GameModel* m_gameModel;  // Le GameModel géré par NetworkManager
    QString m_playerPseudo;
    QString m_playerAvatar;

    // Lobbies privés
    QVariantList m_lobbyPlayers;

    // Ping/Pong pour détecter les déconnexions
    QTimer* m_pingTimer;
    qint64 m_lastPongReceived;

    // Reconnexion automatique
    QTimer* m_reconnectTimer;
    QString m_serverUrl;
    bool m_wasInGame;
    int m_reconnectAttempts;
};

#endif // NETWORKMANAGER_H