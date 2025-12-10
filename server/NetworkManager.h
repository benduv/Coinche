#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "GameModel.h"  // ⭐ Inclure GameModel

class NetworkManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString matchmakingStatus READ matchmakingStatus NOTIFY matchmakingStatusChanged)
    Q_PROPERTY(int playersInQueue READ playersInQueue NOTIFY playersInQueueChanged)
    Q_PROPERTY(QJsonArray myCards READ myCards NOTIFY gameDataChanged)
    Q_PROPERTY(int myPosition READ myPosition NOTIFY gameDataChanged)
    Q_PROPERTY(QJsonArray opponents READ opponents NOTIFY gameDataChanged)
    Q_PROPERTY(QString playerAvatar READ playerAvatar NOTIFY playerAvatarChanged)

public:
    explicit NetworkManager(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QWebSocket)
        , m_connected(false)
        , m_playersInQueue(0)
        , m_myPosition(-1)
        , m_gameModel(nullptr)
        , m_playerAvatar("avataaars1.svg")
    {
        connect(m_socket, &QWebSocket::connected, this, &NetworkManager::onConnected);
        connect(m_socket, &QWebSocket::disconnected, this, &NetworkManager::onDisconnected);
        connect(m_socket, &QWebSocket::textMessageReceived, 
                this, &NetworkManager::onMessageReceived);
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

        // Initialiser avec les données
        m_gameModel->initOnlineGame(position, cards, opps);

        // Définir l'avatar du joueur local
        m_gameModel->setPlayerAvatar(position, m_playerAvatar);

        qDebug() << "GameModel cree et initialise avec avatar:" << m_playerAvatar;

        // Émettre signal pour que QML navigue vers CoincheView
        emit gameModelReady();
    }

    Q_INVOKABLE void connectToServer(const QString &url) {
        qDebug() << "Connexion au serveur:" << url;
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

signals:
    void connectedChanged();
    void matchmakingStatusChanged();
    void playersInQueueChanged();
    void gameDataChanged();
    void gameFound(int playerPosition, QJsonArray opponents);
    void gameModelReady();  // ⭐ Nouveau signal
    void cardPlayed(QString playerId, int cardIndex);
    void bidMade(QString playerId, int bidValue, int suit);
    void playerDisconnected(QString playerId);
    void errorOccurred(QString error);
    void registerSuccess(QString playerName, QString avatar);
    void registerFailed(QString error);
    void loginSuccess(QString playerName, QString avatar);
    void loginFailed(QString error);
    void messageReceived(QString message);  // Pour que QML puisse écouter tous les messages
    void playerAvatarChanged();

private slots:
    void onConnected() {
        qDebug() << "Connecte au serveur";
        m_connected = true;
        emit connectedChanged();
    }

    void onDisconnected() {
        qDebug() << "Deconnecte du serveur";
        m_connected = false;
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
            qDebug() << "Enregistre avec ID:" << m_playerId;
        }
        else if (type == "matchmakingStatus") {
            m_matchmakingStatus = obj["status"].toString();
            m_playersInQueue = obj["playersInQueue"].toInt();
            emit matchmakingStatusChanged();
            emit playersInQueueChanged();
        }
        else if (type == "gameFound") {
            qDebug() << "=============== GAME FOUND ===============";
            
            m_myPosition = obj["playerPosition"].toInt();
            m_myCards = obj["myCards"].toArray();
            m_opponents = obj["opponents"].toArray();
            
            qDebug() << "Position:" << m_myPosition;
            qDebug() << "Nombre de cartes:" << m_myCards.size();
            qDebug() << "Nombre d'adversaires:" << m_opponents.size();
            
            emit gameDataChanged();
            emit gameFound(m_myPosition, m_opponents);
            
            qDebug() << "Signal gameFound emis";
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

            qDebug() << "NetworkManager - Manche terminee!";
            qDebug() << "  Scores totaux: Team1 =" << scoreTotalTeam1 << ", Team2 =" << scoreTotalTeam2;

            // Transmettre au GameModel
            if (m_gameModel) {
                QJsonObject scoreData;
                scoreData["scoreTotalTeam1"] = scoreTotalTeam1;
                scoreData["scoreTotalTeam2"] = scoreTotalTeam2;
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
    QString m_playerAvatar;
};

#endif // NETWORKMANAGER_H