#include "GameManager.h"
#include <iostream>

GameManager::GameManager()
{
}

GameManager::GameManager(const std::vector<Player> &players, const Carte::Couleur &couleur, int idxFirstPlayer)
    : m_players(players)
    , m_couleurAnnoncee(couleur)
    , m_idxFirstPlayer(idxFirstPlayer)
{
}

GameManager::~GameManager()
{
}

void GameManager::runTurn()
{
    Carte *carte = nullptr;
    Carte *carteWinning = nullptr;
    Carte *carteAtout = nullptr;
    std::array<Carte *, 4> pli;
    int scoreTeam1 = 0;
    int scoreTeam2 = 0;
    int idxPlayerWinning = m_idxFirstPlayer;
    int scorePli = 0;
    for(int i = 0 ; i < 8 ; i++) {
        for(int j = m_idxFirstPlayer ; j < m_idxFirstPlayer + 4 ; j++) {
            if(j == m_idxFirstPlayer) {  // le premier joueur joue ce qu'il veut
                carte = m_players[j%4].playCarte();
                m_couleurDemandee = carte->getCouleur();
                carteWinning = new Carte(*carte);
            } else { // les autres doivent suivre a la couleur demandée
                carte = m_players[j%4].playCarte(m_couleurDemandee, m_couleurAnnoncee, carteAtout);
                if(carte->getCouleur() == m_couleurDemandee || carte->getCouleur() == m_couleurAnnoncee) {
                    if(*carteWinning < *carte) {
                        carteWinning = new Carte(*carte);
                        idxPlayerWinning = j%4;
                    }
                }
                
            }
            
            // si une carte d'atout est joué on la sauve pour devoir checker si on joue plus haut
            if(carte->getCouleur() == m_couleurAnnoncee) {  
                carteAtout = new Carte(*carte);
            }

            pli[j%4] = carte;
            scorePli += carte->getValeurDeLaCarte();
            if(idxPlayerWinning % 2 == 0) {
                scoreTeam1 += scorePli;
            } else {
                scoreTeam2 += scorePli;
            }

            // Pour le calcul du gagnant du pli, exclure les carte qui ne sont ni couleur demandée ni atout pour la comparaison
            
        }

        if(carteAtout != nullptr) {
            delete carteAtout;
            carteAtout = nullptr;
        }
        
        delete carteWinning;

        std::cout << "****************************************************" << std::endl;
        std::cout << "******************* FIN DE PLI ******************" << std::endl;
        std::cout << "Player: " << m_players[idxPlayerWinning].getName() << " gagne le pli" << std::endl;
        std::cout << "Il vaut score: " << scorePli << std::endl;
        m_players[idxPlayerWinning].addPli(pli);
        m_idxFirstPlayer = idxPlayerWinning;
        scorePli = 0;
    }

    std::cout << "****************************************************" << std::endl;
    std::cout << "******************* FIN DE MANCHE ******************" << std::endl;
    std::cout << " Score team 1 : " << scoreTeam1 << " - Score team 2 : " << scoreTeam2 << std::endl;


}

