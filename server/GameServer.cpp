// GameServer.cpp
// Implémentation de GameServer

#include "GameServer.h"

// Les implémentations des méthodes de GameServer seront déplacées ici
// pour alléger le fichier header

void GameServer::onNewConnection() {
    QWebSocket *socket = m_server->nextPendingConnection();

    qInfo() << "Nouvelle connexion depuis" << socket->peerAddress();
    
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

void GameServer::onTextMessageReceived(const QString &message) {
    QWebSocket *sender = qobject_cast<QWebSocket*>(this->sender());
    if (!sender) return;

    // Vérifier que le socket est encore connecté
    // Des messages peuvent être en queue même après disconnected()
    if (sender->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "GameServer - Message ignore (socket deconnecte)";
        return;
    }

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
    } else if (type == "deleteAccount") {
        handleDeleteAccount(sender, obj);
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
    } else if (type == "sendContactMessage") {
        handleSendContactMessage(sender, obj);
    } else if (type == "reportCrash") {
        handleReportCrash(sender, obj);
    } else if (type == "forgotPassword") {
        handleForgotPassword(sender, obj);
    } else if (type == "changePassword") {
        handleChangePassword(sender, obj);
    } else if (type == "changePseudo") {
        handleChangePseudo(sender, obj);
    } else if (type == "changeEmail") {
        handleChangeEmail(sender, obj);
    }
}

