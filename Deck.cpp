#include <iostream>
#include "Deck.h"
#include <Windows.h>
#include <algorithm>
#include <random>

Deck::Deck(/*Carte::Couleur atoutCouleur*/)
    //: m_atoutCouleur(atoutCouleur)
{
    for(Carte::Chiffre ch =  Carte::SEPT ; ch <= Carte::AS ; ch = static_cast<Carte::Chiffre>(static_cast<int>(ch) + 1))
    {        
        for(Carte::Couleur co = Carte::COEUR ; co <= Carte::PIQUE ; co = static_cast<Carte::Couleur>(static_cast<int>(co) + 1))
        {
            Carte* carte = nullptr;
            
            /*if(m_atoutCouleur == co)
                carte = new Atout(co, ch);
            else*/
            carte = new Carte(co, ch);
            //carte->printCarte();
            deck.push_back(carte);
        }
    }
}

Deck::~Deck()
{
    for(Carte* carte : deck) {
        delete carte;
    }
    deck.clear();
}

void Deck::printDeck()
{
    for(Carte* c : deck) {
        c->printCarte();
    }
}

void Deck::shuffleDeck()
{
    auto rng = std::default_random_engine {};
    std::shuffle(std::begin(deck), std::end(deck), rng);
}

void Deck::setAtout(Carte::Couleur atoutCouleur)
{
    for(auto & elt : deck) {
        if(elt->getCouleur() == atoutCouleur) {
            elt->setAtout(true);
        }
    }

    /*for(auto & elt : deck) {
        std::cout << "Valeur de la carte: " << elt->getValeurDeLaCarte() << std::endl; 
    }*/
}

void Deck::distribute(std::vector<Carte *> &main1, std::vector<Carte *> &main2, std::vector<Carte *> &main3, std::vector<Carte *> &main4)
{
    int cpt = 1;
    for(auto & elt : deck) {
        if(cpt % 4 == 1) {
            main1.push_back(elt);
        } else if (cpt % 4 == 2) {
            main2.push_back(elt);
        } else if (cpt % 4 == 3) {
            main3.push_back(elt);
        } else {
            main4.push_back(elt);
        }
        cpt++;
    }

    
    for(auto elt : main1) {
        std::cout << " MAIN 1" << std::endl;
        elt->printCarte();
    }
    
    for(auto elt : main2) {
        std::cout << " MAIN 2" << std::endl;
        elt->printCarte();
    }
    
    for(auto elt : main3) {
        std::cout << " MAIN 3" << std::endl;
        elt->printCarte();
    }
    
    for(auto elt : main4) {
        std::cout << " MAIN 4" << std::endl;
        elt->printCarte();
    }
}
