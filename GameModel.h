#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include "HandModel.h"
#include "Player.h"
#include "Carte.h"

struct CarteDuPli {
    int playerId;
    Carte* carte;
};

class GameModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(HandModel* player0Hand READ player0Hand CONSTANT)
    Q_PROPERTY(HandModel* player1Hand READ player1Hand CONSTANT)
    Q_PROPERTY(HandModel* player2Hand READ player2Hand CONSTANT)
    Q_PROPERTY(HandModel* player3Hand READ player3Hand CONSTANT)
    Q_PROPERTY(int myPosition READ myPosition NOTIFY myPositionChanged)
    Q_PROPERTY(int currentPlayer READ currentPlayer NOTIFY currentPlayerChanged)
    Q_PROPERTY(QString currentPlayerName READ currentPlayerName NOTIFY currentPlayerChanged)
    Q_PROPERTY(QList<QVariant> currentPli READ currentPli NOTIFY currentPliChanged)
    Q_PROPERTY(bool biddingPhase READ biddingPhase NOTIFY biddingPhaseChanged)
    Q_PROPERTY(int biddingPlayer READ biddingPlayer NOTIFY biddingPlayerChanged)
    Q_PROPERTY(QString lastBid READ lastBid NOTIFY lastBidChanged)
    Q_PROPERTY(int lastBidValue READ lastBidValue NOTIFY lastBidChanged)
    Q_PROPERTY(QString lastBidSuit READ lastBidSuit NOTIFY lastBidChanged)
    Q_PROPERTY(int scoreTeam1 READ scoreTeam1 NOTIFY scoreTeam1Changed)
    Q_PROPERTY(int scoreTeam2 READ scoreTeam2 NOTIFY scoreTeam2Changed)

public:
    explicit GameModel(QObject *parent = nullptr);
    ~GameModel();

    HandModel* player0Hand() const;
    HandModel* player1Hand() const;
    HandModel* player2Hand() const;
    HandModel* player3Hand() const;
    
    int myPosition() const;
    int currentPlayer() const;
    QString currentPlayerName() const;
    QList<QVariant> currentPli() const;
    bool biddingPhase() const;
    int biddingPlayer() const;
    int lastBidValue() const;
    int scoreTeam1() const;
    int scoreTeam2() const;
    QString lastBid() const;
    QString lastBidSuit() const;
    
    // Initialiser la partie avec les données du serveur
    Q_INVOKABLE void initOnlineGame(int myPosition, const QJsonArray& myCards, const QJsonArray& opponents);
    
    // Actions du joueur local
    Q_INVOKABLE void playCard(int cardIndex);
    Q_INVOKABLE void makeBid(int bidValue, int suitValue);
    Q_INVOKABLE void passBid();
    
    // Recevoir les mises à jour du serveur
    Q_INVOKABLE void updateGameState(const QJsonObject& state);
    Q_INVOKABLE void receivePlayerAction(int playerIndex, const QString& action, const QVariant& data);
    Q_INVOKABLE void refreshAllHands();

signals:
    void myPositionChanged();
    void currentPlayerChanged();
    void currentPliChanged();
    void biddingPhaseChanged();
    void biddingPlayerChanged();
    void lastBidChanged();
    void scoreTeam1Changed();
    void scoreTeam2Changed();
    void gameInitialized();
    
    // Signaux vers NetworkManager
    void cardPlayedLocally(int cardIndex);
    void bidMadeLocally(int bidValue, int suitValue);

private:
    HandModel* getHandModelByPosition(int position);
    Player* getPlayerByPosition(int position);
    void refreshHand(int playerIndex);

    HandModel* m_player0Hand;
    HandModel* m_player1Hand;
    HandModel* m_player2Hand;
    HandModel* m_player3Hand;
    
    int m_myPosition;
    int m_currentPlayer;
    QString m_currentPlayerName;
    QList<CarteDuPli> m_currentPli;
    bool m_biddingPhase;
    int m_biddingPlayer;
    int m_scoreTeam1;
    int m_scoreTeam2;
    Player::Annonce m_lastBidAnnonce;
    Carte::Couleur m_lastBidCouleur;
    
    QList<Player*> m_onlinePlayers;  // Tous les joueurs de la partie
};

#endif // GAMEMODEL_H