#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "Player.h"
#include "Carte.h"
#include "Deck.h"
#include <iostream>

struct CarteDuPli {
    int playerId;
    Carte* carte;
};

// Modèle pour une main de cartes
class HandModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum CardRoles {
        ValueRole = Qt::UserRole + 1,
        SuitRole,
        FaceUpRole,
        IsAtoutRole,
        IsPlayableRole
    };

    explicit HandModel(QObject *parent = nullptr)
        : QAbstractListModel(parent)
        , m_player(nullptr)
        , m_faceUp(true)
        , m_couleurDemandee(static_cast<Carte::Couleur>(7))
        , m_couleurAtout(static_cast<Carte::Couleur>(7))
        , m_carteAtout(nullptr)
        , m_idxPlayerWinning(-1)
        , m_isCurrentPlayer(false)
    {
    }

    // Définir le joueur source
    void setPlayer(Player* player, bool faceUp = true) {
        beginResetModel();
        m_player = player;
        m_faceUp = faceUp;
        endResetModel();
    }

    // Mettre à jour le contexte de jeu pour déterminer les cartes jouables
    void setGameContext(const Carte::Couleur& couleurDemandee, 
                       const Carte::Couleur& couleurAtout,
                       Carte* carteAtout,
                       int idxPlayerWinning,
                       bool isCurrentPlayer) {
        std::cout << "couleur demandee dans HandModel: " << couleurDemandee << std::endl;
        m_couleurDemandee = couleurDemandee;
        m_couleurAtout = couleurAtout;
        m_carteAtout = carteAtout;
        m_idxPlayerWinning = idxPlayerWinning;
        m_isCurrentPlayer = isCurrentPlayer;
        
        // Notifier QML que les états "playable" ont changé
        if (rowCount() > 0) {
            emit dataChanged(index(0), index(rowCount() - 1), {IsPlayableRole});
        }
    }

    // Rafraîchir les données
    void refresh() {
        beginResetModel();
        endResetModel();
    }

    // Nombre de cartes dans la main
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid() || !m_player)
            return 0;
        return m_player->getMain().size();
    }

    // Données pour chaque carte
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || !m_player)
            return QVariant();

        const auto& main = m_player->getMain();
        if (index.row() >= main.size())
            return QVariant();

        const Carte* carte = main[index.row()];
        if (!carte)
            return QVariant();

        switch (role) {
            case ValueRole:
                return static_cast<int>(carte->getChiffre());
            case SuitRole:
                return static_cast<int>(carte->getCouleur());
            case FaceUpRole:
                return m_faceUp;
            case IsAtoutRole:
                return false; // À adapter selon la logique
            case IsPlayableRole:
                if (!m_isCurrentPlayer || !m_player) {
                    return false;
                }
                return m_player->isCartePlayable(index.row(), m_couleurDemandee, 
                                                 m_couleurAtout, m_carteAtout, 
                                                 m_idxPlayerWinning);
            default:
                return QVariant();
        }
    }

    // Noms des rôles pour QML
    QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles;
        roles[ValueRole] = "value";
        roles[SuitRole] = "suit";
        roles[FaceUpRole] = "faceUp";
        roles[IsAtoutRole] = "isAtout";
        roles[IsPlayableRole] = "isPlayable";
        return roles;
    }

private:
    Player* m_player;
    bool m_faceUp;
    Carte::Couleur m_couleurDemandee;
    Carte::Couleur m_couleurAtout;
    Carte* m_carteAtout;
    int m_idxPlayerWinning;
    bool m_isCurrentPlayer;
};

