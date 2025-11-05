#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class NetworkManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString matchmakingStatus READ matchmakingStatus NOTIFY matchmakingStatusChanged)
    Q_PROPERTY(int playersInQueue READ playersInQueue NOTIFY playersInQueueChanged)

public:
    explicit NetworkManager(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QWebSocket)
        , m_connected(false)
        , m_playersInQueue(0)
    {
        connect(m_socket, &QWebSocket::connected, this, &NetworkManager::onConnected);
        connect(m_socket, &QWebSocket::disconnected, this, &NetworkManager::onDisconnected);
        connect(m_socket, &QWebSocket::textMessageReceived, 
                this, &NetworkManager::onMessageReceived);
    }

    ~NetworkManager() {
        m_socket->close();
        delete m_socket;
    }

    bool connected() const { return m_connected; }
    QString matchmakingStatus() const { return m_matchmakingStatus; }
    int playersInQueue() const { return m_playersInQueue; }

    // Méthodes appelables depuis QML
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
    void gameFound(int playerPosition, QJsonArray opponents);
    void cardPlayed(QString playerId, int cardIndex);
    void bidMade(QString playerId, int bidValue, int suit);
    void playerDisconnected(QString playerId);
    void errorOccurred(QString error);

private slots:
    void onConnected() {
        qDebug() << "Connecte au serveur";
        m_connected = true;
        emit connectedChanged();
    }

    void onDisconnected() {
        qDebug() << "Deconnecté du serveur";
        m_connected = false;
        emit connectedChanged();
    }

    void onMessageReceived(const QString &message) {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();

        qDebug() << "NetWorkManager - Message recu:" << type;

        if (type == "registered") {
            m_playerId = obj["playerId"].toString();
            qDebug() << "Enregistre avec ID:" << m_playerId;
        }
        else if (type == "matchmakingStatus") {
            m_matchmakingStatus = obj["status"].toString();
            m_playersInQueue = obj["playersInQueue"].toInt();
            emit matchmakingStatusChanged();
            emit playersInQueueChanged();
        }
        else if (type == "gameFound") {
            int position = obj["playerPosition"].toInt();
            QJsonArray opponents = obj["opponents"].toArray();
            emit gameFound(position, opponents);
        }
        else if (type == "cardPlayed") {
            QString playerId = obj["playerId"].toString();
            int cardIndex = obj["cardIndex"].toInt();
            emit cardPlayed(playerId, cardIndex);
        }
        else if (type == "bidMade") {
            QString playerId = obj["playerId"].toString();
            int bidValue = obj["bidValue"].toInt();
            int suit = obj["suit"].toInt();
            emit bidMade(playerId, bidValue, suit);
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
};

#endif // NETWORKMANAGER_H