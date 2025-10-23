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

void Player::getAnnonce(Annonce &annonce)
{
    annonce = m_annonce;
}

void Player::getCouleurAnnonce(Carte::Couleur &couleur)
{
    couleur = m_couleur;
}

int Player::getIndex()
{
    return m_index;
}

void Player::setIndex(int index)
{
    m_index = index;
}

void Player::printAnnonce()
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

Carte* Player::playCarte()
{
    int carteIdx = 0;
    
    //printMain();
    std::cout << "Selectionnez: " << std::endl;
    int i = 0;
    for(auto &elt : m_main) {
        std::cout << i << ": ";
        elt->printCarte();
        i++;
    }
    std::cin >> carteIdx;
    Carte* carteJouee = m_main[carteIdx];  // On retourne directement la carte existante
    m_main.erase(m_main.begin()+carteIdx);
    //printMain();
    return carteJouee;
}

Carte* Player::playCarte(const Carte::Couleur &couleurDemandee, const Carte::Couleur &couleurAtout, Carte* carteAtout)
{
    int carteIdx = 0;
    bool validSelection = false;
    
    //printMain();
    std::cout << "Joueur << " << m_name << ", jouez une carte..." << std::endl;
    std::cout << "Couleur demandee : " << couleurDemandee << std::endl;
    std::cout << "Selectionnez: " << std::endl;
    int i = 0;
    for(auto &elt : m_main) {
        std::cout << i << ": ";
        elt->printCarte();
        i++;
    }
    while(!validSelection) {
        std::cin >> carteIdx;
        // std::cout << "m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee): " << (m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee)) << std::endl;
        // std::cout << "m_main[carteIdx]->getCouleur() != couleurAtout" << (m_main[carteIdx]->getCouleur() != couleurAtout) << std::endl;
        // std::cout << "hasCouleur(couleurAtout): " << hasCouleur(couleurAtout) << std::endl;

        if(m_main[carteIdx]->getCouleur() != couleurDemandee && hasCouleur(couleurDemandee) /*&& carteAtout != nullptr*/) {
            std::cout << "Vous avez la couleur demandee, veuillez selectionner une carte de cette couleur..." << std::endl;
        } else if(m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee) && 
        m_main[carteIdx]->getCouleur() != couleurAtout && hasCouleur(couleurAtout) ) {
             std::cout << "Vous avez de l'atout, veuillez selectionner une carte de cette couleur..." << std::endl;
        } else if ((m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee) &&
        m_main[carteIdx]->getCouleur() == couleurAtout && carteAtout != nullptr)
        || couleurDemandee == couleurAtout) {
            std::cout << "BEEEEEEEEEEEN1 " << std::endl;
            if(*carteAtout < *m_main[carteIdx]) {
                std::cout << "BEEEEEEEEEEEN2 " << std::endl;
                validSelection = true;
            } else {
                std::cout << "BEEEEEEEEEEEN3 " << std::endl;
                if(hasHigher(carteAtout)) {
                    std::cout << "Vous avez un atout plus fort que l'atout joue precedemment, " << std::endl;
                    std::cout << "veuillez selectionner un atout plus fort..." << std::endl;
                } else {
                    validSelection = true;
                }
            }
        } else {
            validSelection = true;
        }
    }


    Carte* carteJouee = m_main[carteIdx];  // On récupère directement le pointeur
    m_main.erase(m_main.begin()+carteIdx);  // On retire la carte de la main
    carteJouee->printCarte();
    //printMain();
    return carteJouee;  // On retourne le pointeur original
}

bool Player::hasCouleur(const Carte::Couleur &couleur)
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

bool Player::hasHigher(Carte *carte)
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
