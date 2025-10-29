#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "Player.h"
#include "Carte.h"
#include <iostream>

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
                return false; // À adapter selon votre logique
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

public:
    explicit GameModel(const std::vector<std::reference_wrapper<std::unique_ptr<Player>>>& players, 
                      QObject *parent = nullptr)
        : QObject(parent)
        , m_players(players)
        , m_currentPlayer(0)
        , m_couleurDemandee(static_cast<Carte::Couleur>(7))
        , m_couleurAtout(Carte::COEUR)  // À initialiser avec la vraie valeur
        , m_carteAtout(nullptr)
        , m_idxPlayerWinning(-1)
    {
        // Créer les modèles de mains
        m_player0Hand = new HandModel(this);
        m_player1Hand = new HandModel(this);
        m_player2Hand = new HandModel(this);
        m_player3Hand = new HandModel(this);

        // Associer chaque modèle à son joueur
        if (players.size() >= 1) m_player0Hand->setPlayer(players[0].get().get(), true);   // Face visible
        if (players.size() >= 2) m_player1Hand->setPlayer(players[1].get().get(), false);  // Face cachée
        if (players.size() >= 3) m_player2Hand->setPlayer(players[2].get().get(), false);  // Face cachée
        if (players.size() >= 4) m_player3Hand->setPlayer(players[3].get().get(), false);  // Face cachée
        
        // Mettre à jour les états initiaux
        updatePlayableStates();
    }

    HandModel* player0Hand() const { return m_player0Hand; }
    HandModel* player1Hand() const { return m_player1Hand; }
    HandModel* player2Hand() const { return m_player2Hand; }
    HandModel* player3Hand() const { return m_player3Hand; }
    
    int currentPlayer() const { return m_currentPlayer; }
    
    QString currentPlayerName() const {
        if (m_currentPlayer >= 0 && m_currentPlayer < m_players.size()) {
            auto& player = m_players[m_currentPlayer].get();
            if (player) {
                return QString::fromStdString(player->getName());
            }
        }
        return "";
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
        }
        
        // Si c'est un atout, mettre à jour carteAtout
        if (carteJouee->getCouleur() == m_couleurAtout) {
            if (!m_carteAtout || *m_carteAtout < *carteJouee) {
                m_carteAtout = carteJouee;
                m_idxPlayerWinning = playerIndex;
            }
        } else if (carteJouee->getCouleur() == m_couleurDemandee) {
            // Comparer avec la carte gagnante actuelle
            if (m_carteAtout == nullptr) {
                auto& winningPlayer = m_players[m_idxPlayerWinning].get();
                // Logique de comparaison simplifiée - à adapter
                m_idxPlayerWinning = playerIndex;
            }
        }
        
        // Retirer la carte de la main (vous devriez avoir une méthode pour ça)
        player->removeCard(cardIndex);

        // Rafraîchir l'affichage de la main
        refreshHand(playerIndex);
        
        // Passer au joueur suivant
        m_currentPlayer = (m_currentPlayer + 1) % 4;
        emit currentPlayerChanged();
        
        // Mettre à jour les cartes jouables pour le nouveau joueur
        updatePlayableStates();
        
        emit cardPlayed(playerIndex, cardIndex);
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
        m_idxPlayerWinning = -1;
        updatePlayableStates();
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

private:
    const std::vector<std::reference_wrapper<std::unique_ptr<Player>>>& m_players;
    HandModel* m_player0Hand;
    HandModel* m_player1Hand;
    HandModel* m_player2Hand;
    HandModel* m_player3Hand;
    int m_currentPlayer;
    
    // Contexte du pli en cours
    Carte::Couleur m_couleurDemandee;
    Carte::Couleur m_couleurAtout;
    Carte* m_carteAtout;
    int m_idxPlayerWinning;
};

#endif // GAMEMODEL_H