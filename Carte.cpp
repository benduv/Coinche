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

Carte::Couleur Carte::getCouleur()
{
    return m_couleur;
}

Carte::Chiffre Carte::getChiffre()
{
    return m_chiffre;
}

int Carte::getValeurDeLaCarte()
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

void Carte::setAtout(bool isAtout)
{
    m_isAtout = isAtout;
}

void Carte::printCarte()
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

bool Carte::operator<(Carte &other) {
    bool retValue = false;
    if(m_isAtout == false && other.m_isAtout == true)
        retValue = true;
    else if(m_isAtout == true && other.m_isAtout == false)
        retValue = false;
    else {
        int valeurA = this->getValeurDeLaCarte();
        int valeurB = other.getValeurDeLaCarte();
        if(this->getValeurDeLaCarte() != other.getValeurDeLaCarte()) {
            retValue = this->getValeurDeLaCarte() < other.getValeurDeLaCarte();
        } else {
            retValue = this->m_chiffre < other.m_chiffre;
        }
    }

    return retValue;


}
