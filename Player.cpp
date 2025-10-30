#include "Player.h"
#include "Carte.h"
#include <iostream>

Player::Player(std::string name, std::vector<Carte *> main, int index)
    : m_name(name)
    , m_main(main)
    , m_index(index)
    , m_annonce(ANNONCEINVALIDE) 
{
}

Player::~Player()
{
}

std::vector<Carte> Player::getCartes() const {
    std::vector<Carte> cartes;
    for (const auto* carte : m_main) {
        if (carte) {
            cartes.push_back(*carte);
        }
    }
    return cartes;
}

void Player::annonce(Annonce &annoncePrec, Carte::Couleur &couleurAnnonce)
{
    int annonce = 0;
    std::cout << "Player: " << m_name << std::endl;
    std::cout << "Annonces: 1: 80, 2: 90, 3: 100, 4: 110, 5:120, 6: 130, 7: 140, 8: 150, 9: 160, 10: Capot, 11: Generale, 12: Passe" << std::endl;
    std::cout << "Choisissez une valeur entre 1 et 12..." << std::endl;
    std::cin >> annonce;
    while(annoncePrec >= annonce || PASSE < annonce) {
        std::cout << "Annonce invalide, veuillez faire une annonce valide..." << std::endl;
        std::cin >> annonce;  
    }
    m_annonce = static_cast<Annonce>(annonce);
    if(static_cast<Annonce>(annonce) != PASSE) {
        annoncePrec = static_cast<Annonce>(annonce);
        int couleur = 0;
        std::cout << "Choississez la couleur: 1: Coeur, 2: Carreaux, 3: Trefle, 4: Pique" << std::endl;
        std::cin >> couleur;
        while(1 > couleur || couleur > 4) {
            std::cout << "Annonce invalide, veuillez faire une annonce valide..." << std::endl;
            std::cin >> couleur; 
        }
        m_couleur = static_cast<Carte::Couleur>(couleurAnnonce);
        m_couleur = static_cast<Carte::Couleur>(couleur+2);
    }
    
    
}

void Player::getAnnonce(Annonce &annonce) const
{
    annonce = m_annonce;
}

void Player::getCouleurAnnonce(Carte::Couleur &couleur) const
{
    couleur = m_couleur;
}

int Player::getIndex() const
{
    return m_index;
}

void Player::setIndex(int index)
{
    m_index = index;
}

void Player::printAnnonce() const
{   
    std::cout << "Annonce finale: ";
    if(m_annonce == QUATREVINGT)
        std::cout << "80 ";
    else if (m_annonce == QUATREVINGTDIX)
        std::cout << "90 ";
    else if (m_annonce == CENT)
        std::cout << "100 ";
    else if (m_annonce == CENTDIX)
        std::cout << "110 ";
    else if (m_annonce == CENTVINGT)
        std::cout << "120 ";
    else if (m_annonce == CENTTRENTRE)
        std::cout << "130 ";
    else if (m_annonce == CENTQUARANTE)
        std::cout << "140 ";
    else if (m_annonce == CENTCINQUANTE)
        std::cout << "150 ";
    else if (m_annonce == CENTSOIXANTE)
        std::cout << "160 ";
    else if (m_annonce == CAPOT)
        std::cout << "Capot ";
    else if (m_annonce == GENERALE)
        std::cout << "Generale ";

    if(m_couleur == Carte::COEUR)
        std::cout << "Coeur" << std::endl;
    else if(m_couleur == Carte::CARREAU)
        std::cout << "Carreau" << std::endl;
    else if(m_couleur == Carte::TREFLE)
        std::cout << "Trefle" << std::endl;
    else if(m_couleur == Carte::PIQUE)
        std::cout << "Pique" << std::endl;


}

void Player::printMain() const
{
    std::cout << "Main courante: ";
    for(auto &elt : m_main) {
        elt->printCarte();
    }
}

std::string Player::getName() const
{
    return m_name;
}

std::vector<Carte*> Player::getMain() const
{
    return m_main;
}

void Player::addPli(std::array<Carte *, 4> &pli)
{
    m_plis.push_back(pli);
}

