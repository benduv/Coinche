#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslKey>
#include <QFile>
#include <QMap>
#include <QQueue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QRandomGenerator>
#include "Player.h"
#include "Deck.h"
#include "Carte.h"
#include "GameModel.h"
#include "DatabaseManager.h"

// Connexion réseau d'un joueur (pas la logique métier)
struct PlayerConnection {
    QWebSocket* socket;
    QString connectionId;      // ID unique WebSocket
    QString playerName;
    QString avatar;            // Avatar du joueur
    int gameRoomId;
    int playerIndex;           // Position dans la partie (0-3)
    QString lobbyPartnerId;    // ID du partenaire de lobby (vide si pas de partenaire)
};

// Une partie de jeu avec la vraie logique
struct GameRoom {
    int roomId;
    QList<QString> connectionIds;  // IDs des connexions WebSocket actuelles
    QList<QString> originalConnectionIds;  // IDs originaux pour reconnexion
    QList<QString> playerNames;  // Noms des joueurs pour reconnexion
    QList<QString> playerAvatars;  // Avatars des joueurs pour reconnexion
    QString gameState; // "waiting", "bidding", "playing", "finished"

    // Les objets de jeu réels
    std::vector<std::unique_ptr<Player>> players;  // Les 4 joueurs
    std::vector<bool> isBot;  // true si le joueur à cet index est un bot
    Deck deck;

    // État de la partie
    Carte::Couleur couleurAtout = Carte::COULEURINVALIDE;
    bool isToutAtout = false;  // Mode Tout Atout : toutes les cartes sont des atouts
    bool isSansAtout = false;  // Mode Sans Atout : aucune carte n'est atout
    int currentPlayerIndex = 0;
    int biddingPlayer = 0;
    int firstPlayerIndex = 0;  // Joueur qui commence les enchères ET qui jouera en premier

    // Gestion des enchères
    int passedBidsCount = 0;
    Player::Annonce lastBidAnnonce = Player::ANNONCEINVALIDE;
    Carte::Couleur lastBidCouleur = Carte::COULEURINVALIDE;
    int lastBidderIndex = -1;
    bool coinched = false;  // True si COINCHE a été annoncé
    bool surcoinched = false;  // True si SURCOINCHE a été annoncé
    QTimer* surcoincheTimer = nullptr;  // Timer pour le timeout de surcoinche
    int surcoincheTimeLeft = 0;  // Temps restant en secondes
    int coinchePlayerIndex = -1;  // Index du joueur qui a coinché
    int surcoinchePlayerIndex = -1;  // Index du joueur qui a surcoinché

    // Timer pour détecter les joueurs qui ne répondent pas
    QTimer* turnTimeout = nullptr;  // Timer pour timeout du tour (15 secondes)
    int turnTimeoutGeneration = 0;  // Compteur pour invalider les anciens timeouts

    // Timer pour détecter les joueurs qui ne répondent pas pendant les enchères
    QTimer* bidTimeout = nullptr;  // Timer pour timeout des enchères (15 secondes)
    int bidTimeoutGeneration = 0;  // Compteur pour invalider les anciens timeouts d'enchères

    // Pli en cours
    std::vector<std::pair<int, Carte*>> currentPli;  // pair<playerIndex, carte>
    Carte::Couleur couleurDemandee = Carte::COULEURINVALIDE;
    bool waitingForNextPli = false;  // True pendant l'attente de 1500ms entre les plis

    // Scores
    int scoreTeam1 = 0;  // Équipe 0: Joueurs 0 et 2
    int scoreTeam2 = 0;  // Équipe 1: Joueurs 1 et 3
    int scoreMancheTeam1 = 0;
    int scoreMancheTeam2 = 0;

    // Plis gagnés dans la manche en cours
    std::vector<std::pair<int, Carte*>> plisTeam1;  // Cartes gagnées par équipe 1
    std::vector<std::pair<int, Carte*>> plisTeam2;  // Cartes gagnées par équipe 2

    // Compteur de plis par joueur (pour CAPOT et GENERALE)
    int plisCountPlayer0 = 0;
    int plisCountPlayer1 = 0;
    int plisCountPlayer2 = 0;
    int plisCountPlayer3 = 0;

    // Belote (détectée au début de la phase de jeu)
    bool beloteTeam1 = false;
    bool beloteTeam2 = false;

    // Suivi des cartes de la belote jouées (pour animations)
    bool beloteRoiJoue = false;   // Roi de l'atout joué
    bool beloteDameJouee = false; // Dame de l'atout jouée

    // Tracking des cartes jouées pour l'IA des bots
    // Structure: playedCards[couleur][chiffre] = true si la carte a été jouée
    std::map<Carte::Couleur, std::map<Carte::Chiffre, bool>> playedCards;

    // Initialise le tracking des cartes jouées (à appeler au début de chaque manche)
    void resetPlayedCards() {
        playedCards.clear();
        std::array<Carte::Couleur, 4> couleurs = {Carte::COEUR, Carte::TREFLE, Carte::CARREAU, Carte::PIQUE};
        std::array<Carte::Chiffre, 8> chiffres = {Carte::SEPT, Carte::HUIT, Carte::NEUF, Carte::DIX,
                                                   Carte::VALET, Carte::DAME, Carte::ROI, Carte::AS};
        for (auto couleur : couleurs) {
            for (auto chiffre : chiffres) {
                playedCards[couleur][chiffre] = false;
            }
        }
    }

    // Marque une carte comme jouée
    void markCardAsPlayed(Carte* carte) {
        if (carte) {
            playedCards[carte->getCouleur()][carte->getChiffre()] = true;
        }
    }

    // Vérifie si une carte a été jouée
    bool isCardPlayed(Carte::Couleur couleur, Carte::Chiffre chiffre) const {
        auto itCouleur = playedCards.find(couleur);
        if (itCouleur != playedCards.end()) {
            auto itChiffre = itCouleur->second.find(chiffre);
            if (itChiffre != itCouleur->second.end()) {
                return itChiffre->second;
            }
        }
        return false;
    }

    GameModel* gameModel = nullptr;
};

// Lobby privé pour jouer avec des amis
struct PrivateLobby {
    QString code;  // Code à 4 caractères
    QString hostPlayerName;  // Nom du joueur hôte
    QList<QString> playerNames;  // Noms des joueurs dans le lobby
    QList<QString> playerAvatars;  // Avatars des joueurs
    QList<bool> readyStatus;  // Statut "prêt" de chaque joueur
};

class GameServer : public QObject {
    Q_OBJECT

public:
    // Constructeur avec support SSL optionnel
    // Si certPath et keyPath sont fournis, le serveur démarre en mode sécurisé (WSS)
    // Sinon, il démarre en mode non sécurisé (WS)
    explicit GameServer(quint16 port, QObject *parent = nullptr,
                        const QString &certPath = QString(),
                        const QString &keyPath = QString())
        : QObject(parent)
        , m_server(nullptr)
        , m_nextRoomId(1)
        , m_dbManager(new DatabaseManager(this))
    {
        // Initialiser la base de donnees
        if (!m_dbManager->initialize("coinche.db")) {
            qCritical() << "Echec de l'initialisation de la base de donnees";
        }

        // Déterminer le mode (sécurisé ou non)
        bool useSecureMode = !certPath.isEmpty() && !keyPath.isEmpty();

        if (useSecureMode) {
            // Mode sécurisé WSS
            m_server = new QWebSocketServer("CoinchServer", QWebSocketServer::SecureMode, this);

            // Charger le certificat SSL
            QFile certFile(certPath);
            if (!certFile.open(QIODevice::ReadOnly)) {
                qCritical() << "Impossible d'ouvrir le certificat:" << certPath;
                return;
            }
            QSslCertificate certificate(&certFile, QSsl::Pem);
            certFile.close();

            // Charger la clé privée
            QFile keyFile(keyPath);
            if (!keyFile.open(QIODevice::ReadOnly)) {
                qCritical() << "Impossible d'ouvrir la clé privée:" << keyPath;
                return;
            }
            QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
            keyFile.close();

            // Configurer SSL
            QSslConfiguration sslConfig;
            sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
            sslConfig.setLocalCertificate(certificate);
            sslConfig.setPrivateKey(sslKey);
            sslConfig.setProtocol(QSsl::TlsV1_2OrLater);

            m_server->setSslConfiguration(sslConfig);

            qDebug() << "Mode sécurisé (WSS) activé";
        } else {
            // Mode non sécurisé WS (pour développement local)
            m_server = new QWebSocketServer("CoinchServer", QWebSocketServer::NonSecureMode, this);
            qDebug() << "Mode non sécurisé (WS) - Utilisez SSL en production!";
        }

        if (m_server->listen(QHostAddress::Any, port)) {
            qDebug() << "Serveur demarre sur le port" << port << (useSecureMode ? "(WSS)" : "(WS)");
            connect(m_server, &QWebSocketServer::newConnection,
                    this, &GameServer::onNewConnection);
        } else {
            qDebug() << "Erreur: impossible de demarrer le serveur";
        }

        // Initialiser le timer de matchmaking avec bots (30 secondes)
        m_matchmakingTimer = new QTimer(this);
        m_matchmakingTimer->setInterval(30000);  // 30 secondes
        m_lastQueueSize = 0;
        connect(m_matchmakingTimer, &QTimer::timeout, this, &GameServer::onMatchmakingTimeout);
    }

    ~GameServer() {
        m_server->close();
        
        // Libére toutes les GameRooms
        qDeleteAll(m_gameRooms.values());
        
        // Libére toutes les connexions
        qDeleteAll(m_connections.values());
    }

private slots:
    void onNewConnection() {
        QWebSocket *socket = m_server->nextPendingConnection();
        
        qDebug() << "Nouvelle connexion depuis" << socket->peerAddress();
        
        connect(socket, &QWebSocket::textMessageReceived,
                this, &GameServer::onTextMessageReceived);
        connect(socket, &QWebSocket::disconnected,
                this, &GameServer::onDisconnected);
        
        // Envoi un message de bienvenue
        QJsonObject welcome;
        welcome["type"] = "connected";
        welcome["message"] = "Connecté au serveur";
        sendMessage(socket, welcome);
    }

    void onTextMessageReceived(const QString &message) {
        QWebSocket *sender = qobject_cast<QWebSocket*>(this->sender());
        if (!sender) return;

        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();

        qDebug() << "GameServer - Message recu:" << type;

        if (type == "register") {
            handleRegister(sender, obj);
        } else if (type == "registerAccount") {
            handleRegisterAccount(sender, obj);
        } else if (type == "loginAccount") {
            handleLoginAccount(sender, obj);
        } else if (type == "getStats") {
            handleGetStats(sender, obj);
        } else if (type == "joinMatchmaking") {
            handleJoinMatchmaking(sender);
        } else if (type == "leaveMatchmaking") {
            handleLeaveMatchmaking(sender);
        } else if (type == "playCard") {
            handlePlayCard(sender, obj);
        } else if (type == "makeBid") {
            handleMakeBid(sender, obj);
        } else if (type == "forfeit") {
            handleForfeit(sender);
        } else if (type == "createPrivateLobby") {
            handleCreatePrivateLobby(sender);
        } else if (type == "joinPrivateLobby") {
            handleJoinPrivateLobby(sender, obj);
        } else if (type == "lobbyReady") {
            handleLobbyReady(sender, obj);
        } else if (type == "startLobbyGame") {
            handleStartLobbyGame(sender);
        } else if (type == "leaveLobby") {
            handleLeaveLobby(sender);
        } else if (type == "updateAvatar") {
            handleUpdateAvatar(sender, obj);
        } else if (type == "rehumanize") {
            handleRehumanize(sender);
        }
    }

    void onDisconnected() {
        QWebSocket *socket = qobject_cast<QWebSocket*>(this->sender());
        if (!socket) return;

        qDebug() << "Client deconnecte";

        // Trouve la connexion
        QString connectionId;
        bool wasInQueue = false;
        for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
            if (it.value()->socket == socket) {
                connectionId = it.key();

                // Retire de la file d'attente et note si le joueur était en attente
                wasInQueue = m_matchmakingQueue.contains(connectionId);
                m_matchmakingQueue.removeAll(connectionId);

                // Notifie la room si le joueur était en partie
                if (it.value()->gameRoomId != -1) {
                    handlePlayerDisconnect(connectionId);
                }

                delete it.value();
                m_connections.erase(it);
                break;
            }
        }

        // Si le joueur était dans la queue, notifier les autres joueurs
        if (wasInQueue) {
            qDebug() << "Joueur deconnecte etait dans la queue. Queue size:" << m_matchmakingQueue.size();

            QJsonObject updateResponse;
            updateResponse["type"] = "matchmakingStatus";
            updateResponse["status"] = "searching";
            updateResponse["playersInQueue"] = m_matchmakingQueue.size();

            for (const QString &queuedConnectionId : m_matchmakingQueue) {
                if (m_connections.contains(queuedConnectionId)) {
                    sendMessage(m_connections[queuedConnectionId]->socket, updateResponse);
                }
            }
        }

        socket->deleteLater();
    }

