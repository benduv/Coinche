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
    
    // Vos objets de jeu réels
    std::vector<std::unique_ptr<Player>> players;  // Les 4 vrais joueurs
    Deck deck;
    
    // État de la partie
    Carte::Couleur couleurAtout;
    int currentPlayerIndex;
    // ... autres données
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
        
        // Libérer toutes les GameRooms
        qDeleteAll(m_gameRooms.values());
        
        // Libérer toutes les connexions
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
        
        // Envoyer message de bienvenue
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
        
        // Trouver la connexion
        QString connectionId;
        for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
            if (it.value()->socket == socket) {
                connectionId = it.key();
                
                // Retirer de la file d'attente
                m_matchmakingQueue.removeAll(connectionId);
                
                // Notifier la room si le joueur était en partie
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

        qDebug() << "Joueur enregistré:" << playerName << "ID:" << connectionId;
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

            // Essayer de créer une partie si 4 joueurs
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
            GameRoom* room = new GameRoom();  // Créer sur le heap
            room->roomId = roomId;
            room->connectionIds = connectionIds;
            room->gameState = "waiting";
            
            // Créer les vrais joueurs du jeu
            for (int i = 0; i < 4; i++) {
                PlayerConnection* conn = m_connections[connectionIds[i]];
                conn->gameRoomId = roomId;
                conn->playerIndex = i;
                
                // Créer un vrai Player avec votre classe
                std::vector<Carte*> emptyHand;
                auto player = std::make_unique<Player>(
                    conn->playerName.toStdString(),
                    emptyHand,
                    i
                );
                room->players.push_back(std::move(player));
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
            
            m_gameRooms[roomId] = room;  // Stocker le pointeur

            qDebug() << "Partie créée! Room ID:" << roomId;

                // Deck deck;
    // deck.shuffleDeck();

    // std::vector<Carte*> main1, main2, main3, main4;
    // deck.distribute(main1, main2, main3, main4);

            // std::vector<std::unique_ptr<Player>> players;
            // players.push_back(std::make_unique<Player>("Joueur1", main1, 0));
            // players.push_back(std::make_unique<Player>("Joueur2", main2, 1));
            // players.push_back(std::make_unique<Player>("Joueur3", main3, 2));
            // players.push_back(std::make_unique<Player>("Joueur4", main4, 3));

            std::vector<std::reference_wrapper<std::unique_ptr<Player>>> playerRefs;
            for (auto& player : room->players) {
                player->sortHand();
                playerRefs.push_back(std::ref(player));
            }
            
            GameModel gameModel(playerRefs, deck);
            engine.rootContext()->setContextProperty("gameModel", &gameModel);

            // Notifier tous les joueurs
            notifyGameStart(roomId, connectionIds);
        }
    }

    void notifyGameStart(int roomId, const QList<QString> &connectionIds) {
        for (int i = 0; i < connectionIds.size(); i++) {
            PlayerConnection *conn = m_connections[connectionIds[i]];
            if (!conn) continue;

            QJsonObject msg;
            msg["type"] = "gameFound";
            msg["roomId"] = roomId;
            msg["playerPosition"] = i; // Position du joueur (0-3)
            
            QJsonArray opponents;
            for (int j = 0; j < connectionIds.size(); j++) {
                if (i != j) {
                    QJsonObject opp;
                    opp["position"] = j;
                    opp["name"] = m_connections[connectionIds[j]]->playerName;
                    opponents.append(opp);
                }
            }
            msg["opponents"] = opponents;

            sendMessage(conn->socket, msg);
        }
    }

    void handlePlayCard(QWebSocket *socket, const QJsonObject &data) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        PlayerConnection* conn = m_connections[connectionId];
        int roomId = conn->gameRoomId;
        if (roomId == -1) return;

        GameRoom* room = m_gameRooms[roomId];  // Pointeur
        int playerIndex = conn->playerIndex;
        int cardIndex = data["cardIndex"].toInt();
        
        // Utiliser la vraie logique de votre classe Player
        Player* player = room->players[playerIndex].get();
        
        // Vérifier si la carte est jouable avec VOTRE logique
        // (cette partie dépend de votre implémentation du jeu)
        
        // Broadcaster l'action à tous les joueurs de la room
        QJsonObject msg;
        msg["type"] = "cardPlayed";
        msg["playerIndex"] = playerIndex;
        msg["cardIndex"] = cardIndex;

        broadcastToRoom(roomId, msg);
    }

    void handleMakeBid(QWebSocket *socket, const QJsonObject &data) {
        QString connectionId = getConnectionIdBySocket(socket);
        if (connectionId.isEmpty()) return;

        PlayerConnection* conn = m_connections[connectionId];
        int roomId = conn->gameRoomId;
        if (roomId == -1) return;

        QJsonObject msg;
        msg["type"] = "bidMade";
        msg["playerIndex"] = conn->playerIndex;
        msg["bidValue"] = data["bidValue"].toInt();
        msg["suit"] = data["suit"].toInt();

        broadcastToRoom(roomId, msg);
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