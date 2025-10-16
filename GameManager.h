#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "Carte.h"
#include "Player.h"
#include <vector>

class GameManager
{
    public:
        GameManager();

        GameManager(const std::vector<Player> &players, const Carte::Couleur &couleur, int idxFirstPlayer);

        ~GameManager();

        void runTurn();

    private:
        Carte::Couleur m_couleurAnnoncee;
        Carte::Couleur m_couleurDemandee;
        std::vector<Player> m_players;
        int m_idxFirstPlayer;


};

#endif