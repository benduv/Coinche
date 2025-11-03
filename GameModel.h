#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QList>
#include <QDebug>
#include "Player.h"
#include "Carte.h"
#include "Deck.h"
#include "HandModel.h"

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
    Q_PROPERTY(int scoreTotalTeam1 READ scoreTotalTeam1 NOTIFY scoreTotalTeam1Changed)
    Q_PROPERTY(int scoreTotalTeam2 READ scoreTotalTeam2 NOTIFY scoreTotalTeam2Changed)

public:
    explicit GameModel(const std::vector<std::reference_wrapper<std::unique_ptr<Player>>>& players, Deck &deck, QObject *parent = nullptr);
    ~GameModel();

    HandModel* player0Hand() const;
    HandModel* player1Hand() const;
    HandModel* player2Hand() const;
    HandModel* player3Hand() const;

    int currentPlayer() const;
    QList<QVariant> currentPli() const;
    QString currentPlayerName() const;
    bool biddingPhase() const;
    int biddingPlayer() const;
    int lastBidValue() const;
    int scoreTeam1() const;
    int scoreTeam2() const;
    int scoreTotalTeam1() const;
    int scoreTotalTeam2() const;
    QString lastBid() const;
    QString lastBidSuit() const;

    Q_INVOKABLE void makeBid(int bidValue, int suitValue);
    Q_INVOKABLE void passBid();
    Q_INVOKABLE void playCard(int playerIndex, int cardIndex);
    Q_INVOKABLE void refreshAllHands();
    Q_INVOKABLE void resetPli();
    Q_INVOKABLE void revealPlayerCards(int playerIndex, bool reveal);

signals:
    void currentPlayerChanged();
    void currentPliChanged();
    void biddingPhaseChanged();
    void biddingPlayerChanged();
    void lastBidChanged();
    void scoreTeam1Changed();
    void scoreTeam2Changed();
    void scoreTotalTeam1Changed();
    void scoreTotalTeam2Changed();

private:
    void refreshHand(int playerIndex);
    void updatePlayableStates();
    void processAIBids();
    void endBiddingPhase();
    void endManche();

    const std::vector<std::reference_wrapper<std::unique_ptr<Player>>>& m_players;
    Deck &m_deck;
    HandModel* m_player0Hand;
    HandModel* m_player1Hand;
    HandModel* m_player2Hand;
    HandModel* m_player3Hand;
    int m_currentPlayer;
    QList<CarteDuPli> m_currentPli;

    Carte::Couleur m_couleurDemandee;
    Carte::Couleur m_couleurAtout;
    Carte* m_carteAtout;
    int m_idxPlayerStartManche;
    int m_idxPlayerWinning;
    int m_idxPlayerStartPli;
    Carte* m_carteWinning;
    int m_scorePli;
    int m_scoreTeam1;
    int m_scoreTeam2;
    int m_scoreTotalTeam1;
    int m_scoreTotalTeam2;

    bool m_biddingPhase;
    int m_biddingPlayer;
    int m_passCount;
    int m_lastBidPlayer;
    Player::Annonce m_lastBidAnnonce;
    Carte::Couleur m_lastBidCouleur;
};

#endif // GAMEMODEL_H
