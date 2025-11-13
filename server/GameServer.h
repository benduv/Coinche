#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QMap>
#include <QQueue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include "Player.h"
#include "Deck.h"
#include "Carte.h"
#include "GameModel.h"

// Connexion réseau d'un joueur (pas la logique métier)
struct PlayerConnection {
    QWebSocket* socket;
    QString connectionId;      // ID unique WebSocket
    QString playerName;
    int gameRoomId;
    int playerIndex;           // Position dans la partie (0-3)
};

// Une partie de jeu avec la vraie logique
struct GameRoom {
    int roomId;
    QList<QString> connectionIds;  // IDs des connexions WebSocket
    QString gameState; // "waiting", "bidding", "playing", "finished"

    // Les objets de jeu réels
    std::vector<std::unique_ptr<Player>> players;  // Les 4 joueurs
    Deck deck;

    // État de la partie
    Carte::Couleur couleurAtout = Carte::COULEURINVALIDE;
    int currentPlayerIndex = 0;
    int biddingPlayer = 0;
    int firstPlayerIndex = 0;  // Joueur qui commence les enchères ET qui jouera en premier

    // Gestion des enchères
    int passedBidsCount = 0;
    Player::Annonce lastBidAnnonce = Player::ANNONCEINVALIDE;
    Carte::Couleur lastBidCouleur = Carte::COULEURINVALIDE;
    int lastBidderIndex = -1;

    // Pli en cours
    std::vector<std::pair<int, Carte*>> currentPli;  // pair<playerIndex, carte>
    Carte::Couleur couleurDemandee = Carte::COULEURINVALIDE;

    GameModel* gameModel = nullptr;
};

