#include "GameManager.h"
#include <iostream>
#include <array>

GameManager::GameManager()
{
}

GameManager::GameManager(const std::vector<std::reference_wrapper<std::unique_ptr<Player>>> &players, const Carte::Couleur &couleur, int idxFirstPlayer)
    : m_players(players)
    , m_couleurAnnoncee(couleur)
    , m_idxFirstPlayer(idxFirstPlayer)
{
}

GameManager::~GameManager()
{
}

// void GameManager::playCard(int playerIndex, int cardIndex)
// {
//     if (playerIndex < 0 || playerIndex >= m_players.size())
//         return;

//     auto& player = m_players[playerIndex];
//     if (!player.get())
//         return;

//     // Vérifie si c'est le tour du joueur
//     if (playerIndex != m_currentPlayerIndex)
//         return;

//     Carte* carteJouee = nullptr;

//     // Utilise la logique existante de playCarte
//     // if (playerIndex == m_idxFirstPlayer) {
//     //     // Le premier joueur peut jouer n'importe quelle carte
//     //     carteJouee = player.get()->playCarte();
//     // } else {
//     //     carteJouee = player.get()->playCarte(m_couleurDemandee, m_couleurAnnoncee, nullptr, m_idxFirstPlayer);
//     // }
    
//     if (carteJouee) {
//         // La carte a été jouée avec succès
//         emit cardPlayed(playerIndex, *carteJouee);
        
//         // Met à jour le joueur courant
//         m_currentPlayerIndex = (m_currentPlayerIndex + 1) % m_players.size();
//         emit currentPlayerChanged();
//     }
// }

// void GameManager::runTurn()
// {
//     Carte *carte = nullptr;
//     Carte *carteWinning = nullptr;
//     Carte *carteAtout = nullptr;
//     std::array<Carte *, 4> pli;
//     int scoreTeam1 = 0;
//     int scoreTeam2 = 0;
//     int idxPlayerWinning = m_idxFirstPlayer;
//     int scorePli = 0;
//     int nbPli = m_players[0].get()->getMain().size();

//     // Pour chaque pli
//     for(int i = 0 ; i < nbPli; i++) {
//         carteWinning = nullptr;
//         carteAtout = nullptr;
//         scorePli = 0;

//         // Pour chaque joueur dans le pli
//         for(int j = m_idxFirstPlayer ; j < m_idxFirstPlayer + 4 ; j++) {
//             if(j == m_idxFirstPlayer) {  // le premier joueur joue ce qu'il veut
//                 carte = m_players[j%4].get()->playCarte();
//                 m_couleurDemandee = carte->getCouleur();
//                 carteWinning = carte;  // On garde une référence à la carte originale
//                 idxPlayerWinning = j%4;
//             } else { // les autres doivent suivre à la couleur demandée
//                 carte = m_players[j%4].get()->playCarte(m_couleurDemandee, m_couleurAnnoncee, carteAtout, idxPlayerWinning);
//                 if(carte->getCouleur() == m_couleurDemandee || carte->getCouleur() == m_couleurAnnoncee) {
//                     if(carteWinning && *carteWinning < *carte) {
//                         carteWinning = carte;  // On garde une référence à la carte originale
//                         idxPlayerWinning = j%4;
//                     }
//                 }
//             }
            
//             // si une carte d'atout est jouée, on la garde comme référence
//             if(carte->getCouleur() == m_couleurAnnoncee && carteAtout == nullptr) {  
//                 carteAtout = carte;  // On garde une référence à la carte originale
//                 std::cout << "ATOUT JOUE: " << std::endl;
//                 carteAtout->printCarte();
//             } else if(carte->getCouleur() == m_couleurAnnoncee && carteAtout != nullptr) {
//                 if(*carteAtout < *carte) {
//                     carteAtout = carte;  // On garde une référence à la carte originale
//                     std::cout << "ATOUT JOUE SUPERIEUR AU PRECEDENT: " << std::endl;
//                     carteAtout->printCarte();
//                 }
//             }

//             // Stocke la carte dans le pli
//             pli[j%4] = carte;
//             scorePli += carte->getValeurDeLaCarte();
//         }

//         std::cout << "****************************************************" << std::endl;
//         std::cout << "******************* FIN DE PLI ******************" << std::endl;
//         std::cout << "Player: " << m_players[idxPlayerWinning].get()->getName() << " gagne le pli" << std::endl;
//         std::cout << "Il vaut score: " << scorePli << std::endl;
        
//         // Mise à jour des scores
//         if(idxPlayerWinning % 2 == 0) {
//             scoreTeam1 += scorePli;
//         } else {
//             scoreTeam2 += scorePli;
//         }

//         // Donner le pli au gagnant
//         m_players[idxPlayerWinning].get()->addPli(pli);
//         m_idxFirstPlayer = idxPlayerWinning;
//     }

//     std::cout << "****************************************************" << std::endl;
//     std::cout << "******************* FIN DE MANCHE ******************" << std::endl;
//     std::cout << " Score team 1 : " << scoreTeam1 << " - Score team 2 : " << scoreTeam2 << std::endl;
// }

