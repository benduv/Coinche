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
    bool coinched = false;  // True si COINCHE a été annoncé
    bool surcoinched = false;  // True si SURCOINCHE a été annoncé
    QTimer* surcoincheTimer = nullptr;  // Timer pour le timeout de surcoinche
    int surcoincheTimeLeft = 0;  // Temps restant en secondes

    // Pli en cours
    std::vector<std::pair<int, Carte*>> currentPli;  // pair<playerIndex, carte>
    Carte::Couleur couleurDemandee = Carte::COULEURINVALIDE;

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

        // Ajoute au pli courant
        room->currentPli.push_back(std::make_pair(playerIndex, cartePlayed));

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
            pointsPli += carte->getValeurDeLaCarte();
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

        // Notifie le gagnant du pli avec les scores de manche mis à jour
        QJsonObject pliFinishedMsg;
        pliFinishedMsg["type"] = "pliFinished";
        pliFinishedMsg["winnerId"] = gagnantIndex;
        pliFinishedMsg["scoreMancheTeam1"] = room->scoreMancheTeam1;
        pliFinishedMsg["scoreMancheTeam2"] = room->scoreMancheTeam2;
        broadcastToRoom(roomId, pliFinishedMsg);

        // Vérifier si la manche est terminée (tous les joueurs n'ont plus de cartes)
        bool mancheTerminee = true;
        for (const auto& player : room->players) {
            if (!player->getMain().empty()) {
                mancheTerminee = false;
                break;
            }
        }

        if (mancheTerminee) {
            qDebug() << "GameServer - Manche terminee, calcul des scores...";
            finishManche(roomId);
        } else {
            // Réinitialise pour le prochain pli
            room->currentPli.clear();
            room->couleurDemandee = Carte::COULEURINVALIDE;
            room->currentPlayerIndex = gagnantIndex;  // Le gagnant commence le prochain pli

            // Notifie avec les cartes jouables pour le nouveau pli
            notifyPlayersWithPlayableCards(roomId);
        }
    }

    void finishManche(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        // Les scores de manche ont déjà été calculés pendant les plis
        // On ajoute juste le bonus du dernier pli (+10 points)
        int lastWinner = room->currentPlayerIndex; // C'est le gagnant du dernier pli
        if (lastWinner == 0 || lastWinner == 2) {
            room->scoreMancheTeam1 += 10;
        } else {
            room->scoreMancheTeam2 += 10;
        }

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

        if (isCapotAnnonce) {
            // CAPOT: l'equipe qui a annonce doit faire tous les 8 plis
            int plisTeamAnnonceur = 0;
            if (team1HasBid) {
                plisTeamAnnonceur = room->plisCountPlayer0 + room->plisCountPlayer2;
            } else {
                plisTeamAnnonceur = room->plisCountPlayer1 + room->plisCountPlayer3;
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
                if (pointsRealisesTeam1 >= valeurContrat) {
                    // Contrat réussi: (valeurContrat + pointsRealisés) × multiplicateur
                    scoreToAddTeam1 = (valeurContrat + pointsRealisesTeam1) * multiplicateur;
                    scoreToAddTeam2 = 0;
                    qDebug() << "GameServer - Equipe 1 reussit son contrat COINCHE!";
                    qDebug() << "  Team1 marque:" << scoreToAddTeam1 << "((" << valeurContrat << "+" << pointsRealisesTeam1 << ")*" << multiplicateur << ")";
                    qDebug() << "  Team2 marque: 0";
                } else {
                    // Contrat échoué: équipe adverse marque (valeurContrat + 160) × multiplicateur
                    scoreToAddTeam1 = 0;
                    scoreToAddTeam2 = (valeurContrat + 160) * multiplicateur;
                    qDebug() << "GameServer - Equipe 1 echoue son contrat COINCHE!";
                    qDebug() << "  Team1 marque: 0";
                    qDebug() << "  Team2 marque:" << scoreToAddTeam2 << "((" << valeurContrat << "+160)*" << multiplicateur << ")";
                }
            } else {
                // Team2 a annoncé, vérifie si elle réussit
                if (pointsRealisesTeam2 >= valeurContrat) {
                    // Contrat réussi: (valeurContrat + pointsRealisés) × multiplicateur
                    scoreToAddTeam1 = 0;
                    scoreToAddTeam2 = (valeurContrat + pointsRealisesTeam2) * multiplicateur;
                    qDebug() << "GameServer - Equipe 2 reussit son contrat COINCHE!";
                    qDebug() << "  Team1 marque: 0";
                    qDebug() << "  Team2 marque:" << scoreToAddTeam2 << "((" << valeurContrat << "+" << pointsRealisesTeam2 << ")*" << multiplicateur << ")";
                } else {
                    // Contrat échoué: équipe adverse marque (valeurContrat + 160) × multiplicateur
                    scoreToAddTeam1 = (valeurContrat + 160) * multiplicateur;
                    scoreToAddTeam2 = 0;
                    qDebug() << "GameServer - Equipe 2 echoue son contrat COINCHE!";
                    qDebug() << "  Team1 marque:" << scoreToAddTeam1 << "((" << valeurContrat << "+160)*" << multiplicateur << ")";
                    qDebug() << "  Team2 marque: 0";
                }
            }
        } else if (team1HasBid) {
            // L'équipe 1 a annoncé (contrat normal)
            if (pointsRealisesTeam1 >= valeurContrat) {
                // Vérifier si Team1 a fait un CAPOT non annoncé (tous les 8 plis)
                if (capotReussi) {
                    // CAPOT non annoncé: 250 + valeurContrat
                    scoreToAddTeam1 = 250 + valeurContrat;
                    scoreToAddTeam2 = 0;
                    qDebug() << "GameServer - Equipe 1 fait un CAPOT non annonce!";
                    qDebug() << "  Team1 marque:" << scoreToAddTeam1 << "(250+" << valeurContrat << ")";
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
                if (capotReussi) {
                    // CAPOT non annoncé: 250 + valeurContrat
                    scoreToAddTeam1 = 0;
                    scoreToAddTeam2 = 250 + valeurContrat;
                    qDebug() << "GameServer - Equipe 2 fait un CAPOT non annonce!";
                    qDebug() << "  Team1 marque: 0";
                    qDebug() << "  Team2 marque:" << scoreToAddTeam2 << "(250+" << valeurContrat << ")";
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

        // Envoi les scores aux clients
        QJsonObject scoreMsg;
        scoreMsg["type"] = "mancheFinished";
        scoreMsg["scoreTotalTeam1"] = room->scoreTeam1;
        scoreMsg["scoreTotalTeam2"] = room->scoreTeam2;
        broadcastToRoom(roomId, scoreMsg);

        // Vérifier si une équipe a atteint 1000 points
        bool team1Won = room->scoreTeam1 >= 1000;
        bool team2Won = room->scoreTeam2 >= 1000;

        if (team1Won || team2Won) {
            // Une ou les deux équipes ont dépassé 1000 points
            if (team1Won && team2Won) {
                // Les deux équipes ont dépassé 1000, celle avec le plus de points gagne
                int winner = (room->scoreTeam1 > room->scoreTeam2) ? 1 : 2;
                qDebug() << "GameServer - Les deux equipes ont depasse 1000 points!";
                qDebug() << "GameServer - Equipe" << winner << "gagne avec"
                         << ((winner == 1) ? room->scoreTeam1 : room->scoreTeam2) << "points";

                QJsonObject gameOverMsg;
                gameOverMsg["type"] = "gameOver";
                gameOverMsg["winner"] = winner;
                gameOverMsg["scoreTeam1"] = room->scoreTeam1;
                gameOverMsg["scoreTeam2"] = room->scoreTeam2;
                broadcastToRoom(roomId, gameOverMsg);

                room->gameState = "finished";
            } else if (team1Won) {
                qDebug() << "GameServer - Equipe 1 gagne avec" << room->scoreTeam1 << "points!";

                QJsonObject gameOverMsg;
                gameOverMsg["type"] = "gameOver";
                gameOverMsg["winner"] = 1;
                gameOverMsg["scoreTeam1"] = room->scoreTeam1;
                gameOverMsg["scoreTeam2"] = room->scoreTeam2;
                broadcastToRoom(roomId, gameOverMsg);

                room->gameState = "finished";
            } else {
                qDebug() << "GameServer - Equipe 2 gagne avec" << room->scoreTeam2 << "points!";

                QJsonObject gameOverMsg;
                gameOverMsg["type"] = "gameOver";
                gameOverMsg["winner"] = 2;
                gameOverMsg["scoreTeam1"] = room->scoreTeam1;
                gameOverMsg["scoreTeam2"] = room->scoreTeam2;
                broadcastToRoom(roomId, gameOverMsg);

                room->gameState = "finished";
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
            qDebug() << "  Équipe 1:" << room->plisTeam1.size() << "cartes";
            qDebug() << "  Équipe 2:" << room->plisTeam2.size() << "cartes";

            // Rassembler toutes les cartes : équipe 1 d'abord, puis équipe 2
            std::vector<Carte*> allCards;
            for (const auto& pair : room->plisTeam1) {
                allCards.push_back(pair.second);
            }
            for (const auto& pair : room->plisTeam2) {
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

        // Trier les mains pour la synchronisation avec les clients
        for (int i = 0; i < 4; i++) {
            room->players[i]->sortHand();
        }

        // Réinitialiser l'état de la partie pour les enchères
        room->gameState = "bidding";
        room->passedBidsCount = 0;
        room->lastBidAnnonce = Player::ANNONCEINVALIDE;
        room->lastBidCouleur = Carte::COULEURINVALIDE;
        room->lastBidderIndex = -1;
        room->couleurAtout = Carte::COULEURINVALIDE;
        room->coinched = false;
        room->surcoinched = false;
        room->currentPli.clear();
        room->couleurDemandee = Carte::COULEURINVALIDE;

        // Le joueur suivant commence les enchères (rotation)
        room->firstPlayerIndex = (room->firstPlayerIndex + 1) % 4;
        room->currentPlayerIndex = room->firstPlayerIndex;
        room->biddingPlayer = room->firstPlayerIndex;

        qDebug() << "GameServer - Nouvelle manche: joueur" << room->firstPlayerIndex << "commence les enchères";

        // Notifier tous les joueurs de la nouvelle manche avec leurs nouvelles cartes
        notifyNewManche(roomId);
    }

    void notifyNewManche(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        qDebug() << "Envoi des notifications de nouvelle manche à" << room->connectionIds.size() << "joueurs";

        for (int i = 0; i < room->connectionIds.size(); i++) {
            PlayerConnection *conn = m_connections[room->connectionIds[i]];
            if (!conn) continue;

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
            qDebug() << "GameServer - Joueur" << playerIndex << "COINCHE l'enchère!";

            // Broadcast le COINCHE
            QJsonObject msg;
            msg["type"] = "bidMade";
            msg["playerIndex"] = playerIndex;
            msg["bidValue"] = bidValue;
            msg["suit"] = suit;
            broadcastToRoom(roomId, msg);

            // Démarrer le timer de 10 secondes pour permettre la surcoinche
            startSurcoincheTimer(roomId);
            return;
        }

        if (annonce == Player::SURCOINCHE) {
            // Vérifier qu'un COINCHE a été annoncé
            if (!room->coinched) {
                qDebug() << "GameServer - SURCOINCHE impossible: aucun COINCHE en cours";
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
            qDebug() << "GameServer - Joueur" << playerIndex << "SURCOINCHE l'enchère!";

            // Arrêter le timer de surcoinche
            stopSurcoincheTimer(roomId);

            // Notifier tous les joueurs que la surcoinche a été acceptée (masquer le bouton)
            QJsonObject timeoutMsg;
            timeoutMsg["type"] = "surcoincheTimeout";
            broadcastToRoom(roomId, timeoutMsg);

            // Broadcast le SURCOINCHE
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
                qDebug() << "GameServer - SURCOINCHE annoncé! Fin des enchères, lancement phase de jeu";
                for (int i = 0; i < 4; i++) {
                    room->players[i]->setAtout(room->lastBidCouleur);
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
            qDebug() << "GameServer - Fin des encheres! Lancement phase de jeu";
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
                room->players[i]->setAtout(room->lastBidCouleur);
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

        room->gameState = "playing";
        room->couleurAtout = room->lastBidCouleur;
        // Le joueur qui a commencé les enchères joue en premier
        room->currentPlayerIndex = room->firstPlayerIndex;

        qDebug() << "Phase de jeu demarree - Atout:" << static_cast<int>(room->couleurAtout)
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

        // Notifie les autres joueurs
        QJsonObject msg;
        msg["type"] = "playerDisconnected";
        msg["playerIndex"] = conn->playerIndex;
        broadcastToRoom(roomId, msg, connectionId);

        // Termine la partie et libére la mémoire
        GameRoom* room = m_gameRooms.value(roomId);
        if (room) {
            delete room;  // Libére la GameRoom
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
    QMap<int, GameRoom*> m_gameRooms;
    int m_nextRoomId;
};

#endif // GAMESERVER_H