class GameServer : public QObject {
    Q_OBJECT

public:
    explicit GameServer(quint16 port, QObject *parent = nullptr)
        : QObject(parent)
        , m_server(new QWebSocketServer("CoinchServer", QWebSocketServer::NonSecureMode, this))
        , m_nextRoomId(1)
    {
        if (m_server->listen(QHostAddress::Any, port)) {
            qDebug() << "Serveur demarre sur le port" << port;
            connect(m_server, &QWebSocketServer::newConnection,
                    this, &GameServer::onNewConnection);
        } else {
            qDebug() << "Erreur: impossible de démarrer le serveur";
        }
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
        } else if (type == "joinMatchmaking") {
            handleJoinMatchmaking(sender);
        } else if (type == "leaveMatchmaking") {
            handleLeaveMatchmaking(sender);
        } else if (type == "playCard") {
            handlePlayCard(sender, obj);
        } else if (type == "makeBid") {
            handleMakeBid(sender, obj);
        }
    }

    void onDisconnected() {
        QWebSocket *socket = qobject_cast<QWebSocket*>(this->sender());
        if (!socket) return;

        qDebug() << "Client deconnecte";
        
        // Trouve la connexion
        QString connectionId;
        for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
            if (it.value()->socket == socket) {
                connectionId = it.key();
                
                // Retire de la file d'attente
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
        
        socket->deleteLater();
    }

private:
    void handleRegister(QWebSocket *socket, const QJsonObject &data) {
        QString connectionId = QUuid::createUuid().toString();
        QString playerName = data["playerName"].toString();

        PlayerConnection *conn = new PlayerConnection{
            socket, 
            connectionId, 
            playerName, 
            -1,    // Pas encore en partie
            -1     // Pas encore de position
        };
        m_connections[connectionId] = conn;

        QJsonObject response;
        response["type"] = "registered";
        response["connectionId"] = connectionId;
        response["playerName"] = playerName;
        sendMessage(socket, response);

        qDebug() << "Joueur enregistre:" << playerName << "ID:" << connectionId;
    }

    void handleJoinMatchmaking(QWebSocket *socket) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        if (!m_matchmakingQueue.contains(connectionId)) {
            m_matchmakingQueue.enqueue(connectionId);
            qDebug() << "Joueur en attente:" << connectionId 
                     << "Queue size:" << m_matchmakingQueue.size();

            QJsonObject response;
            response["type"] = "matchmakingStatus";
            response["status"] = "searching";
            response["playersInQueue"] = m_matchmakingQueue.size();
            sendMessage(socket, response);

            // Essaye de créer une partie si 4 joueurs
            tryCreateGame();
        }
    }

    void handleLeaveMatchmaking(QWebSocket *socket) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        m_matchmakingQueue.removeAll(connectionId);

        QJsonObject response;
        response["type"] = "matchmakingStatus";
        response["status"] = "left";
        sendMessage(socket, response);
    }

    void tryCreateGame() {
        if (m_matchmakingQueue.size() >= 4) {
            QList<QString> connectionIds;
            for (int i = 0; i < 4; i++) {
                connectionIds.append(m_matchmakingQueue.dequeue());
            }

            int roomId = m_nextRoomId++;
            GameRoom* room = new GameRoom();  // Créé sur le heap
            room->roomId = roomId;
            room->connectionIds = connectionIds;
            room->gameState = "waiting";
            
            // Crée les joueurs du jeu
            for (int i = 0; i < 4; i++) {
                PlayerConnection* conn = m_connections[connectionIds[i]];
                conn->gameRoomId = roomId;
                conn->playerIndex = i;
                
                std::vector<Carte*> emptyHand;
                auto player = std::make_unique<Player>(
                    conn->playerName.toStdString(),
                    emptyHand,
                    i
                );
                room->players.push_back(std::move(player));
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
                // Trier la main de chaque joueur pour maintenir la synchronisation avec les clients
                room->players[i]->sortHand();
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

        }
    }

    void notifyGameStart(int roomId, const QList<QString> &connectionIds) {
        GameRoom* room = m_gameRooms[roomId];
        if (!room) return;

        qDebug() << "Envoi des notifications gameFound à" << connectionIds.size() << "joueurs";

        
        for (int i = 0; i < connectionIds.size(); i++) {
            PlayerConnection *conn = m_connections[connectionIds[i]];
            if (!conn) {
                qDebug() << "Erreur: Connexion introuvable pour ID" << connectionIds[i];
                continue;
            } 

            QJsonObject msg;
            msg["type"] = "gameFound";
            msg["roomId"] = roomId;
            msg["playerPosition"] = i;
            
            // Envoi les cartes du joueur
            QJsonArray myCards;
            const auto& playerHand = room->players[i]->getMain();
            qDebug() << "Envoi de" << playerHand.size() << "cartes au joueur" << i;

            for (const auto* carte : playerHand) {
                if (carte) {
                    QJsonObject cardObj;
                    cardObj["value"] = static_cast<int>(carte->getChiffre());
                    cardObj["suit"] = static_cast<int>(carte->getCouleur());
                    myCards.append(cardObj);
                }
            }
            msg["myCards"] = myCards;
            
            // Infos sur les adversaires
            QJsonArray opponents;
            for (int j = 0; j < connectionIds.size(); j++) {
                if (i != j) {
                    QJsonObject opp;
                    opp["position"] = j;
                    opp["name"] = m_connections[connectionIds[j]]->playerName;
                    opp["cardCount"] = int(room->players[j]->getMain().size());
                    opponents.append(opp);
                }
            }
            msg["opponents"] = opponents;
            qDebug() << "Envoi gameFound à" << conn->playerName << "position" << i;


            sendMessage(conn->socket, msg);
        }
        qDebug() << "Toutes les notifications envoyées";

    }

    void handlePlayCard(QWebSocket *socket, const QJsonObject &data) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        PlayerConnection* conn = m_connections[connectionId];
        int roomId = conn->gameRoomId;
        if (roomId == -1) return;

        GameRoom* room = m_gameRooms[roomId];
        if (!room) return;

        int playerIndex = conn->playerIndex;
        int cardIndex = data["cardIndex"].toInt();

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

        // Ajouter au pli courant
        room->currentPli.push_back(std::make_pair(playerIndex, cartePlayed));

        // IMPORTANT : Retirer la carte de la main du joueur côté serveur
        // Cela maintient la synchronisation avec les clients
        player->removeCard(cardIndex);

        qDebug() << "GameServer - Carte retirée, main du joueur" << playerIndex
                 << "contient maintenant" << player->getMain().size() << "cartes";

        qDebug() << "GameServer - Carte jouee par joueur" << playerIndex
                 << "- Pli:" << room->currentPli.size() << "/4";

        // Broadcast l'action à tous les joueurs avec les infos de la carte
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
            // Passe au joueur suivant
            room->currentPlayerIndex = (room->currentPlayerIndex + 1) % 4;

            // Notifier avec les cartes jouables pour le nouveau joueur
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
        carteGagnante->printCarte();

        for (size_t i = 1; i < room->currentPli.size(); i++) {
            Carte* c = room->currentPli[i].second;
            qDebug() << "Comparaison avec la carte du joueur" << room->currentPli[i].first << ":";
            c->printCarte();
            if (*carteGagnante < *c) {
                carteGagnante = c;
                gagnantIndex = room->currentPli[i].first;
                carteGagnante->printCarte();
            }
        }

        qDebug() << "GameServer - Pli termine, gagnant: joueur" << gagnantIndex;

        // Notifie le gagnant du pli
        QJsonObject pliFinishedMsg;
        pliFinishedMsg["type"] = "pliFinished";
        pliFinishedMsg["winnerId"] = gagnantIndex;
        broadcastToRoom(roomId, pliFinishedMsg);

        // Réinitialise pour le prochain pli
        room->currentPli.clear();
        room->couleurDemandee = Carte::COULEURINVALIDE;
        room->currentPlayerIndex = gagnantIndex;  // Le gagnant commence le prochain pli

        // Notifier avec les cartes jouables pour le nouveau pli
        notifyPlayersWithPlayableCards(roomId);
    }

    void handleMakeBid(QWebSocket *socket, const QJsonObject &data) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        PlayerConnection* conn = m_connections[connectionId];
        int roomId = conn->gameRoomId;
        if (roomId == -1) return;

        GameRoom* room = m_gameRooms[roomId];
        if (!room) return;

        int playerIndex = conn->playerIndex;
        int bidValue = data["bidValue"].toInt();
        int suit = data["suit"].toInt();

        Player::Annonce annonce = static_cast<Player::Annonce>(bidValue);

        // Mise à jour l'état de la room
        if (annonce == Player::PASSE) {
            room->passedBidsCount++;
            qDebug() << "GameServer - Joueur" << playerIndex << "passe ("
                     << room->passedBidsCount << "/3 passes)";
        } else {
            room->lastBidAnnonce = annonce;
            room->lastBidCouleur = static_cast<Carte::Couleur>(suit);
            room->lastBidderIndex = playerIndex;
            room->passedBidsCount = 0;  // Reset le compteur
            qDebug() << "GameServer - Nouvelle enchère:" << bidValue << "couleur:" << suit;
        }

        // Broadcast l'enchère à tous
        QJsonObject msg;
        msg["type"] = "bidMade";
        msg["playerIndex"] = playerIndex;
        msg["bidValue"] = bidValue;
        msg["suit"] = suit;
        broadcastToRoom(roomId, msg);

        // Vérifie si phase d'enchères terminée
        if (room->passedBidsCount >= 3 && room->lastBidAnnonce != Player::ANNONCEINVALIDE) {
            qDebug() << "GameServer - Fin des enchères! Lancement phase de jeu";
            for (int i = 0; i < 4; i++) {
                room->players[i]->setAtout(room->lastBidCouleur);
            }
            startPlayingPhase(roomId);
        } else {
            // Passe au joueur suivant
            room->currentPlayerIndex = (room->currentPlayerIndex + 1) % 4;
            room->biddingPlayer = room->currentPlayerIndex;

            QJsonObject stateMsg;
            stateMsg["type"] = "gameState";
            stateMsg["currentPlayer"] = room->currentPlayerIndex;
            stateMsg["biddingPlayer"] = room->biddingPlayer;
            stateMsg["biddingPhase"] = true;
            broadcastToRoom(roomId, stateMsg);
        }
    }

    void startPlayingPhase(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        room->gameState = "playing";
        room->couleurAtout = room->lastBidCouleur;
        // Le joueur qui a commencé les enchères joue en premier
        room->currentPlayerIndex = room->firstPlayerIndex;

        qDebug() << "Phase de jeu demarree - Atout:" << static_cast<int>(room->couleurAtout)
                 << "Premier joueur:" << room->currentPlayerIndex
                 << "(Gagnant encheres:" << room->lastBidderIndex << ")";

        // Notifie tous les joueurs du changement de phase avec cartes jouables
        notifyPlayersWithPlayableCards(roomId);
    }

    void notifyPlayersWithPlayableCards(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        int currentPlayer = room->currentPlayerIndex;

        // Calculer les cartes jouables pour le joueur actuel
        QJsonArray playableCards = calculatePlayableCards(room, currentPlayer);

        // Envoyer à tous les joueurs
        QJsonObject stateMsg;
        stateMsg["type"] = "gameState";
        stateMsg["biddingPhase"] = false;
        stateMsg["currentPlayer"] = currentPlayer;
        stateMsg["atout"] = static_cast<int>(room->couleurAtout);
        stateMsg["playableCards"] = playableCards;  // Liste des indices des cartes jouables

        // Si on est au début de la phase de jeu, inclure les infos d'enchères
        if (room->currentPli.empty() && currentPlayer == room->firstPlayerIndex) {
            stateMsg["biddingWinnerId"] = room->lastBidderIndex;
            stateMsg["biddingWinnerAnnonce"] = static_cast<int>(room->lastBidAnnonce);
        }

        broadcastToRoom(roomId, stateMsg);
    }

    QJsonArray calculatePlayableCards(GameRoom* room, int playerIndex) {
        QJsonArray playableIndices;

        if (room->gameState != "playing") {
            // Pendant les enchères, aucune carte n'est jouable
            return playableIndices;
        }

        Player* player = room->players[playerIndex].get();
        if (!player) return playableIndices;

        // Déterminer la carte gagnante actuelle du pli
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

        // Vérifier chaque carte
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

        // Notifier les autres joueurs
        QJsonObject msg;
        msg["type"] = "playerDisconnected";
        msg["playerIndex"] = conn->playerIndex;
        broadcastToRoom(roomId, msg, connectionId);

        // Terminer la partie et libérer la mémoire
        GameRoom* room = m_gameRooms.value(roomId);
        if (room) {
            delete room;  // Libérer la GameRoom
            m_gameRooms.remove(roomId);
        }
    }

    QString getConnectionIdBySocket(QWebSocket *socket) {
        for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
            if (it.value()->socket == socket) {
                return it.key();
            }
        }
        return QString();
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
    QMap<int, GameRoom*> m_gameRooms; // roomId → GameRoom*
    int m_nextRoomId;
};

#endif // GAMESERVER_H