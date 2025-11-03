#include "GameModel.h"
#include <iostream>

GameModel::GameModel(const std::vector<std::reference_wrapper<std::unique_ptr<Player>>>& players, Deck &deck, QObject *parent)
    : QObject(parent)
    , m_players(players)
    , m_deck(deck)
    , m_currentPlayer(0)
    , m_couleurDemandee(static_cast<Carte::Couleur>(7))
    , m_couleurAtout(Carte::COEUR)
    , m_carteAtout(nullptr)
    , m_idxPlayerWinning(-1)
    , m_idxPlayerStartPli(0)
    , m_idxPlayerStartManche(0)
    , m_carteWinning(nullptr)
    , m_scorePli(0)
    , m_scoreTeam1(0)
    , m_scoreTeam2(0)
    , m_scoreTotalTeam1(0)
    , m_scoreTotalTeam2(0)
    , m_biddingPhase(true)
    , m_biddingPlayer(0)
    , m_passCount(0)
    , m_lastBidPlayer(-1)
    , m_lastBidAnnonce(Player::ANNONCEINVALIDE)
    , m_lastBidCouleur(Carte::CARREAU)
{
    m_player0Hand = new HandModel(this);
    m_player1Hand = new HandModel(this);
    m_player2Hand = new HandModel(this);
    m_player3Hand = new HandModel(this);

    if (m_players.size() >= 1) m_player0Hand->setPlayer(m_players[0].get().get(), true);
    if (m_players.size() >= 2) m_player1Hand->setPlayer(m_players[1].get().get(), false);
    if (m_players.size() >= 3) m_player2Hand->setPlayer(m_players[2].get().get(), false);
    if (m_players.size() >= 4) m_player3Hand->setPlayer(m_players[3].get().get(), false);

    updatePlayableStates();
}

GameModel::~GameModel()
{
    delete m_player0Hand;
    delete m_player1Hand;
    delete m_player2Hand;
    delete m_player3Hand;
}

HandModel* GameModel::player0Hand() const { return m_player0Hand; }
HandModel* GameModel::player1Hand() const { return m_player1Hand; }
HandModel* GameModel::player2Hand() const { return m_player2Hand; }
HandModel* GameModel::player3Hand() const { return m_player3Hand; }

int GameModel::currentPlayer() const {
    std::cout << "Current player index requested: " << m_currentPlayer << std::endl;
    return m_currentPlayer;
}

QList<QVariant> GameModel::currentPli() const {
    QList<QVariant> pliList;
    for (const auto &cdp : m_currentPli) {
        QVariantMap map;
        map["playerId"] = cdp.playerId;
        if (cdp.carte) {
            map["value"] = static_cast<int>(cdp.carte->getChiffre());
            map["suit"] = static_cast<int>(cdp.carte->getCouleur());
        } else {
            map["value"] = -1;
            map["suit"] = -1;
        }
        pliList.append(map);
    }
    return pliList;
}

QString GameModel::currentPlayerName() const {
    if (m_currentPlayer >= 0 && m_currentPlayer < m_players.size()) {
        auto& player = m_players[m_currentPlayer].get();
        if (player) {
            return QString::fromStdString(player->getName());
        }
    }
    return "";
}

bool GameModel::biddingPhase() const { return m_biddingPhase; }
int GameModel::biddingPlayer() const { return m_biddingPlayer; }
int GameModel::lastBidValue() const { return static_cast<int>(m_lastBidAnnonce); }
int GameModel::scoreTeam1() const { return m_scoreTeam1; }
int GameModel::scoreTeam2() const { return m_scoreTeam2; }
int GameModel::scoreTotalTeam1() const { return m_scoreTotalTeam1; }
int GameModel::scoreTotalTeam2() const { return m_scoreTotalTeam2; }

QString GameModel::lastBid() const {
    if (m_lastBidAnnonce == Player::ANNONCEINVALIDE) return "Aucune annonce";
    if (m_lastBidAnnonce == Player::PASSE) return "Passe";

    QString bid;
    switch(m_lastBidAnnonce) {
        case Player::QUATREVINGT: bid = "80"; break;
        case Player::QUATREVINGTDIX: bid = "90"; break;
        case Player::CENT: bid = "100"; break;
        case Player::CENTDIX: bid = "110"; break;
        case Player::CENTVINGT: bid = "120"; break;
        case Player::CENTTRENTE: bid = "130"; break;
        case Player::CENTQUARANTE: bid = "140"; break;
        case Player::CENTCINQUANTE: bid = "150"; break;
        case Player::CENTSOIXANTE: bid = "160"; break;
        case Player::CAPOT: bid = "Capot"; break;
        case Player::GENERALE: bid = "Generale"; break;
        default: bid = "?";
    }
    return bid;
}