bool Player::isCartePlayable(int carteIdx, const Carte::Couleur &couleurDemandee, 
                     const Carte::Couleur &couleurAtout, Carte* carteAtout, 
                     int idxPlayerWinning) const {

    std::cout << "Verification si la carte est jouable pour le jouer: " << m_name << " et carteIdx = " << carteIdx << std::endl;
    
    if (carteIdx < 0 || carteIdx >= m_main.size()) {
        return false;
        std::cout << "Index de carte invalide." << std::endl;
    }
    
    const Carte* carte = m_main[carteIdx];

    std::cout << "Couleur demandee: " << couleurDemandee << ", Couleur atout: " << couleurAtout << std::endl;
    // std::cout << "Carte atout: " << (carteAtout ? "existe" : "nullptr") << std::endl;
    // if(carteAtout) {
    //     std::cout << "Carte atout details - Couleur: " << carteAtout->getCouleur() << ", Chiffre: " << carteAtout->getChiffre() << std::endl;
    // }
    
    // Premier joueur peut jouer n'importe quoi
    if (couleurDemandee == Carte::COULEURINVALIDE) {
        std::cout << "Premier joueur, carte jouable." << std::endl;
        return true;
    }
    
    // Si la carte est de la couleur demandée, toujours jouable
    if (carte->getCouleur() == couleurDemandee) {
        std::cout << "Carte de la couleur demandee, carte jouable. Carte couleur : " << carte->getCouleur() << " carte chiffre : " << carte->getChiffre() << std::endl;
        return true;
    }

    if(m_main[carteIdx]->getCouleur() != couleurDemandee && hasCouleur(couleurDemandee) /*&& carteAtout != nullptr*/) {
        std::cout << "Vous avez la couleur demandee, veuillez selectionner une carte de cette couleur..." << std::endl;
        return false;
    } 
    else if(m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee) && 
            (idxPlayerWinning + 2)%4 == m_index) {
        // Si pas la couleur demandée mais que son partenaire tient le pli, alors le joueur peut se defausser
        return true;
    }
    else if(m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee) && 
            m_main[carteIdx]->getCouleur() != couleurAtout && hasCouleur(couleurAtout) ) {
        std::cout << "Vous avez de l'atout, veuillez selectionner une carte de cette couleur..." << std::endl;
        return false;
    } else if ((m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee) &&
                m_main[carteIdx]->getCouleur() == couleurAtout && carteAtout != nullptr)
                || couleurDemandee == couleurAtout) {
        if(*carteAtout < *m_main[carteIdx]) {
            return true;
        } else {
            if(hasHigher(carteAtout)) {
                std::cout << "Vous avez un atout plus fort que l'atout joue precedemment, " << std::endl;
                std::cout << "veuillez selectionner un atout plus fort..." << std::endl;
                return false;
            } else {
                return true;
            }
        }
    } else {
        return true;
    }
        
    // Si on n'a pas la couleur demandée
    // if (!hasCouleur(couleurDemandee)) {
    //     // Si c'est notre partenaire qui gagne, on peut se défausser
    //     if ((idxPlayerWinning + 2) % 4 == m_index) {
    //         std::cout << "Partenaire gagnant, carte jouable." << std::endl;
    //         return true;
    //     }
        
    //     // Si on doit couper
    //     if (carte->getCouleur() == couleurAtout) {
    //         // Si pas d'atout joué, n'importe quel atout est ok
    //         if (carteAtout == nullptr) {
    //             std::cout << "Pas d'atout joue precedemment, carte jouable." << std::endl;
    //             return true;
    //         }
    //         // Si atout joué, on doit monter si possible
    //         if (*carteAtout < *carte) {
    //             std::cout << "Atout joue precedemment, mais on peut monter, carte jouable." << std::endl;
    //             return true;
    //         }
    //         // Si on ne peut pas monter, vérifier qu'on n'a pas mieux
    //         if (!hasHigher(carteAtout)) {
    //             std::cout << "Atout joue precedemment, on ne peut pas monter, carte jouable." << std::endl;
    //             return true;
    //         }
    //         // Sinon on ne peut pas jouer cette carte
    //         std::cout << "Atout joue precedemment, on peut monter, carte non jouable." << std::endl;
    //         return false;
    //     }
        
    //     // Si on n'a pas l'atout mais on devrait en avoir
    //     if (hasCouleur(couleurAtout)) {
    //         std::cout << "On doit couper avec un atout, carte non jouable." << std::endl;
    //         return false;
    //     }
        
    //     // Sinon on peut se défausser
    //     //return true;
    // }
    
    // // On a la couleur mais on ne la joue pas
    // return false;
}

void Player::setAtout(const Carte::Couleur &couleurAtout)
{
    for(auto &carte : m_main) {
        if(carte->getCouleur() == couleurAtout) {
            carte->setAtout(true);
        } else {
            carte->setAtout(false);
        }
    }
}

void Player::removeCard(int cardIndex)
{
    if (cardIndex >= 0 && cardIndex < m_main.size()) {
        m_main.erase(m_main.begin() + cardIndex);
    }
}

