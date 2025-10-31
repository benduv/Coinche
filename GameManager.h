#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "Carte.h"
#include "Player.h"
#include <vector>
#include <memory>
#include <QObject>

class GameManager : public QObject
{
    // Q_OBJECT
    // Q_PROPERTY(Carte::Couleur couleurAnnoncee READ getCouleurAnnoncee NOTIFY couleurAnnonceeChanged)
    // Q_PROPERTY(Carte::Couleur couleurDemandee READ getCouleurDemandee NOTIFY couleurDemandeeChanged)
    // Q_PROPERTY(int currentPlayerIndex READ getCurrentPlayerIndex NOTIFY currentPlayerChanged)
    public:
        GameManager();

        GameManager(const std::vector<std::reference_wrapper<std::unique_ptr<Player>>> &players, const Carte::Couleur &couleur, int idxFirstPlayer);

        ~GameManager();

        //void runTurn();

    // signals:
    //     void couleurAnnonceeChanged();
    //     void couleurDemandeeChanged();
    //     void currentPlayerChanged();
    //     void cardPlayed(int playerIndex, const Carte &carte);
    //     void turnCompleted();

    // public slots:
    //     void playCard(int playerIndex, int cardIndex);
        
    public:
        // Carte::Couleur getCouleurAnnoncee() const { return m_couleurAnnoncee; }
        // Carte::Couleur getCouleurDemandee() const { return m_couleurDemandee; }
        int getCurrentPlayerIndex() const { return m_currentPlayerIndex; }

    private:
        Carte::Couleur m_couleurAnnoncee;
        Carte::Couleur m_couleurDemandee;
        std::vector<std::reference_wrapper<std::unique_ptr<Player>>> m_players;
        int m_idxFirstPlayer;
        int m_currentPlayerIndex;


};

#endif