QString GameModel::lastBidSuit() const {
    switch(m_lastBidCouleur) {
        case Carte::COEUR: return "♥ Cœur";
        case Carte::CARREAU: return "♦ Carreau";
        case Carte::TREFLE: return "♣ Trèfle";
        case Carte::PIQUE: return "♠ Pique";
        default: return "";
    }
}

void GameModel::makeBid(int bidValue, int suitValue) {
    if (!m_biddingPhase) return;

    Player::Annonce annonce = static_cast<Player::Annonce>(bidValue);
    Carte::Couleur couleur = static_cast<Carte::Couleur>(suitValue);

    if (annonce <= m_lastBidAnnonce && annonce != Player::PASSE) {
        qDebug() << "Annonce invalide : doit être supérieure à la précédente";
        return;
    }

    auto& player = m_players[m_biddingPlayer].get();
    if (!player) return;

    if (annonce == Player::PASSE) {
        m_passCount++;
    } else {
        m_lastBidPlayer = m_biddingPlayer;
        m_lastBidAnnonce = annonce;
        m_lastBidCouleur = couleur;
        m_passCount = 0;
        emit lastBidChanged();
    }

    m_biddingPlayer = (m_biddingPlayer + 1) % 4;
    emit biddingPlayerChanged();

    

    if (m_passCount >= 3) {
        std::cout << "Fin de la phase d'annonces." << std::endl;
        endBiddingPhase();
    } else {
        //processAIBids();
    }
}

void GameModel::passBid() {
    makeBid(static_cast<int>(Player::PASSE), 0);
}

void GameModel::playCard(int playerIndex, int cardIndex) {
    if (playerIndex != m_currentPlayer) {
        qDebug() << "Ce n'est pas le tour de ce joueur!";
        return;
    }
    if (playerIndex < 0 || playerIndex >= m_players.size()) {
        return;
    }
    auto& player = m_players[playerIndex].get();
    if (!player) return;
    const auto& main = player->getMain();
    if (cardIndex < 0 || cardIndex >= main.size()) {
        return;
    }

    if (!player->isCartePlayable(cardIndex, m_couleurDemandee, m_couleurAtout, m_carteAtout, m_idxPlayerWinning)) {
        qDebug() << "Cette carte n'est pas jouable!";
        return;
    }

    Carte* carteJouee = player->getMain()[cardIndex];

    if (m_couleurDemandee == static_cast<Carte::Couleur>(7)) {
        m_couleurDemandee = carteJouee->getCouleur();
        m_idxPlayerWinning = playerIndex;
        m_carteWinning = carteJouee;
    } else {
        if(carteJouee->getCouleur() == m_couleurDemandee || carteJouee->getCouleur() == m_couleurAtout) {
            if(m_carteWinning && *m_carteWinning < *carteJouee) {
                m_carteWinning = carteJouee;
                m_idxPlayerWinning = playerIndex;
            }
        }
    }

    if(carteJouee->getCouleur() == m_couleurAtout && m_carteAtout == nullptr) {
        m_carteAtout = carteJouee;
        std::cout << "ATOUT JOUE: " << std::endl;
        m_carteAtout->printCarte();
    } else if(carteJouee->getCouleur() == m_couleurAtout && m_carteAtout != nullptr) {
        if(*m_carteAtout < *carteJouee) {
            m_carteAtout = carteJouee;
            std::cout << "ATOUT JOUE SUPERIEUR AU PRECEDENT: " << std::endl;
            m_carteAtout->printCarte();
        }
    }
    m_scorePli += carteJouee->getValeurDeLaCarte();

    player->removeCard(cardIndex);
    refreshHand(playerIndex);
    CarteDuPli cdP;
    cdP.playerId = playerIndex;
    cdP.carte = carteJouee;
    m_currentPli.append(cdP);
    emit currentPliChanged();

    if(m_currentPli.size() == 4) {
        std::cout << "****************************************************" << std::endl;
        std::cout << "******************* FIN DE PLI ******************" << std::endl;
        std::cout << "Player: " << m_players[m_idxPlayerWinning].get()->getName() << " gagne le pli" << std::endl;
        std::cout << "Il vaut score: " << m_scorePli << std::endl;

        if(m_idxPlayerWinning % 2 == 0) {
            m_scoreTeam1 += m_scorePli;
            emit scoreTeam1Changed();
        } else {
            m_scoreTeam2 += m_scorePli;
            emit scoreTeam2Changed();
        }

        m_idxPlayerStartPli = m_idxPlayerWinning;
        resetPli();
        if(player->getMain().empty()) {
            std::cout << "La manche est terminee!" << std::endl;
            endManche();
        }
        return;
    }

    m_currentPlayer = (m_currentPlayer + 1) % 4;
    emit currentPlayerChanged();
    updatePlayableStates();
}

