#include <iostream>
#include "Carte.h"

Carte::Carte()
{
}

Carte::Carte(const Carte &carte)
    : m_couleur(carte.m_couleur)
    , m_chiffre(carte.m_chiffre)
    , m_isAtout(carte.m_isAtout)
{
}

Carte::Carte(Couleur couleur, Chiffre chiffre)
    : m_couleur(couleur), m_chiffre(chiffre)
{
}

Carte::~Carte()
{
}

Carte::Couleur Carte::getCouleur() const
{
    return m_couleur;
}

Carte::Chiffre Carte::getChiffre() const
{
    return m_chiffre;
}

int Carte::getValeurDeLaCarte() const
{
    int value = 0;
    if(!m_isAtout) {
        if(m_chiffre == VALET)
            value = 2;
        else if (m_chiffre == DAME)
            value = 3;
        else if (m_chiffre == ROI)
            value = 4;
        else if (m_chiffre == DIX)
            value = 10;
        else if (m_chiffre == AS)
            value = 11;
    } else {
        if(m_chiffre == VALET)
            value = 20;
        else if (m_chiffre == NEUF)
            value = 14;
        else if (m_chiffre == DAME)
            value = 3;
        else if (m_chiffre == ROI)
            value = 4;
        else if (m_chiffre == DIX)
            value = 10;
        else if (m_chiffre == AS)
            value = 11;
    }

    return value;
}

int Carte::getOrdreCarteForte() const
{
    int value = 0;
    if(m_chiffre == SEPT)
            value = 0;
    else if(m_chiffre == HUIT)
            value = 1;
    else if(!m_isAtout) {
        if(m_chiffre == NEUF)
            value = 2;
        if(m_chiffre == VALET)
            value = 3;
        else if (m_chiffre == DAME)
            value = 4;
        else if (m_chiffre == ROI)
            value = 5;
        else if (m_chiffre == DIX)
            value = 6;
        else if (m_chiffre == AS)
            value = 7;
    } else {
        if(m_chiffre == DAME)
            value = 2;
        else if (m_chiffre == ROI)
            value = 3;
        else if (m_chiffre == 10)
            value = 4;
        else if (m_chiffre == AS)
            value = 5;
        else if (m_chiffre == NEUF)
            value = 6;
        else if (m_chiffre == VALET)
            value = 7;
    }
    std::cout << "getOrdreCarteForte called: " << value << std::endl;
    return value;
}

void Carte::setAtout(bool isAtout)
{
    m_isAtout = isAtout;
}

void Carte::printCarte() const
{
    if(m_chiffre <= Carte::DIX)
        std::cout << m_chiffre << " ";
    else if(m_chiffre == Carte::VALET)
        std::cout << "V ";
    else if(m_chiffre == Carte::DAME)
        std::cout << "D ";
    else if(m_chiffre == Carte::ROI)
        std::cout << "R ";
    else
        std::cout << "A ";

    if(m_couleur == Carte::COEUR)
        std::cout << "Coeur - ";
    else if(m_couleur == Carte::CARREAU)
        std::cout << "Carreau - ";
    else if(m_couleur == Carte::TREFLE)
        std::cout << "Trefle - ";
    else if(m_couleur == Carte::PIQUE)
        std::cout << "Pique - ";

    if(m_isAtout == true) {
        std::cout << " Atout" << std::endl;
    } else {
        std::cout << " Pas Atout" << std::endl;
    }
}

bool Carte::operator<(const Carte &other) const {
    bool retValue = false;
    if(m_isAtout == false && other.m_isAtout == true)
        retValue = true;
    else if(m_isAtout == true && other.m_isAtout == false)
        retValue = false;
    else {
        std::cout << "Comparaison des cartes: " ;
        this->printCarte();
        std::cout << "getOrdreCarteForte = " << this->getOrdreCarteForte() << std::endl;
        other.printCarte();
        std::cout << "getOrdreCarteForte() = " << other.getOrdreCarteForte() << std::endl;        
        retValue = this->getOrdreCarteForte() < other.getOrdreCarteForte();
    }

    return retValue;


}
