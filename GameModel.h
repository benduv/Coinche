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

// Structure pour sauvegarder les cartes du dernier pli (avec copie des valeurs)
struct CarteDuPliSauvegardee {
    int playerId;
    Carte::Chiffre chiffre;
    Carte::Couleur couleur;
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
    Q_PROPERTY(int lastBidderIndex READ lastBidderIndex NOTIFY lastBidderIndexChanged)
    Q_PROPERTY(int playerIndex READ playerIndex NOTIFY myPositionChanged)
    Q_PROPERTY(int scoreTeam1 READ scoreTeam1 NOTIFY scoreTeam1Changed)
    Q_PROPERTY(int scoreTeam2 READ scoreTeam2 NOTIFY scoreTeam2Changed)
    Q_PROPERTY(int scoreTotalTeam1 READ scoreTotalTeam1 NOTIFY scoreTotalTeam1Changed)
    Q_PROPERTY(int scoreTotalTeam2 READ scoreTotalTeam2 NOTIFY scoreTotalTeam2Changed)
    Q_PROPERTY(bool surcoincheAvailable READ surcoincheAvailable NOTIFY surcoincheAvailableChanged)
    Q_PROPERTY(int surcoincheTimeLeft READ surcoincheTimeLeft NOTIFY surcoincheTimeLeftChanged)
    Q_PROPERTY(bool showCoincheAnimation READ showCoincheAnimation NOTIFY showCoincheAnimationChanged)
    Q_PROPERTY(bool showSurcoincheAnimation READ showSurcoincheAnimation NOTIFY showSurcoincheAnimationChanged)
    Q_PROPERTY(bool showBeloteAnimation READ showBeloteAnimation NOTIFY showBeloteAnimationChanged)
    Q_PROPERTY(bool showRebeloteAnimation READ showRebeloteAnimation NOTIFY showRebeloteAnimationChanged)
    Q_PROPERTY(QList<QVariant> lastPliCards READ lastPliCards NOTIFY lastPliCardsChanged)
    Q_PROPERTY(int distributionPhase READ distributionPhase NOTIFY distributionPhaseChanged)
    Q_PROPERTY(QList<QVariant> playerBids READ playerBids NOTIFY playerBidsChanged)

public:
    explicit GameModel(QObject *parent = nullptr);
    ~GameModel();

    HandModel* player0Hand() const;
    HandModel* player1Hand() const;
    HandModel* player2Hand() const;
    HandModel* player3Hand() const;
    
    int myPosition() const;
    int playerIndex() const;
    int currentPlayer() const;
    QString currentPlayerName() const;
    QList<QVariant> currentPli() const;
    bool biddingPhase() const;
    int biddingPlayer() const;
    int lastBidValue() const;
    int lastBidderIndex() const;
    int scoreTeam1() const;
    int scoreTeam2() const;
    int scoreTotalTeam1() const;
    int scoreTotalTeam2() const;
    QString lastBid() const;
    QString lastBidSuit() const;
    bool surcoincheAvailable() const;
    int surcoincheTimeLeft() const;
    bool showCoincheAnimation() const;
    bool showSurcoincheAnimation() const;
    bool showBeloteAnimation() const;
    bool showRebeloteAnimation() const;
    QList<QVariant> lastPliCards() const;
    int distributionPhase() const;
    QList<QVariant> playerBids() const;

    // Initialiser la partie avec les données du serveur
    Q_INVOKABLE void initOnlineGame(int myPosition, const QJsonArray& myCards, const QJsonArray& opponents);
    
    // Actions du joueur local
    Q_INVOKABLE void playCard(int cardIndex);
    Q_INVOKABLE void makeBid(int bidValue, int suitValue);
    Q_INVOKABLE void passBid();
    Q_INVOKABLE void coincheBid();
    Q_INVOKABLE void surcoincheBid();
    
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
    void lastBidderIndexChanged();
    void scoreTeam1Changed();
    void scoreTeam2Changed();
    void scoreTotalTeam1Changed();
    void scoreTotalTeam2Changed();
    void surcoincheAvailableChanged();
    void surcoincheTimeLeftChanged();
    void showCoincheAnimationChanged();
    void showSurcoincheAnimationChanged();
    void showBeloteAnimationChanged();
    void showRebeloteAnimationChanged();
    void lastPliCardsChanged();
    void distributionPhaseChanged();
    void playerBidsChanged();
    void gameInitialized();

    // Signaux vers NetworkManager
    void cardPlayedLocally(int cardIndex);
    void bidMadeLocally(int bidValue, int suitValue);

private:
    HandModel* getHandModelByPosition(int position);
    Player* getPlayerByPosition(int position);
    void refreshHand(int playerIndex);
    void distributeCards(int startIdx, int endIdx, const std::vector<Carte*>& myCards);

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
    int m_scoreTeam1;          // Score de la manche en cours
    int m_scoreTeam2;          // Score de la manche en cours
    int m_scoreTotalTeam1;     // Score total de la partie
    int m_scoreTotalTeam2;     // Score total de la partie
    Player::Annonce m_lastBidAnnonce;
    Carte::Couleur m_lastBidCouleur;
    int m_lastBidderIndex;
    bool m_surcoincheAvailable;
    int m_surcoincheTimeLeft;
    bool m_showCoincheAnimation;
    bool m_showSurcoincheAnimation;
    bool m_showBeloteAnimation;
    bool m_showRebeloteAnimation;
    QList<CarteDuPliSauvegardee> m_lastPliCards;  // Cartes du dernier pli terminé (avec copie des valeurs)
    int m_distributionPhase;  // 0=pas de distribution, 1=3 cartes, 2=2 cartes, 3=3 cartes
    QList<QVariantMap> m_playerBids;  // Annonces de chaque joueur {value, suit, text}

    QList<Player*> m_onlinePlayers;  // Tous les joueurs de la partie
};

#endif // GAMEMODEL_H