void GameModel::refreshAllHands() {
    m_player0Hand->refresh();
    m_player1Hand->refresh();
    m_player2Hand->refresh();
    m_player3Hand->refresh();
}

void GameModel::refreshHand(int playerIndex) {
    switch(playerIndex) {
        case 0: m_player0Hand->refresh(); break;
        case 1: m_player1Hand->refresh(); break;
        case 2: m_player2Hand->refresh(); break;
        case 3: m_player3Hand->refresh(); break;
    }
}

void GameModel::updatePlayableStates() {
    for (int i = 0; i < 4; i++) {
        HandModel* hand = nullptr;
        switch(i) {
            case 0: hand = m_player0Hand; break;
            case 1: hand = m_player1Hand; break;
            case 2: hand = m_player2Hand; break;
            case 3: hand = m_player3Hand; break;
        }

        if (hand) {
            hand->setGameContext(m_couleurDemandee, m_couleurAtout, m_carteAtout, m_idxPlayerWinning, i == m_currentPlayer);
        }
    }
}

void GameModel::resetPli() {
    m_couleurDemandee = static_cast<Carte::Couleur>(7);
    m_carteAtout = nullptr;
    m_idxPlayerStartPli = m_idxPlayerWinning;
    m_currentPlayer = m_idxPlayerStartPli;
    std::cout << "Nouveau pli, premier joueur: " << m_currentPlayer << std::endl;
    m_scorePli = 0;
    emit currentPlayerChanged();
    updatePlayableStates();
    m_currentPli.clear();
    emit currentPliChanged();
}

void GameModel::revealPlayerCards(int playerIndex, bool reveal) {
    HandModel* hand = nullptr;
    switch(playerIndex) {
        case 0: hand = m_player0Hand; break;
        case 1: hand = m_player1Hand; break;
        case 2: hand = m_player2Hand; break;
        case 3: hand = m_player3Hand; break;
    }

    if (hand && playerIndex < m_players.size()) {
        hand->setPlayer(m_players[playerIndex].get().get(), reveal);
    }
}

void GameModel::processAIBids() {
    while (m_biddingPlayer != 0 && m_passCount < 3) {
        auto& player = m_players[m_biddingPlayer].get();
        if (!player) break;

        m_passCount++;
        m_biddingPlayer = (m_biddingPlayer + 1) % 4;
        emit biddingPlayerChanged();
    }
}

void GameModel::endBiddingPhase() {
    m_biddingPhase = false;
    emit biddingPhaseChanged();

    if (m_lastBidPlayer >= 0) {
        m_couleurAtout = m_lastBidCouleur;
        m_players[0].get()->setAtout(m_couleurAtout);
        m_players[1].get()->setAtout(m_couleurAtout);
        m_players[2].get()->setAtout(m_couleurAtout);
        m_players[3].get()->setAtout(m_couleurAtout);
    }

    updatePlayableStates();
    qDebug() << "Fin des annonces. Atout:" << static_cast<int>(m_couleurAtout);
}