// Modèle du jeu complet
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
    explicit GameModel(const std::vector<std::reference_wrapper<std::unique_ptr<Player>>>& players, Deck &deck,
                      QObject *parent = nullptr)
        : QObject(parent)
        , m_players(players)
        , m_deck(deck)
        , m_currentPlayer(0)
        , m_couleurDemandee(static_cast<Carte::Couleur>(7))
        , m_couleurAtout(Carte::COEUR)  // À initialiser avec la vraie valeur
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
        // m_players[0].get()->setAtout(m_couleurAtout);
        // m_players[1].get()->setAtout(m_couleurAtout);
        // m_players[2].get()->setAtout(m_couleurAtout);
        // m_players[3].get()->setAtout(m_couleurAtout);

        // Créer les modèles de mains
        m_player0Hand = new HandModel(this);
        m_player1Hand = new HandModel(this);
        m_player2Hand = new HandModel(this);
        m_player3Hand = new HandModel(this);

        // Associer chaque modèle à son joueur
        if (m_players.size() >= 1) m_player0Hand->setPlayer(m_players[0].get().get(), true);   // Face visible
        if (m_players.size() >= 2) m_player1Hand->setPlayer(m_players[1].get().get(), false);  // Face cachée
        if (m_players.size() >= 3) m_player2Hand->setPlayer(m_players[2].get().get(), false);  // Face cachée
        if (m_players.size() >= 4) m_player3Hand->setPlayer(m_players[3].get().get(), false);  // Face cachée
        
        // Mettre à jour les états initiaux
        updatePlayableStates();
    }

    HandModel* player0Hand() const { return m_player0Hand; }
    HandModel* player1Hand() const { return m_player1Hand; }
    HandModel* player2Hand() const { return m_player2Hand; }
    HandModel* player3Hand() const { return m_player3Hand; }
    
    int currentPlayer() const { 
        std::cout << "Current player index requested: " << m_currentPlayer << std::endl;
        return m_currentPlayer; }

    QList<QVariant> currentPli() const { 
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
    
    QString currentPlayerName() const {
        if (m_currentPlayer >= 0 && m_currentPlayer < m_players.size()) {
            auto& player = m_players[m_currentPlayer].get();
            if (player) {
                return QString::fromStdString(player->getName());
            }
        }
        return "";
    }

    bool biddingPhase() const { return m_biddingPhase; }
    int biddingPlayer() const { 
        std::cout << "Bidding player index requested: " << m_biddingPlayer << std::endl;
        return m_biddingPlayer; }
    int lastBidValue() const { return static_cast<int>(m_lastBidAnnonce); }
    int scoreTeam1() const { return m_scoreTeam1; }
    int scoreTeam2() const { return m_scoreTeam2; }
    int scoreTotalTeam1() const { return m_scoreTotalTeam1; }
    int scoreTotalTeam2() const { return m_scoreTotalTeam2; }
    
    QString lastBid() const {
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
    
    QString lastBidSuit() const {
        switch(m_lastBidCouleur) {
            case Carte::COEUR: return "♥ Cœur";
            case Carte::CARREAU: return "♦ Carreau";
            case Carte::TREFLE: return "♣ Trèfle";
            case Carte::PIQUE: return "♠ Pique";
            default: return "";
        }
    }

    // Faire une annonce
    Q_INVOKABLE void makeBid(int bidValue, int suitValue) {
        if (!m_biddingPhase) return;
        //if (m_biddingPlayer != 0) return; // Seulement le joueur humain pour l'instant
        
        Player::Annonce annonce = static_cast<Player::Annonce>(bidValue);
        Carte::Couleur couleur = static_cast<Carte::Couleur>(suitValue);
        
        // Vérifier que l'annonce est valide (supérieure à la précédente)
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
            m_passCount = 0; // Reset le compteur
            emit lastBidChanged();
        }
        
        // Passer au joueur suivant
        m_biddingPlayer = (m_biddingPlayer + 1) % 4;
        emit biddingPlayerChanged();
        
        // Si 3 joueurs ont passé, fin des annonces
        if (m_passCount >= 3) {
            std::cout << "Fin de la phase d'annonces." << std::endl;
            endBiddingPhase();
        } else {
            // Faire annoncer les IA
            // std::cout << "processAIBids" << std::endl;
            // processAIBids();
        }
    }
    
    // Passer son tour
    Q_INVOKABLE void passBid() {
        makeBid(static_cast<int>(Player::PASSE), 0);
    }

    // Jouer une carte
    Q_INVOKABLE void playCard(int playerIndex, int cardIndex) {
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

        // Vérifier que la carte est jouable (sécurité)
        if (!player->isCartePlayable(cardIndex, m_couleurDemandee, m_couleurAtout, 
                                     m_carteAtout, m_idxPlayerWinning)) {
            qDebug() << "Cette carte n'est pas jouable!";
            return;
        }

        // Jouer la carte
        Carte* carteJouee = player->getMain()[cardIndex];
                
        // Si c'est le premier joueur du pli, définir la couleur demandée
        if (m_couleurDemandee == static_cast<Carte::Couleur>(7)) {
            m_couleurDemandee = carteJouee->getCouleur();
            m_idxPlayerWinning = playerIndex;
            m_carteWinning = carteJouee;
        } else { // les autres doivent suivre à la couleur demandée
            if(carteJouee->getCouleur() == m_couleurDemandee || carteJouee->getCouleur() == m_couleurAtout) {
                if(m_carteWinning && *m_carteWinning < *carteJouee) {
                    m_carteWinning = carteJouee;  // On garde une référence à la carte originale
                    m_idxPlayerWinning = playerIndex;
                }
            }
        }
            
        // si une carte d'atout est jouée, on la garde comme référence
        if(carteJouee->getCouleur() == m_couleurAtout && m_carteAtout == nullptr) {  
            m_carteAtout = carteJouee;  // On garde une référence à la carte originale
            std::cout << "ATOUT JOUE: " << std::endl;
            m_carteAtout->printCarte();
        } else if(carteJouee->getCouleur() == m_couleurAtout && m_carteAtout != nullptr) {
            if(*m_carteAtout < *carteJouee) {
                m_carteAtout = carteJouee;  // On garde une référence à la carte originale
                std::cout << "ATOUT JOUE SUPERIEUR AU PRECEDENT: " << std::endl;
                m_carteAtout->printCarte();
            }
        }

        m_scorePli += carteJouee->getValeurDeLaCarte();
        
        // Retirer la carte de la main
        player->removeCard(cardIndex);

        // Rafraîchir l'affichage de la main
        refreshHand(playerIndex);

        CarteDuPli cdP;
        cdP.playerId = playerIndex;
        cdP.carte = carteJouee;
        m_currentPli.append(cdP);
        emit currentPliChanged();

        std::cout << "Joueur gagnant actuel" << player->getName() << " a joué: ";

        if(m_currentPli.size() == 4) {
            // Le pli est complet, gérer la fin du pli ailleurs
            std::cout << "****************************************************" << std::endl;
            std::cout << "******************* FIN DE PLI ******************" << std::endl;
            std::cout << "Player: " << m_players[m_idxPlayerWinning].get()->getName() << " gagne le pli" << std::endl;
            std::cout << "Il vaut score: " << m_scorePli << std::endl;
            
            // Mise à jour des scores
            if(m_idxPlayerWinning % 2 == 0) {
                m_scoreTeam1 += m_scorePli;
                emit scoreTeam1Changed();
            } else {
                m_scoreTeam2 += m_scorePli;
                emit scoreTeam2Changed();
            }

            // Le gagnant du pli commencera le prochain pli
            m_idxPlayerStartPli = m_idxPlayerWinning;

            resetPli();

            if(player->getMain().empty()) {
                std::cout << "La manche est terminée!" << std::endl;
                endManche();
            }
            return;
        }
        
        // Passer au joueur suivant
        m_currentPlayer = (m_currentPlayer + 1) % 4;
        emit currentPlayerChanged();
        
        // Mettre à jour les cartes jouables pour le nouveau joueur
        updatePlayableStates();
        
        //emit cardPlayed(playerIndex, cardIndex);
    }

    // Rafraîchir toutes les mains
    Q_INVOKABLE void refreshAllHands() {
        m_player0Hand->refresh();
        m_player1Hand->refresh();
        m_player2Hand->refresh();
        m_player3Hand->refresh();
    }

    // Rafraîchir une main spécifique
    void refreshHand(int playerIndex) {
        switch(playerIndex) {
            case 0: m_player0Hand->refresh(); break;
            case 1: m_player1Hand->refresh(); break;
            case 2: m_player2Hand->refresh(); break;
            case 3: m_player3Hand->refresh(); break;
        }
    }
    
    // Mettre à jour les états "playable" de toutes les cartes
    void updatePlayableStates() {
        for (int i = 0; i < 4; i++) {
            HandModel* hand = nullptr;
            switch(i) {
                case 0: hand = m_player0Hand; break;
                case 1: hand = m_player1Hand; break;
                case 2: hand = m_player2Hand; break;
                case 3: hand = m_player3Hand; break;
            }
            
            if (hand) {
                hand->setGameContext(m_couleurDemandee, m_couleurAtout, 
                                   m_carteAtout, m_idxPlayerWinning,
                                   i == m_currentPlayer);
            }
        }
    }
    
    // Réinitialiser le pli (à appeler quand un pli est terminé)
    Q_INVOKABLE void resetPli() {
        m_couleurDemandee = static_cast<Carte::Couleur>(7);
        m_carteAtout = nullptr;
        m_idxPlayerStartPli = m_idxPlayerWinning;
        m_currentPlayer = m_idxPlayerStartPli;

        std::cout << "Nouveau pli, premier joueur: " << m_currentPlayer << std::endl;
        m_scorePli = 0;
        emit currentPlayerChanged();
        m_idxPlayerWinning = -1;
        updatePlayableStates();
        m_currentPli.clear();
        emit currentPliChanged();
    }

    // Révéler les cartes d'un joueur (pour débug)
    Q_INVOKABLE void revealPlayerCards(int playerIndex, bool reveal) {
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

signals:
    void currentPlayerChanged();
    void cardPlayed(int playerIndex, int cardIndex);
    void currentPliChanged();
    void biddingPhaseChanged();
    void biddingPlayerChanged();
    void lastBidChanged();
    void scoreTeam1Changed();
    void scoreTeam2Changed();
    void scoreTotalTeam1Changed();
    void scoreTotalTeam2Changed();

private:

    void processAIBids() {
        while (m_biddingPlayer != 0 && m_passCount < 3) {
            auto& player = m_players[m_biddingPlayer].get();
            if (!player) break;
            
            // Pour l'instant, les IA passent automatiquement
            // TODO: Implémenter une vraie logique d'IA
            m_passCount++;
            
            m_biddingPlayer = (m_biddingPlayer + 1) % 4;
            emit biddingPlayerChanged();
        }
    }
    
    // Fin de la phase d'annonces
    void endBiddingPhase() {
        m_biddingPhase = false;
        emit biddingPhaseChanged();
        
        // Définir la couleur d'atout
        if (m_lastBidPlayer >= 0) {
            m_couleurAtout = m_lastBidCouleur;
            m_players[0].get()->setAtout(m_couleurAtout);
            m_players[1].get()->setAtout(m_couleurAtout);
            m_players[2].get()->setAtout(m_couleurAtout);
            m_players[3].get()->setAtout(m_couleurAtout);
        }

        // m_currentPlayer = m_idxPlayerStartPli;
        // std::cout << "Premier joueur du jeu apres annonces: " << m_currentPlayer << std::endl;
        // emit currentPlayerChanged();
        
        // Mettre à jour les cartes jouables
        updatePlayableStates();
        
        qDebug() << "Fin des annonces. Atout:" << static_cast<int>(m_couleurAtout);
    }

    void endManche() {
        // Calcul si l'equipe qui a choisit l'atout a reussi son contrat
        int scoreAtoutTeam = (m_lastBidPlayer % 2 == 0) ? m_scoreTeam1 : m_scoreTeam2;
        int scoreOtherTeam = (m_lastBidPlayer % 2 == 0) ? m_scoreTeam2 : m_scoreTeam1;
        int lastBidPoints = Player::convertAnnonceEnPoint(m_lastBidAnnonce);

        // Ajoute 10 points a l'equipe qui fait le dernier pli
        if(m_idxPlayerWinning % 2 == 0) {
            m_scoreTeam1 += 10;
            scoreAtoutTeam += 10;
        } else {
            m_scoreTeam2 += 10;
            scoreAtoutTeam += 10;
        }
        
        if(m_lastBidAnnonce == Player::CAPOT) {
            std::cout << "CAPOT" << std::endl;
            // TODO
            return;
        }
        if(m_lastBidAnnonce == Player::GENERALE) {
            std::cout << "GENERALE" << std::endl;
            // TODO
            return;
        }
        if (scoreAtoutTeam >= lastBidPoints) {
            std::cout << "L'equipe partant a reussi son contrat!" << std::endl;
            int scoreManche = lastBidPoints + scoreAtoutTeam;
            std::cout << "Score manche: " << scoreManche << std::endl;
            if (m_lastBidPlayer % 2 == 0) {  // L'équipe 1  a gagné la manche
                m_scoreTotalTeam1 += scoreManche;
                m_scoreTotalTeam2 += scoreOtherTeam;
                std::cout << "Score manche equipe 1: " << m_scoreTotalTeam1 << std::endl;
            } else { // L'équipe 2 a gagné la manche
                m_scoreTotalTeam2 += scoreManche;
                m_scoreTotalTeam1 += scoreOtherTeam;
                std::cout << "Score manche equipe 2: " << m_scoreTotalTeam2 << std::endl;
            }

        } else {
            std::cout << "L'equipe partante a echoue son contrat!" << std::endl;
            int scoreManche = 160 + lastBidPoints;
            // L'équipe adverse marque tous les points
            if (m_lastBidPlayer % 2 == 0) {  // L'équipe 1 a perdu la manche
                m_scoreTotalTeam2 += scoreManche;
                std::cout << "Score manche equipe 2: " << m_scoreTotalTeam2 << std::endl;
            } else {  // L'equipe 2 a perdu la manche
                m_scoreTotalTeam1 += scoreManche;
                std::cout << "Score manche equipe 1: " << m_scoreTotalTeam1 << std::endl;
            }
        }

        // Réinitialiser les scores et les mains pour une nouvelle manche
        m_scoreTeam1 = 0;
        m_scoreTeam2 = 0;
        emit scoreTeam1Changed();
        emit scoreTeam2Changed();
        emit scoreTotalTeam1Changed();
        emit scoreTotalTeam2Changed();

        if(m_scoreTotalTeam1 >= 1000 || m_scoreTotalTeam2 >= 1000) {
            std::cout << "Fin de la partie!" << std::endl;
            // TODO: Afficher un message de fin de partie avec le gagnant
        } else {
            std::cout << "Nouvelle manche!" << std::endl;
            // Réinitialiser les mains des joueurs
            // for (auto& playerRef : m_players) {
            //     auto& player = playerRef.get();
            //     if (player) {
            //         player->clearHand();
            //     }
            // }
            // Réinitialiser les variables de jeu
            m_currentPli.clear();
            m_couleurDemandee = static_cast<Carte::Couleur>(7);
            m_carteAtout = nullptr;
            m_idxPlayerWinning = -1;
            //m_idxPlayerStartPli = (m_idxPlayerStartPli + 1) % 4;
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

            // TODO: créer le deck en fonction de l'ordre des plis fait. Ne pas melanger le deck mais couper les cartes.
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
            //refreshAllHands();

            std::cout << "idFirstPlayer apres reinitialisation: " << m_idxPlayerStartPli << std::endl;
            // updatePlayableStates();
            // refreshAllHands();

            emit biddingPhaseChanged();
            emit biddingPlayerChanged();
            emit lastBidChanged();
            emit currentPlayerChanged();
        }
    }

    const std::vector<std::reference_wrapper<std::unique_ptr<Player>>>& m_players;
    Deck &m_deck;
    HandModel* m_player0Hand;
    HandModel* m_player1Hand;
    HandModel* m_player2Hand;
    HandModel* m_player3Hand;
    int m_currentPlayer;
    QList<CarteDuPli> m_currentPli;
    
    // Contexte du pli en cours
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

    // Phase d'annonces
    bool m_biddingPhase;
    int m_biddingPlayer;
    int m_passCount;
    int m_lastBidPlayer;
    Player::Annonce m_lastBidAnnonce;
    Carte::Couleur m_lastBidCouleur;
};

#endif // GAMEMODEL_H