#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "Carte.h"
#include "Player.h"
#include <vector>
#include <memory>

class GameManager
{
    public:
        GameManager();

        GameManager(const std::vector<std::reference_wrapper<std::unique_ptr<Player>>> &players, const Carte::Couleur &couleur, int idxFirstPlayer);

        ~GameManager();

        void runTurn();

    private:
        Carte::Couleur m_couleurAnnoncee;
        Carte::Couleur m_couleurDemandee;
        std::vector<std::reference_wrapper<std::unique_ptr<Player>>> m_players;  // Stocke des références aux unique_ptr
        int m_idxFirstPlayer;


};

#endif