private:
    // Calcule la valeur d'une carte en mode Tout Atout
    int getCardValueToutAtout(Carte* carte) const {
        if (!carte) return 0;

        Carte::Chiffre chiffre = carte->getChiffre();
        switch (chiffre) {
            case Carte::SEPT:   return 0;
            case Carte::HUIT:   return 0;
            case Carte::DAME:   return 1;  // Dame vaut 1 point en TA
            case Carte::ROI:    return 3;
            case Carte::DIX:    return 5;
            case Carte::AS:     return 6;  // As vaut 6 points en TA
            case Carte::NEUF:   return 9;
            case Carte::VALET:  return 14;
            default:            return 0;
        }
    }

    // Calcule la valeur d'une carte en mode Sans Atout
    int getCardValueSansAtout(Carte* carte) const {
        if (!carte) return 0;

        Carte::Chiffre chiffre = carte->getChiffre();
        switch (chiffre) {
            case Carte::SEPT:   return 0;
            case Carte::HUIT:   return 0;
            case Carte::NEUF:   return 0;
            case Carte::VALET:  return 2;
            case Carte::DAME:   return 3;
            case Carte::ROI:    return 4;
            case Carte::DIX:    return 10;
            case Carte::AS:     return 19;
            default:            return 0;
        }
    }

    void handleRegister(QWebSocket *socket, const QJsonObject &data) {
        QString connectionId = QUuid::createUuid().toString();
        QString playerName = data["playerName"].toString();
        QString avatar = data["avatar"].toString();
        if (avatar.isEmpty()) avatar = "avataaars1.svg";

        // Si le joueur n'a pas de nom (invité), générer un nom unique
        if (playerName.isEmpty()) {
            static int guestCounter = 0;
            playerName = QString("Invité %1").arg(++guestCounter);
        }

        PlayerConnection *conn = new PlayerConnection{
            socket,
            connectionId,
            playerName,
            avatar,
            -1,    // Pas encore en partie
            -1     // Pas encore de position
        };
        m_connections[connectionId] = conn;

        QJsonObject response;
        response["type"] = "registered";
        response["connectionId"] = connectionId;
        response["playerName"] = playerName;
        response["avatar"] = avatar;
        sendMessage(socket, response);

        qDebug() << "Joueur enregistre:" << playerName << "Avatar:" << avatar << "ID:" << connectionId;

        // Vérifier si le joueur peut se reconnecter à une partie en cours
        qDebug() << "Verification reconnexion pour" << playerName << "- m_playerNameToRoomId.contains:" << m_playerNameToRoomId.contains(playerName);
        if (m_playerNameToRoomId.contains(playerName)) {
            int roomId = m_playerNameToRoomId[playerName];
            GameRoom* room = m_gameRooms.value(roomId);
            qDebug() << "RoomId:" << roomId << "Room valide:" << (room != nullptr) << "GameState:" << (room ? room->gameState : "N/A");

            if (room && room->gameState != "finished") {
                // Trouver l'index du joueur dans la partie
                int playerIndex = -1;
                for (int i = 0; i < room->playerNames.size(); i++) {
                    if (room->playerNames[i] == playerName) {
                        playerIndex = i;
                        break;
                    }
                }

                qDebug() << "PlayerIndex:" << playerIndex << "isBot:" << (playerIndex != -1 ? room->isBot[playerIndex] : false);

                // Reconnexion si:
                // 1. Le joueur est marqué comme bot (déconnexion détectée par le serveur)
                // 2. OU le joueur a une ancienne connexion différente (reconnexion rapide avant détection)
                bool isDifferentConnection = (playerIndex != -1 &&
                                             playerIndex < room->connectionIds.size() &&
                                             room->connectionIds[playerIndex] != connectionId);

                if (playerIndex != -1 && (room->isBot[playerIndex] || isDifferentConnection)) {
                    qDebug() << "Reconnexion detectee pour" << playerName << "a la partie" << roomId << "position" << playerIndex;
                    qDebug() << "  isBot:" << room->isBot[playerIndex] << "isDifferentConnection:" << isDifferentConnection;
                    handleReconnection(connectionId, roomId, playerIndex);
                } else if (playerIndex == -1) {
                    qDebug() << "ERREUR: Joueur" << playerName << "dans m_playerNameToRoomId mais pas trouve dans room->playerNames";
                } else {
                    qDebug() << "Joueur" << playerName << "deja connecte avec la meme connexion";
                }
            }
        }
    }

    void handleReconnection(const QString& connectionId, int roomId, int playerIndex) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        PlayerConnection* conn = m_connections[connectionId];
        if (!conn) return;

        qDebug() << "GameServer - Reconnexion du joueur" << playerIndex << "(" << conn->playerName << ") a la partie" << roomId;

        // Mettre à jour la connexion
        conn->gameRoomId = roomId;
        conn->playerIndex = playerIndex;

        // Remplacer l'ID de connexion dans la room
        room->connectionIds[playerIndex] = connectionId;

        // Vérifier si le joueur était un bot (remplacé pendant sa déconnexion)
        bool wasBot = room->isBot[playerIndex];
        // Note: On enverra la notification botReplacement APRÈS gameFound et gameState
        // pour que le client ait le temps de se configurer

        // Préparer les données de l'état actuel de la partie pour le joueur qui se reconnecte
        QJsonObject reconnectMsg;
        reconnectMsg["type"] = "gameFound";  // Même message que pour démarrer une partie
        reconnectMsg["roomId"] = roomId;
        reconnectMsg["playerPosition"] = playerIndex;
        reconnectMsg["reconnection"] = true;  // Marquer comme reconnexion

        // Envoyer les cartes actuelles du joueur
        QJsonArray myCards;
        const auto& playerHand = room->players[playerIndex]->getMain();
        for (const auto* carte : playerHand) {
            if (carte) {
                QJsonObject cardObj;
                cardObj["value"] = static_cast<int>(carte->getChiffre());
                cardObj["suit"] = static_cast<int>(carte->getCouleur());
                myCards.append(cardObj);
            }
        }
        reconnectMsg["myCards"] = myCards;

        // Infos sur les adversaires
        QJsonArray opponents;
        for (int j = 0; j < room->connectionIds.size(); j++) {
            if (playerIndex != j) {
                QJsonObject opp;
                opp["position"] = j;
                opp["name"] = room->playerNames[j];
                opp["avatar"] = room->playerAvatars[j];
                opp["cardCount"] = int(room->players[j]->getMain().size());
                opponents.append(opp);
            }
        }
        reconnectMsg["opponents"] = opponents;

        sendMessage(conn->socket, reconnectMsg);

        // Notifier tous les autres joueurs de la reconnexion
        QJsonObject playerReconnectedMsg;
        playerReconnectedMsg["type"] = "playerReconnected";
        playerReconnectedMsg["playerIndex"] = playerIndex;
        playerReconnectedMsg["playerName"] = conn->playerName;
        broadcastToRoom(roomId, playerReconnectedMsg, connectionId);

        qDebug() << "GameServer - Joueur" << playerIndex << "reconnecte avec succes";

        // Annuler la défaite enregistrée lors de la déconnexion
        if (!conn->playerName.isEmpty()) {
            // Décrémenter le compteur de parties jouées (annule la défaite)
            m_dbManager->cancelDefeat(conn->playerName);
            qDebug() << "Stats corrigees pour" << conn->playerName << "- Defaite annulee";
        }

        // Envoyer l'état actuel du jeu
        if (room->gameState == "bidding") {
            QJsonObject stateMsg;
            stateMsg["type"] = "gameState";
            stateMsg["currentPlayer"] = room->currentPlayerIndex;
            stateMsg["biddingPlayer"] = room->biddingPlayer;
            stateMsg["biddingPhase"] = true;
            sendMessage(conn->socket, stateMsg);
        } else if (room->gameState == "playing") {
            // Envoyer l'état de jeu avec l'atout et les cartes jouables si c'est son tour
            QJsonObject stateMsg;
            stateMsg["type"] = "gameState";
            stateMsg["biddingPhase"] = false;
            stateMsg["currentPlayer"] = room->currentPlayerIndex;
            stateMsg["atout"] = static_cast<int>(room->couleurAtout);
            stateMsg["isToutAtout"] = room->isToutAtout;
            stateMsg["isSansAtout"] = room->isSansAtout;

            // Envoyer les scores (manche et total)
            // Note: scoreTeam1/scoreTeam2 sont les scores TOTAUX de la partie
            // scoreMancheTeam1/scoreMancheTeam2 sont les scores de la manche en cours
            stateMsg["scoreTeam1"] = room->scoreMancheTeam1;
            stateMsg["scoreTeam2"] = room->scoreMancheTeam2;
            stateMsg["scoreTotalTeam1"] = room->scoreTeam1;
            stateMsg["scoreTotalTeam2"] = room->scoreTeam2;

            // Note: Les cartes du joueur sont déjà envoyées dans gameFound.myCards
            // Pas besoin de les renvoyer ici, cela évite la duplication

            // Si c'est le tour du joueur, envoyer aussi les cartes jouables
            if (room->currentPlayerIndex == playerIndex) {
                QJsonArray playableCards = calculatePlayableCards(room, playerIndex);
                stateMsg["playableCards"] = playableCards;
            }

            // RECONNEXION: Envoyer le pli en cours pour resynchroniser l'affichage
            if (!room->currentPli.empty()) {
                QJsonArray reconnectionPli;
                for (const auto& cardPlay : room->currentPli) {
                    QJsonObject cardObj;
                    cardObj["playerIndex"] = cardPlay.first;
                    cardObj["value"] = static_cast<int>(cardPlay.second->getChiffre());
                    cardObj["suit"] = static_cast<int>(cardPlay.second->getCouleur());
                    reconnectionPli.append(cardObj);
                }
                stateMsg["reconnectionPli"] = reconnectionPli;
                qDebug() << "GameServer - Reconnexion: Envoi du pli en cours avec" << reconnectionPli.size() << "cartes";
            }

            sendMessage(conn->socket, stateMsg);
        }

        // IMPORTANT: Envoyer botReplacement APRÈS gameFound et gameState
        // pour que le client ait le temps de créer le GameModel et charger CoincheView
        if (wasBot) {
            qDebug() << "GameServer - Le joueur" << playerIndex << "était un bot, envoi de la notification (après gameState)";
            // Utiliser un petit délai pour laisser le temps au client de se configurer
            QTimer::singleShot(500, this, [this, connectionId, playerIndex]() {
                if (!m_connections.contains(connectionId)) return;
                PlayerConnection* conn = m_connections[connectionId];
                if (!conn || !conn->socket) return;

                QJsonObject notification;
                notification["type"] = "botReplacement";
                notification["message"] = "Vous avez été remplacé par un bot pendant votre absence.";
                sendMessage(conn->socket, notification);
                qDebug() << "GameServer - Notification botReplacement envoyée au joueur" << playerIndex;
            });
        }
    }

    void handleUpdateAvatar(QWebSocket *socket, const QJsonObject &data) {
        QString newAvatar = data["avatar"].toString();
        if (newAvatar.isEmpty()) {
            qDebug() << "Avatar vide reçu, ignoré";
            return;
        }

        // Trouver la connexion du joueur
        QString connectionId;
        for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
            if (it.value()->socket == socket) {
                connectionId = it.key();
                break;
            }
        }

        if (connectionId.isEmpty()) {
            qDebug() << "Impossible de trouver la connexion pour la mise à jour d'avatar";
            return;
        }

        PlayerConnection* conn = m_connections[connectionId];
        conn->avatar = newAvatar;

        qDebug() << "Avatar mis à jour pour" << conn->playerName << ":" << newAvatar;

        // Confirmation au client
        QJsonObject response;
        response["type"] = "avatarUpdated";
        response["avatar"] = newAvatar;
        sendMessage(socket, response);

        // Si le joueur est dans un lobby, notifier les autres joueurs
        for (auto it = m_privateLobbies.begin(); it != m_privateLobbies.end(); ++it) {
            PrivateLobby* lobby = it.value();
            bool playerInLobby = false;

            // Vérifier si le joueur est dans ce lobby
            for (const QString& playerName : lobby->playerNames) {
                if (playerName == conn->playerName) {
                    playerInLobby = true;

                    // Mettre à jour l'avatar dans le lobby
                    int playerIndex = lobby->playerNames.indexOf(playerName);
                    if (playerIndex >= 0 && playerIndex < lobby->playerAvatars.size()) {
                        lobby->playerAvatars[playerIndex] = newAvatar;
                    }
                    break;
                }
            }

            if (playerInLobby) {
                // Envoyer la liste mise à jour à tous les joueurs du lobby
                sendLobbyUpdate(lobby->code);
                break;
            }
        }
    }

    // Handler pour réhumaniser un joueur après qu'il ait été remplacé par un bot
    void handleRehumanize(QWebSocket *socket) {
        // Trouver la connexion du joueur
        QString connectionId;
        for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
            if (it.value()->socket == socket) {
                connectionId = it.key();
                break;
            }
        }

        if (connectionId.isEmpty()) {
            qDebug() << "handleRehumanize - Connexion non trouvée";
            return;
        }

        PlayerConnection* conn = m_connections[connectionId];
        int roomId = conn->gameRoomId;
        int playerIndex = conn->playerIndex;  // Utiliser directement l'index stocké dans la connexion

        qDebug() << "handleRehumanize - connectionId:" << connectionId
                 << "roomId:" << roomId << "playerIndex:" << playerIndex;

        if (roomId == -1 || !m_gameRooms.contains(roomId)) {
            qDebug() << "handleRehumanize - Joueur pas dans une room";
            return;
        }

        GameRoom* room = m_gameRooms[roomId];

        if (playerIndex < 0 || playerIndex >= 4) {
            qDebug() << "handleRehumanize - Index joueur invalide:" << playerIndex;
            return;
        }

        qDebug() << "handleRehumanize - isBot[" << playerIndex << "] =" << room->isBot[playerIndex];

        // Réhumaniser le joueur
        if (room->isBot[playerIndex]) {
            room->isBot[playerIndex] = false;
            qDebug() << "handleRehumanize - Joueur" << playerIndex << "redevient humain";

            // Confirmer au client
            QJsonObject response;
            response["type"] = "rehumanizeSuccess";
            sendMessage(socket, response);

            // Si c'est le tour de ce joueur
            if (room->currentPlayerIndex == playerIndex) {
                if (room->gameState == "playing") {
                    // Phase de jeu : envoyer les cartes jouables et démarrer le timer de jeu
                    qDebug() << "handleRehumanize - C'est le tour du joueur (phase jeu), envoi des cartes jouables";
                    // Le timer de jeu sera géré par notifyPlayersWithPlayableCards qui sera appelé
                    // On n'appelle pas directement notifyPlayersWithPlayableCards car il broadcast à tous
                    // Au lieu de cela, on envoie juste les cartes jouables à ce joueur
                    QJsonObject stateMsg;
                    stateMsg["type"] = "gameState";
                    stateMsg["currentPlayer"] = playerIndex;
                    stateMsg["playableCards"] = calculatePlayableCards(room, playerIndex);
                    sendMessage(socket, stateMsg);
                } else if (room->gameState == "bidding") {
                    // Phase d'enchères : démarrer le timer de timeout
                    qDebug() << "handleRehumanize - C'est le tour du joueur (phase enchères), démarrage timer";
                    startBidTimeout(roomId, playerIndex);
                }
            }
        } else {
            qDebug() << "handleRehumanize - Joueur" << playerIndex << "était déjà humain";
        }
    }

    void handleRegisterAccount(QWebSocket *socket, const QJsonObject &data) {
        QString pseudo = data["pseudo"].toString();
        QString email = data["email"].toString();
        QString password = data["password"].toString();
        QString avatar = data["avatar"].toString();

        qDebug() << "GameServer - Tentative creation compte:" << pseudo << email << "avatar:" << avatar;

        QString errorMsg;
        if (m_dbManager->createAccount(pseudo, email, password, avatar, errorMsg)) {
            // Succes
            QJsonObject response;
            response["type"] = "registerAccountSuccess";
            response["playerName"] = pseudo;
            response["avatar"] = avatar;
            sendMessage(socket, response);
            qDebug() << "Compte cree avec succes:" << pseudo;
        } else {
            // Echec
            QJsonObject response;
            response["type"] = "registerAccountFailed";
            response["error"] = errorMsg;
            sendMessage(socket, response);
            qDebug() << "Echec creation compte:" << errorMsg;
        }
    }

    void handleLoginAccount(QWebSocket *socket, const QJsonObject &data) {
        QString email = data["email"].toString();
        QString password = data["password"].toString();

        qDebug() << "GameServer - Tentative connexion:" << email;

        QString pseudo;
        QString avatar;
        QString errorMsg;
        if (m_dbManager->authenticateUser(email, password, pseudo, avatar, errorMsg)) {
            // Succès - Créer une connexion et enregistrer le joueur
            QString connectionId = QUuid::createUuid().toString();

            PlayerConnection *conn = new PlayerConnection{
                socket,
                connectionId,
                pseudo,
                avatar,
                -1,    // Pas encore en partie
                -1     // Pas encore de position
            };
            m_connections[connectionId] = conn;

            QJsonObject response;
            response["type"] = "loginAccountSuccess";
            response["playerName"] = pseudo;
            response["avatar"] = avatar;
            response["connectionId"] = connectionId;
            sendMessage(socket, response);
            qDebug() << "Connexion reussie:" << pseudo << "avatar:" << avatar << "ID:" << connectionId;

            // Vérifier si le joueur peut se reconnecter à une partie en cours
            if (m_playerNameToRoomId.contains(pseudo)) {
                int roomId = m_playerNameToRoomId[pseudo];
                GameRoom* room = m_gameRooms.value(roomId);

                if (room && room->gameState != "finished") {
                    // Trouver l'index du joueur dans la partie
                    int playerIndex = -1;
                    for (int i = 0; i < room->playerNames.size(); i++) {
                        if (room->playerNames[i] == pseudo) {
                            playerIndex = i;
                            break;
                        }
                    }

                    // Reconnexion si:
                    // 1. Le joueur est marqué comme bot (déconnexion détectée par le serveur)
                    // 2. OU le joueur a une ancienne connexion différente (reconnexion rapide avant détection)
                    bool isDifferentConnection = (playerIndex != -1 &&
                                                 playerIndex < room->connectionIds.size() &&
                                                 room->connectionIds[playerIndex] != connectionId);

                    if (playerIndex != -1 && (room->isBot[playerIndex] || isDifferentConnection)) {
                        qDebug() << "Reconnexion detectee pour" << pseudo << "a la partie" << roomId << "position" << playerIndex;
                        qDebug() << "  isBot:" << room->isBot[playerIndex] << "isDifferentConnection:" << isDifferentConnection;
                        handleReconnection(connectionId, roomId, playerIndex);
                    }
                }
            }
        } else {
            // Echec
            QJsonObject response;
            response["type"] = "loginAccountFailed";
            response["error"] = errorMsg;
            sendMessage(socket, response);
            qDebug() << "Echec connexion:" << errorMsg;
        }
    }

    void handleGetStats(QWebSocket *socket, const QJsonObject &data) {
        QString pseudo = data["pseudo"].toString();

        qDebug() << "GameServer - Demande de stats pour:" << pseudo;

        DatabaseManager::PlayerStats stats = m_dbManager->getPlayerStats(pseudo);

        QJsonObject response;
        response["type"] = "statsData";
        response["gamesPlayed"] = stats.gamesPlayed;
        response["gamesWon"] = stats.gamesWon;
        response["winRatio"] = stats.winRatio;
        response["coincheAttempts"] = stats.coincheAttempts;
        response["coincheSuccess"] = stats.coincheSuccess;
        response["capotRealises"] = stats.capotRealises;
        response["capotAnnoncesRealises"] = stats.capotAnnoncesRealises;
        response["capotAnnoncesTentes"] = stats.capotAnnoncesTentes;
        response["generaleAttempts"] = stats.generaleAttempts;
        response["generaleSuccess"] = stats.generaleSuccess;
        response["annoncesCoinchees"] = stats.annoncesCoinchees;
        response["annoncesCoincheesgagnees"] = stats.annoncesCoincheesgagnees;
        response["surcoincheAttempts"] = stats.surcoincheAttempts;
        response["surcoincheSuccess"] = stats.surcoincheSuccess;
        response["annoncesSurcoinchees"] = stats.annoncesSurcoinchees;
        response["annoncesSurcoincheesGagnees"] = stats.annoncesSurcoincheesGagnees;
        response["maxWinStreak"] = stats.maxWinStreak;

        qDebug() << "Stats envoyees pour:" << pseudo
                 << "- Parties:" << stats.gamesPlayed
                 << "Victoires:" << stats.gamesWon
                 << "Ratio:" << stats.winRatio
                 << "Capots:" << stats.capotRealises
                 << "Generales:" << stats.generaleSuccess
                 << "Annonces coinchées:" << stats.annoncesCoinchees;

        sendMessage(socket, response);
    }

    void handleJoinMatchmaking(QWebSocket *socket) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        if (!m_matchmakingQueue.contains(connectionId)) {
            m_matchmakingQueue.enqueue(connectionId);
            qDebug() << "Joueur en attente:" << connectionId
                     << "Queue size:" << m_matchmakingQueue.size();

            // Notifier TOUS les joueurs en attente du nombre de joueurs
            QJsonObject response;
            response["type"] = "matchmakingStatus";
            response["status"] = "searching";
            response["playersInQueue"] = m_matchmakingQueue.size();

            // Envoyer à tous les joueurs dans la queue
            for (const QString &queuedConnectionId : m_matchmakingQueue) {
                if (m_connections.contains(queuedConnectionId)) {
                    sendMessage(m_connections[queuedConnectionId]->socket, response);
                }
            }

            // Redémarrer le timer de matchmaking (30 secondes d'inactivité)
            // Le timer est réinitialisé à chaque nouveau joueur
            m_lastQueueSize = m_matchmakingQueue.size();
            m_matchmakingTimer->start();
            qDebug() << "Timer matchmaking démarré/redémarré - 30 secondes avant création avec bots";

            // Essaye de créer une partie si 4 joueurs
            tryCreateGame();
        }
    }

    void handleLeaveMatchmaking(QWebSocket *socket) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        m_matchmakingQueue.removeAll(connectionId);
        qDebug() << "Joueur quitte la queue:" << connectionId
                 << "Queue size:" << m_matchmakingQueue.size();

        // Notifier le joueur qui quitte
        QJsonObject response;
        response["type"] = "matchmakingStatus";
        response["status"] = "left";
        sendMessage(socket, response);

        // Si la queue est vide, arrêter le timer
        if (m_matchmakingQueue.isEmpty()) {
            m_matchmakingTimer->stop();
            qDebug() << "Timer matchmaking arrêté - queue vide";
        }

        // Notifier TOUS les joueurs restants du nouveau nombre de joueurs
        QJsonObject updateResponse;
        updateResponse["type"] = "matchmakingStatus";
        updateResponse["status"] = "searching";
        updateResponse["playersInQueue"] = m_matchmakingQueue.size();

        for (const QString &queuedConnectionId : m_matchmakingQueue) {
            if (m_connections.contains(queuedConnectionId)) {
                sendMessage(m_connections[queuedConnectionId]->socket, updateResponse);
            }
        }
    }

    void tryCreateGame() {
        if (m_matchmakingQueue.size() >= 4) {
            // Prendre 4 joueurs de la queue
            QList<QString> queuedPlayers;
            for (int i = 0; i < 4; i++) {
                queuedPlayers.append(m_matchmakingQueue.dequeue());
            }

            // Organiser les joueurs pour que les partenaires de lobby soient ensemble
            QList<QString> connectionIds(4);

            // Chercher une paire de partenaires
            QString partner1, partner2;
            for (const QString &playerId : queuedPlayers) {
                PlayerConnection* conn = m_connections[playerId];
                if (!conn->lobbyPartnerId.isEmpty() && queuedPlayers.contains(conn->lobbyPartnerId)) {
                    partner1 = playerId;
                    partner2 = conn->lobbyPartnerId;
                    qDebug() << "Paire de partenaires trouvée:" << conn->playerName << "et" << m_connections[partner2]->playerName;
                    break;
                }
            }

            if (!partner1.isEmpty()) {
                // Placer les partenaires aux positions 0 et 2
                connectionIds[0] = partner1;
                connectionIds[2] = partner2;

                // Réinitialiser les marqueurs de partenariat
                m_connections[partner1]->lobbyPartnerId = "";
                m_connections[partner2]->lobbyPartnerId = "";

                // Placer les autres joueurs aux positions 1 et 3
                int otherIndex = 1;
                for (const QString &playerId : queuedPlayers) {
                    if (playerId != partner1 && playerId != partner2) {
                        connectionIds[otherIndex] = playerId;
                        otherIndex = 3;  // Passer à la position 3 pour le deuxième joueur
                    }
                }

                qDebug() << "Partie créée avec partenaires de lobby aux positions 0 et 2";
            } else {
                // Pas de partenaires, utiliser l'ordre normal
                connectionIds = queuedPlayers;
                qDebug() << "Partie créée sans partenaires de lobby";
            }

            int roomId = m_nextRoomId++;
            GameRoom* room = new GameRoom();  // Créé sur le heap
            room->roomId = roomId;
            room->connectionIds = connectionIds;
            room->originalConnectionIds = connectionIds;  // Sauvegarder les IDs originaux
            room->gameState = "waiting";

            // Crée les joueurs du jeu
            for (int i = 0; i < 4; i++) {
                PlayerConnection* conn = m_connections[connectionIds[i]];
                conn->gameRoomId = roomId;
                conn->playerIndex = i;

                // Sauvegarder le nom et avatar du joueur pour reconnexion
                room->playerNames.append(conn->playerName);
                room->playerAvatars.append(conn->avatar);
                m_playerNameToRoomId[conn->playerName] = roomId;

                std::vector<Carte*> emptyHand;
                auto player = std::make_unique<Player>(
                    conn->playerName.toStdString(),
                    emptyHand,
                    i
                );
                room->players.push_back(std::move(player));
                room->isBot.push_back(false);  // Initialement, aucun joueur n'est un bot
            }
            
            // Distribue les cartes
            room->deck.shuffleDeck();
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 8; j++) {
                    Carte* carte = room->deck.drawCard();
                    if (carte) {
                        room->players[i]->addCardToHand(carte);
                    }
                }
            }

            m_gameRooms[roomId] = room;  // Stock le pointeur

            // Init le premier joueur (celui qui commence les enchères)
            room->firstPlayerIndex = 0;  // Joueur 0 commence
            room->currentPlayerIndex = 0;
            room->biddingPlayer = 0;
            room->gameState = "bidding";

            qDebug() << "Partie creee! Room ID:" << roomId << "- Premier joueur:" << room->firstPlayerIndex;

            // Notifie tous les joueurs
            notifyGameStart(roomId, connectionIds);

            qDebug() << "Notifications gameFound envoyees à" << connectionIds.size() << "joueurs";

            // Si le premier joueur à annoncer est un bot, le faire annoncer automatiquement
            if (room->isBot[room->currentPlayerIndex]) {
                QTimer::singleShot(800, this, [this, roomId]() {
                    GameRoom* room = m_gameRooms.value(roomId);
                    if (room && room->gameState == "bidding") {
                        playBotBid(roomId, room->currentPlayerIndex);
                    }
                });
            } else {
                // Joueur humain : démarrer le timer de timeout pour les enchères
                startBidTimeout(roomId, room->currentPlayerIndex);
            }

            // Arrêter le timer de matchmaking car la partie a commencé
            m_matchmakingTimer->stop();
        }
    }

    // Slot appelé après 30 secondes d'inactivité dans la queue de matchmaking
    void onMatchmakingTimeout() {
        qDebug() << "MATCHMAKING TIMEOUT - 30 secondes écoulées sans nouveaux joueurs";
        qDebug() << "Joueurs dans la queue:" << m_matchmakingQueue.size();

        // S'il y a au moins 1 joueur dans la queue, créer une partie avec des bots
        if (m_matchmakingQueue.size() > 0 && m_matchmakingQueue.size() < 4) {
            createGameWithBots();
        }

        // Arrêter le timer
        m_matchmakingTimer->stop();
    }

    // Génère un avatar aléatoire parmi les 24 disponibles
    QString getRandomBotAvatar() {
        int avatarNumber = 1 + QRandomGenerator::global()->bounded(24);  // 1 à 24
        return QString("avataaars%1.svg").arg(avatarNumber);
    }

    // Crée une partie en complétant avec des bots
    void createGameWithBots() {
        int humanPlayers = m_matchmakingQueue.size();
        int botsNeeded = 4 - humanPlayers;

        qDebug() << "Création d'une partie avec" << humanPlayers << "humain(s) et" << botsNeeded << "bot(s)";

        // Prendre tous les joueurs humains de la queue
        QList<QString> connectionIds;
        while (!m_matchmakingQueue.isEmpty()) {
            connectionIds.append(m_matchmakingQueue.dequeue());
        }

        // Créer la room
        int roomId = m_nextRoomId++;
        GameRoom* room = new GameRoom();
        room->roomId = roomId;
        room->gameState = "waiting";

        // Ajouter les joueurs humains
        for (int i = 0; i < humanPlayers; i++) {
            PlayerConnection* conn = m_connections[connectionIds[i]];
            conn->gameRoomId = roomId;
            conn->playerIndex = i;

            room->connectionIds.append(connectionIds[i]);
            room->originalConnectionIds.append(connectionIds[i]);
            room->playerNames.append(conn->playerName);
            room->playerAvatars.append(conn->avatar);
            m_playerNameToRoomId[conn->playerName] = roomId;

            std::vector<Carte*> emptyHand;
            auto player = std::make_unique<Player>(
                conn->playerName.toStdString(),
                emptyHand,
                i
            );
            room->players.push_back(std::move(player));
            room->isBot.push_back(false);
        }

        // Ajouter les bots
        for (int i = humanPlayers; i < 4; i++) {
            // Générer un nom de bot aléatoire (Bot + nombre entre 100 et 999)
            int botNumber = 100 + QRandomGenerator::global()->bounded(900);
            QString botName = QString("Bot%1").arg(botNumber);

            // Vérifier que le nom n'est pas déjà pris
            while (m_playerNameToRoomId.contains(botName)) {
                botNumber = 100 + QRandomGenerator::global()->bounded(900);
                botName = QString("Bot%1").arg(botNumber);
            }

            // Avatar aléatoire pour le bot
            QString botAvatar = getRandomBotAvatar();

            qDebug() << "Ajout du bot:" << botName << "avec avatar:" << botAvatar << "à la position" << i;

            room->connectionIds.append("");  // Pas de connexion pour les bots
            room->originalConnectionIds.append("");
            room->playerNames.append(botName);
            room->playerAvatars.append(botAvatar);

            std::vector<Carte*> emptyHand;
            auto player = std::make_unique<Player>(
                botName.toStdString(),
                emptyHand,
                i
            );
            room->players.push_back(std::move(player));
            room->isBot.push_back(true);  // Marquer comme bot
        }

        // Distribuer les cartes
        room->deck.shuffleDeck();
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 8; j++) {
                Carte* carte = room->deck.drawCard();
                if (carte) {
                    room->players[i]->addCardToHand(carte);
                }
            }
        }

        m_gameRooms[roomId] = room;

        // Initialiser le premier joueur
        room->firstPlayerIndex = 0;
        room->currentPlayerIndex = 0;
        room->biddingPlayer = 0;
        room->gameState = "bidding";

        qDebug() << "Partie avec bots créée! Room ID:" << roomId;

        // Notifier les joueurs humains
        notifyGameStart(roomId, connectionIds);

        // Si le premier joueur à annoncer est un bot, le faire annoncer automatiquement
        if (room->isBot[room->currentPlayerIndex]) {
            QTimer::singleShot(800, this, [this, roomId]() {
                GameRoom* room = m_gameRooms.value(roomId);
                if (room && room->gameState == "bidding") {
                    playBotBid(roomId, room->currentPlayerIndex);
                }
            });
        } else {
            // Joueur humain : démarrer le timer de timeout pour les enchères
            startBidTimeout(roomId, room->currentPlayerIndex);
        }
    }

    void notifyGameStart(int roomId, const QList<QString> &connectionIds) {
        GameRoom* room = m_gameRooms[roomId];
        if (!room) return;

        qDebug() << "Envoi des notifications gameFound à" << connectionIds.size() << "joueurs humains";

        // Parcourir uniquement les joueurs humains (ceux avec une connexion)
        for (int i = 0; i < connectionIds.size(); i++) {
            if (connectionIds[i].isEmpty()) continue;  // Skip les bots

            PlayerConnection *conn = m_connections[connectionIds[i]];
            if (!conn) {
                qDebug() << "Erreur: Connexion introuvable pour ID" << connectionIds[i];
                continue;
            }

            // Trouver la position réelle du joueur dans la room
            int playerPosition = conn->playerIndex;

            QJsonObject msg;
            msg["type"] = "gameFound";
            msg["roomId"] = roomId;
            msg["playerPosition"] = playerPosition;

            // Envoi les cartes du joueur
            QJsonArray myCards;
            const auto& playerHand = room->players[playerPosition]->getMain();
            qDebug() << "Envoi de" << playerHand.size() << "cartes au joueur" << playerPosition;

            for (const auto* carte : playerHand) {
                if (carte) {
                    QJsonObject cardObj;
                    cardObj["value"] = static_cast<int>(carte->getChiffre());
                    cardObj["suit"] = static_cast<int>(carte->getCouleur());
                    myCards.append(cardObj);
                }
            }
            msg["myCards"] = myCards;

            // Infos sur TOUS les autres joueurs (humains et bots) depuis room->playerNames
            QJsonArray opponents;
            for (int j = 0; j < 4; j++) {
                if (playerPosition != j) {
                    QJsonObject opp;
                    opp["position"] = j;
                    opp["name"] = room->playerNames[j];
                    opp["avatar"] = room->playerAvatars[j];
                    opp["cardCount"] = int(room->players[j]->getMain().size());
                    opp["isBot"] = static_cast<bool>(room->isBot[j]);
                    opponents.append(opp);
                }
            }
            msg["opponents"] = opponents;
            qDebug() << "Envoi gameFound a" << conn->playerName << "position" << playerPosition;

            sendMessage(conn->socket, msg);
        }
        qDebug() << "Toutes les notifications envoyees";

    }

    void handlePlayCard(QWebSocket *socket, const QJsonObject &data) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        PlayerConnection* conn = m_connections[connectionId];
        int roomId = conn->gameRoomId;
        if (roomId == -1) return;

        GameRoom* room = m_gameRooms[roomId];
        if (!room) return;

        // Arrêter le timer de timeout du tour et invalider les anciens callbacks
        if (room->turnTimeout) {
            room->turnTimeout->stop();
            room->turnTimeoutGeneration++;  // Invalider les anciens callbacks en queue
            qDebug() << "GameServer - Timer de timeout arrêté (joueur a joué), génération:" << room->turnTimeoutGeneration;
        }

        int playerIndex = conn->playerIndex;
        int cardIndex = data["cardIndex"].toInt();

        // Check que le jeu est en phase de jeu (pas d'annonces)
        if (room->gameState != "playing") {
            qDebug() << "GameServer - Erreur: tentative de jouer une carte pendant la phase d'annonces";
            return;
        }

        // Check qu'on n'est pas en attente entre deux plis
        if (room->waitingForNextPli) {
            qDebug() << "GameServer - Erreur: tentative de jouer pendant l'attente entre les plis (joueur" << playerIndex << ")";
            return;
        }

        // Check que c'est bien le tour du joueur
        if (room->currentPlayerIndex != playerIndex) {
            qDebug() << "GameServer - Erreur: ce n'est pas le tour du joueur" << playerIndex;
            return;
        }

        Player* player = room->players[playerIndex].get();
        if (!player || cardIndex < 0 || cardIndex >= player->getMain().size()) {
            qDebug() << "GameServer - Erreur: index de carte invalide" << cardIndex;
            return;
        }

        // Détermine la carte la plus forte du pli actuel
        Carte* carteGagnante = nullptr;
        int idxPlayerWinning = -1;
        if (!room->currentPli.empty()) {
            carteGagnante = room->currentPli[0].second;
            idxPlayerWinning = room->currentPli[0].first;

            for (size_t i = 1; i < room->currentPli.size(); i++) {
                Carte* c = room->currentPli[i].second;
                if (*carteGagnante < *c) {
                    carteGagnante = c;
                    idxPlayerWinning = room->currentPli[i].first;
                }
            }
        }

        // Check si la carte est jouable
        bool isPlayable = player->isCartePlayable(
            cardIndex,
            room->couleurDemandee,
            room->couleurAtout,
            carteGagnante,
            idxPlayerWinning
        );

        if (!isPlayable) {
            qDebug() << "GameServer - Erreur: carte non jouable selon les regles";

            // Envoi un message d'erreur au joueur
            QJsonObject errorMsg;
            errorMsg["type"] = "error";
            errorMsg["message"] = "Cette carte n'est pas jouable";
            sendMessage(socket, errorMsg);
            return;
        }

        // La carte est valide, la récupérer et la retirer de la main
        Carte* cartePlayed = player->getMain()[cardIndex];

        qDebug() << "GameServer - Joueur" << playerIndex
                 << "joue la carte - Index:" << cardIndex
                 << "Valeur:" << static_cast<int>(cartePlayed->getChiffre())
                 << "Couleur:" << static_cast<int>(cartePlayed->getCouleur());

        // Si c'est la première carte du pli, définir la couleur demandée
        if (room->currentPli.empty()) {
            room->couleurDemandee = cartePlayed->getCouleur();
        }

        // Ajoute au pli courant
        room->currentPli.push_back(std::make_pair(playerIndex, cartePlayed));

        // Marquer la carte comme jouée pour le tracking IA
        room->markCardAsPlayed(cartePlayed);

        // IMPORTANT : Retirer la carte de la main du joueur côté serveur
        // Cela maintient la synchronisation avec les clients
        player->removeCard(cardIndex);

        qDebug() << "GameServer - Carte retirée, main du joueur" << playerIndex
                 << "contient maintenant" << player->getMain().size() << "cartes";

        qDebug() << "GameServer - Carte jouee par joueur" << playerIndex
                 << "- Pli:" << room->currentPli.size() << "/4";

        // Vérifier si c'est une carte de la belote (Roi ou Dame de l'atout)
        bool isBeloteCard = false;
        bool isRoi = (cartePlayed->getChiffre() == Carte::ROI);
        bool isDame = (cartePlayed->getChiffre() == Carte::DAME);
        bool isAtout = (cartePlayed->getCouleur() == room->couleurAtout);

        if (isAtout && (isRoi || isDame)) {
            // Vérifier si ce joueur a la belote
            int teamIndex = playerIndex % 2; // 0 pour team1, 1 pour team2
            bool hasBelote = (teamIndex == 0) ? room->beloteTeam1 : room->beloteTeam2;

            if (hasBelote) {
                isBeloteCard = true;

                // Vérifier si c'est la première ou la deuxième carte de la belote jouée
                if (!room->beloteRoiJoue && !room->beloteDameJouee) {
                    // Première carte de la belote jouée - afficher "Belote"
                    qDebug() << "GameServer - Joueur" << playerIndex << "joue la première carte de la belote (BELOTE)";

                    if (isRoi) {
                        room->beloteRoiJoue = true;
                    } else {
                        room->beloteDameJouee = true;
                    }

                    // Broadcaster l'animation "Belote"
                    QJsonObject beloteMsg;
                    beloteMsg["type"] = "belote";
                    beloteMsg["playerIndex"] = playerIndex;
                    broadcastToRoom(roomId, beloteMsg);

                } else if ((isRoi && room->beloteDameJouee) || (isDame && room->beloteRoiJoue)) {
                    // Deuxième carte de la belote jouée - afficher "Rebelote"
                    qDebug() << "GameServer - Joueur" << playerIndex << "joue la deuxième carte de la belote (REBELOTE)";

                    if (isRoi) {
                        room->beloteRoiJoue = true;
                    } else {
                        room->beloteDameJouee = true;
                    }

                    // Broadcaster l'animation "Rebelote"
                    QJsonObject rebeloteMsg;
                    rebeloteMsg["type"] = "rebelote";
                    rebeloteMsg["playerIndex"] = playerIndex;
                    broadcastToRoom(roomId, rebeloteMsg);
                }
            }
        }

        // Broadcast l'action à tous les joueurs avec les infos de la carte
        qDebug() << "GameServer - Avant broadcast - Carte:"
                 << "Valeur:" << static_cast<int>(cartePlayed->getChiffre())
                 << "Couleur:" << static_cast<int>(cartePlayed->getCouleur());

        QJsonObject msg;
        msg["type"] = "cardPlayed";
        msg["playerIndex"] = playerIndex;
        msg["cardIndex"] = cardIndex;
        msg["cardValue"] = static_cast<int>(cartePlayed->getChiffre());
        msg["cardSuit"] = static_cast<int>(cartePlayed->getCouleur());

        qDebug() << "GameServer - Message envoyé:" << msg;
        broadcastToRoom(roomId, msg);

        // Si le pli est complet (4 cartes)
        if (room->currentPli.size() == 4) {
            finishPli(roomId);
        } else {
            // Passe au joueur suivant
            room->currentPlayerIndex = (room->currentPlayerIndex + 1) % 4;

            // Notifie les cartes jouables pour le nouveau joueur
            notifyPlayersWithPlayableCards(roomId);
        }
    }

    void finishPli(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        // Détermine le gagnant du pli
        Carte* carteGagnante = room->currentPli[0].second;
        int gagnantIndex = room->currentPli[0].first;
        qDebug() << "***************************GameServer - Determination du gagnant du pli";
        qDebug() << "Carte gagnante initiale:";
        //carteGagnante->printCarte();

        for (size_t i = 1; i < room->currentPli.size(); i++) {
            Carte* c = room->currentPli[i].second;
            qDebug() << "Comparaison avec la carte du joueur" << room->currentPli[i].first << ":";
            //c->printCarte();
            if (*carteGagnante < *c) {
                carteGagnante = c;
                gagnantIndex = room->currentPli[i].first;
                carteGagnante->printCarte();
            }
        }

        qDebug() << "GameServer - Pli termine, gagnant: joueur" << gagnantIndex;

        // Incrementer le compteur de plis du gagnant
        switch (gagnantIndex) {
            case 0: room->plisCountPlayer0++; break;
            case 1: room->plisCountPlayer1++; break;
            case 2: room->plisCountPlayer2++; break;
            case 3: room->plisCountPlayer3++; break;
        }

        // Calculer les points de ce pli
        int pointsPli = 0;
        for (const auto& pair : room->currentPli) {
            Carte* carte = pair.second;
            if (room->isToutAtout) {
                // Mode Tout Atout: valeurs spéciales
                pointsPli += getCardValueToutAtout(carte);
            } else if (room->isSansAtout) {
                // Mode Sans Atout: valeurs spéciales
                pointsPli += getCardValueSansAtout(carte);
            } else {
                // Mode normal
                pointsPli += carte->getValeurDeLaCarte();
            }
        }

        // Ajouter les cartes du pli à l'équipe gagnante et mettre à jour le score de manche
        // Les cartes sont stockées dans plisTeam1/plisTeam2 dans l'ordre des plis gagnés
        // Équipe 1: joueurs 0 et 2, Équipe 2: joueurs 1 et 3
        if (gagnantIndex == 0 || gagnantIndex == 2) {
            // Équipe 1 gagne le pli
            for (const auto& pair : room->currentPli) {
                room->plisTeam1.push_back(pair);
            }
            room->scoreMancheTeam1 += pointsPli;
        } else {
            // Équipe 2 gagne le pli
            for (const auto& pair : room->currentPli) {
                room->plisTeam2.push_back(pair);
            }
            room->scoreMancheTeam2 += pointsPli;
        }

        qDebug() << "GameServer - Points du pli:" << pointsPli;
        qDebug() << "GameServer - Scores de manche: Team1 =" << room->scoreMancheTeam1
                 << ", Team2 =" << room->scoreMancheTeam2;

        // Vérifier si la manche est terminée (tous les joueurs n'ont plus de cartes)
        bool mancheTerminee = true;
        for (const auto& player : room->players) {
            if (!player->getMain().empty()) {
                mancheTerminee = false;
                break;
            }
        }

        // Si c'est le dernier pli, ajouter le bonus de 10 points AVANT d'envoyer pliFinished
        if (mancheTerminee) {
            if (gagnantIndex == 0 || gagnantIndex == 2) {
                room->scoreMancheTeam1 += 10;
            } else {
                room->scoreMancheTeam2 += 10;
            }
            qDebug() << "GameServer - Dernier pli, ajout du bonus +10 points";
            qDebug() << "GameServer - Scores de manche finaux: Team1 =" << room->scoreMancheTeam1
                     << ", Team2 =" << room->scoreMancheTeam2;
        }

        // Notifie le gagnant du pli avec les scores de manche mis à jour
        QJsonObject pliFinishedMsg;
        pliFinishedMsg["type"] = "pliFinished";
        pliFinishedMsg["winnerId"] = gagnantIndex;
        pliFinishedMsg["scoreMancheTeam1"] = room->scoreMancheTeam1;
        pliFinishedMsg["scoreMancheTeam2"] = room->scoreMancheTeam2;
        broadcastToRoom(roomId, pliFinishedMsg);

        // Bloquer les requêtes de jeu pendant l'attente entre les plis
        room->waitingForNextPli = true;

        if (mancheTerminee) {
            qDebug() << "GameServer - Manche terminee, attente de 1500ms avant de commencer la nouvelle manche...";

            // Attendre 1500ms pour laisser les joueurs voir le dernier pli
            QTimer::singleShot(1500, this, [this, roomId]() {
                GameRoom* room = m_gameRooms.value(roomId);
                if (room) room->waitingForNextPli = false;  // Débloquer les requêtes
                qDebug() << "GameServer - Calcul des scores de la manche...";
                finishManche(roomId);
            });
        } else {
            qDebug() << "GameServer - Pli termine, attente de 1500ms avant le prochain pli...";

            // Attendre 1500ms pour laisser les joueurs voir le pli gagné
            QTimer::singleShot(1500, this, [this, roomId, gagnantIndex]() {
                GameRoom* room = m_gameRooms.value(roomId);
                if (!room) return;

                // Débloquer les requêtes de jeu
                room->waitingForNextPli = false;

                // Réinitialise pour le prochain pli
                room->currentPli.clear();
                room->couleurDemandee = Carte::COULEURINVALIDE;
                room->currentPlayerIndex = gagnantIndex;  // Le gagnant commence le prochain pli

                // Notifie avec les cartes jouables pour le nouveau pli
                notifyPlayersWithPlayableCards(roomId);
            });
        }
    }

    void finishManche(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        // Les scores de manche ont déjà été calculés pendant les plis
        // Le bonus du dernier pli (+10 points) a déjà été ajouté dans finishPli()
        int pointsRealisesTeam1 = room->scoreMancheTeam1;
        int pointsRealisesTeam2 = room->scoreMancheTeam2;

        // Ajoute les points de Belote (détectée au début de la phase de jeu)
        if (room->beloteTeam1) {
            pointsRealisesTeam1 += 20;
            qDebug() << "GameServer - +20 points de Belote pour l'Équipe 1";
        }
        if (room->beloteTeam2) {
            pointsRealisesTeam2 += 20;
            qDebug() << "GameServer - +20 points de Belote pour l'Équipe 2";
        }

        qDebug() << "GameServer - Points realises dans la manche (avec Belote):";
        qDebug() << "  Equipe 1 (joueurs 0 et 2):" << pointsRealisesTeam1 << "points";
        qDebug() << "  Equipe 2 (joueurs 1 et 3):" << pointsRealisesTeam2 << "points";

        qDebug() << "GameServer - Plis gagnes par joueur:";
        qDebug() << "  Joueur 0:" << room->plisCountPlayer0 << "plis";
        qDebug() << "  Joueur 1:" << room->plisCountPlayer1 << "plis";
        qDebug() << "  Joueur 2:" << room->plisCountPlayer2 << "plis";
        qDebug() << "  Joueur 3:" << room->plisCountPlayer3 << "plis";

        // Détermine quelle équipe a fait l'enchère
        // Équipe 1: joueurs 0 et 2, Équipe 2: joueurs 1 et 3
        bool team1HasBid = (room->lastBidderIndex == 0 || room->lastBidderIndex == 2);
        int valeurContrat = Player::getContractValue(room->lastBidAnnonce);

        qDebug() << "GameServer - Contrat: valeur =" << valeurContrat
                 << ", equipe =" << (team1HasBid ? 1 : 2)
                 << ", annonce =" << static_cast<int>(room->lastBidAnnonce);

        // Verification des conditions speciales CAPOT et GENERALE
        bool isCapotAnnonce = (room->lastBidAnnonce == Player::CAPOT);
        bool isGeneraleAnnonce = (room->lastBidAnnonce == Player::GENERALE);
        bool capotReussi = false;
        bool generaleReussie = false;

        // Vérifier si une équipe a fait un CAPOT (8 plis) même sans l'avoir annoncé
        int plisTeam1 = room->plisCountPlayer0 + room->plisCountPlayer2;
        int plisTeam2 = room->plisCountPlayer1 + room->plisCountPlayer3;
        bool capotNonAnnonceTeam1 = (!isCapotAnnonce && !isGeneraleAnnonce && plisTeam1 == 8);
        bool capotNonAnnonceTeam2 = (!isCapotAnnonce && !isGeneraleAnnonce && plisTeam2 == 8);

        // Si un capot non annoncé est réalisé, on doit le marquer comme réussi pour les stats
        if (capotNonAnnonceTeam1 || capotNonAnnonceTeam2) {
            capotReussi = true;
        }

        if (isCapotAnnonce) {
            // CAPOT: l'equipe qui a annonce doit faire tous les 8 plis
            int plisTeamAnnonceur = 0;
            if (team1HasBid) {
                plisTeamAnnonceur = plisTeam1;
            } else {
                plisTeamAnnonceur = plisTeam2;
            }
            capotReussi = (plisTeamAnnonceur == 8);
            qDebug() << "GameServer - CAPOT annonce: equipe a fait" << plisTeamAnnonceur << "/8 plis -"
                     << (capotReussi ? "REUSSI" : "ECHOUE");
        } else if (isGeneraleAnnonce) {
            // GENERALE: le joueur qui a annonce doit faire tous les 8 plis seul
            int plisJoueurAnnonceur = 0;
            switch (room->lastBidderIndex) {
                case 0: plisJoueurAnnonceur = room->plisCountPlayer0; break;
                case 1: plisJoueurAnnonceur = room->plisCountPlayer1; break;
                case 2: plisJoueurAnnonceur = room->plisCountPlayer2; break;
                case 3: plisJoueurAnnonceur = room->plisCountPlayer3; break;
            }
            generaleReussie = (plisJoueurAnnonceur == 8);
            qDebug() << "GameServer - GENERALE annoncee par joueur" << room->lastBidderIndex
                     << ": a fait" << plisJoueurAnnonceur << "/8 plis -"
                     << (generaleReussie ? "REUSSIE" : "ECHOUEE");
        }

        // Applique les règles du contrat
        int scoreToAddTeam1 = 0;
        int scoreToAddTeam2 = 0;

        // Multiplicateur COINCHE/SURCOINCHE
        int multiplicateur = 1;
        if (room->surcoinched) {
            multiplicateur = 4;
        } else if (room->coinched) {
            multiplicateur = 2;
        }

        // Regles speciales pour CAPOT et GENERALE
        if (isCapotAnnonce) {
            if (capotReussi) {
                // CAPOT reussi
                // Normal: 250 + 250 = 500
                // Coinché: 500 * 2 = 1000
                // Surcoinché: 500 * 4 = 2000
                int scoreCapot = 500 * multiplicateur;
                if (team1HasBid) {
                    scoreToAddTeam1 = scoreCapot;
                    scoreToAddTeam2 = 0;
                    qDebug() << "GameServer - Equipe 1 reussit son CAPOT!";
                    if (multiplicateur > 1) {
                        qDebug() << "  Team1 marque:" << scoreCapot << "(500*" << multiplicateur << ")";
                    } else {
                        qDebug() << "  Team1 marque:" << scoreCapot << "(250+250)";
                    }
                    qDebug() << "  Team2 marque: 0";
                } else {
                    scoreToAddTeam1 = 0;
                    scoreToAddTeam2 = scoreCapot;
                    qDebug() << "GameServer - Equipe 2 reussit son CAPOT!";
                    qDebug() << "  Team1 marque: 0";
                    if (multiplicateur > 1) {
                        qDebug() << "  Team2 marque:" << scoreCapot << "(500*" << multiplicateur << ")";
                    } else {
                        qDebug() << "  Team2 marque:" << scoreCapot << "(250+250)";
                    }
                }
            } else {
                // CAPOT echoue: equipe adverse marque les memes points
                // Normal: 160 + 250 = 410
                // Coinché: (160 + 250) * 2 = 820
                // Surcoinché: (160 + 250) * 4 = 1640
                int scoreCapotEchoue = (160 + 250) * multiplicateur;
                if (team1HasBid) {
                    scoreToAddTeam1 = 0;
                    scoreToAddTeam2 = scoreCapotEchoue;
                    qDebug() << "GameServer - Equipe 1 echoue son CAPOT!";
                    qDebug() << "  Team1 marque: 0";
                    if (multiplicateur > 1) {
                        qDebug() << "  Team2 marque:" << scoreCapotEchoue << "((160+250)*" << multiplicateur << ")";
                    } else {
                        qDebug() << "  Team2 marque:" << scoreCapotEchoue << "(160+250)";
                    }
                } else {
                    scoreToAddTeam1 = scoreCapotEchoue;
                    scoreToAddTeam2 = 0;
                    qDebug() << "GameServer - Equipe 2 echoue son CAPOT!";
                    if (multiplicateur > 1) {
                        qDebug() << "  Team1 marque:" << scoreCapotEchoue << "((160+250)*" << multiplicateur << ")";
                    } else {
                        qDebug() << "  Team1 marque:" << scoreCapotEchoue << "(160+250)";
                    }
                    qDebug() << "  Team2 marque: 0";
                }
            }
        } else if (isGeneraleAnnonce) {
            if (generaleReussie) {
                // GENERALE reussie
                // Normal: 500 + 500 = 1000
                // Coinché: 1000 * 2 = 2000
                // Surcoinché: 1000 * 4 = 4000
                int scoreGenerale = 1000 * multiplicateur;
                if (team1HasBid) {
                    scoreToAddTeam1 = scoreGenerale;
                    scoreToAddTeam2 = 0;
                    qDebug() << "GameServer - Joueur" << room->lastBidderIndex << "(Equipe 1) reussit sa GENERALE!";
                    if (multiplicateur > 1) {
                        qDebug() << "  Team1 marque:" << scoreGenerale << "(1000*" << multiplicateur << ")";
                    } else {
                        qDebug() << "  Team1 marque:" << scoreGenerale << "(500+500)";
                    }
                    qDebug() << "  Team2 marque: 0";
                } else {
                    scoreToAddTeam1 = 0;
                    scoreToAddTeam2 = scoreGenerale;
                    qDebug() << "GameServer - Joueur" << room->lastBidderIndex << "(Equipe 2) reussit sa GENERALE!";
                    qDebug() << "  Team1 marque: 0";
                    if (multiplicateur > 1) {
                        qDebug() << "  Team2 marque:" << scoreGenerale << "(1000*" << multiplicateur << ")";
                    } else {
                        qDebug() << "  Team2 marque:" << scoreGenerale << "(500+500)";
                    }
                }
            } else {
                // GENERALE echouee: equipe adverse marque les memes points
                // Normal: 160 + 500 = 660
                // Coinché: (160 + 500) * 2 = 1320
                // Surcoinché: (160 + 500) * 4 = 2640
                int scoreGeneraleEchoue = (160 + 500) * multiplicateur;
                if (team1HasBid) {
                    scoreToAddTeam1 = 0;
                    scoreToAddTeam2 = scoreGeneraleEchoue;
                    qDebug() << "GameServer - Joueur" << room->lastBidderIndex << "(Equipe 1) echoue sa GENERALE!";
                    qDebug() << "  Team1 marque: 0";
                    if (multiplicateur > 1) {
                        qDebug() << "  Team2 marque:" << scoreGeneraleEchoue << "((160+500)*" << multiplicateur << ")";
                    } else {
                        qDebug() << "  Team2 marque:" << scoreGeneraleEchoue << "(160+500)";
                    }
                } else {
                    scoreToAddTeam1 = scoreGeneraleEchoue;
                    scoreToAddTeam2 = 0;
                    qDebug() << "GameServer - Joueur" << room->lastBidderIndex << "(Equipe 2) echoue sa GENERALE!";
                    if (multiplicateur > 1) {
                        qDebug() << "  Team1 marque:" << scoreGeneraleEchoue << "((160+500)*" << multiplicateur << ")";
                    } else {
                        qDebug() << "  Team1 marque:" << scoreGeneraleEchoue << "(160+500)";
                    }
                    qDebug() << "  Team2 marque: 0";
                }
            }
        } else if (room->coinched || room->surcoinched) {
            // COINCHE ou SURCOINCHE: scoring spécial
            // Si réussi par l'équipe qui a annoncé: (valeurContrat + pointsRealisés) × multiplicateur
            // Si échoué: équipe adverse marque (valeurContrat + 160) × multiplicateur
            int multiplicateur = room->surcoinched ? 4 : 2;  // SURCOINCHE = ×4, COINCHE = ×2

            if (team1HasBid) {
                // Team1 a annoncé, vérifie si elle réussit
                bool contractReussi = (pointsRealisesTeam1 >= valeurContrat);

                if (contractReussi) {
                    // Vérifier si Team1 a fait un CAPOT non annoncé (tous les 8 plis)
                    if (capotNonAnnonceTeam1) {
                        // CAPOT non annoncé coinché: (250 + pointsRéalisés) × multiplicateur
                        scoreToAddTeam1 = (250 + pointsRealisesTeam1) * multiplicateur;
                        scoreToAddTeam2 = 0;
                        qDebug() << "GameServer - Equipe 1 fait un CAPOT non annonce COINCHE!";
                        qDebug() << "  Team1 marque:" << scoreToAddTeam1 << "((250+" << pointsRealisesTeam1 << ")*" << multiplicateur << ")";
                        qDebug() << "  Team2 marque: 0";
                    } else {
                        // Contrat réussi: (valeurContrat + pointsRealisés) × multiplicateur
                        scoreToAddTeam1 = (valeurContrat + pointsRealisesTeam1) * multiplicateur;
                        scoreToAddTeam2 = 0;
                        qDebug() << "GameServer - Equipe 1 reussit son contrat COINCHE!";
                        qDebug() << "  Team1 marque:" << scoreToAddTeam1 << "((" << valeurContrat << "+" << pointsRealisesTeam1 << ")*" << multiplicateur << ")";
                        qDebug() << "  Team2 marque: 0";
                    }

                    // Mettre à jour les stats de surcoinche réussie (le contrat a réussi)
                    if (room->surcoinched && room->surcoinchePlayerIndex != -1 && !room->isBot[room->surcoinchePlayerIndex]) {
                        PlayerConnection* surcoincheConn = m_connections[room->connectionIds[room->surcoinchePlayerIndex]];
                        if (surcoincheConn && !surcoincheConn->playerName.isEmpty()) {
                            m_dbManager->updateSurcoincheStats(surcoincheConn->playerName, false, true);
                            qDebug() << "Stats surcoinche réussie pour:" << surcoincheConn->playerName;
                        }
                    }
                } else {
                    // Contrat échoué: équipe adverse marque (valeurContrat + 160) × multiplicateur
                    scoreToAddTeam1 = 0;
                    scoreToAddTeam2 = (valeurContrat + 160) * multiplicateur;
                    qDebug() << "GameServer - Equipe 1 echoue son contrat COINCHE!";
                    qDebug() << "  Team1 marque: 0";
                    qDebug() << "  Team2 marque:" << scoreToAddTeam2 << "((" << valeurContrat << "+160)*" << multiplicateur << ")";

                    // Mettre à jour les stats de coinche réussie (le contrat a échoué, donc la coinche a réussi)
                    if (room->coinched && room->coinchePlayerIndex != -1 && !room->isBot[room->coinchePlayerIndex]) {
                        PlayerConnection* coincheConn = m_connections[room->connectionIds[room->coinchePlayerIndex]];
                        if (coincheConn && !coincheConn->playerName.isEmpty()) {
                            m_dbManager->updateCoincheStats(coincheConn->playerName, false, true);
                            qDebug() << "Stats coinche réussie pour:" << coincheConn->playerName;
                        }
                    }

                    // Note: La surcoinche n'est PAS réussie ici car le contrat a échoué
                    // (la surcoinche est faite par l'équipe qui annonce, donc si elle échoue son contrat, la surcoinche échoue aussi)
                }

                // Mettre à jour les stats d'annonces coinchées pour les joueurs de l'équipe 1
                for (int i = 0; i < room->connectionIds.size(); i++) {
                    if (room->isBot[i]) continue;  // Skip bots
                    PlayerConnection* conn = m_connections[room->connectionIds[i]];
                    if (!conn || conn->playerName.isEmpty()) continue;

                    int playerTeam = (i % 2 == 0) ? 1 : 2;
                    if (playerTeam == 1) {
                        // Ce joueur fait partie de l'équipe 1 qui a fait l'annonce coinchée
                        m_dbManager->updateAnnonceCoinchee(conn->playerName, contractReussi);
                        qDebug() << "Stats annonce coinchée pour:" << conn->playerName << "Réussie:" << contractReussi;
                    }
                }

                // Si surcoinchée, mettre à jour les stats du joueur qui avait coinché
                if (room->surcoinched && room->coinchePlayerIndex != -1 && !room->isBot[room->coinchePlayerIndex]) {
                    PlayerConnection* coincheConn = m_connections[room->connectionIds[room->coinchePlayerIndex]];
                    if (coincheConn && !coincheConn->playerName.isEmpty()) {
                        // Le joueur qui a coinché subit maintenant une surcoinche
                        // Si le contrat réussit → le joueur qui a coinché perd (won = false)
                        // Si le contrat échoue → le joueur qui a coinché gagne quand même (won = true)
                        m_dbManager->updateAnnonceSurcoinchee(coincheConn->playerName, !contractReussi);
                        qDebug() << "Stats surcoinche subie pour:" << coincheConn->playerName << "Gagnée:" << !contractReussi;
                    }
                }
            } else {
                // Team2 a annoncé, vérifie si elle réussit
                bool contractReussi = (pointsRealisesTeam2 >= valeurContrat);

                if (contractReussi) {
                    // Vérifier si Team2 a fait un CAPOT non annoncé (tous les 8 plis)
                    if (capotNonAnnonceTeam2) {
                        // CAPOT non annoncé coinché: (250 + pointsRéalisés) × multiplicateur
                        scoreToAddTeam1 = 0;
                        scoreToAddTeam2 = (250 + pointsRealisesTeam2) * multiplicateur;
                        qDebug() << "GameServer - Equipe 2 fait un CAPOT non annonce COINCHE!";
                        qDebug() << "  Team1 marque: 0";
                        qDebug() << "  Team2 marque:" << scoreToAddTeam2 << "((250+" << pointsRealisesTeam2 << ")*" << multiplicateur << ")";
                    } else {
                        // Contrat réussi: (valeurContrat + pointsRealisés) × multiplicateur
                        scoreToAddTeam1 = 0;
                        scoreToAddTeam2 = (valeurContrat + pointsRealisesTeam2) * multiplicateur;
                        qDebug() << "GameServer - Equipe 2 reussit son contrat COINCHE!";
                        qDebug() << "  Team1 marque: 0";
                        qDebug() << "  Team2 marque:" << scoreToAddTeam2 << "((" << valeurContrat << "+" << pointsRealisesTeam2 << ")*" << multiplicateur << ")";
                    }

                    // Mettre à jour les stats de surcoinche réussie (le contrat a réussi)
                    if (room->surcoinched && room->surcoinchePlayerIndex != -1 && !room->isBot[room->surcoinchePlayerIndex]) {
                        PlayerConnection* surcoincheConn = m_connections[room->connectionIds[room->surcoinchePlayerIndex]];
                        if (surcoincheConn && !surcoincheConn->playerName.isEmpty()) {
                            m_dbManager->updateSurcoincheStats(surcoincheConn->playerName, false, true);
                            qDebug() << "Stats surcoinche réussie pour:" << surcoincheConn->playerName;
                        }
                    }
                } else {
                    // Contrat échoué: équipe adverse marque (valeurContrat + 160) × multiplicateur
                    scoreToAddTeam1 = (valeurContrat + 160) * multiplicateur;
                    scoreToAddTeam2 = 0;
                    qDebug() << "GameServer - Equipe 2 echoue son contrat COINCHE!";
                    qDebug() << "  Team1 marque:" << scoreToAddTeam1 << "((" << valeurContrat << "+160)*" << multiplicateur << ")";
                    qDebug() << "  Team2 marque: 0";

                    // Mettre à jour les stats de coinche réussie (le contrat a échoué, donc la coinche a réussi)
                    if (room->coinched && room->coinchePlayerIndex != -1 && !room->isBot[room->coinchePlayerIndex]) {
                        PlayerConnection* coincheConn = m_connections[room->connectionIds[room->coinchePlayerIndex]];
                        if (coincheConn && !coincheConn->playerName.isEmpty()) {
                            m_dbManager->updateCoincheStats(coincheConn->playerName, false, true);
                            qDebug() << "Stats coinche réussie pour:" << coincheConn->playerName;
                        }
                    }

                    // Note: La surcoinche n'est PAS réussie ici car le contrat a échoué
                    // (la surcoinche est faite par l'équipe qui annonce, donc si elle échoue son contrat, la surcoinche échoue aussi)
                }

                // Mettre à jour les stats d'annonces coinchées pour les joueurs de l'équipe 2
                for (int i = 0; i < room->connectionIds.size(); i++) {
                    if (room->isBot[i]) continue;  // Skip bots
                    PlayerConnection* conn = m_connections[room->connectionIds[i]];
                    if (!conn || conn->playerName.isEmpty()) continue;

                    int playerTeam = (i % 2 == 0) ? 1 : 2;
                    if (playerTeam == 2) {
                        // Ce joueur fait partie de l'équipe 2 qui a fait l'annonce coinchée
                        m_dbManager->updateAnnonceCoinchee(conn->playerName, contractReussi);
                        qDebug() << "Stats annonce coinchée pour:" << conn->playerName << "Réussie:" << contractReussi;
                    }
                }

                // Si surcoinchée, mettre à jour les stats du joueur qui avait coinché
                if (room->surcoinched && room->coinchePlayerIndex != -1 && !room->isBot[room->coinchePlayerIndex]) {
                    PlayerConnection* coincheConn = m_connections[room->connectionIds[room->coinchePlayerIndex]];
                    if (coincheConn && !coincheConn->playerName.isEmpty()) {
                        // Le joueur qui a coinché subit maintenant une surcoinche
                        // Si le contrat réussit → le joueur qui a coinché perd (won = false)
                        // Si le contrat échoue → le joueur qui a coinché gagne quand même (won = true)
                        m_dbManager->updateAnnonceSurcoinchee(coincheConn->playerName, !contractReussi);
                        qDebug() << "Stats surcoinche subie pour:" << coincheConn->playerName << "Gagnée:" << !contractReussi;
                    }
                }
            }
        } else if (team1HasBid) {
            // L'équipe 1 a annoncé (contrat normal)
            if (pointsRealisesTeam1 >= valeurContrat) {
                // Vérifier si Team1 a fait un CAPOT non annoncé (tous les 8 plis)
                if (capotNonAnnonceTeam1) {
                    // CAPOT non annoncé: 250 + pointsRéalisés
                    scoreToAddTeam1 = 250 + pointsRealisesTeam1;
                    scoreToAddTeam2 = 0;
                    qDebug() << "GameServer - Equipe 1 fait un CAPOT non annonce!";
                    qDebug() << "  Team1 marque:" << scoreToAddTeam1 << "(250+" << pointsRealisesTeam1 << ")";
                    qDebug() << "  Team2 marque: 0";
                } else {
                    // Contrat réussi: valeurContrat + pointsRéalisés
                    scoreToAddTeam1 = valeurContrat + pointsRealisesTeam1;
                    scoreToAddTeam2 = pointsRealisesTeam2;
                    qDebug() << "GameServer - Equipe 1 reussit son contrat!";
                    qDebug() << "  Team1 marque:" << scoreToAddTeam1 << "(" << valeurContrat << "+" << pointsRealisesTeam1 << ")";
                    qDebug() << "  Team2 marque:" << scoreToAddTeam2;
                }
            } else {
                // Contrat échoué: équipe 1 marque 0, équipe 2 marque 160 + valeurContrat
                scoreToAddTeam1 = 0;
                scoreToAddTeam2 = 160 + valeurContrat;
                qDebug() << "GameServer - Equipe 1 echoue son contrat!";
                qDebug() << "  Team1 marque: 0";
                qDebug() << "  Team2 marque:" << scoreToAddTeam2 << "(160+" << valeurContrat << ")";
            }
        } else {
            // L'équipe 2 a annoncé (contrat normal)
            if (pointsRealisesTeam2 >= valeurContrat) {
                // Vérifier si Team2 a fait un CAPOT non annoncé (tous les 8 plis)
                if (capotNonAnnonceTeam2) {
                    // CAPOT non annoncé: 250 + pointsRéalisés
                    scoreToAddTeam1 = 0;
                    scoreToAddTeam2 = 250 + pointsRealisesTeam2;
                    qDebug() << "GameServer - Equipe 2 fait un CAPOT non annonce!";
                    qDebug() << "  Team1 marque: 0";
                    qDebug() << "  Team2 marque:" << scoreToAddTeam2 << "(250+" << pointsRealisesTeam2 << ")";
                } else {
                    // Contrat réussi: valeurContrat + pointsRéalisés
                    scoreToAddTeam1 = pointsRealisesTeam1;
                    scoreToAddTeam2 = valeurContrat + pointsRealisesTeam2;
                    qDebug() << "GameServer - Équipe 2 réussit son contrat!";
                    qDebug() << "  Team1 marque:" << scoreToAddTeam1;
                    qDebug() << "  Team2 marque:" << scoreToAddTeam2 << "(" << valeurContrat << "+" << pointsRealisesTeam2 << ")";
                }
            } else {
                // Contrat échoué: équipe 2 marque 0, équipe 1 marque 160 + valeurContrat
                scoreToAddTeam1 = 160 + valeurContrat;
                scoreToAddTeam2 = 0;
                qDebug() << "GameServer - Équipe 2 échoue son contrat!";
                qDebug() << "  Team1 marque:" << scoreToAddTeam1 << "(160+" << valeurContrat << ")";
                qDebug() << "  Team2 marque: 0";
            }
        }

        // Ajouter les points de belote (20 points)
        if (room->beloteTeam1) {
            scoreToAddTeam1 += 20;
            qDebug() << "GameServer - Equipe 1 a la belote: +20 points";
        }
        if (room->beloteTeam2) {
            scoreToAddTeam2 += 20;
            qDebug() << "GameServer - Equipe 2 a la belote: +20 points";
        }

        // Ajoute les scores de la manche aux scores totaux
        room->scoreTeam1 += scoreToAddTeam1;
        room->scoreTeam2 += scoreToAddTeam2;

        qDebug() << "GameServer - Scores totaux:";
        qDebug() << "  Equipe 1:" << room->scoreTeam1;
        qDebug() << "  Equipe 2:" << room->scoreTeam2;

        // Mettre à jour les statistiques de capot et générale
        if (isCapotAnnonce) {
            // Un capot a été annoncé, mettre à jour les stats pour les joueurs de l'équipe qui a annoncé
            for (int i = 0; i < room->connectionIds.size(); i++) {
                PlayerConnection* conn = m_connections[room->connectionIds[i]];
                if (!conn || conn->playerName.isEmpty()) continue;

                int playerTeam = (i % 2 == 0) ? 1 : 2;
                bool isPlayerInBiddingTeam = (team1HasBid && playerTeam == 1) || (!team1HasBid && playerTeam == 2);

                if (isPlayerInBiddingTeam) {
                    // Ce joueur fait partie de l'équipe qui a annoncé le capot
                    m_dbManager->updateCapotAnnonceTente(conn->playerName);
                    if (capotReussi) {
                        m_dbManager->updateCapotStats(conn->playerName, true);  // Capot annoncé réussi
                        qDebug() << "Stats capot annoncé réussi pour:" << conn->playerName;
                    }
                }
            }
        } else if (capotReussi) {
            // Capot réalisé mais non annoncé
            for (int i = 0; i < room->connectionIds.size(); i++) {
                PlayerConnection* conn = m_connections[room->connectionIds[i]];
                if (!conn || conn->playerName.isEmpty()) continue;

                int playerTeam = (i % 2 == 0) ? 1 : 2;
                int plisTeamRealisateur = (playerTeam == 1) ? (room->plisCountPlayer0 + room->plisCountPlayer2) : (room->plisCountPlayer1 + room->plisCountPlayer3);

                if (plisTeamRealisateur == 8) {
                    // Ce joueur fait partie de l'équipe qui a réalisé le capot
                    m_dbManager->updateCapotStats(conn->playerName, false);  // Capot non annoncé
                    qDebug() << "Stats capot non annoncé pour:" << conn->playerName;
                }
            }
        }

        if (isGeneraleAnnonce) {
            // Une générale a été annoncée, mettre à jour les stats pour le joueur qui l'a annoncée
            if (room->lastBidderIndex >= 0 && room->lastBidderIndex < room->connectionIds.size() && !room->isBot[room->lastBidderIndex]) {
                PlayerConnection* conn = m_connections[room->connectionIds[room->lastBidderIndex]];
                if (conn && !conn->playerName.isEmpty()) {
                    m_dbManager->updateGeneraleStats(conn->playerName, generaleReussie);
                    qDebug() << "Stats générale pour:" << conn->playerName << "(joueur" << room->lastBidderIndex << ") - Réussite:" << generaleReussie;
                }
            }
        }

        // Envoi les scores aux clients
        qDebug() << "GameServer - Envoi mancheFinished avec:";
        qDebug() << "  scoreMancheTeam1:" << scoreToAddTeam1;
        qDebug() << "  scoreMancheTeam2:" << scoreToAddTeam2;
        qDebug() << "  scoreTotalTeam1:" << room->scoreTeam1;
        qDebug() << "  scoreTotalTeam2:" << room->scoreTeam2;

        QJsonObject scoreMsg;
        scoreMsg["type"] = "mancheFinished";
        scoreMsg["scoreTotalTeam1"] = room->scoreTeam1;
        scoreMsg["scoreTotalTeam2"] = room->scoreTeam2;
        scoreMsg["scoreMancheTeam1"] = scoreToAddTeam1;  // Points finaux attribués pour cette manche
        scoreMsg["scoreMancheTeam2"] = scoreToAddTeam2;  // Points finaux attribués pour cette manche
        // Ajouter l'information du capot pour l'animation
        if (capotNonAnnonceTeam1) {
            scoreMsg["capotTeam"] = 1;
            qDebug() << "GameServer - Envoi notification CAPOT Team1 aux clients";
        } else if (capotNonAnnonceTeam2) {
            scoreMsg["capotTeam"] = 2;
            qDebug() << "GameServer - Envoi notification CAPOT Team2 aux clients";
        } else if (isCapotAnnonce && capotReussi) {
            scoreMsg["capotTeam"] = team1HasBid ? 1 : 2;
            qDebug() << "GameServer - Envoi notification CAPOT annoncé réussi Team" << (team1HasBid ? 1 : 2) << "aux clients";
        } else {
            scoreMsg["capotTeam"] = 0;
        }
        broadcastToRoom(roomId, scoreMsg);

        // Vérifier si une équipe a atteint 1000 points
        bool team1Won = room->scoreTeam1 >= 1000;
        bool team2Won = room->scoreTeam2 >= 1000;

        if (team1Won || team2Won) {
            // Une ou les deux équipes ont dépassé 1000 points
            int winner = 0;
            if (team1Won && team2Won) {
                // Les deux équipes ont dépassé 1000, celle avec le plus de points gagne
                winner = (room->scoreTeam1 > room->scoreTeam2) ? 1 : 2;
                qDebug() << "GameServer - Les deux equipes ont depasse 1000 points!";
                qDebug() << "GameServer - Equipe" << winner << "gagne avec"
                         << ((winner == 1) ? room->scoreTeam1 : room->scoreTeam2) << "points";
            } else if (team1Won) {
                winner = 1;
                qDebug() << "GameServer - Equipe 1 gagne avec" << room->scoreTeam1 << "points!";
            } else {
                winner = 2;
                qDebug() << "GameServer - Equipe 2 gagne avec" << room->scoreTeam2 << "points!";
            }

            // Mettre à jour les statistiques pour tous les joueurs enregistrés
            for (int i = 0; i < room->connectionIds.size(); i++) {
                PlayerConnection* conn = m_connections[room->connectionIds[i]];
                if (!conn || conn->playerName.isEmpty()) continue;

                // Vérifier si ce joueur est dans l'équipe gagnante
                int playerTeam = (i % 2 == 0) ? 1 : 2;  // Équipe 1: joueurs 0 et 2, Équipe 2: joueurs 1 et 3
                bool won = (playerTeam == winner);

                // Mettre à jour les stats de partie
                m_dbManager->updateGameStats(conn->playerName, won);
                qDebug() << "Stats de partie mises a jour pour" << conn->playerName << "Won:" << won;
            }


            QJsonObject gameOverMsg;
            gameOverMsg["type"] = "gameOver";
            gameOverMsg["winner"] = winner;
            gameOverMsg["scoreTeam1"] = room->scoreTeam1;
            gameOverMsg["scoreTeam2"] = room->scoreTeam2;
            broadcastToRoom(roomId, gameOverMsg);

            room->gameState = "finished";

            // Nettoyer la map de reconnexion pour cette partie terminée
            for (const QString& playerName : room->playerNames) {
                m_playerNameToRoomId.remove(playerName);
            }
        } else {
            // Aucune équipe n'a atteint 1000 points, on démarre une nouvelle manche
            qDebug() << "GameServer - Demarrage d'une nouvelle manche...";
            startNewManche(roomId);
        }
    }

    void startNewManche(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        qDebug() << "GameServer - Nouvelle manche: envoi de l'animation aux clients";

        // Envoyer le message d'animation "Nouvelle Manche" à tous les joueurs
        QJsonObject newMancheMsg;
        newMancheMsg["type"] = "newMancheAnimation";
        broadcastToRoom(roomId, newMancheMsg);

        // Attendre 3 secondes pour l'animation avant de distribuer les cartes
        QTimer::singleShot(3000, this, [this, roomId]() {
            doStartNewManche(roomId);
        });
    }

    void doStartNewManche(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        qDebug() << "GameServer - Nouvelle manche: melange et distribution des cartes";

        // Nettoyer les plis de la manche precedente
        room->plisTeam1.clear();
        room->plisTeam2.clear();
        room->scoreMancheTeam1 = 0;
        room->scoreMancheTeam2 = 0;
        room->beloteTeam1 = false;
        room->beloteTeam2 = false;
        room->beloteRoiJoue = false;
        room->beloteDameJouee = false;
        room->waitingForNextPli = false;  // Réinitialiser le flag d'attente entre les plis

        // Réinitialiser le tracking des cartes jouées pour l'IA
        room->resetPlayedCards();

        // Reinitialiser les compteurs de plis par joueur
        room->plisCountPlayer0 = 0;
        room->plisCountPlayer1 = 0;
        room->plisCountPlayer2 = 0;
        room->plisCountPlayer3 = 0;

        // Réinitialiser les mains des joueurs
        for (auto& player : room->players) {
            // Vider la main actuelle
            player->clearHand();
        }

        // Préparer le deck pour la nouvelle manche
        if (room->plisTeam1.empty() && room->plisTeam2.empty()) {
            // Première manche : créer un nouveau deck
            qDebug() << "GameServer - Première manche : création d'un nouveau deck";
            room->deck.resetDeck();
            room->deck.shuffleDeck();
        } else {
            // Manche suivante : reconstruire le deck avec les plis des équipes
            qDebug() << "GameServer - Reconstruction du deck :";
            qDebug() << "  Equipe 1:" << room->plisTeam1.size() << "cartes";
            qDebug() << "  Equipe 2:" << room->plisTeam2.size() << "cartes";

            // Rassembler toutes les cartes : équipe 1 d'abord, puis équipe 2
            std::vector<Carte*> allCards;
            for (const auto& pair : room->plisTeam1) {
                // Réinitialiser l'état d'atout de la carte
                pair.second->setAtout(false);
                allCards.push_back(pair.second);
            }
            for (const auto& pair : room->plisTeam2) {
                // Réinitialiser l'état d'atout de la carte
                pair.second->setAtout(false);
                allCards.push_back(pair.second);
            }

            room->deck.rebuildFromCards(allCards);

            // Couper le deck (comme dans la vraie partie)
            room->deck.cutDeck();
            qDebug() << "GameServer - Deck coupé";
        }

        // Redistribuer les cartes en 3-2-3 (méthode réaliste)
        std::vector<Carte*> main1, main2, main3, main4;
        room->deck.distribute323(main1, main2, main3, main4);

        // Ajouter les cartes aux mains des joueurs
        for (Carte* carte : main1) {
            room->players[0]->addCardToHand(carte);
        }
        for (Carte* carte : main2) {
            room->players[1]->addCardToHand(carte);
        }
        for (Carte* carte : main3) {
            room->players[2]->addCardToHand(carte);
        }
        for (Carte* carte : main4) {
            room->players[3]->addCardToHand(carte);
        }

        // Réinitialiser l'état d'atout de toutes les cartes distribuées
        for (int i = 0; i < 4; i++) {
            const auto& main = room->players[i]->getMain();
            for (Carte* carte : main) {
                carte->setAtout(false);
            }
        }

        // Réinitialiser l'état de la partie pour les enchères
        room->gameState = "bidding";
        room->passedBidsCount = 0;
        room->lastBidAnnonce = Player::ANNONCEINVALIDE;
        room->lastBidCouleur = Carte::COULEURINVALIDE;
        room->lastBidderIndex = -1;
        room->couleurAtout = Carte::COULEURINVALIDE;
        room->isToutAtout = false;  // Réinitialiser le mode Tout Atout
        room->isSansAtout = false;  // Réinitialiser le mode Sans Atout
        room->coinched = false;
        room->surcoinched = false;
        room->coinchePlayerIndex = -1;
        room->currentPli.clear();
        room->couleurDemandee = Carte::COULEURINVALIDE;

        // Le joueur suivant commence les enchères (rotation)
        room->firstPlayerIndex = (room->firstPlayerIndex + 1) % 4;
        room->currentPlayerIndex = room->firstPlayerIndex;
        room->biddingPlayer = room->firstPlayerIndex;

        qDebug() << "GameServer - Nouvelle manche: joueur" << room->firstPlayerIndex << "commence les enchères";

        // Notifier tous les joueurs de la nouvelle manche avec leurs nouvelles cartes
        notifyNewManche(roomId);

        // Si le premier joueur à annoncer est un bot, le faire annoncer automatiquement
        if (room->isBot[room->currentPlayerIndex]) {
            int firstBidder = room->currentPlayerIndex;
            QTimer::singleShot(800, this, [this, roomId, firstBidder]() {
                GameRoom* room = m_gameRooms.value(roomId);
                if (!room || room->gameState != "bidding") return;

                // Revérifier que le joueur est toujours un bot et que c'est son tour
                if (room->currentPlayerIndex != firstBidder || !room->isBot[firstBidder]) {
                    qDebug() << "playBotBid ANNULÉ - Joueur" << firstBidder << "n'est plus bot ou ce n'est plus son tour";
                    return;
                }

                playBotBid(roomId, firstBidder);
            });
        } else {
            // Joueur humain : démarrer le timer de timeout pour les enchères
            startBidTimeout(roomId, room->currentPlayerIndex);
        }
    }

    void notifyNewManche(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        qDebug() << "Envoi des notifications de nouvelle manche a" << room->connectionIds.size() << "joueurs";

        for (int i = 0; i < room->connectionIds.size(); i++) {
            // Envoyer à tous les joueurs connectés, même les bots
            // Car un joueur peut être bot temporairement et se réhumaniser
            PlayerConnection *conn = m_connections.value(room->connectionIds[i]);
            if (!conn || !conn->socket) {
                qDebug() << "Joueur" << i << "pas de connexion valide, skip";
                continue;
            }

            QJsonObject msg;
            msg["type"] = "newManche";
            msg["roomId"] = roomId;
            msg["playerPosition"] = i;
            msg["biddingPlayer"] = room->biddingPlayer;
            msg["currentPlayer"] = room->currentPlayerIndex;

            // Envoi les nouvelles cartes du joueur
            QJsonArray myCards;
            const auto& playerHand = room->players[i]->getMain();
            for (const auto* carte : playerHand) {
                if (carte) {
                    QJsonObject cardObj;
                    cardObj["value"] = static_cast<int>(carte->getChiffre());
                    cardObj["suit"] = static_cast<int>(carte->getCouleur());
                    myCards.append(cardObj);
                }
            }
            msg["myCards"] = myCards;

            sendMessage(conn->socket, msg);
            qDebug() << "Nouvelle manche envoyée au joueur" << i;
        }
    }

    void handleMakeBid(QWebSocket *socket, const QJsonObject &data) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        PlayerConnection* conn = m_connections[connectionId];
        int roomId = conn->gameRoomId;
        if (roomId == -1) return;

        GameRoom* room = m_gameRooms[roomId];
        if (!room) return;

        // Arrêter le timer de timeout des enchères et invalider les anciens callbacks
        if (room->bidTimeout) {
            room->bidTimeout->stop();
            room->bidTimeoutGeneration++;
            qDebug() << "handleMakeBid - Timer de timeout enchères arrêté (joueur a annoncé), génération:" << room->bidTimeoutGeneration;
        }

        int playerIndex = conn->playerIndex;
        int bidValue = data["bidValue"].toInt();
        int suit = data["suit"].toInt();

        Player::Annonce annonce = static_cast<Player::Annonce>(bidValue);

        // Gestion COINCHE et SURCOINCHE
        if (annonce == Player::COINCHE) {
            // Vérifier qu'il y a une enchère en cours
            if (room->lastBidAnnonce == Player::ANNONCEINVALIDE) {
                qDebug() << "GameServer - COINCHE impossible: aucune enchère en cours";
                return;
            }

            // Vérifier que le joueur est dans l'équipe adverse
            int lastBidderTeam = room->lastBidderIndex % 2;  // 0 ou 1
            int playerTeam = playerIndex % 2;  // 0 ou 1
            if (lastBidderTeam == playerTeam) {
                qDebug() << "GameServer - COINCHE impossible: joueur de la même équipe";
                return;
            }

            room->coinched = true;
            room->coinchePlayerIndex = playerIndex;  // Enregistrer qui a coinché
            qDebug() << "GameServer - Joueur" << playerIndex << "COINCHE l'enchère!";

            // Enregistrer la tentative de coinche dans les stats (si joueur enregistré)
            if (!room->isBot[playerIndex]) {
                PlayerConnection* coincheConn = m_connections[room->connectionIds[playerIndex]];
                if (coincheConn && !coincheConn->playerName.isEmpty()) {
                    m_dbManager->updateCoincheStats(coincheConn->playerName, true, false);
                }
            }

            // Broadcast COINCHE
            QJsonObject msg;
            msg["type"] = "bidMade";
            msg["playerIndex"] = playerIndex;
            msg["bidValue"] = bidValue;
            msg["suit"] = suit;
            broadcastToRoom(roomId, msg);

            // Mettre à jour l'état: plus personne n'est en train d'annoncer
            QJsonObject stateMsg;
            stateMsg["type"] = "gameState";
            stateMsg["biddingPlayer"] = -1;  // Plus personne n'annonce
            stateMsg["biddingPhase"] = true;  // Toujours en phase d'enchères (pour coinche/surcoinche)
            broadcastToRoom(roomId, stateMsg);

            // Démarrer le timer de 10 secondes pour permettre la surcoinche
            startSurcoincheTimer(roomId);
            return;
        }

        if (annonce == Player::SURCOINCHE) {
            // Vérifie que COINCHE a été annoncé
            if (!room->coinched) {
                qDebug() << "GameServer - SURCOINCHE impossible: aucune COINCHE en cours";
                return;
            }

            // Vérifier que le joueur est dans l'équipe qui a fait l'enchère
            int lastBidderTeam = room->lastBidderIndex % 2;  // 0 ou 1
            int playerTeam = playerIndex % 2;  // 0 ou 1
            if (lastBidderTeam != playerTeam) {
                qDebug() << "GameServer - SURCOINCHE impossible: joueur de l'équipe adverse";
                return;
            }

            room->surcoinched = true;
            room->surcoinchePlayerIndex = playerIndex;
            qDebug() << "GameServer - Joueur" << playerIndex << "SURCOINCHE l'enchère!";

            // Enregistrer la tentative de surcoinche dans les stats (si joueur enregistré)
            if (!room->isBot[playerIndex]) {
                PlayerConnection* surcoincheConn = m_connections[room->connectionIds[playerIndex]];
                if (surcoincheConn && !surcoincheConn->playerName.isEmpty()) {
                    m_dbManager->updateSurcoincheStats(surcoincheConn->playerName, true, false);
                }
            }

            // Arrêter le timer de surcoinche
            stopSurcoincheTimer(roomId);

            // Notifier tous les joueurs que la surcoinche a été acceptée (masquer le bouton)
            QJsonObject timeoutMsg;
            timeoutMsg["type"] = "surcoincheTimeout";
            broadcastToRoom(roomId, timeoutMsg);

            // Broadcast la SURCOINCHE
            QJsonObject msg;
            msg["type"] = "bidMade";
            msg["playerIndex"] = playerIndex;
            msg["bidValue"] = bidValue;
            msg["suit"] = suit;
            broadcastToRoom(roomId, msg);

            // Attendre 2 secondes pour afficher l'animation "Surcoinche !"
            qDebug() << "GameServer - Attente de 2 secondes pour afficher l'animation Surcoinche";
            QTimer::singleShot(2000, this, [this, roomId]() {
                GameRoom* room = m_gameRooms.value(roomId);
                if (!room) return;

                // Fin des enchères, lancement phase de jeu
                qDebug() << "GameServer - SURCOINCHE annoncé! Fin des encheres, lancement phase de jeu";
                for (int i = 0; i < 4; i++) {
                    // IMPORTANT: Réinitialiser toutes les cartes avant de définir le nouvel atout
                    // Cela évite que les cartes gardent l'état atout de la manche précédente (TA/SA)
                    const auto& main = room->players[i]->getMain();
                    for (Carte* carte : main) {
                        carte->setAtout(false);
                    }

                    if (room->isToutAtout) {
                        room->players[i]->setAllCardsAsAtout();
                    } else if (room->isSansAtout) {
                        room->players[i]->setNoAtout();
                    } else {
                        room->players[i]->setAtout(room->lastBidCouleur);
                    }
                }
                startPlayingPhase(roomId);
            });
            return;
        }

        // Mise à jour de l'état de la room pour enchères normales
        if (annonce == Player::PASSE) {
            room->passedBidsCount++;
            qDebug() << "GameServer - Joueur" << playerIndex << "passe ("
                     << room->passedBidsCount << "/3 passes)";
        } else {
            room->lastBidAnnonce = annonce;

            // Gérer les modes spéciaux: Tout Atout (suit == 7) et Sans Atout (suit == 8)
            if (suit == 7) {
                room->isToutAtout = true;
                room->isSansAtout = false;
                // En mode TA, utiliser COULEURINVALIDE pour ne pas highlighter une couleur spécifique
                room->lastBidCouleur = Carte::COULEURINVALIDE;
                qDebug() << "GameServer - Mode TOUT ATOUT activé! (isToutAtout=true, isSansAtout=false)";
            } else if (suit == 8) {
                room->isToutAtout = false;
                room->isSansAtout = true;
                // En mode SA, utiliser COULEURINVALIDE pour ne pas highlighter une couleur spécifique
                room->lastBidCouleur = Carte::COULEURINVALIDE;
                qDebug() << "GameServer - Mode SANS ATOUT activé! (isToutAtout=false, isSansAtout=true)";
            } else {
                room->isToutAtout = false;
                room->isSansAtout = false;
                room->lastBidCouleur = static_cast<Carte::Couleur>(suit);
                qDebug() << "GameServer - Mode NORMAL activé pour couleur" << suit << "(isToutAtout=false, isSansAtout=false)";
            }

            room->lastBidderIndex = playerIndex;
            room->passedBidsCount = 0;  // Reset le compteur
            qDebug() << "GameServer - Nouvelle enchere:" << bidValue << "couleur:" << suit;
        }

        // Broadcast l'enchère à tous
        QJsonObject msg;
        msg["type"] = "bidMade";
        msg["playerIndex"] = playerIndex;
        msg["bidValue"] = bidValue;
        msg["suit"] = suit;
        broadcastToRoom(roomId, msg);

        // Verifie si phase d'encheres terminee
        if (room->passedBidsCount >= 3 && room->lastBidAnnonce != Player::ANNONCEINVALIDE) {
            qDebug() << "GameServer - Fin des encheres! Lancement phase de jeu";
            for (int i = 0; i < 4; i++) {
                // IMPORTANT: Réinitialiser toutes les cartes avant de définir le nouvel atout
                const auto& main = room->players[i]->getMain();
                for (Carte* carte : main) {
                    carte->setAtout(false);
                }

                if (room->isToutAtout) {
                    room->players[i]->setAllCardsAsAtout();
                } else if (room->isSansAtout) {
                    room->players[i]->setNoAtout();
                } else {
                    room->players[i]->setAtout(room->lastBidCouleur);
                }
            }
            startPlayingPhase(roomId);
        } else if (room->passedBidsCount >= 4 && room->lastBidAnnonce == Player::ANNONCEINVALIDE) {
            // Tous les joueurs ont passe sans annonce -> nouvelle manche
            qDebug() << "GameServer - Tous les joueurs ont passe! Nouvelle manche";
            startNewManche(roomId);
        } else {
            // Passe au joueur suivant (utiliser la fonction centralisée qui gère aussi le timer)
            advanceToNextBidder(roomId);
        }
    }

    void handleForfeit(QWebSocket *socket) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        PlayerConnection* conn = m_connections[connectionId];
        int roomId = conn->gameRoomId;
        if (roomId == -1) return;

        GameRoom* room = m_gameRooms[roomId];
        if (!room) return;

        int playerIndex = conn->playerIndex;
        qDebug() << "GameServer - Joueur" << playerIndex << "(" << conn->playerName << ") abandonne la partie";

        // Incrémenter le compteur de parties jouées (défaite) pour ce joueur
        if (!conn->playerName.isEmpty()) {
            m_dbManager->updateGameStats(conn->playerName, false);  // false = défaite
            qDebug() << "Stats mises a jour pour" << conn->playerName << "- Defaite enregistree";
        }

        // Remplacer le joueur par un bot
        room->isBot[playerIndex] = true;
        qDebug() << "Joueur" << playerIndex << "remplace par un bot";

        // Retirer le joueur de la partie - il ne pourra plus la rejoindre
        // Ceci s'applique UNIQUEMENT aux abandons volontaires (clic sur "Quitter")
        // Les déconnexions involontaires passent par onDisconnected() qui garde le joueur dans la partie
        conn->gameRoomId = -1;
        conn->playerIndex = -1;

        // IMPORTANT: Retirer le joueur de la map de reconnexion pour qu'il ne puisse pas
        // rejoindre cette partie, même s'il clique sur "Jouer" ensuite
        m_playerNameToRoomId.remove(conn->playerName);

        qDebug() << "Joueur" << conn->playerName << "retire de la partie (abandon volontaire), peut rejoindre une nouvelle partie";

        // Notifier tous les joueurs qu'un joueur a abandonné et a été remplacé par un bot
        QJsonObject forfeitMsg;
        forfeitMsg["type"] = "playerForfeited";
        forfeitMsg["playerIndex"] = playerIndex;
        forfeitMsg["playerName"] = conn->playerName;
        broadcastToRoom(roomId, forfeitMsg);

        // Si c'est le tour du joueur qui abandonne, faire jouer le bot immédiatement
        if (room->currentPlayerIndex == playerIndex) {
            if (room->gameState == "bidding") {
                // Phase d'enchères : passer automatiquement
                QTimer::singleShot(500, this, [this, roomId, playerIndex]() {
                    playBotBid(roomId, playerIndex);
                });
            } else if (room->gameState == "playing") {
                // Phase de jeu : jouer une carte aléatoire
                QTimer::singleShot(500, this, [this, roomId, playerIndex]() {
                    playBotCard(roomId, playerIndex);
                });
            }
        }
    }

    // Évalue la force d'une main pour une couleur d'atout donnée
    // Retourne un score estimé de points que le bot peut espérer faire
    int evaluateHandForSuit(Player* player, Carte::Couleur atoutCouleur) {
        int score = 0;
        int atoutCount = 0;
        bool hasValetAtout = false;
        bool hasNeufAtout = false;
        bool hasRoiAtout = false;
        bool hasDameAtout = false;

        const auto& main = player->getMain();

        for (Carte* carte : main) {
            Carte::Couleur couleur = carte->getCouleur();
            Carte::Chiffre chiffre = carte->getChiffre();

            if (couleur == atoutCouleur) {
                // C'est un atout
                atoutCount++;

                if (chiffre == Carte::VALET) {
                    hasValetAtout = true;
                    score += 20; // Valet d'atout : 20 points
                } else if (chiffre == Carte::NEUF) {
                    hasNeufAtout = true;
                    score += 14; // 9 d'atout : 14 points
                } else if (chiffre == Carte::AS) {
                    score += 11;
                } else if (chiffre == Carte::DIX) {
                    score += 10;
                } else if (chiffre == Carte::ROI) {
                    hasRoiAtout = true;
                    score += 4;
                } else if (chiffre == Carte::DAME) {
                    hasDameAtout = true;
                    score += 3;
                }
            } else {
                // Hors atout
                if (chiffre == Carte::AS) {
                    score += 10; // Bonus pour As hors atout (maître potentiel)
                }
            }
        }

        // Bonus pour la belote (Roi + Dame d'atout)
        if (hasRoiAtout && hasDameAtout) {
            score += 20;
        }

        // Bonus pour nombre d'atouts (plus on en a, mieux c'est)
        if (atoutCount >= 5) {
            score += 30;
        } else if (atoutCount >= 4) {
            score += 20;
        } else if (atoutCount >= 3) {
            score += 10;
        } else if (atoutCount < 2) {
            // Pénalité si trop peu d'atouts
            score -= 20;
        }

        // Bonus si on a le Valet ET le 9 d'atout (très fort)
        if (hasValetAtout && hasNeufAtout) {
            score += 15;
        }

        return score;
    }

    // Évalue la main pour soutenir l'annonce du partenaire
    // Retourne un score bonus à ajouter si le bot veut surenchérir sur l'atout du partenaire
    int evaluateHandForPartnerSuit(Player* player, Carte::Couleur partnerAtout) {
        int score = 0;
        int atoutCount = 0;
        bool hasValetAtout = false;
        bool hasNeufAtout = false;
        bool hasRoiAtout = false;
        bool hasDameAtout = false;

        const auto& main = player->getMain();

        for (Carte* carte : main) {
            Carte::Couleur couleur = carte->getCouleur();
            Carte::Chiffre chiffre = carte->getChiffre();

            if (couleur == partnerAtout) {
                // C'est un atout (la couleur du partenaire)
                atoutCount++;

                if (chiffre == Carte::VALET) {
                    hasValetAtout = true;
                    score += 15; // Valet d'atout pour soutien : +15
                } else if (chiffre == Carte::NEUF) {
                    hasNeufAtout = true;
                    score += 10; // 9 d'atout pour soutien : +10
                } else if (chiffre == Carte::ROI) {
                    hasRoiAtout = true;
                } else if (chiffre == Carte::DAME) {
                    hasDameAtout = true;
                }
            } else {
                // Hors atout - les As sont très utiles pour le partenaire
                if (chiffre == Carte::AS) {
                    score += 10; // As hors atout : +10 (on peut faire des plis)
                }
            }
        }

        // Bonus pour la belote (Roi + Dame de l'atout du partenaire)
        if (hasRoiAtout && hasDameAtout) {
            score += 15; // Belote pour soutien : +15
        }

        // Bonus si on a des atouts pour soutenir
        if (hasNeufAtout && atoutCount >= 2) {
            score += 10; // 9 + autres atouts : bon soutien
        }

        // Bonus si on a le Valet (très fort pour soutenir)
        if (hasValetAtout) {
            score += 5; // Bonus additionnel si on a le Valet
        }

        return score;
    }

    // Convertit un score d'évaluation en annonce
    Player::Annonce scoreToAnnonce(int score, Player::Annonce currentBid) {
        Player::Annonce annonce = Player::PASSE;

        // Définir l'annonce en fonction du score
        if (score >= 140) {
            annonce = Player::CENTSOIXANTE;
        } else if (score >= 130) {
            annonce = Player::CENTCINQUANTE;
        } else if (score >= 120) {
            annonce = Player::CENTQUARANTE;
        } else if (score >= 110) {
            annonce = Player::CENTTRENTE;
        } else if (score >= 100) {
            annonce = Player::CENTVINGT;
        } else if (score >= 90) {
            annonce = Player::CENTDIX;
        } else if (score >= 80) {
            annonce = Player::CENT;
        } else if (score >= 70) {
            annonce = Player::QUATREVINGTDIX;
        } else if (score >= 60) {
            annonce = Player::QUATREVINGT;
        }

        // S'assurer que l'annonce est supérieure à l'annonce actuelle
        if (annonce != Player::PASSE && annonce <= currentBid) {
            // Essayer d'annoncer un cran au-dessus si le score le permet
            if (currentBid < Player::CENTSOIXANTE && score >= 60) {
                annonce = static_cast<Player::Annonce>(static_cast<int>(currentBid) + 1);
                // Vérifier qu'on ne dépasse pas nos moyens
                int requiredScore = 60 + (static_cast<int>(annonce) - 1) * 10;
                if (score < requiredScore) {
                    annonce = Player::PASSE;
                }
            } else {
                annonce = Player::PASSE;
            }
        }

        return annonce;
    }

    void playBotBid(int roomId, int playerIndex) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room || room->currentPlayerIndex != playerIndex) return;

        // IMPORTANT: Vérifier que le joueur est toujours un bot
        // Il peut avoir été réhumanisé entre-temps
        if (!room->isBot[playerIndex]) {
            qDebug() << "playBotBid - ANNULÉ: Joueur" << playerIndex << "n'est plus un bot (réhumanisé)";
            return;
        }

        // Arrêter le timer de timeout et invalider les anciens callbacks
        if (room->bidTimeout) {
            room->bidTimeout->stop();
            room->bidTimeoutGeneration++;
            qDebug() << "playBotBid - Timer de timeout enchères arrêté, génération:" << room->bidTimeoutGeneration;
        }

        Player* player = room->players[playerIndex].get();

        // Évaluer la main pour chaque couleur d'atout possible (annonce propre)
        int bestOwnScore = 0;
        Carte::Couleur bestOwnCouleur = Carte::COULEURINVALIDE;

        std::array<Carte::Couleur, 4> couleurs = {Carte::COEUR, Carte::TREFLE, Carte::CARREAU, Carte::PIQUE};

        for (Carte::Couleur couleur : couleurs) {
            int score = evaluateHandForSuit(player, couleur);
            qDebug() << "Bot" << playerIndex << "- Evaluation couleur" << static_cast<int>(couleur) << ":" << score;
            if (score > bestOwnScore) {
                bestOwnScore = score;
                bestOwnCouleur = couleur;
            }
        }

        // Vérifier si le partenaire a déjà annoncé
        int partnerIndex = (playerIndex + 2) % 4;
        bool partnerHasBid = (room->lastBidderIndex == partnerIndex &&
                              room->lastBidAnnonce != Player::ANNONCEINVALIDE);

        int bestScore = bestOwnScore;
        Carte::Couleur bestCouleur = bestOwnCouleur;

        if (partnerHasBid) {
            // Le partenaire a annoncé - évaluer si on peut soutenir
            Carte::Couleur partnerCouleur = room->lastBidCouleur;
            int supportScore = evaluateHandForPartnerSuit(player, partnerCouleur);

            qDebug() << "Bot" << playerIndex << "- Partenaire a annoncé en"
                     << static_cast<int>(partnerCouleur) << ", score soutien:" << supportScore;

            // Si on a un bon soutien, calculer le score équivalent pour comparer
            if (supportScore >= 25) {
                int supportTotalScore = supportScore + 60; // Score équivalent pour surenchérir

                // Comparer : soutenir le partenaire vs annoncer soi-même
                if (supportTotalScore > bestOwnScore) {
                    qDebug() << "Bot" << playerIndex << "- Préfère soutenir le partenaire ("
                             << supportTotalScore << ") vs propre annonce (" << bestOwnScore << ")";
                    bestScore = supportTotalScore;
                    bestCouleur = partnerCouleur;
                } else {
                    qDebug() << "Bot" << playerIndex << "- Préfère sa propre annonce ("
                             << bestOwnScore << ") vs soutien (" << supportTotalScore << ")";
                }
            }
        }

        // Déterminer l'annonce en fonction du score
        Player::Annonce annonce = scoreToAnnonce(bestScore, room->lastBidAnnonce);

        qDebug() << "GameServer - Bot joueur" << playerIndex << "meilleur score:" << bestScore
                 << "couleur:" << static_cast<int>(bestCouleur) << "annonce:" << static_cast<int>(annonce);

        if (annonce == Player::PASSE) {
            // Le bot passe
            qDebug() << "GameServer - Bot joueur" << playerIndex << "passe";
            room->passedBidsCount++;

            // Broadcast l'enchère à tous
            QJsonObject msg;
            msg["type"] = "bidMade";
            msg["playerIndex"] = playerIndex;
            msg["bidValue"] = static_cast<int>(Player::PASSE);
            msg["suit"] = 0;
            broadcastToRoom(roomId, msg);

            // Vérifier si phase d'enchères terminée
            if (room->passedBidsCount >= 3 && room->lastBidAnnonce != Player::ANNONCEINVALIDE) {
                qDebug() << "GameServer - Fin des encheres! Lancement phase de jeu";
                for (int i = 0; i < 4; i++) {
                    // IMPORTANT: Réinitialiser toutes les cartes avant de définir le nouvel atout
                    const auto& main = room->players[i]->getMain();
                    for (Carte* carte : main) {
                        carte->setAtout(false);
                    }

                    if (room->isToutAtout) {
                        room->players[i]->setAllCardsAsAtout();
                    } else if (room->isSansAtout) {
                        room->players[i]->setNoAtout();
                    } else {
                        room->players[i]->setAtout(room->lastBidCouleur);
                    }
                }
                startPlayingPhase(roomId);
            } else if (room->passedBidsCount >= 4 && room->lastBidAnnonce == Player::ANNONCEINVALIDE) {
                // Tous les joueurs ont passé sans annonce -> nouvelle manche
                qDebug() << "GameServer - Tous les joueurs ont passe! Nouvelle manche";
                startNewManche(roomId);
            } else {
                // Passer au joueur suivant
                advanceToNextBidder(roomId);
            }
        } else {
            // Le bot fait une annonce
            qDebug() << "GameServer - Bot joueur" << playerIndex << "annonce"
                     << static_cast<int>(annonce) << "en" << static_cast<int>(bestCouleur);

            room->passedBidsCount = 0;  // Réinitialiser le compteur de passes
            room->lastBidAnnonce = annonce;
            room->lastBidCouleur = bestCouleur;
            room->lastBidderIndex = playerIndex;
            room->couleurAtout = bestCouleur;

            // IMPORTANT: Les bots ne parlent que des couleurs normales (COEUR, TREFLE, CARREAU, PIQUE)
            // Si un joueur a parlé TA ou SA et qu'un bot surenchérit avec une couleur normale,
            // il faut désactiver les modes TA/SA
            room->isToutAtout = false;
            room->isSansAtout = false;
            qDebug() << "GameServer - Bot surenchérit avec couleur normale, désactivation TA/SA (isToutAtout=false, isSansAtout=false)";

            // Broadcast l'enchère à tous
            QJsonObject msg;
            msg["type"] = "bidMade";
            msg["playerIndex"] = playerIndex;
            msg["bidValue"] = static_cast<int>(annonce);
            msg["suit"] = static_cast<int>(bestCouleur);
            broadcastToRoom(roomId, msg);

            // Passer au joueur suivant
            advanceToNextBidder(roomId);
        }
    }

    // Fonction utilitaire pour passer au prochain enchérisseur
    void advanceToNextBidder(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        room->currentPlayerIndex = (room->currentPlayerIndex + 1) % 4;
        room->biddingPlayer = room->currentPlayerIndex;

        QJsonObject stateMsg;
        stateMsg["type"] = "gameState";
        stateMsg["currentPlayer"] = room->currentPlayerIndex;
        stateMsg["biddingPlayer"] = room->biddingPlayer;
        stateMsg["biddingPhase"] = true;
        broadcastToRoom(roomId, stateMsg);

        // Si le prochain joueur est aussi un bot, le faire jouer
        if (room->isBot[room->currentPlayerIndex]) {
            QTimer::singleShot(800, this, [this, roomId]() {
                GameRoom* room = m_gameRooms.value(roomId);
                if (room && room->gameState == "bidding") {
                    playBotBid(roomId, room->currentPlayerIndex);
                }
            });
        } else {
            // Joueur humain : démarrer le timer de timeout pour les enchères
            startBidTimeout(roomId, room->currentPlayerIndex);
        }
    }

    // Démarre le timer de timeout pour la phase d'enchères (15 secondes)
    void startBidTimeout(int roomId, int currentBidder) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room || room->gameState != "bidding") return;

        // Créer le timer si nécessaire
        if (!room->bidTimeout) {
            room->bidTimeout = new QTimer(this);
            room->bidTimeout->setSingleShot(true);
        }

        // Arrêter et déconnecter l'ancien timer
        room->bidTimeout->stop();
        disconnect(room->bidTimeout, nullptr, this, nullptr);

        // Incrémenter la génération pour invalider les anciens callbacks en queue
        room->bidTimeoutGeneration++;
        int currentGeneration = room->bidTimeoutGeneration;

        qDebug() << "startBidTimeout - Timer de 15s démarré pour joueur" << currentBidder << "(génération:" << currentGeneration << ")";

        // Démarrer le nouveau timer
        connect(room->bidTimeout, &QTimer::timeout, this, [this, roomId, currentBidder, currentGeneration]() {
            GameRoom* room = m_gameRooms.value(roomId);
            if (!room || room->gameState != "bidding") return;

            // Vérifier que ce timeout est toujours valide (pas un ancien signal en queue)
            if (room->bidTimeoutGeneration != currentGeneration) {
                qDebug() << "BID TIMEOUT - Ignoré (ancienne génération:" << currentGeneration
                         << "actuelle:" << room->bidTimeoutGeneration << ")";
                return;
            }

            // Vérifier que c'est toujours le tour de ce joueur
            if (room->currentPlayerIndex != currentBidder) {
                qDebug() << "BID TIMEOUT - Ignoré (joueur actuel:" << room->currentPlayerIndex
                         << ", timeout pour:" << currentBidder << ")";
                return;
            }

            qDebug() << "BID TIMEOUT - Joueur" << currentBidder << "n'a pas annoncé dans les temps!";

            // Marquer le joueur comme bot
            if (!room->isBot[currentBidder]) {
                room->isBot[currentBidder] = true;
                qDebug() << "BID TIMEOUT - Joueur" << currentBidder << "remplacé par un bot";

                // Envoyer une notification au joueur
                if (currentBidder < room->connectionIds.size()) {
                    PlayerConnection* conn = m_connections.value(room->connectionIds[currentBidder]);
                    if (conn && conn->socket) {
                        QJsonObject botMsg;
                        botMsg["type"] = "botReplacement";
                        botMsg["message"] = "Vous avez été remplacé par un bot car vous n'avez pas annoncé à temps.";
                        sendMessage(conn->socket, botMsg);
                    }
                }
            }

            // Faire jouer le bot à la place du joueur
            playBotBid(roomId, currentBidder);
        });

        room->bidTimeout->start(15000);  // 15 secondes
    }

    // Vérifie si le partenaire est le joueur qui gagne actuellement le pli
    bool isPartnerWinning(int playerIndex, int winningPlayerIndex) {
        // Les partenaires sont aux positions 0-2 et 1-3
        return (playerIndex % 2) == (winningPlayerIndex % 2);
    }

    // Calcule la valeur totale des points dans le pli actuel
    int calculatePliPoints(const std::vector<std::pair<int, Carte*>>& pli) {
        int points = 0;
        for (const auto& pair : pli) {
            points += pair.second->getValeurDeLaCarte();
        }
        return points;
    }

    // Trouve l'index de la carte avec la plus petite valeur
    int findLowestValueCard(Player* player, const std::vector<int>& playableIndices) {
        const auto& main = player->getMain();
        int lowestIdx = playableIndices[0];
        int lowestValue = main[lowestIdx]->getValeurDeLaCarte() * 100 + main[lowestIdx]->getOrdreCarteForte();

        for (int idx : playableIndices) {
            int value = main[idx]->getValeurDeLaCarte() * 100 + main[idx]->getOrdreCarteForte();
            if (value < lowestValue) {
                lowestValue = value;
                lowestIdx = idx;
            }
        }
        return lowestIdx;
    }

    // Trouve l'index de la carte avec la plus petite valeur en SANS ATOUT
    // Évite de défausser les As (cartes maîtres) si possible
    int findLowestValueCardSansAtout(Player* player, const std::vector<int>& playableIndices) {
        const auto& main = player->getMain();

        // D'abord, chercher les cartes qui ne sont PAS des As
        std::vector<int> nonAceIndices;
        for (int idx : playableIndices) {
            if (main[idx]->getChiffre() != Carte::AS) {
                nonAceIndices.push_back(idx);
            }
        }

        // Si on a des cartes qui ne sont pas des As, trouver la plus faible parmi elles
        if (!nonAceIndices.empty()) {
            return findLowestValueCard(player, nonAceIndices);
        }

        // Sinon, on doit défausser un As (on n'a que des As)
        return findLowestValueCard(player, playableIndices);
    }

    // Trouve l'index de la carte avec la plus petite valeur en TOUT ATOUT
    // Évite de défausser les Valets (cartes maîtres) si possible
    int findLowestValueCardToutAtout(Player* player, const std::vector<int>& playableIndices) {
        const auto& main = player->getMain();

        // D'abord, chercher les cartes qui ne sont PAS des Valets
        std::vector<int> nonJackIndices;
        for (int idx : playableIndices) {
            if (main[idx]->getChiffre() != Carte::VALET) {
                nonJackIndices.push_back(idx);
            }
        }

        // Si on a des cartes qui ne sont pas des Valets, trouver la plus faible parmi elles
        if (!nonJackIndices.empty()) {
            return findLowestValueCard(player, nonJackIndices);
        }

        // Sinon, on doit défausser un Valet (on n'a que des Valets)
        return findLowestValueCard(player, playableIndices);
    }

    // Trouve l'index de la carte avec la plus petite valeur en évitant l'atout
    // Si toutes les cartes jouables sont des atouts, retourne le plus petit atout
    int findLowestValueCardAvoidTrump(Player* player, const std::vector<int>& playableIndices,
                                       Carte::Couleur couleurAtout) {
        const auto& main = player->getMain();

        // D'abord, chercher les cartes hors atout
        std::vector<int> nonTrumpIndices;
        for (int idx : playableIndices) {
            if (main[idx]->getCouleur() != couleurAtout) {
                nonTrumpIndices.push_back(idx);
            }
        }

        // Si on a des cartes hors atout, trouver la plus faible parmi elles
        if (!nonTrumpIndices.empty()) {
            int lowestIdx = nonTrumpIndices[0];
            int lowestValue = main[lowestIdx]->getValeurDeLaCarte() * 100 + main[lowestIdx]->getOrdreCarteForte();

            for (int idx : nonTrumpIndices) {
                int value = main[idx]->getValeurDeLaCarte() * 100 + main[idx]->getOrdreCarteForte();
                if (value < lowestValue) {
                    lowestValue = value;
                    lowestIdx = idx;
                }
            }
            return lowestIdx;
        }

        // Sinon, retourner la plus faible carte (qui sera un atout)
        return findLowestValueCard(player, playableIndices);
    }

    // Trouve l'index de la carte avec la plus grande valeur
    int findHighestValueCard(Player* player, const std::vector<int>& playableIndices) {
        const auto& main = player->getMain();
        int highestIdx = playableIndices[0];
        int highestValue = main[highestIdx]->getValeurDeLaCarte() * 100 + main[highestIdx]->getOrdreCarteForte();

        for (int idx : playableIndices) {
            int value = main[idx]->getValeurDeLaCarte() * 100 + main[idx]->getOrdreCarteForte();
            if (value > highestValue) {
                highestValue = value;
                highestIdx = idx;
            }
        }
        return highestIdx;
    }

    // Trouve la carte la plus faible qui bat la carte gagnante actuelle
    int findLowestWinningCard(Player* player, const std::vector<int>& playableIndices,
                              Carte* carteGagnante, Carte::Couleur couleurAtout) {
        const auto& main = player->getMain();
        int bestIdx = -1;
        int bestValue = 9999;

        for (int idx : playableIndices) {
            Carte* carte = main[idx];
            // Vérifier si cette carte bat la carte gagnante
            if (*carteGagnante < *carte) {
                int value = carte->getValeurDeLaCarte() * 100 + carte->getOrdreCarteForte();
                if (value < bestValue) {
                    bestValue = value;
                    bestIdx = idx;
                }
            }
        }
        return bestIdx;
    }

    // Choisit la meilleure carte à jouer selon la stratégie
    // Compte le nombre d'atouts restants chez les autres joueurs
    int countRemainingTrumps(GameRoom* room, Player* player) {
        int remaining = 0;
        Carte::Couleur atout = room->couleurAtout;

        std::array<Carte::Chiffre, 8> chiffres = {Carte::SEPT, Carte::HUIT, Carte::NEUF, Carte::DIX,
                                                   Carte::VALET, Carte::DAME, Carte::ROI, Carte::AS};

        for (auto chiffre : chiffres) {
            // Si la carte n'a pas été jouée et n'est pas dans ma main, elle est chez un autre
            if (!room->isCardPlayed(atout, chiffre)) {
                bool inMyHand = false;
                for (Carte* c : player->getMain()) {
                    if (c->getCouleur() == atout && c->getChiffre() == chiffre) {
                        inMyHand = true;
                        break;
                    }
                }
                if (!inMyHand) {
                    remaining++;
                }
            }
        }
        return remaining;
    }

    // Vérifie si l'As d'une couleur a été joué
    bool isAcePlayed(GameRoom* room, Carte::Couleur couleur) {
        return room->isCardPlayed(couleur, Carte::AS);
    }

    // Vérifie si le 10 d'une couleur est maître (l'As est tombé)
    bool isTenMaster(GameRoom* room, Carte::Couleur couleur, Player* player) {
        // Le 10 est maître si l'As de cette couleur a été joué
        // et si le 10 n'est pas lui-même dans ma main (sinon je le sais déjà)
        return isAcePlayed(room, couleur);
    }

    // Vérifie si une carte hors atout est maître (toutes les cartes plus fortes sont tombées)
    // Ordre hors atout: As > 10 > Roi > Dame > Valet > 9 > 8 > 7
    bool isMasterCard(GameRoom* room, Carte* carte) {
        Carte::Couleur couleur = carte->getCouleur();
        Carte::Chiffre chiffre = carte->getChiffre();

        // L'As est toujours maître s'il est encore en jeu
        if (chiffre == Carte::AS) {
            return true;
        }

        // Le 10 est maître si l'As est tombé
        if (chiffre == Carte::DIX) {
            return room->isCardPlayed(couleur, Carte::AS);
        }

        // Le Roi est maître si l'As et le 10 sont tombés
        if (chiffre == Carte::ROI) {
            return room->isCardPlayed(couleur, Carte::AS) &&
                   room->isCardPlayed(couleur, Carte::DIX);
        }

        // La Dame est maître si As, 10 et Roi sont tombés
        if (chiffre == Carte::DAME) {
            return room->isCardPlayed(couleur, Carte::AS) &&
                   room->isCardPlayed(couleur, Carte::DIX) &&
                   room->isCardPlayed(couleur, Carte::ROI);
        }

        // Le Valet est maître si As, 10, Roi et Dame sont tombés
        if (chiffre == Carte::VALET) {
            return room->isCardPlayed(couleur, Carte::AS) &&
                   room->isCardPlayed(couleur, Carte::DIX) &&
                   room->isCardPlayed(couleur, Carte::ROI) &&
                   room->isCardPlayed(couleur, Carte::DAME);
        }

        // Les autres cartes (9, 8, 7) ne sont généralement pas considérées comme maîtres
        return false;
    }

    // Vérifie si le Valet d'atout est tombé
    bool isTrumpJackPlayed(GameRoom* room) {
        return room->isCardPlayed(room->couleurAtout, Carte::VALET);
    }

    // Vérifie si le 9 d'atout est tombé
    bool isTrumpNinePlayed(GameRoom* room) {
        return room->isCardPlayed(room->couleurAtout, Carte::NEUF);
    }

    // Compte le nombre d'atouts déjà joués (tombés)
    int countPlayedTrumps(GameRoom* room) {
        int count = 0;
        Carte::Couleur atout = room->couleurAtout;

        std::array<Carte::Chiffre, 8> chiffres = {Carte::SEPT, Carte::HUIT, Carte::NEUF, Carte::DIX,
                                                   Carte::VALET, Carte::DAME, Carte::ROI, Carte::AS};

        for (auto chiffre : chiffres) {
            if (room->isCardPlayed(atout, chiffre)) {
                count++;
            }
        }
        return count;
    }

    // Stratégie pour SANS ATOUT (SA): pas d'atout, les cartes maîtres sont très importantes
    // L'ordre de force est: As > 10 > Roi > Dame > Valet > 9 > 8 > 7
    int chooseBestCardSansAtout(GameRoom* room, Player* player, int playerIndex,
                                const std::vector<int>& playableIndices,
                                Carte* carteGagnante, int idxPlayerWinning) {
        const auto& main = player->getMain();
        bool isAttackingTeam = (playerIndex % 2) == (room->lastBidderIndex % 2);

        // Cas 1: Le bot commence le pli
        if (room->currentPli.empty()) {
            qDebug() << "Bot" << playerIndex << "commence le pli en SANS ATOUT";

            // Stratégie: Jouer les As en priorité (cartes maîtres)
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getChiffre() == Carte::AS) {
                    qDebug() << "Bot" << playerIndex << "- [SA] Joue un As";
                    return idx;
                }
            }

            // Jouer un 10 si l'As de cette couleur est déjà tombé (le 10 devient maître)
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getChiffre() == Carte::DIX && isAcePlayed(room, carte->getCouleur())) {
                    qDebug() << "Bot" << playerIndex << "- [SA] Joue un 10 maître";
                    return idx;
                }
            }

            // Si on attaque, jouer les cartes fortes (Roi, 10)
            if (isAttackingTeam) {
                for (int idx : playableIndices) {
                    Carte* carte = main[idx];
                    if (carte->getChiffre() == Carte::ROI || carte->getChiffre() == Carte::DIX) {
                        qDebug() << "Bot" << playerIndex << "- [SA] Joue une carte forte";
                        return idx;
                    }
                }
            }

            // Sinon, jouer la carte la plus faible pour économiser les fortes
            return findLowestValueCardSansAtout(player, playableIndices);
        }

        // Cas 2: Le partenaire gagne le pli
        if (isPartnerWinning(playerIndex, idxPlayerWinning)) {
            qDebug() << "Bot" << playerIndex << "- [SA] Partenaire gagne, défausse (évite As)";
            return findLowestValueCardSansAtout(player, playableIndices);
        }

        // Cas 3: L'adversaire gagne le pli
        // Essayer de reprendre avec une carte plus forte
        int bestCardIdx = -1;
        int bestOrder = -1;

        for (int idx : playableIndices) {
            Carte* carte = main[idx];
            // Vérifier si cette carte peut battre la carte gagnante
            if (carte->getCouleur() == carteGagnante->getCouleur()) {
                int order = carte->getOrdreCarteForte();
                if (order > carteGagnante->getOrdreCarteForte() && order > bestOrder) {
                    bestOrder = order;
                    bestCardIdx = idx;
                }
            }
        }

        // Si on peut gagner, jouer la plus petite carte gagnante
        if (bestCardIdx != -1) {
            qDebug() << "Bot" << playerIndex << "- [SA] Peut gagner, joue carte gagnante";
            return bestCardIdx;
        }

        // Sinon, défausser la plus petite carte (évite les As)
        qDebug() << "Bot" << playerIndex << "- [SA] Ne peut pas gagner, défausse (évite As)";
        return findLowestValueCardSansAtout(player, playableIndices);
    }

    // Stratégie pour TOUT ATOUT (TA): toutes les cartes sont atouts
    // L'ordre de force est: Valet > 9 > As > 10 > Roi > Dame > 8 > 7
    // IMPORTANT: On est obligé de monter si on peut (règle du jeu)
    int chooseBestCardToutAtout(GameRoom* room, Player* player, int playerIndex,
                                const std::vector<int>& playableIndices,
                                Carte* carteGagnante, int idxPlayerWinning) {
        const auto& main = player->getMain();
        bool isAttackingTeam = (playerIndex % 2) == (room->lastBidderIndex % 2);

        // Cas 1: Le bot commence le pli
        if (room->currentPli.empty()) {
            qDebug() << "Bot" << playerIndex << "commence le pli en TOUT ATOUT";

            // Stratégie: Jouer les cartes maîtres (Valet, 9, As)
            // Jouer le Valet si on l'a
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getChiffre() == Carte::VALET) {
                    qDebug() << "Bot" << playerIndex << "- [TA] Joue un Valet";
                    return idx;
                }
            }

            // Jouer un 9 si on l'a
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getChiffre() == Carte::NEUF) {
                    qDebug() << "Bot" << playerIndex << "- [TA] Joue un 9";
                    return idx;
                }
            }

            // Jouer un As si on l'a
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getChiffre() == Carte::AS) {
                    qDebug() << "Bot" << playerIndex << "- [TA] Joue un As";
                    return idx;
                }
            }

            // Sinon, jouer la carte la plus faible pour économiser les fortes
            return findLowestValueCardToutAtout(player, playableIndices);
        }

        // Cas 2: Le partenaire gagne le pli
        if (isPartnerWinning(playerIndex, idxPlayerWinning)) {
            qDebug() << "Bot" << playerIndex << "- [TA] Partenaire gagne";

            // En TA, on doit MONTER si on peut, même si le partenaire gagne
            // Chercher si on peut monter
            int lowestWinningIdx = -1;
            int lowestWinningOrder = 999;

            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                int order = carte->getOrdreCarteForte();
                if (order > carteGagnante->getOrdreCarteForte() && order < lowestWinningOrder) {
                    lowestWinningOrder = order;
                    lowestWinningIdx = idx;
                }
            }

            // Si on peut monter, jouer la plus petite carte qui monte
            if (lowestWinningIdx != -1) {
                qDebug() << "Bot" << playerIndex << "- [TA] Monte sur le partenaire (obligatoire)";
                return lowestWinningIdx;
            }

            // Si on ne peut pas monter, défausser la plus petite (évite les Valets)
            qDebug() << "Bot" << playerIndex << "- [TA] Ne peut pas monter, défausse (évite Valets)";
            return findLowestValueCardToutAtout(player, playableIndices);
        }

        // Cas 3: L'adversaire gagne le pli
        // On DOIT monter si on peut
        int lowestWinningIdx = -1;
        int lowestWinningOrder = 999;

        for (int idx : playableIndices) {
            Carte* carte = main[idx];
            int order = carte->getOrdreCarteForte();
            if (order > carteGagnante->getOrdreCarteForte() && order < lowestWinningOrder) {
                lowestWinningOrder = order;
                lowestWinningIdx = idx;
            }
        }

        // Si on peut monter, jouer la plus petite carte qui monte
        if (lowestWinningIdx != -1) {
            qDebug() << "Bot" << playerIndex << "- [TA] Monte sur l'adversaire";
            return lowestWinningIdx;
        }

        // Si on ne peut pas monter, défausser la plus petite (évite les Valets)
        qDebug() << "Bot" << playerIndex << "- [TA] Ne peut pas monter, défausse (évite Valets)";
        return findLowestValueCardToutAtout(player, playableIndices);
    }

    int chooseBestCard(GameRoom* room, Player* player, int playerIndex,
                       const std::vector<int>& playableIndices,
                       Carte* carteGagnante, int idxPlayerWinning) {
        const auto& main = player->getMain();

        // Détection des modes spéciaux: TA (7) et SA (8)
        if (static_cast<int>(room->couleurAtout) == 8) {
            // Mode SANS ATOUT
            qDebug() << "Bot" << playerIndex << "utilise la stratégie SANS ATOUT";
            return chooseBestCardSansAtout(room, player, playerIndex, playableIndices, carteGagnante, idxPlayerWinning);
        }

        if (static_cast<int>(room->couleurAtout) == 7) {
            // Mode TOUT ATOUT
            qDebug() << "Bot" << playerIndex << "utilise la stratégie TOUT ATOUT";
            return chooseBestCardToutAtout(room, player, playerIndex, playableIndices, carteGagnante, idxPlayerWinning);
        }

        // Stratégie classique pour les couleurs normales (Coeur, Trèfle, Carreau, Pique)
        // Cas 1: Le bot commence le pli (premier à jouer)
        if (room->currentPli.empty()) {
            qDebug() << "Bot" << playerIndex << "commence le pli - analyse de la main";

            // Analyser les atouts en main
            int valetAtoutIdx = -1;
            int neufAtoutIdx = -1;
            int otherAtoutCount = 0;
            bool hasAsHorsAtout = false;

            for (size_t i = 0; i < playableIndices.size(); i++) {
                int idx = playableIndices[i];
                Carte* carte = main[idx];
                if (carte->getCouleur() == room->couleurAtout) {
                    if (carte->getChiffre() == Carte::VALET) {
                        valetAtoutIdx = idx;
                    } else if (carte->getChiffre() == Carte::NEUF) {
                        neufAtoutIdx = idx;
                    } else {
                        otherAtoutCount++;
                    }
                } else if (carte->getChiffre() == Carte::AS) {
                    hasAsHorsAtout = true;
                }
            }

            // Compter les atouts restants chez les autres
            int remainingTrumps = countRemainingTrumps(room, player);

            // Trouver le plus petit atout en main (hors Valet et 9)
            int smallestAtoutIdx = -1;
            int smallestAtoutOrder = 999;
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getCouleur() == room->couleurAtout &&
                    carte->getChiffre() != Carte::VALET && carte->getChiffre() != Carte::NEUF) {
                    int order = carte->getOrdreCarteForte();
                    if (order < smallestAtoutOrder) {
                        smallestAtoutOrder = order;
                        smallestAtoutIdx = idx;
                    }
                }
            }

            int totalAtouts = (valetAtoutIdx >= 0 ? 1 : 0) + (neufAtoutIdx >= 0 ? 1 : 0) + otherAtoutCount;

            // Vérifier si le bot est dans l'équipe qui a pris (fait l'annonce)
            // L'équipe qui prend est celle du lastBidderIndex
            bool isAttackingTeam = (playerIndex % 2) == (room->lastBidderIndex % 2);

            // Compter les atouts déjà tombés
            int playedTrumps = countPlayedTrumps(room);

            // Vérifier si on doit continuer à chasser les atouts
            // Arrêter dès que 5+ atouts sont tombés
            bool shouldStopChasing = (playedTrumps >= 5);

            qDebug() << "Bot" << playerIndex << "- Valet:" << (valetAtoutIdx >= 0)
                     << "9:" << (neufAtoutIdx >= 0) << "autres atouts:" << otherAtoutCount
                     << "total atouts:" << totalAtouts << "equipe attaque:" << isAttackingTeam
                     << "As hors atout:" << hasAsHorsAtout << "atouts restants adversaires:" << remainingTrumps
                     << "atouts tombes:" << playedTrumps << "arreter chasse:" << shouldStopChasing;

            // === STRATÉGIE ÉQUIPE QUI ATTAQUE ===
            // Si 5+ atouts sont tombés, arrêter la chasse et jouer les cartes maîtres hors atout
            if (isAttackingTeam && shouldStopChasing) {
                qDebug() << "Bot" << playerIndex << "- [ATTAQUE] 5+ atouts tombés, arrêt chasse, recherche cartes maîtres";

                // Jouer les cartes maîtres hors atout en priorité
                for (int idx : playableIndices) {
                    Carte* carte = main[idx];
                    if (carte->getCouleur() != room->couleurAtout && isMasterCard(room, carte)) {
                        qDebug() << "Bot" << playerIndex << "- [ATTAQUE] Joue carte maître hors atout (chasse arrêtée)";
                        return idx;
                    }
                }
                // Si pas de carte maître, continuer avec la stratégie normale (en bas)
            }

            // Que ce soit le joueur qui a parlé ou son partenaire, on doit faire tomber les atouts
            // Mais on arrête si 5+ atouts sont tombés
            if (isAttackingTeam && remainingTrumps > 0 && totalAtouts >= 1 && !shouldStopChasing) {
                qDebug() << "Bot" << playerIndex << "- [ATTAQUE] Entre dans strategie chasse aux atouts";

                // Si j'ai le Valet, je le joue directement (carte maîtresse)
                if (valetAtoutIdx >= 0) {
                    qDebug() << "Bot" << playerIndex << "- [ATTAQUE] J'ai le Valet, je le joue";
                    return valetAtoutIdx;
                }

                // Si le Valet est tombé et j'ai le 9 (qui devient maître), je le joue
                if (neufAtoutIdx >= 0 && isTrumpJackPlayed(room)) {
                    qDebug() << "Bot" << playerIndex << "- [ATTAQUE] Valet tombé, je joue le 9 (maître)";
                    return neufAtoutIdx;
                }

                // Si j'ai le 9 + autres atouts (et le Valet pas encore tombé),
                // je joue un autre atout (pas le 9) pour garder le 9 maître pour plus tard
                if (neufAtoutIdx >= 0 && otherAtoutCount > 0 && smallestAtoutIdx >= 0 && !isTrumpJackPlayed(room)) {
                    qDebug() << "Bot" << playerIndex << "- [ATTAQUE] J'ai le 9 + autres, je joue un petit atout";
                    return smallestAtoutIdx;
                }

                // Si j'ai seulement le 9 (sans autre atout) et le Valet pas tombé,
                // je le joue quand même pour aider (mon partenaire a probablement le Valet)
                if (neufAtoutIdx >= 0 && otherAtoutCount == 0 && !isTrumpJackPlayed(room)) {
                    qDebug() << "Bot" << playerIndex << "- [ATTAQUE] J'ai seulement le 9, je le joue";
                    return neufAtoutIdx;
                }

                // Si j'ai d'autres atouts (sans Valet ni 9), je joue le plus petit
                // pour faire tomber les atouts adverses
                if (smallestAtoutIdx >= 0) {
                    qDebug() << "Bot" << playerIndex << "- [ATTAQUE] Je joue un atout pour faire tomber";
                    return smallestAtoutIdx;
                }
            }

            // Si le Valet est tombé mais on a le 9 et on attaque, jouer le 9 SEULEMENT si chasse pas arrêtée
            if (isAttackingTeam && valetAtoutIdx < 0 && neufAtoutIdx >= 0 &&
                isTrumpJackPlayed(room) && remainingTrumps > 0 && !shouldStopChasing) {
                qDebug() << "Bot" << playerIndex << "- [ATTAQUE] Continue avec le 9 (Valet tombé)";
                return neufAtoutIdx;
            }

            // === STRATÉGIE COMMUNE : Jouer les cartes maîtres hors atout ===
            // Si les atouts adverses sont tombés ou qu'on défend (économiser atouts), jouer les As
            if (remainingTrumps == 0 || !isAttackingTeam || (valetAtoutIdx < 0 && neufAtoutIdx < 0)) {
                for (int idx : playableIndices) {
                    Carte* carte = main[idx];
                    if (carte->getCouleur() != room->couleurAtout && carte->getChiffre() == Carte::AS) {
                        qDebug() << "Bot" << playerIndex << "- Joue un As hors atout";
                        return idx;
                    }
                }
            }

            // Jouer un 10 si l'As de cette couleur est tombé (le 10 est maître)
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getCouleur() != room->couleurAtout && carte->getChiffre() == Carte::DIX) {
                    if (isAcePlayed(room, carte->getCouleur())) {
                        qDebug() << "Bot" << playerIndex << "- Joue un 10 maître (As tombé)";
                        return idx;
                    }
                }
            }

            // === STRATÉGIE ÉQUIPE QUI DÉFEND ===
            if (!isAttackingTeam) {
                // Jouer une carte maître hors atout si on en a (le pli est vide donc pas d'atout dedans)
                // Chercher d'abord les cartes avec le plus de valeur (As, 10, Roi...)
                for (int idx : playableIndices) {
                    Carte* carte = main[idx];
                    if (carte->getCouleur() != room->couleurAtout && isMasterCard(room, carte)) {
                        qDebug() << "Bot" << playerIndex << "- [DEFENSE] Joue une carte maître hors atout en partant";
                        return idx;
                    }
                }

                // Sinon, jouer la carte la plus faible en évitant l'atout
                qDebug() << "Bot" << playerIndex << "- [DEFENSE] Évite de jouer atout en partant";
                return findLowestValueCardAvoidTrump(player, playableIndices, room->couleurAtout);
            }

            // Sinon, jouer la carte la plus faible pour économiser les fortes
            return findLowestValueCard(player, playableIndices);
        }

        // Cas 2: Le partenaire est en train de gagner le pli
        if (isPartnerWinning(playerIndex, idxPlayerWinning)) {
            qDebug() << "Bot" << playerIndex << "- partenaire gagne";

            // Vérifier si le partenaire a joué à l'atout (première carte du pli)
            bool partnerPlayedTrump = false;
            if (!room->currentPli.empty()) {
                Carte* firstCard = room->currentPli[0].second;
                partnerPlayedTrump = (firstCard->getCouleur() == room->couleurAtout);
            }

            // Compter les atouts tombés pour savoir si on doit encore jouer atout
            int playedTrumps = countPlayedTrumps(room);

            // Si le partenaire joue à l'atout et qu'on est dans l'équipe qui attaque,
            // jouer le Valet si on l'a pour être sûr de gagner le pli
            // MAIS seulement si moins de 5 atouts sont tombés (sinon inutile)
            bool isAttackingTeam = (playerIndex % 2) == (room->lastBidderIndex % 2);

            if (partnerPlayedTrump && isAttackingTeam && playedTrumps < 5) {
                // Chercher le Valet d'atout dans ma main
                for (int idx : playableIndices) {
                    Carte* carte = main[idx];
                    if (carte->getCouleur() == room->couleurAtout && carte->getChiffre() == Carte::VALET) {
                        qDebug() << "Bot" << playerIndex << "- Partenaire joue atout, je joue le Valet pour gagner";
                        return idx;
                    }
                }

                // Vérifier si le Valet d'atout est dans le pli actuel (joué par n'importe qui)
                bool jackInCurrentPli = false;
                for (const auto& pair : room->currentPli) {
                    Carte* carte = pair.second;
                    if (carte->getCouleur() == room->couleurAtout && carte->getChiffre() == Carte::VALET) {
                        jackInCurrentPli = true;
                        break;
                    }
                }

                // Analyser mes atouts en main
                int neufAtoutIdx = -1;
                int otherAtoutIdx = -1;  // Plus petit atout hors 9
                int otherAtoutOrder = 999;

                for (int idx : playableIndices) {
                    Carte* carte = main[idx];
                    if (carte->getCouleur() == room->couleurAtout) {
                        if (carte->getChiffre() == Carte::NEUF) {
                            neufAtoutIdx = idx;
                        } else {
                            int order = carte->getOrdreCarteForte();
                            if (order < otherAtoutOrder) {
                                otherAtoutOrder = order;
                                otherAtoutIdx = idx;
                            }
                        }
                    }
                }

                // Si le Valet est dans le pli actuel et que j'ai le 9 + d'autres atouts,
                // je joue un autre atout pour garder le 9 (qui sera maître après ce pli)
                if (jackInCurrentPli && neufAtoutIdx >= 0 && otherAtoutIdx >= 0) {
                    qDebug() << "Bot" << playerIndex << "- Valet dans le pli, je garde le 9 et joue un autre atout";
                    return otherAtoutIdx;
                }

                // Si on a le 9 et le Valet est tombé (dans un pli précédent), jouer le 9 pour gagner
                if (isTrumpJackPlayed(room) && !jackInCurrentPli) {
                    for (int idx : playableIndices) {
                        Carte* carte = main[idx];
                        if (carte->getCouleur() == room->couleurAtout && carte->getChiffre() == Carte::NEUF) {
                            qDebug() << "Bot" << playerIndex << "- Partenaire joue atout, je joue le 9 (Valet tombé avant)";
                            return idx;
                        }
                    }
                }
            }

            // Si c'est le dernier joueur et le pli a beaucoup de points, charger
            if (room->currentPli.size() == 3) {
                int pliPoints = calculatePliPoints(room->currentPli);
                if (pliPoints >= 15) {
                    // Charger avec une carte à points (10 ou As)
                    for (int idx : playableIndices) {
                        Carte* carte = main[idx];
                        if (carte->getCouleur() != room->couleurAtout &&
                            (carte->getChiffre() == Carte::DIX || carte->getChiffre() == Carte::AS)) {
                            qDebug() << "Bot" << playerIndex << "charge le pli avec points";
                            return idx;
                        }
                    }
                }
            }

            // Pour l'équipe qui défend: ne JAMAIS jouer atout si possible
            if (!isAttackingTeam) {
                qDebug() << "Bot" << playerIndex << "- [DEFENSE] Partenaire gagne, évite atout";
                return findLowestValueCardAvoidTrump(player, playableIndices, room->couleurAtout);
            }

            // Pour l'équipe qui attaque avec 5+ atouts tombés: éviter de jouer atout inutilement
            if (isAttackingTeam && playedTrumps >= 5) {
                qDebug() << "Bot" << playerIndex << "- [ATTAQUE] Partenaire gagne, 5+ atouts tombés, évite atout";
                return findLowestValueCardAvoidTrump(player, playableIndices, room->couleurAtout);
            }

            // Jouer la carte la plus faible
            return findLowestValueCard(player, playableIndices);
        }

        // Cas 3: Un adversaire gagne le pli - essayer de prendre
        qDebug() << "Bot" << playerIndex << "- adversaire gagne, essaie de prendre";

        // Vérifier si un atout a été joué dans le pli
        bool trumpInPli = false;
        for (const auto& pair : room->currentPli) {
            if (pair.second->getCouleur() == room->couleurAtout) {
                trumpInPli = true;
                break;
            }
        }

        // Déterminer si on est en défense
        bool isAttackingTeam = (playerIndex % 2) == (room->lastBidderIndex % 2);

        // Pour l'équipe qui défend: jouer une carte maître hors atout si:
        // - Aucun atout dans le pli
        // - La carte maître est de la couleur demandée
        // L'équipe qui attaque garde ses cartes maîtres pour faire le plus de plis possible
        if (!isAttackingTeam && !trumpInPli) {
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getCouleur() != room->couleurAtout &&
                    carte->getCouleur() == room->couleurDemandee &&
                    isMasterCard(room, carte)) {
                    qDebug() << "Bot" << playerIndex << "- [DEFENSE] Joue une carte maître à la couleur demandée (pas d'atout dans le pli)";
                    return idx;
                }
            }
        }

        int pliPoints = calculatePliPoints(room->currentPli);

        // Si le pli contient des points significatifs (> 10), essayer de prendre
        if (pliPoints >= 10 || room->currentPli.size() == 3) {
            int winningCardIdx = findLowestWinningCard(player, playableIndices,
                                                        carteGagnante, room->couleurAtout);
            if (winningCardIdx >= 0) {
                qDebug() << "Bot" << playerIndex << "prend le pli avec carte gagnante";
                return winningCardIdx;
            }
        }

        // Si on ne peut pas gagner ou le pli ne vaut pas le coup, jouer petit
        return findLowestValueCard(player, playableIndices);
    }

    void playBotCard(int roomId, int playerIndex) {
        qDebug() << "===== playBotCard appele pour joueur" << playerIndex << "isBot:" << (m_gameRooms.value(roomId) ? m_gameRooms.value(roomId)->isBot[playerIndex] : false);

        GameRoom* room = m_gameRooms.value(roomId);
        if (!room || room->currentPlayerIndex != playerIndex || room->gameState != "playing") {
            qDebug() << "playBotCard - Verification echouee: room=" << (room != nullptr)
                     << "currentPlayer=" << (room ? room->currentPlayerIndex : -1)
                     << "expected=" << playerIndex;
            return;
        }

        // IMPORTANT: Vérifier que le joueur est toujours un bot
        // Il peut avoir été réhumanisé entre-temps (reconnexion + clic OK)
        if (!room->isBot[playerIndex]) {
            qDebug() << "playBotCard - ANNULÉ: Joueur" << playerIndex << "n'est plus un bot (réhumanisé)";
            return;
        }

        // Arrêter le timer de timeout et invalider les anciens callbacks
        if (room->turnTimeout) {
            room->turnTimeout->stop();
            room->turnTimeoutGeneration++;  // Invalider les anciens callbacks en queue
            qDebug() << "playBotCard - Timer de timeout arrêté, génération:" << room->turnTimeoutGeneration;
        }

        Player* player = room->players[playerIndex].get();
        if (!player || player->getMain().empty()) return;

        // Calculer les cartes jouables
        Carte* carteGagnante = nullptr;
        int idxPlayerWinning = -1;
        if (!room->currentPli.empty()) {
            carteGagnante = room->currentPli[0].second;
            idxPlayerWinning = room->currentPli[0].first;

            for (size_t i = 1; i < room->currentPli.size(); i++) {
                Carte* c = room->currentPli[i].second;
                if (*carteGagnante < *c) {
                    carteGagnante = c;
                    idxPlayerWinning = room->currentPli[i].first;
                }
            }
        }

        // Trouver toutes les cartes jouables
        std::vector<int> playableIndices;
        const auto& main = player->getMain();
        for (size_t i = 0; i < main.size(); i++) {
            bool isPlayable = player->isCartePlayable(
                static_cast<int>(i),
                room->couleurDemandee,
                room->couleurAtout,
                carteGagnante,
                idxPlayerWinning
            );

            if (isPlayable) {
                playableIndices.push_back(static_cast<int>(i));
            }
        }

        if (playableIndices.empty()) {
            qDebug() << "GameServer - Bot joueur" << playerIndex << "n'a aucune carte jouable!";
            return;
        }

        // Stratégie de jeu intelligente
        int cardIndex = chooseBestCard(room, player, playerIndex, playableIndices,
                                        carteGagnante, idxPlayerWinning);

        qDebug() << "GameServer - Bot joueur" << playerIndex << "joue la carte a l'index" << cardIndex;

        Carte* cartePlayed = player->getMain()[cardIndex];

        // Si c'est la première carte du pli, définir la couleur demandée
        if (room->currentPli.empty()) {
            room->couleurDemandee = cartePlayed->getCouleur();
        }

        // Ajouter au pli courant
        room->currentPli.push_back(std::make_pair(playerIndex, cartePlayed));

        // Marquer la carte comme jouée pour le tracking IA
        room->markCardAsPlayed(cartePlayed);

        // Retirer la carte de la main
        player->removeCard(cardIndex);

        qDebug() << "GameServer - Bot a joué la carte, main contient maintenant" << player->getMain().size() << "cartes";

        // Vérifier si c'est une carte de la belote (Roi ou Dame de l'atout)
        bool isRoi = (cartePlayed->getChiffre() == Carte::ROI);
        bool isDame = (cartePlayed->getChiffre() == Carte::DAME);
        bool isAtout = (cartePlayed->getCouleur() == room->couleurAtout);

        if (isAtout && (isRoi || isDame)) {
            int teamIndex = playerIndex % 2;
            bool hasBelote = (teamIndex == 0) ? room->beloteTeam1 : room->beloteTeam2;

            if (hasBelote) {
                if (!room->beloteRoiJoue && !room->beloteDameJouee) {
                    // Première carte de la belote
                    if (isRoi) {
                        room->beloteRoiJoue = true;
                    } else {
                        room->beloteDameJouee = true;
                    }

                    QJsonObject beloteMsg;
                    beloteMsg["type"] = "belote";
                    beloteMsg["playerIndex"] = playerIndex;
                    broadcastToRoom(roomId, beloteMsg);

                } else if ((isRoi && room->beloteDameJouee) || (isDame && room->beloteRoiJoue)) {
                    // Deuxième carte de la belote
                    if (isRoi) {
                        room->beloteRoiJoue = true;
                    } else {
                        room->beloteDameJouee = true;
                    }

                    QJsonObject rebeloteMsg;
                    rebeloteMsg["type"] = "rebelote";
                    rebeloteMsg["playerIndex"] = playerIndex;
                    broadcastToRoom(roomId, rebeloteMsg);
                }
            }
        }

        // Broadcast l'action
        QJsonObject msg;
        msg["type"] = "cardPlayed";
        msg["playerIndex"] = playerIndex;
        msg["cardIndex"] = cardIndex;
        msg["cardValue"] = static_cast<int>(cartePlayed->getChiffre());
        msg["cardSuit"] = static_cast<int>(cartePlayed->getCouleur());
        broadcastToRoom(roomId, msg);

        // Si le pli est complet (4 cartes)
        if (room->currentPli.size() == 4) {
            finishPli(roomId);
        } else {
            // Passer au joueur suivant
            room->currentPlayerIndex = (room->currentPlayerIndex + 1) % 4;
            qDebug() << "playBotCard - Prochain joueur:" << room->currentPlayerIndex
                     << "isBot:" << room->isBot[room->currentPlayerIndex];

            // notifyPlayersWithPlayableCards gère déjà le scheduling des bots
            notifyPlayersWithPlayableCards(roomId);
        }
    }

    void startSurcoincheTimer(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        qDebug() << "GameServer - Attente de 2 secondes avant d'afficher le bouton Surcoinche (animation Coinche)";

        // Attendre 2 secondes pour permettre l'animation "Coinche !" de s'afficher
        QTimer::singleShot(2000, this, [this, roomId]() {
            GameRoom* room = m_gameRooms.value(roomId);
            if (!room) return;

            // Initialiser le temps restant à 10 secondes
            room->surcoincheTimeLeft = 10;

            // Créer le timer s'il n'existe pas
            if (!room->surcoincheTimer) {
                room->surcoincheTimer = new QTimer(this);
                connect(room->surcoincheTimer, &QTimer::timeout, this, [this, roomId]() {
                    onSurcoincheTimerTick(roomId);
                });
            }

            // Démarrer le timer (tick chaque seconde)
            room->surcoincheTimer->start(1000);

            qDebug() << "GameServer - Timer de surcoinche démarré pour la room" << roomId;

            // Envoyer l'offre de surcoinche aux joueurs de l'équipe qui a fait l'annonce
            QJsonObject offerMsg;
            offerMsg["type"] = "surcoincheOffer";
            offerMsg["timeLeft"] = room->surcoincheTimeLeft;
            broadcastToRoom(roomId, offerMsg);
        });
    }

    void stopSurcoincheTimer(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        if (room->surcoincheTimer) {
            room->surcoincheTimer->stop();
            qDebug() << "GameServer - Timer de surcoinche arrêté pour la room" << roomId;
        }

        room->surcoincheTimeLeft = 0;
    }

    void onSurcoincheTimerTick(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        room->surcoincheTimeLeft--;

        qDebug() << "GameServer - Surcoinche timer tick, temps restant:" << room->surcoincheTimeLeft;

        if (room->surcoincheTimeLeft <= 0) {
            // Timeout atteint, arrêter le timer et lancer la phase de jeu
            stopSurcoincheTimer(roomId);

            qDebug() << "GameServer - Timeout surcoinche! Fin des enchères, lancement phase de jeu";

            // Notifier les joueurs du timeout
            QJsonObject timeoutMsg;
            timeoutMsg["type"] = "surcoincheTimeout";
            broadcastToRoom(roomId, timeoutMsg);

            // Lancer la phase de jeu
            for (int i = 0; i < 4; i++) {
                // IMPORTANT: Réinitialiser toutes les cartes avant de définir le nouvel atout
                const auto& main = room->players[i]->getMain();
                for (Carte* carte : main) {
                    carte->setAtout(false);
                }

                if (room->isToutAtout) {
                    room->players[i]->setAllCardsAsAtout();
                } else if (room->isSansAtout) {
                    room->players[i]->setNoAtout();
                } else {
                    room->players[i]->setAtout(room->lastBidCouleur);
                }
            }
            startPlayingPhase(roomId);
        } else {
            // Envoyer la mise à jour du temps restant
            QJsonObject timeUpdateMsg;
            timeUpdateMsg["type"] = "surcoincheTimeUpdate";
            timeUpdateMsg["timeLeft"] = room->surcoincheTimeLeft;
            broadcastToRoom(roomId, timeUpdateMsg);
        }
    }

    void startPlayingPhase(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        // Arrêter le timer de timeout des enchères si actif
        if (room->bidTimeout) {
            room->bidTimeout->stop();
            room->bidTimeoutGeneration++;
            qDebug() << "startPlayingPhase - Timer de timeout enchères arrêté, génération:" << room->bidTimeoutGeneration;
        }

        room->gameState = "playing";
        room->waitingForNextPli = false;  // S'assurer que le flag est désactivé au début de la phase de jeu

        // Définir couleurAtout selon le mode de jeu actuel
        // IMPORTANT: S'assurer que couleurAtout reflète bien le mode de jeu final
        if (room->isToutAtout || room->isSansAtout) {
            // En mode TA/SA, utiliser COULEURINVALIDE (on se base sur les flags)
            room->couleurAtout = Carte::COULEURINVALIDE;
        } else {
            // Mode normal: utiliser la couleur d'atout de la dernière enchère
            room->couleurAtout = room->lastBidCouleur;
        }

        // Le joueur qui a commencé les enchères joue en premier
        room->currentPlayerIndex = room->firstPlayerIndex;

        // Tri des cartes apres avoir defini l'atout
        for (int i = 0; i < 4; i++) {
            if (room->isToutAtout) {
                room->players[i]->sortHandToutAtout();
            } else if (room->isSansAtout) {
                room->players[i]->sortHandSansAtout();
            } else {
                room->players[i]->sortHand();
            }
        }

        qDebug() << "Phase de jeu demarree - Atout:" << static_cast<int>(room->couleurAtout)
                 << "isToutAtout:" << room->isToutAtout
                 << "isSansAtout:" << room->isSansAtout
                 << "Premier joueur:" << room->currentPlayerIndex
                 << "(Gagnant encheres:" << room->lastBidderIndex << ")";

        // Verifier la Belote (Dame + Roi de l'atout) pour chaque equipe
        room->beloteTeam1 = false;
        room->beloteTeam2 = false;

        // Verifier joueurs de l'equipe 1 (joueurs 0 et 2)
        if (room->players[0]->hasBelotte(room->couleurAtout)) {
            room->beloteTeam1 = true;
            qDebug() << "GameServer - Belote detectee pour le joueur 0 (Equipe 1)";
        }
        if (room->players[2]->hasBelotte(room->couleurAtout)) {
            room->beloteTeam1 = true;
            qDebug() << "GameServer - Belote detectee pour le joueur 2 (Equipe 1)";
        }

        // Verifier joueurs de l'equipe 2 (joueurs 1 et 3)
        if (room->players[1]->hasBelotte(room->couleurAtout)) {
            room->beloteTeam2 = true;
            qDebug() << "GameServer - Belote detectee pour le joueur 1 (Equipe 2)";
        }
        if (room->players[3]->hasBelotte(room->couleurAtout)) {
            room->beloteTeam2 = true;
            qDebug() << "GameServer - Belote detectee pour le joueur 3 (Equipe 2)";
        }

        // Notifie tous les joueurs du changement de phase avec cartes jouables
        notifyPlayersWithPlayableCards(roomId);
    }

    void notifyPlayersWithPlayableCards(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        int currentPlayer = room->currentPlayerIndex;

        // Calcule les cartes jouables pour le joueur actuel
        QJsonArray playableCards = calculatePlayableCards(room, currentPlayer);

        // Envoi à tous les joueurs
        QJsonObject stateMsg;
        stateMsg["type"] = "gameState";
        stateMsg["biddingPhase"] = false;
        stateMsg["currentPlayer"] = currentPlayer;
        stateMsg["atout"] = static_cast<int>(room->couleurAtout);
        stateMsg["isToutAtout"] = room->isToutAtout;
        stateMsg["isSansAtout"] = room->isSansAtout;
        stateMsg["playableCards"] = playableCards;  // Liste des indices des cartes jouables

        // Si on est au début de la phase de jeu, inclure les infos d'enchères
        if (room->currentPli.empty() && currentPlayer == room->firstPlayerIndex) {
            stateMsg["biddingWinnerId"] = room->lastBidderIndex;
            stateMsg["biddingWinnerAnnonce"] = static_cast<int>(room->lastBidAnnonce);
        }

        broadcastToRoom(roomId, stateMsg);

        // Si le joueur actuel est déjà marqué comme bot, le faire jouer automatiquement
        if (room->isBot[currentPlayer]) {
            qDebug() << "notifyPlayersWithPlayableCards - Joueur" << currentPlayer << "est un bot, planification playBotCard";

            // Si c'est le début d'un nouveau pli (pli vide), attendre plus longtemps
            // pour laisser le temps au pli précédent d'être nettoyé côté client
            int delay = room->currentPli.empty() ? 2000 : 800;

            QTimer::singleShot(delay, this, [this, roomId, currentPlayer]() {
                GameRoom* room = m_gameRooms.value(roomId);
                if (!room || room->gameState != "playing") return;

                // IMPORTANT: Revérifier que le joueur est toujours un bot
                // Il peut avoir été réhumanisé entre-temps (reconnexion + clic OK)
                if (!room->isBot[currentPlayer]) {
                    qDebug() << "playBotCard ANNULÉ - Joueur" << currentPlayer << "n'est plus un bot (réhumanisé)";
                    return;
                }

                // Vérifier que c'est toujours son tour
                if (room->currentPlayerIndex != currentPlayer) {
                    qDebug() << "playBotCard ANNULÉ - Ce n'est plus le tour du joueur" << currentPlayer;
                    return;
                }

                playBotCard(roomId, currentPlayer);
            });
            return;  // Ne pas exécuter le code du dernier pli automatique ci-dessous
        }

        // Vérifier si le joueur a des cartes en main avant de démarrer le timer
        Player* player = room->players[currentPlayer].get();
        if (!player || player->getMain().empty()) {
            qDebug() << "notifyPlayersWithPlayableCards - Joueur" << currentPlayer << "n'a plus de cartes, pas de timer";
            return;
        }

        // Pour les joueurs humains, démarrer un timer de 15 secondes
        // Si le joueur ne joue pas dans les temps, le marquer comme bot et jouer automatiquement
        if (!room->turnTimeout) {
            room->turnTimeout = new QTimer(this);
            room->turnTimeout->setSingleShot(true);
        }

        // TOUJOURS arrêter et déconnecter l'ancien timer avant d'en créer un nouveau
        // Cela évite que d'anciens signaux en queue ne se déclenchent pour le mauvais joueur
        room->turnTimeout->stop();
        disconnect(room->turnTimeout, nullptr, this, nullptr);

        // Incrémenter la génération pour invalider les anciens callbacks en queue
        room->turnTimeoutGeneration++;
        int currentGeneration = room->turnTimeoutGeneration;

        // Démarrer le nouveau timer
        connect(room->turnTimeout, &QTimer::timeout, this, [this, roomId, currentPlayer, currentGeneration]() {
            GameRoom* room = m_gameRooms.value(roomId);
            if (!room || room->gameState != "playing") return;

            // Vérifier que ce timeout est toujours valide (pas un ancien signal en queue)
            if (room->turnTimeoutGeneration != currentGeneration) {
                qDebug() << "TIMEOUT - Ignoré (ancienne génération:" << currentGeneration
                         << "actuelle:" << room->turnTimeoutGeneration << ")";
                return;
            }

            // Vérifier que c'est toujours le même joueur (qu'aucune carte n'a été jouée entre-temps)
            if (room->currentPlayerIndex == currentPlayer && !room->isBot[currentPlayer]) {
                qDebug() << "TIMEOUT - Joueur" << currentPlayer << "n'a pas joué à temps, marquage comme bot";

                // Marquer le joueur comme bot
                room->isBot[currentPlayer] = true;

                // Notifier le client qu'il a été remplacé par un bot
                QString connectionId = room->connectionIds[currentPlayer];
                if (!connectionId.isEmpty() && m_connections.contains(connectionId)) {
                    PlayerConnection* conn = m_connections[connectionId];
                    if (conn && conn->socket && conn->socket->state() == QAbstractSocket::ConnectedState) {
                        QJsonObject notification;
                        notification["type"] = "botReplacement";
                        notification["message"] = "Vous avez été remplacé par un bot car vous n'avez pas joué à temps.";
                        conn->socket->sendTextMessage(QJsonDocument(notification).toJson(QJsonDocument::Compact));
                        qDebug() << "TIMEOUT - Notification botReplacement envoyée au joueur" << currentPlayer;
                    }
                }

                // Faire jouer le bot immédiatement
                playBotCard(roomId, currentPlayer);
            }
        });

        room->turnTimeout->start(15000);  // 15 secondes (cohérent avec le timer client)
        qDebug() << "notifyPlayersWithPlayableCards - Timer de 15s démarré pour joueur" << currentPlayer << "(génération:" << currentGeneration << ")";

        // Si c'est le dernier pli (tous les joueurs n'ont qu'une carte), jouer automatiquement après un délai
        // IMPORTANT: Ne jouer automatiquement que si on est bien dans la phase de jeu
        qDebug() << "notifyPlayersWithPlayableCards - Verification dernier pli pour joueur" << currentPlayer
                 << "taille main:" << player->getMain().size()
                 << "isBot:" << room->isBot[currentPlayer];
        if (player->getMain().size() == 1 && room->gameState == "playing") {
            qDebug() << "GameServer - Dernier pli detecte, jeu automatique pour joueur" << currentPlayer;

            // Si c'est le début du dernier pli (pli vide), attendre 2000ms pour laisser le temps au pli précédent d'être nettoyé
            // Sinon, attendre seulement 400ms
            int delay = room->currentPli.empty() ? 2000 : 400;

            // Jouer automatiquement après le délai approprié
            QTimer::singleShot(delay, this, [this, roomId, currentPlayer]() {
                GameRoom* room = m_gameRooms.value(roomId);
                if (!room || room->currentPlayerIndex != currentPlayer || room->gameState != "playing") return;

                Player* player = room->players[currentPlayer].get();
                if (!player || player->getMain().empty()) return;

                // Simuler le jeu de la carte (index 0, la seule carte restante)
                qDebug() << "GameServer - Jeu automatique de la dernière carte pour joueur" << currentPlayer;

                Carte* cartePlayed = player->getMain()[0];

                // Si c'est la première carte du pli, définir la couleur demandée
                if (room->currentPli.empty()) {
                    room->couleurDemandee = cartePlayed->getCouleur();
                }

                // Ajouter au pli courant
                room->currentPli.push_back(std::make_pair(currentPlayer, cartePlayed));

                // Marquer la carte comme jouée pour le tracking IA
                room->markCardAsPlayed(cartePlayed);

                // Retirer la carte de la main
                player->removeCard(0);

                // Broadcast l'action
                QJsonObject msg;
                msg["type"] = "cardPlayed";
                msg["playerIndex"] = currentPlayer;
                msg["cardIndex"] = 0;
                msg["cardValue"] = static_cast<int>(cartePlayed->getChiffre());
                msg["cardSuit"] = static_cast<int>(cartePlayed->getCouleur());
                broadcastToRoom(roomId, msg);

                // Si le pli est complet (4 cartes)
                if (room->currentPli.size() == 4) {
                    finishPli(roomId);
                } else {
                    // Passe au joueur suivant
                    room->currentPlayerIndex = (room->currentPlayerIndex + 1) % 4;
                    notifyPlayersWithPlayableCards(roomId);
                }
            });
        }
    }

    QJsonArray calculatePlayableCards(GameRoom* room, int playerIndex) {
        QJsonArray playableIndices;

        if (room->gameState != "playing") {
            // Pendant les enchères, aucune carte n'est jouable
            return playableIndices;
        }

        Player* player = room->players[playerIndex].get();
        if (!player) return playableIndices;

        // Détermine la carte gagnante actuelle du pli
        Carte* carteGagnante = nullptr;
        int idxPlayerWinning = -1;
        if (!room->currentPli.empty()) {
            carteGagnante = room->currentPli[0].second;
            idxPlayerWinning = room->currentPli[0].first;

            for (size_t i = 1; i < room->currentPli.size(); i++) {
                Carte* c = room->currentPli[i].second;
                if (*carteGagnante < *c) {
                    carteGagnante = c;
                    idxPlayerWinning = room->currentPli[i].first;
                }
            }
        }

        // Vérifie chaque carte
        const auto& main = player->getMain();

        for (size_t i = 0; i < main.size(); i++) {
            bool isPlayable = player->isCartePlayable(
                static_cast<int>(i),
                room->couleurDemandee,
                room->couleurAtout,
                carteGagnante,
                idxPlayerWinning,
                room->isToutAtout  // Passer explicitement le flag isToutAtout
            );

            if (isPlayable) {
                playableIndices.append(static_cast<int>(i));
            }
        }

        qDebug() << "Joueur" << playerIndex << ":" << playableIndices.size()
                 << "cartes jouables sur" << main.size();

        return playableIndices;
    }

    void handlePlayerDisconnect(const QString &connectionId) {
        PlayerConnection *conn = m_connections.value(connectionId);
        if (!conn) return;

        int roomId = conn->gameRoomId;
        if (roomId == -1) return;

        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        int playerIndex = conn->playerIndex;
        qDebug() << "GameServer - Joueur" << playerIndex << "(" << conn->playerName << ") déconnecté";

        // Incrémenter le compteur de parties jouées (défaite) pour ce joueur
        if (!conn->playerName.isEmpty()) {
            m_dbManager->updateGameStats(conn->playerName, false);  // false = défaite
            qDebug() << "Stats mises a jour pour" << conn->playerName << "- Defaite enregistree (deconnexion)";
        }

        // Remplacer le joueur par un bot
        /*room->isBot[playerIndex] = true;
        qDebug() << "Joueur" << playerIndex << "remplace par un bot (deconnexion)";

        // Notifier tous les joueurs qu'un joueur s'est déconnecté et a été remplacé par un bot
        QJsonObject dcMsg;
        dcMsg["type"] = "playerDisconnected";
        dcMsg["playerIndex"] = playerIndex;
        dcMsg["playerName"] = conn->playerName;
        broadcastToRoom(roomId, dcMsg, connectionId);

        // Si c'est le tour du joueur déconnecté, faire jouer le bot immédiatement
        if (room->currentPlayerIndex == playerIndex) {
            if (room->gameState == "bidding") {
                // Phase d'enchères : passer automatiquement
                QTimer::singleShot(500, this, [this, roomId, playerIndex]() {
                    playBotBid(roomId, playerIndex);
                });
            } else if (room->gameState == "playing") {
                // Phase de jeu : jouer une carte aléatoire
                QTimer::singleShot(500, this, [this, roomId, playerIndex]() {
                    playBotCard(roomId, playerIndex);
                });
            }
        }*/
    }

    QString getConnectionIdBySocket(QWebSocket *socket) {
        for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
            if (it.value()->socket == socket) {
                return it.key();
            }
        }
        return QString();
    }

    // ========================================
    // Gestion des lobbies privés
    // ========================================

    QString generateLobbyCode() {
        const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        QString code;
        for (int i = 0; i < 4; i++) {
            code += chars[QRandomGenerator::global()->bounded(chars.length())];
        }
        // Vérifier que le code n'existe pas déjà
        if (m_privateLobbies.contains(code)) {
            return generateLobbyCode();  // Régénérer si collision
        }
        return code;
    }

    void handleCreatePrivateLobby(QWebSocket *socket) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        PlayerConnection* conn = m_connections[connectionId];
        if (!conn) return;

        // Générer un code unique
        QString code = generateLobbyCode();

        // Créer le lobby
        PrivateLobby* lobby = new PrivateLobby();
        lobby->code = code;
        lobby->hostPlayerName = conn->playerName;
        lobby->playerNames.append(conn->playerName);
        lobby->playerAvatars.append(conn->avatar);
        lobby->readyStatus.append(false);

        m_privateLobbies[code] = lobby;

        qDebug() << "Lobby privé créé - Code:" << code << "Hôte:" << conn->playerName;

        // Envoyer le code au client
        QJsonObject response;
        response["type"] = "lobbyCreated";
        response["code"] = code;
        sendMessage(socket, response);

        // Envoyer l'état initial du lobby
        sendLobbyUpdate(code);
    }

    void handleJoinPrivateLobby(QWebSocket *socket, const QJsonObject &obj) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        PlayerConnection* conn = m_connections[connectionId];
        if (!conn) return;

        QString code = obj["code"].toString().toUpper();

        // Vérifier que le lobby existe
        if (!m_privateLobbies.contains(code)) {
            QJsonObject error;
            error["type"] = "lobbyError";
            error["message"] = "Lobby introuvable";
            sendMessage(socket, error);
            return;
        }

        PrivateLobby* lobby = m_privateLobbies[code];

        // Vérifier que le lobby n'est pas plein
        if (lobby->playerNames.size() >= 4) {
            QJsonObject error;
            error["type"] = "lobbyError";
            error["message"] = "Lobby complet";
            sendMessage(socket, error);
            return;
        }

        // Vérifier que le joueur n'est pas déjà dans le lobby
        if (lobby->playerNames.contains(conn->playerName)) {
            QJsonObject error;
            error["type"] = "lobbyError";
            error["message"] = "Vous êtes déjà dans ce lobby";
            sendMessage(socket, error);
            return;
        }

        // Ajouter le joueur au lobby
        lobby->playerNames.append(conn->playerName);
        lobby->playerAvatars.append(conn->avatar);
        lobby->readyStatus.append(false);

        qDebug() << "Joueur" << conn->playerName << "a rejoint le lobby" << code;

        // Confirmer au joueur
        QJsonObject response;
        response["type"] = "lobbyJoined";
        response["code"] = code;
        sendMessage(socket, response);

        // Envoyer l'état du lobby à tous
        sendLobbyUpdate(code);
    }

    void handleLobbyReady(QWebSocket *socket, const QJsonObject &obj) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        PlayerConnection* conn = m_connections[connectionId];
        if (!conn) return;

        bool ready = obj["ready"].toBool();

        // Trouver le lobby du joueur
        QString lobbyCode;
        for (auto it = m_privateLobbies.begin(); it != m_privateLobbies.end(); ++it) {
            if (it.value()->playerNames.contains(conn->playerName)) {
                lobbyCode = it.key();
                break;
            }
        }

        if (lobbyCode.isEmpty()) return;

        PrivateLobby* lobby = m_privateLobbies[lobbyCode];
        int playerIndex = lobby->playerNames.indexOf(conn->playerName);
        if (playerIndex >= 0 && playerIndex < lobby->readyStatus.size()) {
            lobby->readyStatus[playerIndex] = ready;
            qDebug() << "Joueur" << conn->playerName << "prêt:" << ready << "dans lobby" << lobbyCode;
            sendLobbyUpdate(lobby->code);
        }
    }

    void handleStartLobbyGame(QWebSocket *socket) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        PlayerConnection* conn = m_connections[connectionId];
        if (!conn) return;

        // Trouver le lobby du joueur
        QString lobbyCode;
        for (auto it = m_privateLobbies.begin(); it != m_privateLobbies.end(); ++it) {
            if (it.value()->hostPlayerName == conn->playerName) {
                lobbyCode = it.key();
                break;
            }
        }

        if (lobbyCode.isEmpty()) {
            QJsonObject error;
            error["type"] = "lobbyError";
            error["message"] = "Vous n'êtes pas l'hôte d'un lobby";
            sendMessage(socket, error);
            return;
        }

        PrivateLobby* lobby = m_privateLobbies[lobbyCode];

        // Vérifier le nombre de joueurs
        int playerCount = lobby->playerNames.size();
        if (playerCount != 2 && playerCount != 4) {
            QJsonObject error;
            error["type"] = "lobbyError";
            error["message"] = "Il faut 2 ou 4 joueurs pour lancer une partie";
            sendMessage(socket, error);
            return;
        }

        // Vérifier que tous sont prêts
        for (bool ready : lobby->readyStatus) {
            if (!ready) {
                QJsonObject error;
                error["type"] = "lobbyError";
                error["message"] = "Tous les joueurs doivent être prêts";
                sendMessage(socket, error);
                return;
            }
        }

        qDebug() << "Lancement de la partie depuis le lobby" << lobbyCode << "avec" << playerCount << "joueurs";

        // Notifier tous les joueurs que la partie va démarrer
        QJsonObject startMsg;
        startMsg["type"] = "lobbyGameStart";
        sendLobbyMessage(lobbyCode, startMsg);

        if (playerCount == 4) {
            // Lancer une partie directement avec ces 4 joueurs
            startLobbyGameWith4Players(lobby);
        } else if (playerCount == 2) {
            // Ajouter les 2 joueurs au matchmaking en tant que partenaires
            startLobbyGameWith2Players(lobby);
        }

        // Supprimer le lobby
        delete lobby;
        m_privateLobbies.remove(lobbyCode);
    }

    void handleLeaveLobby(QWebSocket *socket) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        PlayerConnection* conn = m_connections[connectionId];
        if (!conn) return;

        // Trouver et quitter le lobby
        QString lobbyCode;
        for (auto it = m_privateLobbies.begin(); it != m_privateLobbies.end(); ++it) {
            if (it.value()->playerNames.contains(conn->playerName)) {
                lobbyCode = it.key();
                break;
            }
        }

        if (lobbyCode.isEmpty()) return;

        PrivateLobby* lobby = m_privateLobbies[lobbyCode];
        int playerIndex = lobby->playerNames.indexOf(conn->playerName);

        if (playerIndex >= 0) {
            lobby->playerNames.removeAt(playerIndex);
            lobby->playerAvatars.removeAt(playerIndex);
            lobby->readyStatus.removeAt(playerIndex);

            qDebug() << "Joueur" << conn->playerName << "a quitté le lobby" << lobbyCode;

            // Si le lobby est vide, le supprimer
            if (lobby->playerNames.isEmpty()) {
                qDebug() << "Lobby" << lobbyCode << "supprimé (vide)";
                delete lobby;
                m_privateLobbies.remove(lobbyCode);
            } else {
                // Si c'était l'hôte, désigner un nouvel hôte
                if (lobby->hostPlayerName == conn->playerName) {
                    lobby->hostPlayerName = lobby->playerNames.first();
                    qDebug() << "Nouvel hote du lobby" << lobbyCode << ":" << lobby->hostPlayerName;
                }
                // Mettre à jour les autres joueurs
                sendLobbyUpdate(lobby->code);
            }
        }
    }

    void sendLobbyUpdate(const QString &lobbyCode) {
        if (!m_privateLobbies.contains(lobbyCode)) return;

        PrivateLobby* lobby = m_privateLobbies[lobbyCode];

        QJsonArray playersArray;
        for (int i = 0; i < lobby->playerNames.size(); i++) {
            QJsonObject player;
            player["name"] = lobby->playerNames[i];
            player["avatar"] = lobby->playerAvatars[i];
            player["ready"] = lobby->readyStatus[i];
            player["isHost"] = (lobby->playerNames[i] == lobby->hostPlayerName);
            playersArray.append(player);
        }

        QJsonObject update;
        update["type"] = "lobbyUpdate";
        update["players"] = playersArray;

        sendLobbyMessage(lobbyCode, update);
    }

    void sendLobbyMessage(const QString &lobbyCode, const QJsonObject &message) {
        if (!m_privateLobbies.contains(lobbyCode)) return;

        PrivateLobby* lobby = m_privateLobbies[lobbyCode];

        for (const QString &playerName : lobby->playerNames) {
            // Trouver la connexion du joueur
            for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
                if (it.value()->playerName == playerName) {
                    sendMessage(it.value()->socket, message);
                    break;
                }
            }
        }
    }

    void startLobbyGameWith4Players(PrivateLobby* lobby) {
        // Créer une partie avec les 4 joueurs du lobby
        QList<QString> connectionIds;

        for (const QString &playerName : lobby->playerNames) {
            for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
                if (it.value()->playerName == playerName) {
                    connectionIds.append(it.key());
                    break;
                }
            }
        }

        if (connectionIds.size() != 4) {
            qDebug() << "Erreur: impossible de trouver les 4 connexions";
            return;
        }

        // Créer la partie (similaire à tryCreateGame)
        int roomId = m_nextRoomId++;
        GameRoom* room = new GameRoom();
        room->roomId = roomId;
        room->connectionIds = connectionIds;
        room->originalConnectionIds = connectionIds;
        room->gameState = "waiting";

        // Crée les joueurs du jeu
        for (int i = 0; i < 4; i++) {
            PlayerConnection* conn = m_connections[connectionIds[i]];
            conn->gameRoomId = roomId;
            conn->playerIndex = i;

            // Sauvegarder le nom et avatar du joueur pour reconnexion
            room->playerNames.append(conn->playerName);
            room->playerAvatars.append(conn->avatar);
            m_playerNameToRoomId[conn->playerName] = roomId;

            std::vector<Carte*> emptyHand;
            auto player = std::make_unique<Player>(
                conn->playerName.toStdString(),
                emptyHand,
                i
            );
            room->players.push_back(std::move(player));
            room->isBot.push_back(false);
        }

        // Distribue les cartes
        room->deck.shuffleDeck();
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 8; j++) {
                Carte* carte = room->deck.drawCard();
                if (carte) {
                    room->players[i]->addCardToHand(carte);
                }
            }
        }

        m_gameRooms[roomId] = room;

        // Init le premier joueur (celui qui commence les enchères)
        room->firstPlayerIndex = 0;
        room->currentPlayerIndex = 0;

        qDebug() << "Partie créée depuis lobby (4 joueurs) - Room ID:" << roomId;

        // Notifier tous les joueurs
        for (int i = 0; i < 4; i++) {
            QJsonArray opponentsArray;
            for (int j = 0; j < 4; j++) {
                if (j != i) {
                    QJsonObject opponent;
                    opponent["name"] = room->playerNames[j];
                    opponent["avatar"] = room->playerAvatars[j];
                    opponent["position"] = j;
                    opponentsArray.append(opponent);
                }
            }

            QJsonObject gameFoundMsg;
            gameFoundMsg["type"] = "gameFound";
            gameFoundMsg["playerPosition"] = i;
            gameFoundMsg["opponents"] = opponentsArray;

            sendMessage(m_connections[connectionIds[i]]->socket, gameFoundMsg);
        }

        // Envoyer les mains à chaque joueur
        for (int i = 0; i < 4; i++) {
            QJsonArray cardsArray;
            for (Carte* carte : room->players[i]->getMain()) {
                QJsonObject cardObj;
                cardObj["suit"] = static_cast<int>(carte->getCouleur());
                cardObj["value"] = static_cast<int>(carte->getChiffre());
                cardsArray.append(cardObj);
            }

            QJsonObject cardsMsg;
            cardsMsg["type"] = "cardsDealt";
            cardsMsg["cards"] = cardsArray;
            sendMessage(m_connections[connectionIds[i]]->socket, cardsMsg);
        }

        // Phase d'enchères
        room->gameState = "bidding";
        QJsonObject biddingMsg;
        biddingMsg["type"] = "biddingPhase";
        biddingMsg["currentPlayer"] = room->currentPlayerIndex;
        broadcastToRoom(roomId, biddingMsg);

        // Démarrer le timer pour le premier joueur à annoncer (toujours humain dans un lobby)
        startBidTimeout(roomId, room->currentPlayerIndex);
    }

    void startLobbyGameWith2Players(PrivateLobby* lobby) {
        // Ajouter les 2 joueurs au matchmaking en tant que partenaires
        // Ils seront placés aux positions 0 et 2 (partenaires)
        qDebug() << "Matchmaking avec 2 joueurs partenaires - ajout à la queue";

        QList<QString> lobbyConnectionIds;

        // Récupérer les IDs de connexion des 2 joueurs du lobby
        for (const QString &playerName : lobby->playerNames) {
            for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
                if (it.value()->playerName == playerName) {
                    lobbyConnectionIds.append(it.key());
                    break;
                }
            }
        }

        if (lobbyConnectionIds.size() != 2) {
            qDebug() << "Erreur: impossible de trouver les 2 connexions du lobby";
            return;
        }

        // Marquer les 2 joueurs comme partenaires
        PlayerConnection* player1 = m_connections[lobbyConnectionIds[0]];
        PlayerConnection* player2 = m_connections[lobbyConnectionIds[1]];
        player1->lobbyPartnerId = lobbyConnectionIds[1];
        player2->lobbyPartnerId = lobbyConnectionIds[0];

        qDebug() << "Joueurs marqués comme partenaires:" << player1->playerName << "et" << player2->playerName;

        // Ajouter à la queue de matchmaking
        for (const QString &connId : lobbyConnectionIds) {
            if (!m_matchmakingQueue.contains(connId)) {
                m_matchmakingQueue.enqueue(connId);
                qDebug() << "Joueur du lobby ajouté à la queue:" << m_connections[connId]->playerName;
            }
        }

        // Notifier tous les joueurs en queue
        QJsonObject response;
        response["type"] = "matchmakingStatus";
        response["status"] = "searching";
        response["playersInQueue"] = m_matchmakingQueue.size();

        for (const QString &queuedConnectionId : m_matchmakingQueue) {
            if (m_connections.contains(queuedConnectionId)) {
                sendMessage(m_connections[queuedConnectionId]->socket, response);
            }
        }

        // Vérifier si on peut créer une partie
        tryCreateGame();
    }

    void sendMessage(QWebSocket *socket, const QJsonObject &message) {
        QJsonDocument doc(message);
        socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    }

    void broadcastToRoom(int roomId, const QJsonObject &message, 
                        const QString &excludeConnectionId = QString()) {
        if (!m_gameRooms.contains(roomId)) return;

        GameRoom* room = m_gameRooms[roomId];
        for (const QString &connId : room->connectionIds) {
            if (connId == excludeConnectionId) continue;
            
            PlayerConnection *conn = m_connections.value(connId);
            if (conn && conn->socket) {
                sendMessage(conn->socket, message);
            }
        }
    }

    QWebSocketServer *m_server;
    QMap<QString, PlayerConnection*> m_connections; // connectionId → PlayerConnection
    QQueue<QString> m_matchmakingQueue;
    QMap<int, GameRoom*> m_gameRooms;
    QMap<QString, int> m_playerNameToRoomId;  // playerName → roomId pour reconnexion
    QMap<QString, PrivateLobby*> m_privateLobbies;  // code → PrivateLobby
    int m_nextRoomId;
    DatabaseManager *m_dbManager;

    // Timer pour démarrer une partie avec des bots après 30 secondes d'inactivité
    QTimer *m_matchmakingTimer;
    int m_lastQueueSize;  // Pour détecter si de nouveaux joueurs arrivent
};

#endif // GAMESERVER_H