void GameModel::endManche() {
    int lastBidPoints = Player::convertAnnonceEnPoint(m_lastBidAnnonce);
    int belottePointsAtoutTeam = 0;
    int belottePointsOtherTeam = 0;

    if(m_players[m_lastBidPlayer].get()->getHasBelotte() || m_players[(m_lastBidPlayer+2) % 4].get()->getHasBelotte()) {
        belottePointsAtoutTeam = 20;
        //m_scoreTeam1 += 20;
        std::cout << "Belotte dans l'equipe partante!" << std::endl;
    } else if(m_players[(m_lastBidPlayer+1) % 4].get()->getHasBelotte() || m_players[(m_lastBidPlayer+3) % 4].get()->getHasBelotte()) {
        belottePointsOtherTeam = 20;
        //m_scoreTeam2 += 20;
        std::cout << "Belotte dans l'equipe adverse!" << std::endl;
    } else {
        std::cout << "Aucune belote dans cette manche." << std::endl;
    }

    if(m_idxPlayerWinning % 2 == 0) {
        m_scoreTeam1 += 10;
        std::cout << "Dernier pli fait par l'equipe 1" << std::endl;
    } else {
        std::cout << "Dernier pli fait par l'equipe 2" << std::endl;
        m_scoreTeam2 += 10;
    }

    int scoreAtoutTeam = (m_lastBidPlayer % 2 == 0) ? m_scoreTeam1 : m_scoreTeam2;
    int scoreOtherTeam = (m_lastBidPlayer % 2 == 0) ? m_scoreTeam2 : m_scoreTeam1;

    std::cout << "Score equipe partante avant contrat: " << scoreAtoutTeam << std::endl;
    std::cout << "Score equipe adverse avant contrat: " << scoreOtherTeam << std::endl;
    std::cout << "Points belote equipe partante: " << belottePointsAtoutTeam << std::endl;
    std::cout << "Points belote equipe adverse: " << belottePointsOtherTeam << std::endl;
    std::cout << "Points a atteindre pour contrat: " << lastBidPoints << std::endl;

    if(m_lastBidAnnonce == Player::CAPOT) {
        std::cout << "CAPOT" << std::endl;
    } else if(m_lastBidAnnonce == Player::GENERALE) {
        std::cout << "GENERALE" << std::endl;
    } else if(scoreAtoutTeam + belottePointsAtoutTeam >= lastBidPoints) {
        std::cout << "L'equipe partant a reussi son contrat!" << std::endl;
        int scoreManche = lastBidPoints + scoreAtoutTeam + belottePointsAtoutTeam;
        std::cout << "Score manche: " << scoreManche << std::endl;
        if (m_lastBidPlayer % 2 == 0) {   // Equipe 1 a gagné la manche
            m_scoreTotalTeam1 += scoreManche;
            m_scoreTotalTeam2 += scoreOtherTeam + belottePointsOtherTeam;
            std::cout << "Score manche equipe 1: " << m_scoreTotalTeam1 << std::endl;
        } else {  // Equipe 2 a gagné la manche
            m_scoreTotalTeam2 += scoreManche;
            m_scoreTotalTeam1 += scoreOtherTeam + belottePointsOtherTeam;
            std::cout << "Score manche equipe 2: " << m_scoreTotalTeam2 << std::endl;
        }
    } else {
        std::cout << "L'equipe partante a echoue son contrat!" << std::endl;
        int scoreManche = 160 + lastBidPoints;
        if (m_lastBidPlayer % 2 == 0) {
            m_scoreTotalTeam2 += scoreManche + belottePointsOtherTeam;
            m_scoreTotalTeam1 += belottePointsAtoutTeam;
            std::cout << "Score manche equipe 2: " << m_scoreTotalTeam2 << std::endl;
            std::cout << "Score manche equipe 1: " << m_scoreTotalTeam1 << std::endl;
        } else {
            m_scoreTotalTeam1 += scoreManche + belottePointsOtherTeam;
            m_scoreTotalTeam2 += belottePointsAtoutTeam;
            std::cout << "Score manche equipe 1: " << m_scoreTotalTeam1 << std::endl;
            std::cout << "Score manche equipe 2: " << m_scoreTotalTeam2 << std::endl;
        }
    }

    emit scoreTeam1Changed();
    emit scoreTeam2Changed();
    emit scoreTotalTeam1Changed();
    emit scoreTotalTeam2Changed();

    m_scoreTeam1 = 0;
    m_scoreTeam2 = 0;

    if(m_scoreTotalTeam1 >= 1000 || m_scoreTotalTeam2 >= 1000) {
        std::cout << "Fin de la partie!" << std::endl;
    } else {
        std::cout << "Nouvelle manche!" << std::endl;
        m_currentPli.clear();
        m_couleurDemandee = static_cast<Carte::Couleur>(7);
        m_carteAtout = nullptr;
        m_idxPlayerStartManche = (m_idxPlayerStartManche + 1) % 4;
        std::cout << "Premier joueur de la nouvelle manche: " << m_idxPlayerStartManche << std::endl;

        m_currentPlayer = m_idxPlayerStartManche;
        m_biddingPhase = true;
        m_biddingPlayer = m_idxPlayerStartManche;
        std::cout << "Premier joueur des annonces: " << m_biddingPlayer << std::endl;
        m_passCount = 0;
        m_lastBidPlayer = -1;
        m_lastBidAnnonce = Player::ANNONCEINVALIDE;
        m_carteWinning = nullptr;
        m_scorePli = 0;
        m_lastBidCouleur = Carte::COULEURINVALIDE;

        m_deck.resetDeck();
        m_deck.shuffleDeck();
        for (int i = 0; i < 4; i++) {
            auto& player = m_players[i].get();
            if (player) {
                for (int j = 0; j < 8; j++) {
                    Carte* carte = m_deck.drawCard();
                    if (carte) {
                        player->addCardToHand(carte);
                    }
                }
                refreshHand(i);
            }
        }

        std::cout << "idFirstPlayer apres reinitialisation: " << m_idxPlayerStartPli << std::endl;
        emit biddingPhaseChanged();
        emit biddingPlayerChanged();
        emit lastBidChanged();
        emit currentPlayerChanged();
    }
}
