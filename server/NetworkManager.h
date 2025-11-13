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

public:
    explicit NetworkManager(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QWebSocket)
        , m_connected(false)
        , m_playersInQueue(0)
        , m_myPosition(-1)
        , m_gameModel(nullptr)
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

    // ⭐ Nouvelle méthode pour créer le GameModel
    Q_INVOKABLE void createGameModel(int position, const QJsonArray& cards, const QJsonArray& opps) {
        qDebug() << "Création du GameModel en C++";
        
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
        
        // Initialiser avec les données
        m_gameModel->initOnlineGame(position, cards, opps);
        
        qDebug() << "GameModel créé et initialisé";
        
        // Émettre signal pour que QML navigue vers CoincheView
        emit gameModelReady();
    }

    Q_INVOKABLE void connectToServer(const QString &url) {
        qDebug() << "Connexion au serveur:" << url;
        m_socket->open(QUrl(url));
    }

    Q_INVOKABLE void registerPlayer(const QString &playerName) {
        QJsonObject msg;
        msg["type"] = "register";
        msg["playerName"] = playerName;
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

private slots:
    void onConnected() {
        qDebug() << "Connecté au serveur";
        m_connected = true;
        emit connectedChanged();
    }

    void onDisconnected() {
        qDebug() << "Déconnecté du serveur";
        m_connected = false;
        emit connectedChanged();
    }

    void onMessageReceived(const QString &message) {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();

        qDebug() << "NetWorkManager - Message recu:" << type;

        if (type == "registered") {
            m_playerId = obj["connectionId"].toString();
            qDebug() << "Enregistré avec ID:" << m_playerId;
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
            
            qDebug() << "Signal gameFound émis";
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
                qDebug() << "NetworkManager - État du jeu mis à jour";
            }
        }
        else if (type == "pliFinished") {
            int winnerId = obj["winnerId"].toInt();
            qDebug() << "NetworkManager - Pli terminé, gagnant:" << winnerId;

            // Transmettre au GameModel pour nettoyer le pli
            if (m_gameModel) {
                m_gameModel->receivePlayerAction(winnerId, "pliFinished", winnerId);
            }
        }
        else if (type == "error") {
            QString errorMsg = obj["message"].toString();
            qDebug() << "NetworkManager - Erreur reçue:" << errorMsg;
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
            qDebug() << "Erreur: non connecté au serveur";
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
    
    GameModel* m_gameModel;  // ⭐ Le GameModel géré par NetworkManager
};

#endif // NETWORKMANAGER_H