void GameServer::onDisconnected() {
    QWebSocket *socket = qobject_cast<QWebSocket*>(this->sender());
    if (!socket) return;

    qInfo() << "Client déconnecté - socket:" << socket;

    // Trouve la connexion correspondant à CE socket
    QString connectionId;
    QString playerName;
    int roomId = -1;
    int playerIndex = -1;
    bool wasInQueue = false;

    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        // Vérifier que la connexion existe avant d'accéder à ses membres
        if (!it.value()) {
            qCritical() << "SEGFAULT évité - Connexion null dans m_connections";
            continue;
        }

        if (it.value()->socket == socket) {
            connectionId = it.key();
            playerName = it.value()->playerName;
            roomId = it.value()->gameRoomId;
            playerIndex = it.value()->playerIndex;

            // Retire de la file d'attente et note si le joueur était en attente
            wasInQueue = m_matchmakingQueue.contains(connectionId);
            m_matchmakingQueue.removeAll(connectionId);

            // IMPORTANT: Vérifier que c'est bien la connexion ACTIVE dans la room
            // Si le joueur s'est reconnecté, il a une nouvelle connexion et on ne doit
            // PAS traiter le disconnect de l'ancienne connexion
            if (roomId != -1) {
                GameRoom* room = m_gameRooms.value(roomId);
                if (room && playerIndex >= 0 && playerIndex < room->connectionIds.size()) {
                    QString currentConnId = room->connectionIds[playerIndex];

                    if (currentConnId == connectionId) {
                        // C'est bien la connexion active, traiter la déconnexion
                        qInfo() << "Déconnexion ACTIVE - Joueur" << playerName << "(index" << playerIndex << ")";
                        handlePlayerDisconnect(connectionId);
                    } else if (!currentConnId.isEmpty()) {
                        // Le joueur s'est reconnecté avec une nouvelle connexion
                        qInfo() << "Déconnexion PÉRIMÉE ignorée - Joueur" << playerName << "déjà reconnecté (old:" << connectionId << "current:" << currentConnId << ")";
                    } else {
                        // Le connectionId est vide (déjà traité)
                        qInfo() << "Déconnexion déjà traitée - connectionId vide pour joueur" << playerIndex;
                    }
                } else {
                    qInfo() << "Room introuvable ou index invalide pour connexion" << connectionId;
                }
            }

            // Terminer le tracking de session (lightweight)
            if (!playerName.isEmpty()) {
                m_dbManager->recordSessionEnd(playerName);
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


void GameServer::handleRegister(QWebSocket *socket, const QJsonObject &data) {
    QString playerName = data["playerName"].toString();
    QString avatar = data["avatar"].toString();
    if (avatar.isEmpty()) avatar = "avataaars1.svg";

    // Si le joueur n'a pas de nom (invité), générer un nom unique
    if (playerName.isEmpty()) {
        static int guestCounter = 0;
        playerName = QString("Invité %1").arg(++guestCounter);
    }

    // IMPORTANT: Vérifier si une connexion existe déjà pour ce socket
    // Si oui, la réutiliser au lieu d'en créer une nouvelle
    QString existingConnectionId = getConnectionIdBySocket(socket);
    QString connectionId;
    PlayerConnection *conn = nullptr;

    if (!existingConnectionId.isEmpty()) {
        // Connexion existante trouvée, la réutiliser
        connectionId = existingConnectionId;
        conn = m_connections.value(connectionId);
        if (conn) {
            qDebug() << "Réutilisation de la connexion existante:" << connectionId
                    << "pour" << playerName << "(ancien nom:" << conn->playerName << ")";

            // Mettre à jour le nom et l'avatar si nécessaire
            conn->playerName = playerName;
            conn->avatar = avatar;
            // NE PAS réinitialiser gameRoomId et playerIndex !
        }
    }

    if (!conn) {
        // Pas de connexion existante, en créer une nouvelle
        connectionId = QUuid::createUuid().toString();
        conn = new PlayerConnection{
            socket,
            connectionId,
            playerName,
            avatar,
            -1,    // Pas encore en partie
            -1     // Pas encore de position
        };
        m_connections[connectionId] = conn;
        qDebug() << "Nouvelle connexion créée:" << connectionId << "pour" << playerName;
    }

    QJsonObject response;
    response["type"] = "registered";
    response["connectionId"] = connectionId;
    response["playerName"] = playerName;
    response["avatar"] = avatar;
    sendMessage(socket, response);

    qInfo() << "Joueur enregistré:" << playerName << "ID:" << connectionId;

    // Vérifier si le joueur était en partie avant de se déconnecter
    bool wasInGame = data["wasInGame"].toBool(false);
    qDebug() << "Verification reconnexion pour" << playerName << "- wasInGame:" << wasInGame
            << "m_playerNameToRoomId.contains:" << m_playerNameToRoomId.contains(playerName);

    // Si le joueur était en partie mais n'est plus dans le mapping, la partie s'est terminée
    if (wasInGame && !m_playerNameToRoomId.contains(playerName)) {
        qInfo() << "Partie terminée pendant la déconnexion de" << playerName << "- notification au client";

        QJsonObject notification;
        notification["type"] = "gameNoLongerExists";
        notification["message"] = "La partie s'est terminée pendant votre absence";
        sendMessage(socket, notification);
        return;
    }

    if (m_playerNameToRoomId.contains(playerName)) {
        int roomId = m_playerNameToRoomId[playerName];
        GameRoom* room = m_gameRooms.value(roomId);
        qDebug() << "RoomId:" << roomId << "Room valide:" << (room != nullptr) << "GameState:" << (room ? room->gameState : "N/A");

        // Si la room n'existe plus ou est terminée, informer le client
        if (!room || room->gameState == "finished") {
            qInfo() << "Partie terminée/inexistante pour" << playerName << "- notification au client";

            QJsonObject notification;
            notification["type"] = "gameNoLongerExists";
            notification["message"] = "La partie est terminée ou n'existe plus";
            sendMessage(socket, notification);

            // Nettoyer le mapping
            m_playerNameToRoomId.remove(playerName);
            return;
        }

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

            qDebug() << "DEBUG reconnexion - playerIndex:" << playerIndex
                    << "connectionIds.size:" << room->connectionIds.size()
                    << "currentConnId:" << (playerIndex >= 0 && playerIndex < room->connectionIds.size() ? room->connectionIds[playerIndex] : "N/A")
                    << "newConnId:" << connectionId
                    << "isDifferentConnection:" << isDifferentConnection
                    << "isBot:" << (playerIndex != -1 ? room->isBot[playerIndex] : false);

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

void GameServer::handleReconnection(const QString& connectionId, int roomId, int playerIndex) {
    GameRoom* room = m_gameRooms.value(roomId);
    if (!room) return;

    PlayerConnection* conn = m_connections[connectionId];
    if (!conn) return;

    qInfo() << "Reconnexion - Joueur" << conn->playerName << "(index" << playerIndex << ") rejoint partie" << roomId;

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
    room->players[playerIndex]->sortHand();  // Trier selon l'atout actuel
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
        stateMsg["firstPlayerIndex"] = room->firstPlayerIndex;

        // RECONNEXION: Envoyer les scores même pendant la phase d'enchères
        stateMsg["scoreTeam1"] = room->scoreMancheTeam1;
        stateMsg["scoreTeam2"] = room->scoreMancheTeam2;
        stateMsg["scoreTotalTeam1"] = room->scoreTeam1;
        stateMsg["scoreTotalTeam2"] = room->scoreTeam2;

        // RECONNEXION: Envoyer les informations sur l'annonce en cours (si déjà une annonce)
        if (room->lastBidderIndex >= 0) {
            stateMsg["lastBidderIndex"] = room->lastBidderIndex;
            stateMsg["lastBidAnnonce"] = static_cast<int>(room->lastBidAnnonce);
            stateMsg["lastBidSuit"] = static_cast<int>(room->lastBidSuit);
            stateMsg["isCoinched"] = room->coinched;
            stateMsg["isSurcoinched"] = room->surcoinched;
            qInfo() << "GameServer - Reconnexion (phase enchères): Envoi des infos d'annonce - lastBidder:" << room->lastBidderIndex
                        << "annonce:" << static_cast<int>(room->lastBidAnnonce)
                        << "suit:" << static_cast<int>(room->lastBidSuit);
        }

        sendMessage(conn->socket, stateMsg);
    } else if (room->gameState == "playing") {
        // Envoyer l'état de jeu avec l'atout et les cartes jouables si c'est son tour
        QJsonObject stateMsg;
        stateMsg["type"] = "gameState";
        stateMsg["biddingPhase"] = false;
        stateMsg["currentPlayer"] = room->currentPlayerIndex;
        stateMsg["firstPlayerIndex"] = room->firstPlayerIndex;
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

        // RECONNEXION: Envoyer les informations sur les annonces
        if (room->lastBidderIndex >= 0) {
            stateMsg["lastBidderIndex"] = room->lastBidderIndex;
            stateMsg["lastBidAnnonce"] = static_cast<int>(room->lastBidAnnonce);
            stateMsg["lastBidSuit"] = static_cast<int>(room->lastBidSuit);
            stateMsg["isCoinched"] = room->coinched;
            stateMsg["isSurcoinched"] = room->surcoinched;
            stateMsg["coinchedByPlayerIndex"] = room->coinchePlayerIndex;
            stateMsg["surcoinchedByPlayerIndex"] = room->surcoinchePlayerIndex;
            qInfo() << "GameServer - Reconnexion (phase jeu): Envoi des infos d'annonce - lastBidder:" << room->lastBidderIndex
                        << "annonce:" << static_cast<int>(room->lastBidAnnonce)
                        << "suit:" << static_cast<int>(room->lastBidSuit)
                        << "coinched:" << room->coinched << "by:" << room->coinchePlayerIndex;
        }

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

void GameServer::handleUpdateAvatar(QWebSocket *socket, const QJsonObject &data) {
    QString newAvatar = data["avatar"].toString();
    if (newAvatar.isEmpty()) {
        qDebug() << "Avatar vide reçu, ignoré";
        return;
    }

    // Trouver la connexion du joueur en utilisant la fonction sécurisée
    QString connectionId = getConnectionIdBySocket(socket);
    if (connectionId.isEmpty()) {
        qDebug() << "Impossible de trouver la connexion pour la mise à jour d'avatar";
        return;
    }

    PlayerConnection* conn = m_connections[connectionId];
    if (!conn) {
        qDebug() << "Connexion invalide pour la mise à jour d'avatar";
        return;
    }

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
void GameServer::handleRehumanize(QWebSocket *socket) {
    // Trouver la connexion du joueur en utilisant la fonction sécurisée
    QString connectionId = getConnectionIdBySocket(socket);
    if (connectionId.isEmpty()) {
        qDebug() << "handleRehumanize - Connexion non trouvée";
        return;
    }

    PlayerConnection* conn = m_connections[connectionId];
    if (!conn) {
        qDebug() << "handleRehumanize - Connexion invalide";
        return;
    }

    int roomId = conn->gameRoomId;
    int playerIndex = conn->playerIndex;  // Utiliser directement l'index stocké dans la connexion

    qDebug() << "handleRehumanize - connectionId:" << connectionId
                << "roomId:" << roomId << "playerIndex:" << playerIndex;

    if (roomId == -1 || !m_gameRooms.contains(roomId)) {
        qDebug() << "handleRehumanize - Joueur pas dans une room";
        return;
    }

    GameRoom* room = m_gameRooms[roomId];
    if (!room) {
        qDebug() << "handleRehumanize - Room supprimée (tous les joueurs ont quitté)";
        return;
    }

    if (playerIndex < 0 || playerIndex >= 4) {
        qDebug() << "handleRehumanize - Index joueur invalide:" << playerIndex;
        return;
    }

    qDebug() << "handleRehumanize - isBot[" << playerIndex << "] =" << room->isBot[playerIndex];

    // Réhumaniser le joueur
    if (room->isBot[playerIndex]) {
        room->isBot[playerIndex] = false;
        qInfo() << "Réhumanisation - Joueur" << conn->playerName << "(index" << playerIndex << ") reprend la main dans partie" << roomId;

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

void GameServer::handleRegisterAccount(QWebSocket *socket, const QJsonObject &data) {
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

        // Enregistrer la création de compte dans les statistiques quotidiennes
        m_dbManager->recordNewAccount();
    } else {
        // Echec
        QJsonObject response;
        response["type"] = "registerAccountFailed";
        response["error"] = errorMsg;
        sendMessage(socket, response);
        qDebug() << "Echec creation compte:" << errorMsg;
    }
}

void GameServer::handleLoginAccount(QWebSocket *socket, const QJsonObject &data) {
    QString email = data["email"].toString();
    QString password = data["password"].toString();

    qDebug() << "GameServer - Tentative connexion:" << email;

    QString pseudo;
    QString avatar;
    QString errorMsg;
    bool usingTempPassword = false;
    if (m_dbManager->authenticateUser(email, password, pseudo, avatar, errorMsg, usingTempPassword)) {
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
        response["usingTempPassword"] = usingTempPassword;
        sendMessage(socket, response);
        qDebug() << "Connexion reussie:" << pseudo << "avatar:" << avatar << "ID:" << connectionId;

        // Enregistrer la connexion dans les statistiques quotidiennes
        m_dbManager->recordLogin(pseudo);

        // Démarrer le tracking de session (lightweight - pas de timer)
        m_dbManager->recordSessionStart(pseudo);

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

void GameServer::handleDeleteAccount(QWebSocket *socket, const QJsonObject &data) {
    QString pseudo = data["pseudo"].toString();

    qDebug() << "GameServer - Demande suppression compte:" << pseudo;

    // Vérifier que le joueur est bien connecté avec ce pseudo
    QString connectionId = getConnectionIdBySocket(socket);
    if (connectionId.isEmpty()) {
        QJsonObject response;
        response["type"] = "deleteAccountFailed";
        response["error"] = "Non connecte";
        sendMessage(socket, response);
        return;
    }

    PlayerConnection* conn = m_connections[connectionId];
    if (conn->playerName != pseudo) {
        QJsonObject response;
        response["type"] = "deleteAccountFailed";
        response["error"] = "Pseudo ne correspond pas";
        sendMessage(socket, response);
        return;
    }

    // Supprimer le compte
    QString errorMsg;
    if (m_dbManager->deleteAccount(pseudo, errorMsg)) {
        // Succès
        QJsonObject response;
        response["type"] = "deleteAccountSuccess";
        sendMessage(socket, response);
        qDebug() << "Compte supprime avec succes:" << pseudo;

        // Déconnecter le joueur de toute partie en cours
        if (conn->gameRoomId != -1) {
            // Le joueur est en partie, le traiter comme un forfait
            handleForfeit(socket);
        }

        // Supprimer la connexion
        m_connections.remove(connectionId);
        delete conn;
    } else {
        // Echec
        QJsonObject response;
        response["type"] = "deleteAccountFailed";
        response["error"] = errorMsg;
        sendMessage(socket, response);
        qDebug() << "Echec suppression compte:" << errorMsg;
    }
}

void GameServer::handleForgotPassword(QWebSocket *socket, const QJsonObject &data) {
    QString email = data["email"].toString();

    qDebug() << "GameServer - Demande mot de passe oublie pour:" << email;

    QString tempPassword;
    QString errorMsg;

    if (m_dbManager->setTempPassword(email, tempPassword, errorMsg)) {
        // Success - Send email with temp password
        qDebug() << "Mot de passe temporaire genere:" << tempPassword;
        qDebug() << "SMTP password configure:" << (m_smtpPassword.isEmpty() ? "NON" : "OUI");

        SmtpClient *smtp = new SmtpClient(this);
        smtp->setHost("ssl0.ovh.net", 587);
        smtp->setCredentials("contact@nebuludik.fr", m_smtpPassword);
        smtp->setFrom("contact@nebuludik.fr", "Coinche de l'Espace");

        QString subject = "Réinitialisation de votre mot de passe";
        QString emailBody = QString(
            "Bonjour,\n\n"
            "Vous avez demandé la réinitialisation de votre mot de passe.\n\n"
            "Voici votre mot de passe temporaire : %1\n\n"
            "Ce mot de passe est valide pour une seule connexion. "
            "Vous devrez choisir un nouveau mot de passe permanent lors de votre prochaine connexion.\n\n"
            "Si vous n'avez pas demandé cette réinitialisation, ignorez ce message.\n\n"
            "Cordialement,\n"
            "L'équipe Coinche de l'Espace"
        ).arg(tempPassword);

        QObject::connect(smtp, &SmtpClient::emailSent, [socket, smtp, this](bool success, const QString &error) {
            if (success) {
                QJsonObject response;
                response["type"] = "forgotPasswordSuccess";
                sendMessage(socket, response);
                qDebug() << "Email de reinitialisation envoye avec succes";
            } else {
                QJsonObject response;
                response["type"] = "forgotPasswordFailed";
                response["error"] = "Erreur lors de l'envoi de l'email";
                sendMessage(socket, response);
                qWarning() << "Echec envoi email:" << error;
            }
            smtp->deleteLater();
        });

        smtp->sendEmail(email, subject, emailBody);
    } else {
        // Email not found
        QJsonObject response;
        response["type"] = "forgotPasswordFailed";
        response["error"] = errorMsg.isEmpty() ? "Cette adresse mail ne correspond a aucun compte" : errorMsg;
        sendMessage(socket, response);
        qDebug() << "Echec mot de passe oublie:" << errorMsg;
    }
}

void GameServer::handleChangePassword(QWebSocket *socket, const QJsonObject &data) {
    QString email = data["email"].toString();
    QString newPassword = data["newPassword"].toString();

    qDebug() << "GameServer - Demande changement mot de passe pour:" << email;

    QString errorMsg;
    if (m_dbManager->updatePassword(email, newPassword, errorMsg)) {
        // Success
        QJsonObject response;
        response["type"] = "changePasswordSuccess";
        sendMessage(socket, response);
        qDebug() << "Mot de passe change avec succes pour:" << email;
    } else {
        // Failure
        QJsonObject response;
        response["type"] = "changePasswordFailed";
        response["error"] = errorMsg;
        sendMessage(socket, response);
        qDebug() << "Echec changement mot de passe:" << errorMsg;
    }
}

void GameServer::handleChangePseudo(QWebSocket *socket, const QJsonObject &data) {
    QString currentPseudo = data["currentPseudo"].toString();
    QString newPseudo = data["newPseudo"].toString();

    qDebug() << "GameServer - Demande changement pseudo:" << currentPseudo << "->" << newPseudo;

    // Vérifier que le socket correspond bien au joueur
    QString connId;
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        if (it.value()->socket == socket && it.value()->playerName == currentPseudo) {
            connId = it.key();
            break;
        }
    }

    if (connId.isEmpty()) {
        QJsonObject response;
        response["type"] = "changePseudoFailed";
        response["error"] = "Session invalide";
        sendMessage(socket, response);
        return;
    }

    QString errorMsg;
    if (m_dbManager->updatePseudo(currentPseudo, newPseudo, errorMsg)) {
        // Mettre à jour le nom dans la connexion
        m_connections[connId]->playerName = newPseudo;

        // Mettre à jour m_playerNameToRoomId si le joueur est en partie
        if (m_playerNameToRoomId.contains(currentPseudo)) {
            int roomId = m_playerNameToRoomId.take(currentPseudo);
            m_playerNameToRoomId[newPseudo] = roomId;
        }

        QJsonObject response;
        response["type"] = "changePseudoSuccess";
        response["newPseudo"] = newPseudo;
        sendMessage(socket, response);
        qDebug() << "Pseudo changé avec succès:" << currentPseudo << "->" << newPseudo;
    } else {
        QJsonObject response;
        response["type"] = "changePseudoFailed";
        response["error"] = errorMsg;
        sendMessage(socket, response);
        qDebug() << "Echec changement pseudo:" << errorMsg;
    }
}

void GameServer::handleChangeEmail(QWebSocket *socket, const QJsonObject &data) {
    QString pseudo = data["pseudo"].toString();
    QString newEmail = data["newEmail"].toString();

    qDebug() << "GameServer - Demande changement email pour:" << pseudo;

    // Vérifier que le socket correspond bien au joueur
    bool authorized = false;
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        if (it.value()->socket == socket && it.value()->playerName == pseudo) {
            authorized = true;
            break;
        }
    }

    if (!authorized) {
        QJsonObject response;
        response["type"] = "changeEmailFailed";
        response["error"] = "Session invalide";
        sendMessage(socket, response);
        return;
    }

    QString errorMsg;
    if (m_dbManager->updateEmail(pseudo, newEmail, errorMsg)) {
        QJsonObject response;
        response["type"] = "changeEmailSuccess";
        response["newEmail"] = newEmail;
        sendMessage(socket, response);
        qDebug() << "Email changé avec succès pour:" << pseudo;
    } else {
        QJsonObject response;
        response["type"] = "changeEmailFailed";
        response["error"] = errorMsg;
        sendMessage(socket, response);
        qDebug() << "Echec changement email:" << errorMsg;
    }
}

void GameServer::handleSendContactMessage(QWebSocket *socket, const QJsonObject &data) {
    QString senderName = data["senderName"].toString();
    QString senderEmail = data["senderEmail"].toString();
    QString subject = data["subject"].toString();
    QString message = data["message"].toString();

    qDebug() << "GameServer - Message de contact de:" << senderName << "(" << senderEmail << ") Sujet:" << subject;

    // Valider les donnees
    if (subject.isEmpty() || message.isEmpty()) {
        QJsonObject response;
        response["type"] = "contactMessageFailed";
        response["error"] = "Sujet et message requis";
        sendMessage(socket, response);
        return;
    }

    // Creer le client SMTP
    SmtpClient *smtp = new SmtpClient(this);

    // Configuration SMTP OVH
    smtp->setHost("ssl0.ovh.net", 587);
    smtp->setCredentials("contact@nebuludik.fr", m_smtpPassword);
    smtp->setFrom("contact@nebuludik.fr", "Coinche Game");

    // Construire le corps du message
    QString body = QString("Nouveau message depuis l'application Coinche\n\n"
                            "De: %1\n"
                            "Email: %2\n"
                            "Sujet: %3\n\n"
                            "Message:\n%4\n\n"
                            "---\nEnvoye depuis l'application Coinche")
                            .arg(senderName.isEmpty() ? "Anonyme" : senderName)
                            .arg(senderEmail.isEmpty() ? "Non renseigne" : senderEmail)
                            .arg(subject)
                            .arg(message);

    // Connecter le signal de resultat
    connect(smtp, &SmtpClient::emailSent, this, [this, socket, smtp](bool success, const QString &error) {
        QJsonObject response;
        if (success) {
            response["type"] = "contactMessageSuccess";
            qDebug() << "Email de contact envoye avec succes";
        } else {
            response["type"] = "contactMessageFailed";
            response["error"] = error.isEmpty() ? "Erreur lors de l'envoi" : error;
            qWarning() << "Echec envoi email de contact:" << error;
        }
        sendMessage(socket, response);
        smtp->deleteLater();
    });

    // Envoyer l'email
    smtp->sendEmail("contact@nebuludik.fr", "[Coinche] " + subject, body);
}

void GameServer::handleReportCrash(QWebSocket *socket, const QJsonObject &data) {
    QString errorMsg = data["error"].toString();
    QString stackTrace = data["stackTrace"].toString();
    QString playerName = data["playerName"].toString();
    bool isCritical = data["critical"].toBool(false);  // Flag optionnel pour crash critique

    qWarning() << "CRASH REPORT reçu de:" << playerName << "- Erreur:" << errorMsg;

    // Enregistrer le crash dans les statistiques
    m_dbManager->recordCrash();

    // Log détaillé pour debug
    if (!stackTrace.isEmpty()) {
        qWarning() << "Stack trace:" << stackTrace;
    }

    // Détecter si c'est un crash critique (qui termine l'application)
    QString errorLower = errorMsg.toLower();
    bool isCriticalCrash = isCritical ||
                            errorLower.contains("fatal") ||
                            errorLower.contains("terminate") ||
                            errorLower.contains("kill") ||
                            errorLower.contains("segfault") ||
                            errorLower.contains("abort") ||
                            errorLower.contains("sigterm") ||
                            errorLower.contains("sigsegv");

    // Envoyer un email d'alerte pour les crashes critiques
    if (isCriticalCrash && !m_smtpPassword.isEmpty()) {
        qCritical() << "🚨 CRASH CRITIQUE détecté - Envoi d'email d'alerte";

        SmtpClient *smtp = new SmtpClient(this);
        smtp->setHost("ssl0.ovh.net", 587);
        smtp->setCredentials("contact@nebuludik.fr", m_smtpPassword);
        smtp->setFrom("contact@nebuludik.fr", "Coinche Server - CRASH ALERT");

        // Construire l'email avec toutes les infos
        QString emailBody = QString(
            "🚨 ALERTE CRASH CRITIQUE - Application Coinche\n\n"
            "Un crash critique a été détecté qui a probablement terminé l'application.\n\n"
            "═══════════════════════════════════════════════\n"
            "📊 INFORMATIONS\n"
            "═══════════════════════════════════════════════\n"
            "Joueur: %1\n"
            "Date/Heure: %2\n"
            "Type: Crash Critique\n\n"
            "═══════════════════════════════════════════════\n"
            "❌ MESSAGE D'ERREUR\n"
            "═══════════════════════════════════════════════\n"
            "%3\n\n"
            "═══════════════════════════════════════════════\n"
            "📋 STACK TRACE\n"
            "═══════════════════════════════════════════════\n"
            "%4\n\n"
            "═══════════════════════════════════════════════\n\n"
            "Ce rapport a été généré automatiquement par le serveur Coinche.\n"
            "Action recommandée: Analyser la stack trace et corriger le bug.\n"
        ).arg(playerName.isEmpty() ? "Inconnu" : playerName)
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
            .arg(errorMsg.isEmpty() ? "Aucun message d'erreur fourni" : errorMsg)
            .arg(stackTrace.isEmpty() ? "Aucune stack trace disponible" : stackTrace);

        // Connecter le signal de résultat
        connect(smtp, &SmtpClient::emailSent, this, [smtp](bool success, const QString &error) {
            if (success) {
                qInfo() << "✅ Email d'alerte de crash critique envoyé avec succès";
            } else {
                qWarning() << "❌ Échec de l'envoi de l'email d'alerte de crash:" << error;
            }
            smtp->deleteLater();
        });

        // Envoyer l'email
        QString subject = QString("🚨 CRASH CRITIQUE - Coinche App - %1")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));
        smtp->sendEmail("contact@nebuludik.fr", subject, emailBody);
    }

    // Répondre au client
    QJsonObject response;
    response["type"] = "crashReported";
    response["success"] = true;
    sendMessage(socket, response);
}

void GameServer::handleGetStats(QWebSocket *socket, const QJsonObject &data) {
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

void GameServer::handleJoinMatchmaking(QWebSocket *socket) {
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

        // Redémarrer le timer de matchmaking (20 secondes + 10 secondes de compte à rebours)
        // Le timer est réinitialisé à chaque nouveau joueur
        m_lastQueueSize = m_matchmakingQueue.size();
        m_countdownTimer->stop();  // Arrêter le compte à rebours s'il était en cours
        m_matchmakingTimer->start();
        qDebug() << "Timer matchmaking démarré/redémarré - 20s + 10s countdown avant création avec bots";

        // Essaye de créer une partie si 4 joueurs
        tryCreateGame();
    }
}

void GameServer::handleLeaveMatchmaking(QWebSocket *socket) {
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

    // Si la queue est vide, arrêter les timers
    if (m_matchmakingQueue.isEmpty()) {
        m_matchmakingTimer->stop();
        m_countdownTimer->stop();
        qDebug() << "Timers matchmaking arrêtés - queue vide";
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

void GameServer::tryCreateGame() {
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

        // Enregistrer la création de GameRoom dans les statistiques quotidiennes
        m_dbManager->recordGameRoomCreated();

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

        qInfo() << "Partie créée - Room" << roomId << "- Joueurs:"
                << room->playerNames[0] << "," << room->playerNames[1] << ","
                << room->playerNames[2] << "," << room->playerNames[3];

        // Notifie tous les joueurs
        notifyGameStart(roomId, connectionIds);

        qDebug() << "Notifications gameFound envoyees à" << connectionIds.size() << "joueurs";

        // Si le premier joueur à annoncer est un bot, le faire annoncer automatiquement
        if (room->isBot[room->currentPlayerIndex]) {
            QTimer::singleShot(3000, this, [this, roomId]() {
                GameRoom* room = m_gameRooms.value(roomId);
                if (room && room->gameState == "bidding") {
                    playBotBid(roomId, room->currentPlayerIndex);
                }
            });
        } else {
            // Joueur humain : démarrer le timer de timeout pour les enchères
            // IMPORTANT: Retarder pour laisser le temps au client de
            // recevoir les cartes et d'afficher le panneau d'annonces
            QTimer::singleShot(BID_PANEL_DISPLAY_DELAY_MS, this, [this, roomId]() {
                GameRoom* room = m_gameRooms.value(roomId);
                if (room && room->gameState == "bidding") {
                    startBidTimeout(roomId, room->currentPlayerIndex);
                }
            });
        }

        // Arrêter les timers de matchmaking car la partie a commencé
        m_matchmakingTimer->stop();
        m_countdownTimer->stop();
    }
}

void GameServer::handlePlayCard(QWebSocket *socket, const QJsonObject &data) {
    QString connectionId = getConnectionIdBySocket(socket);
    if (connectionId.isEmpty()) {
        qWarning() << "handlePlayCard - ERREUR: connectionId vide pour socket" << socket;
        return;
    }

    PlayerConnection* conn = m_connections[connectionId];
    if (!conn) {
        qWarning() << "handlePlayCard - ERREUR: connexion non trouvée pour" << connectionId;
        return;
    }
    int roomId = conn->gameRoomId;
    if (roomId == -1) {
        qWarning() << "handlePlayCard - ERREUR: joueur" << connectionId << "n'est pas dans une room";
        return;
    }

    GameRoom* room = m_gameRooms[roomId];
    if (!room) {
        qWarning() << "handlePlayCard - ERREUR: room" << roomId << "n'existe pas";
        return;
    }

    qDebug() << "handlePlayCard - Réception: joueur" << conn->playerIndex << "veut jouer carte" << data["cardIndex"].toInt()
                << "currentPlayer:" << room->currentPlayerIndex << "gameState:" << room->gameState;

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
        qWarning() << "Validation échouée - Tentative de jouer carte pendant enchères (joueur index" << playerIndex << "room" << roomId << ")";
        return;
    }

    // Check qu'on n'est pas en attente entre deux plis
    if (room->waitingForNextPli) {
        qDebug() << "GameServer - Erreur: tentative de jouer pendant l'attente entre les plis (joueur" << playerIndex << ")";
        return;
    }

    // Check que c'est bien le tour du joueur
    if (room->currentPlayerIndex != playerIndex) {
        qWarning() << "Validation échouée - Pas le tour du joueur" << playerIndex << "(tour actuel:" << room->currentPlayerIndex << "room" << roomId << ")";
        return;
    }

    Player* player = room->players[playerIndex].get();
    if (!player || cardIndex < 0 || cardIndex >= player->getMain().size()) {
        qWarning() << "Validation échouée - Index carte invalide" << cardIndex << "(joueur index" << playerIndex << "room" << roomId << ")";
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
        qWarning() << "Validation échouée - Carte non jouable selon règles (joueur index" << playerIndex << "carte" << cardIndex << "room" << roomId << ")";

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

void GameServer::handleMakeBid(QWebSocket *socket, const QJsonObject &data) {
    QString connectionId = getConnectionIdBySocket(socket);
    if (connectionId.isEmpty()) {
        qWarning() << "handleMakeBid - ERREUR: connectionId vide pour socket" << socket;
        return;
    }

    PlayerConnection* conn = m_connections[connectionId];
    if (!conn) {
        qWarning() << "handleMakeBid - ERREUR: connexion non trouvée pour" << connectionId;
        return;
    }
    int roomId = conn->gameRoomId;
    if (roomId == -1) {
        qWarning() << "handleMakeBid - ERREUR: joueur" << connectionId << "n'est pas dans une room";
        return;
    }

    GameRoom* room = m_gameRooms[roomId];
    if (!room) {
        qWarning() << "handleMakeBid - ERREUR: room" << roomId << "n'existe pas";
        return;
    }

    qDebug() << "handleMakeBid - Réception: joueur" << conn->playerIndex << "veut annoncer" << data["bidValue"].toInt()
                << "couleur:" << data["suit"].toInt() << "currentPlayer:" << room->currentPlayerIndex;

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
        room->lastBidSuit = suit;  // Stocker la couleur originale de l'enchère (3-8)

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

void GameServer::handleForfeit(QWebSocket *socket) {
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

    // Notifier tous les joueurs qu'un joueur a abandonné et a été remplacé par un bot
    // IMPORTANT: Envoyer ce message AVANT de retirer le joueur des listes
    QJsonObject forfeitMsg;
    forfeitMsg["type"] = "playerForfeited";
    forfeitMsg["playerIndex"] = playerIndex;
    forfeitMsg["playerName"] = conn->playerName;
    broadcastToRoom(roomId, forfeitMsg);

    // Retirer le joueur de la partie - il ne pourra plus la rejoindre
    // Ceci s'applique UNIQUEMENT aux abandons volontaires (clic sur "Quitter")
    // Les déconnexions involontaires passent par onDisconnected() qui garde le joueur dans la partie
    conn->gameRoomId = -1;
    conn->playerIndex = -1;

    // IMPORTANT: Retirer le connectionId de la room pour qu'il ne reçoive plus les messages de broadcast
    room->connectionIds[playerIndex] = "";

    // IMPORTANT: Retirer le joueur de la map de reconnexion pour qu'il ne puisse pas
    // rejoindre cette partie, même s'il clique sur "Jouer" ensuite
    m_playerNameToRoomId.remove(conn->playerName);

    qInfo() << "Forfait - Joueur" << conn->playerName << "abandonne la partie" << roomId;

    // Vérifier si tous les joueurs sont maintenant des bots
    bool allBots = true;
    for (int i = 0; i < 4; i++) {
        if (!room->isBot[i]) {
            allBots = false;
            break;
        }
    }

    if (allBots) {
        // Vérifier si certains joueurs peuvent encore se reconnecter (AFK) ou si tous ont vraiment quitté
        bool anyPlayerCanReconnect = false;
        for (int i = 0; i < 4; i++) {
            QString playerName = room->playerNames[i];
            // Si le joueur est encore dans la map de reconnexion, il peut revenir (AFK)
            if (m_playerNameToRoomId.contains(playerName) && m_playerNameToRoomId[playerName] == roomId) {
                anyPlayerCanReconnect = true;
                qDebug() << "Joueur" << playerName << "peut encore se reconnecter (AFK, pas abandon volontaire)";
                break;
            }
        }

        if (anyPlayerCanReconnect) {
            qDebug() << "Tous les joueurs sont des bots, mais certains peuvent se reconnecter - GameRoom conservée";
            // Ne pas supprimer la room, les joueurs AFK peuvent revenir
        } else {
            // Tous les joueurs humains ont quitté volontairement - supprimer la GameRoom
            qDebug() << "Tous les joueurs ont quitté volontairement la partie" << roomId << "- Suppression de la GameRoom";

            // Retirer la room des maps
            m_gameRooms.remove(roomId);

            // Supprimer la room (le destructeur nettoie les timers automatiquement)
            delete room;

            qDebug() << "GameRoom" << roomId << "supprimée";
            return;
        }
    }

    // Si c'est le tour du joueur qui abandonne, faire jouer le bot immédiatement
    if (room->currentPlayerIndex == playerIndex) {
        if (room->gameState == "bidding") {
            // Phase d'enchères : passer automatiquement
            QTimer::singleShot(3000, this, [this, roomId, playerIndex]() {
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

void GameServer::handlePlayerDisconnect(const QString &connectionId) {
    PlayerConnection *conn = m_connections.value(connectionId);
    if (!conn) return;

    int roomId = conn->gameRoomId;
    if (roomId == -1) return;

    GameRoom* room = m_gameRooms.value(roomId);
    if (!room) return;

    int playerIndex = conn->playerIndex;
    qInfo() << "Joueur" << conn->playerName << "(index" << playerIndex << ") déconnecté de la partie" << roomId;

    // IMPORTANT: Retirer le connectionId de room->connectionIds pour éviter
    // que broadcastToRoom tente d'envoyer à une connexion invalide
    if (playerIndex >= 0 && playerIndex < room->connectionIds.size()) {
        room->connectionIds[playerIndex] = QString();  // Vider le connectionId
        qDebug() << "ConnectionId retiré de room->connectionIds[" << playerIndex << "] pour éviter envoi à socket invalide";
    }

    // Incrémenter le compteur de parties jouées (défaite) pour ce joueur
    if (!conn->playerName.isEmpty()) {
        m_dbManager->updateGameStats(conn->playerName, false);  // false = défaite
        qDebug() << "Stats mises a jour pour" << conn->playerName << "- Defaite enregistree (deconnexion)";

        // Enregistrer l'abandon dans les statistiques quotidiennes
        m_dbManager->recordPlayerQuit();
    }
}

void GameServer::handleCreatePrivateLobby(QWebSocket *socket) {
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

void GameServer::handleJoinPrivateLobby(QWebSocket *socket, const QJsonObject &obj) {
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

void GameServer::handleLobbyReady(QWebSocket *socket, const QJsonObject &obj) {
    QString connectionId = getConnectionIdBySocket(socket);
    if (connectionId.isEmpty()) return;

    PlayerConnection* conn = m_connections[connectionId];
    if (!conn) return;

    bool ready = obj["ready"].toBool();

    // Trouver le lobby du joueur
    QString lobbyCode;
    for (auto it = m_privateLobbies.begin(); it != m_privateLobbies.end(); ++it) {
        if (it.value() && it.value()->playerNames.contains(conn->playerName)) {
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

void GameServer::handleStartLobbyGame(QWebSocket *socket) {
    QString connectionId = getConnectionIdBySocket(socket);
    if (connectionId.isEmpty()) return;

    PlayerConnection* conn = m_connections[connectionId];
    if (!conn) return;

    // Trouver le lobby du joueur
    QString lobbyCode;
    for (auto it = m_privateLobbies.begin(); it != m_privateLobbies.end(); ++it) {
        if (it.value() && it.value()->hostPlayerName == conn->playerName) {
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

void GameServer::handleLeaveLobby(QWebSocket *socket) {
    QString connectionId = getConnectionIdBySocket(socket);
    if (connectionId.isEmpty()) return;

    PlayerConnection* conn = m_connections[connectionId];
    if (!conn) return;

    // Trouver et quitter le lobby
    QString lobbyCode;
    for (auto it = m_privateLobbies.begin(); it != m_privateLobbies.end(); ++it) {
        if (it.value() && it.value()->playerNames.contains(conn->playerName)) {
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

void GameServer::createGameWithBots() {
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

    // Enregistrer la création de GameRoom dans les statistiques quotidiennes
    m_dbManager->recordGameRoomCreated();

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
        QTimer::singleShot(3000, this, [this, roomId]() {
            GameRoom* room = m_gameRooms.value(roomId);
            if (room && room->gameState == "bidding") {
                playBotBid(roomId, room->currentPlayerIndex);
            }
        });
    } else {
        // Joueur humain : démarrer le timer de timeout pour les enchères
        // IMPORTANT: Retarder pour laisser le temps au client de
        // recevoir les cartes et d'afficher le panneau d'annonces
        QTimer::singleShot(BID_PANEL_DISPLAY_DELAY_MS, this, [this, roomId]() {
            GameRoom* room = m_gameRooms.value(roomId);
            if (room && room->gameState == "bidding") {
                startBidTimeout(roomId, room->currentPlayerIndex);
            }
        });
    }
}

void GameServer::notifyGameStart(int roomId, const QList<QString> &connectionIds) {
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

void GameServer::finishPli(int roomId) {
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


void GameServer::finishManche(int roomId) {
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

    // Utiliser ScoreCalculator pour calculer les scores
    auto scoreResult = ScoreCalculator::calculateMancheScore(
        pointsRealisesTeam1,
        pointsRealisesTeam2,
        valeurContrat,
        team1HasBid,
        room->coinched,
        room->surcoinched,
        isCapotAnnonce,
        capotReussi,
        isGeneraleAnnonce,
        generaleReussie,
        capotNonAnnonceTeam1,
        capotNonAnnonceTeam2
    );

    int scoreToAddTeam1 = scoreResult.scoreTeam1;
    int scoreToAddTeam2 = scoreResult.scoreTeam2;

    // Log des scores calculés
    qDebug() << "GameServer - Scores calcules:";
    qDebug() << "  Team1 marque:" << scoreToAddTeam1;
    qDebug() << "  Team2 marque:" << scoreToAddTeam2;

    // Gestion des stats pour coinche/surcoinche (code existant maintenu)
    int multiplicateur = 1;
    if (room->surcoinched) {
        multiplicateur = 4;
    } else if (room->coinched) {
        multiplicateur = 2;
    }

    // Gestion des stats pour coinche/surcoinche
    if (room->coinched || room->surcoinched) {
        if (team1HasBid) {
            // Team1 a annoncé, vérifie si elle réussit
            bool contractReussi = (pointsRealisesTeam1 >= valeurContrat);

            if (contractReussi) {
                // Mettre à jour les stats de surcoinche réussie (le contrat a réussi)
                if (room->surcoinched && room->surcoinchePlayerIndex != -1 && !room->isBot[room->surcoinchePlayerIndex]) {
                    QString connId = room->connectionIds[room->surcoinchePlayerIndex];
                    PlayerConnection* surcoincheConn = connId.isEmpty() ? nullptr : m_connections.value(connId);
                    if (surcoincheConn && !surcoincheConn->playerName.isEmpty()) {
                        m_dbManager->updateSurcoincheStats(surcoincheConn->playerName, false, true);
                        qDebug() << "Stats surcoinche réussie pour:" << surcoincheConn->playerName;
                    }
                }
            } else {
                // Mettre à jour les stats de coinche réussie (le contrat a échoué, donc la coinche a réussi)
                if (room->coinched && room->coinchePlayerIndex != -1 && !room->isBot[room->coinchePlayerIndex]) {
                    QString connId = room->connectionIds[room->coinchePlayerIndex];
                    PlayerConnection* coincheConn = connId.isEmpty() ? nullptr : m_connections.value(connId);
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
                QString connId = room->connectionIds[i];
                if (connId.isEmpty()) continue;  // Skip déconnectés
                PlayerConnection* conn = m_connections.value(connId);
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
                // Mettre à jour les stats de surcoinche réussie (le contrat a réussi)
                if (room->surcoinched && room->surcoinchePlayerIndex != -1 && !room->isBot[room->surcoinchePlayerIndex]) {
                    QString connId = room->connectionIds[room->surcoinchePlayerIndex];
                    PlayerConnection* surcoincheConn = connId.isEmpty() ? nullptr : m_connections.value(connId);
                    if (surcoincheConn && !surcoincheConn->playerName.isEmpty()) {
                        m_dbManager->updateSurcoincheStats(surcoincheConn->playerName, false, true);
                        qDebug() << "Stats surcoinche réussie pour:" << surcoincheConn->playerName;
                    }
                }
            } else {
                // Mettre à jour les stats de coinche réussie (le contrat a échoué, donc la coinche a réussi)
                if (room->coinched && room->coinchePlayerIndex != -1 && !room->isBot[room->coinchePlayerIndex]) {
                    QString connId = room->connectionIds[room->coinchePlayerIndex];
                    PlayerConnection* coincheConn = connId.isEmpty() ? nullptr : m_connections.value(connId);
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
                QString connId = room->connectionIds[i];
                if (connId.isEmpty()) continue;  // Skip déconnectés
                PlayerConnection* conn = m_connections.value(connId);
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
            QString connId = room->connectionIds[i];
            if (connId.isEmpty()) continue;  // Skip déconnectés
            PlayerConnection* conn = m_connections.value(connId);
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
            QString connId = room->connectionIds[i];
            if (connId.isEmpty()) continue;  // Skip déconnectés
            PlayerConnection* conn = m_connections.value(connId);
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
            QString connId = room->connectionIds[room->lastBidderIndex];
            PlayerConnection* conn = connId.isEmpty() ? nullptr : m_connections.value(connId);
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
            qInfo() << "Partie terminée - Room" << roomId << "- Les deux équipes > 1000";
            qInfo() << "  Équipe" << winner << "gagne avec"
                    << ((winner == 1) ? room->scoreTeam1 : room->scoreTeam2) << "points";
        } else if (team1Won) {
            winner = 1;
            qInfo() << "Partie terminée - Room" << roomId << "- Équipe 1 gagne -" << room->scoreTeam1 << "vs" << room->scoreTeam2;
        } else {
            winner = 2;
            qInfo() << "Partie terminée - Room" << roomId << "- Équipe 2 gagne -" << room->scoreTeam1 << "vs" << room->scoreTeam2;
        }

        // Mettre à jour les statistiques pour tous les joueurs enregistrés
        for (int i = 0; i < room->connectionIds.size(); i++) {
            QString connId = room->connectionIds[i];
            if (connId.isEmpty()) continue;  // Skip déconnectés
            PlayerConnection* conn = m_connections.value(connId);
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

void GameServer::doStartNewManche(int roomId) {
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
    room->lastBidSuit = 0;
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
        QTimer::singleShot(3000, this, [this, roomId, firstBidder]() {
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
        // IMPORTANT: Retarder au début d'une nouvelle manche pour laisser
        // le temps au client de voir l'animation, recevoir les cartes et afficher le panneau
        QTimer::singleShot(BID_PANEL_DISPLAY_DELAY_MS, this, [this, roomId]() {
            GameRoom* room = m_gameRooms.value(roomId);
            if (room && room->gameState == "bidding") {
                startBidTimeout(roomId, room->currentPlayerIndex);
            }
        });
    }
}

void GameServer::startPlayingPhase(int roomId) {
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

void GameServer::notifyPlayersWithPlayableCards(int roomId) {
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

        // Si c'est le début d'un nouveau pli (pli vide), attendre plus longtemps (1100 ms)
        // pour laisser le temps au pli précédent d'être nettoyé côté client
        int delay = room->currentPli.empty() ? 2000 : 1100;

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
            qInfo() << "Bot replacement (timeout jeu) - Joueur index" << currentPlayer << "dans room" << roomId;

            // Marquer le joueur comme bot
            room->isBot[currentPlayer] = true;

            // Notifier le client qu'il a été remplacé par un bot
            QString connectionId = room->connectionIds[currentPlayer];
            if (!connectionId.isEmpty() && m_connections.contains(connectionId)) {
                PlayerConnection* conn = m_connections.value(connectionId);
                if (conn && conn->socket && conn->socket->state() == QAbstractSocket::ConnectedState) {
                    QJsonObject notification;
                    notification["type"] = "botReplacement";
                    notification["message"] = "Un bot a pris le relai car vous n'avez pas joué à temps.";
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
        int delay = room->currentPli.empty() ? 2000 : 800;

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

QJsonArray GameServer::calculatePlayableCards(GameRoom* room, int playerIndex) {
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










