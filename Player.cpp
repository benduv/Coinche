#include "Player.h"
#include "Carte.h"
#include <iostream>
#include <algorithm>

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
    else if (m_annonce == CENTTRENTE)
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

void Player::addCardToHand(Carte* carte)
{
    m_main.push_back(carte);
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

int Player::convertAnnonceEnPoint(const Annonce &annonce)
{
    switch (annonce)
    {
    case Annonce::QUATREVINGT:
        return 80;
        break;
    case Annonce::QUATREVINGTDIX:
        return 90;
        break;
    case Annonce::CENT:
        return 100;
        break;
    case Annonce::CENTDIX:
        return 110;
        break;
    case Annonce::CENTVINGT:
        return 120;
        break;
    case Annonce::CENTTRENTE:
        return 130;
        break;
    case Annonce::CENTQUARANTE:
        return 140; 
        break;
    case Annonce::CENTCINQUANTE:
        return 150;
        break;
    case Annonce::CENTSOIXANTE:
        return 160;
        break;
    case Annonce::CAPOT:
        return 250;
        break;
    case Annonce::GENERALE:
        return 500;
        break;    
    default:
        return 0;
        break;
    }
}

bool Player::isCartePlayable(int carteIdx, const Carte::Couleur &couleurDemandee, 
                     const Carte::Couleur &couleurAtout, Carte* carteAtout, 
                     int idxPlayerWinning) const {

    std::cout << "Verification si la carte est jouable pour le jouer: " << m_name << " et carteIdx = " << carteIdx << std::endl;
    std::cout << "******************************************************* " << std::endl;
    std::cout << "*****************************idxPlayerWinning: " << idxPlayerWinning << " m_index: " << m_index << std::endl; 
    
    if (carteIdx < 0 || carteIdx >= m_main.size()) {
        return false;
        std::cout << "Index de carte invalide." << std::endl;
    }
    
    const Carte* carte = m_main[carteIdx];
    carte->printCarte();

    std::cout << "Couleur demandee: " << couleurDemandee << ", Couleur atout: " << couleurAtout << std::endl;
    
    // Premier joueur peut jouer n'importe quoi
    if (couleurDemandee == Carte::COULEURINVALIDE) {
        std::cout << "Premier joueur, carte jouable." << std::endl;
        return true;
    }
    
    // Si la carte est de la couleur demandée, toujours jouable
    if (carte->getCouleur() == couleurDemandee) {
        //std::cout << "Carte de la couleur demandee, carte jouable. Carte couleur : " << carte->getCouleur() << " carte chiffre : " << carte->getChiffre() << std::endl;
        if(carte->getCouleur() == couleurAtout) {
            //std::cout << "C'est aussi un atout." << std::endl;
            if(carteAtout != nullptr) {
                if(*carteAtout < *carte) {
                    //std::cout << "Atout joue precedemment, mais on peut monter, carte jouable." << std::endl;
                } else {
                    if(hasHigher(carteAtout)) {
                        //std::cout << "Vous avez un atout plus fort que l'atout joue precedemment, " << std::endl;
                        //std::cout << "veuillez selectionner un atout plus fort..." << std::endl;
                        return false;
                    } else {
                        //std::cout << "Atout joue precedemment, on ne peut pas monter, carte jouable." << std::endl;
                    }
                }
            }
        }
        return true;
    }

    if(m_main[carteIdx]->getCouleur() != couleurDemandee && hasCouleur(couleurDemandee)) {
        //std::cout << "Vous avez la couleur demandee, veuillez selectionner une carte de cette couleur..." << std::endl;
        return false;
    } 
    else if(m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee) && 
            (idxPlayerWinning + 2)%4 == m_index) {
        // Si pas la couleur demandée mais que son partenaire tient le pli, alors le joueur peut se defausser
        return true;
    }
    else if(m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee) && 
            m_main[carteIdx]->getCouleur() != couleurAtout && hasCouleur(couleurAtout) ) {
        //std::cout << "Vous avez de l'atout, veuillez selectionner une carte de cette couleur..." << std::endl;
        return false;
    } else if ((m_main[carteIdx]->getCouleur() != couleurDemandee && !hasCouleur(couleurDemandee) &&
                m_main[carteIdx]->getCouleur() == couleurAtout && carteAtout != nullptr)
                || couleurDemandee == couleurAtout) {
        if(*carteAtout < *m_main[carteIdx]) {
            return true;
        } else {
            if(hasHigher(carteAtout)) {
                //std::cout << "Vous avez un atout plus fort que l'atout joue precedemment, " << std::endl;
                //std::cout << "veuillez selectionner un atout plus fort..." << std::endl;
                return false;
            } else {
                return true;
            }
        }
    } else {
        return true;
    }
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

    if(hasBelotte(couleurAtout)) {
        std::cout << "Le joueur " << m_name << " a la belote!" << std::endl;
        m_hasBelotte = true;
    }
}

bool Player::hasBelotte(const Carte::Couleur &couleurAtout) const
{
    bool hasRoiAtout = false;
    bool hasDameAtout = false;
    for (const auto* carte : m_main) {
        if (carte) {
            if (carte->getChiffre() == Carte::ROI && carte->getCouleur() == couleurAtout) {
                hasRoiAtout = true;
            } else if (carte->getChiffre() == Carte::DAME && carte->getCouleur() == couleurAtout) {
                hasDameAtout = true;
            }
        }
    }
    return hasRoiAtout && hasDameAtout;
}

bool Player::getHasBelotte() const
{
    return m_hasBelotte;
}

void Player::removeCard(int cardIndex)
{
    if (cardIndex >= 0 && cardIndex < m_main.size()) {
        m_main.erase(m_main.begin() + cardIndex);
    }
}

void Player::clearHand()
{
    m_main.clear();
    m_hasBelotte = false;
}

void Player::sortHand()
{
    std::sort(m_main.begin(), m_main.end(), [](Carte* a, Carte* b) {
        if (a->getCouleur() == b->getCouleur()) {
            return *a < *b;
        }
        return a->getCouleur() < b->getCouleur();
    });
}

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
            haveHigher = true;
            break;
        }
    }
    return haveHigher;
}