// Carte* Player::playCarte(int carteIdx)
// {
//     //int carteIdx = 0;
    
//     //printMain();
//     //std::cout << "Selectionnez: " << std::endl;
//     int i = 0;
//     for(auto &elt : m_main) {
//         std::cout << i << ": ";
//         elt->printCarte();
//         i++;
//     }
//     //std::cin >> carteIdx;
//     Carte* carteJouee = m_main[carteIdx];  // On retourne directement la carte existante
//     m_main.erase(m_main.begin()+carteIdx);
//     //printMain();
//     return carteJouee;
// }

// Carte* Player::playCarte(const Carte::Couleur &couleurDemandee, const Carte::Couleur &couleurAtout, Carte* carteAtout, int idxPlayerWinning)
// {
//     int carteIdx = 0;
//     bool validSelection = false;
    
//     //printMain();
//     std::cout << "Joueur << " << m_name << ", jouez une carte..." << std::endl;
//     std::cout << "Couleur demandee : " << couleurDemandee << std::endl;
//     std::cout << "Selectionnez: " << std::endl;
//     int i = 0;
//     for(auto &elt : m_main) {
//         std::cout << i << ": ";
//         elt->printCarte();
//         i++;
//     }
//     while(!validSelection) {
//         std::cin >> carteIdx;
//         // std::cout << "m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee): " << (m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee)) << std::endl;
//         // std::cout << "m_main[carteIdx]->getCouleur() != couleurAtout" << (m_main[carteIdx]->getCouleur() != couleurAtout) << std::endl;
//         // std::cout << "hasCouleur(couleurAtout): " << hasCouleur(couleurAtout) << std::endl;

//         // Vérifier d'abord que l'index est valide
//         if (carteIdx < 0 || carteIdx >= m_main.size()) {
//             std::cout << "Index invalide, veuillez choisir un index entre 0 et " << (m_main.size()-1) << std::endl;
//             continue;
//         }

//         std::cout << "idxPlayerWinning: " << idxPlayerWinning << ", m_index: " << m_index << std::endl;
//         std::cout << "idxPlayerWinning + 2)%4: " << (idxPlayerWinning + 2)%4 << std::endl;

//         if(m_main[carteIdx]->getCouleur() != couleurDemandee && hasCouleur(couleurDemandee) /*&& carteAtout != nullptr*/) {
//             std::cout << "Vous avez la couleur demandee, veuillez selectionner une carte de cette couleur..." << std::endl;
//         } 
//         else if(m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee) && 
//                 (idxPlayerWinning + 2)%4 == m_index) {
//             // Si pas la couleur demandée mais que son partenaire tient le pli, alors le joueur peut se defausser
//             validSelection = true;
//         }
//         else if(m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee) && 
//         m_main[carteIdx]->getCouleur() != couleurAtout && hasCouleur(couleurAtout) ) {
//              std::cout << "Vous avez de l'atout, veuillez selectionner une carte de cette couleur..." << std::endl;
//         } else if ((m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee) &&
//         m_main[carteIdx]->getCouleur() == couleurAtout && carteAtout != nullptr)
//         || couleurDemandee == couleurAtout) {
//             if(*carteAtout < *m_main[carteIdx]) {
//                 validSelection = true;
//             } else {
//                 if(hasHigher(carteAtout)) {
//                     std::cout << "Vous avez un atout plus fort que l'atout joue precedemment, " << std::endl;
//                     std::cout << "veuillez selectionner un atout plus fort..." << std::endl;
//                 } else {
//                     validSelection = true;
//                 }
//             }
//         } else {
//             validSelection = true;
//         }
//     }

//     Carte* carteJouee = m_main[carteIdx];  // On récupère directement le pointeur
//     m_main.erase(m_main.begin()+carteIdx);  // On retire la carte de la main
//     carteJouee->printCarte();
//     return carteJouee;  // On retourne le pointeur original
// }

bool Player::hasCouleur(const Carte::Couleur &couleur) const
{
    bool haveCouleur = false;
    for(auto &carte : m_main) {
        if(carte->getCouleur() == couleur) {
            haveCouleur = true;
            break;
        }
    }
    return haveCouleur;
}

bool Player::hasHigher(Carte *carte) const
{
    bool haveHigher = false;
    for(auto &elt : m_main) {
        elt->printCarte();
        carte->printCarte();
        if(*carte < *elt) {
            std::cout << " BEEEEEEEEEN higher" << std::endl;
            haveHigher = true;
            break;
        }
    }
    return haveHigher;
}
