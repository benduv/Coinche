#include <iostream>
#include "Deck.h"
#include <Windows.h>
#include <algorithm>
#include <random>

Deck::Deck()
{
    for(Carte::Chiffre ch =  Carte::SEPT ; ch <= Carte::AS ; ch = static_cast<Carte::Chiffre>(static_cast<int>(ch) + 1))
    {        
        for(Carte::Couleur co = Carte::COEUR ; co <= Carte::PIQUE ; co = static_cast<Carte::Couleur>(static_cast<int>(co) + 1))
        {
            Carte* carte = nullptr;
        
            carte = new Carte(co, ch);
            m_deck.push_back(carte);
        }
    }
}

Deck::~Deck()
{
    for(Carte* carte : m_deck) {
        delete carte;
    }
    m_deck.clear();
}

void Deck::printDeck()
{
    for(Carte* c : m_deck) {
        c->printCarte();
    }
}

void Deck::shuffleDeck()
{
    // auto rng = std::default_random_engine {};
    // std::shuffle(std::begin(m_deck), std::end(m_deck), rng);
    std::random_device rd;  // Utilise une source aléatoire fournie par le système
    std::mt19937 rng(rd()); // Utilisez le générateur Mersenne Twister avec la graine de random_device
    std::shuffle(std::begin(m_deck), std::end(m_deck), rng);
}

Carte* Deck::drawCard()
{
    if(m_deck.empty()) {
        return nullptr;
    }
    Carte* carte = m_deck.back();
    m_deck.pop_back();
    return carte;
}

void Deck::setAtout(Carte::Couleur atoutCouleur)
{
    for(auto & elt : m_deck) {
        if(elt->getCouleur() == atoutCouleur) {
            elt->setAtout(true);
        }
    }

    /*for(auto & elt : deck) {
        std::cout << "Valeur de la carte: " << elt->getValeurDeLaCarte() << std::endl; 
    }*/
}

void Deck::resetDeck()
{
    // Supprimer les cartes existantes
    for(Carte* carte : m_deck) {
        delete carte;
    }
    m_deck.clear();

    // Recréer le deck complet
    for(Carte::Chiffre ch =  Carte::SEPT ; ch <= Carte::AS ; ch = static_cast<Carte::Chiffre>(static_cast<int>(ch) + 1))
    {        
        for(Carte::Couleur co = Carte::COEUR ; co <= Carte::PIQUE ; co = static_cast<Carte::Couleur>(static_cast<int>(co) + 1))
        {
            Carte* carte = nullptr;
        
            carte = new Carte(co, ch);
            m_deck.push_back(carte);
        }
    }
}

void Deck::distribute(std::vector<Carte *> &main1, std::vector<Carte *> &main2, std::vector<Carte *> &main3, std::vector<Carte *> &main4)
{
    int cpt = 1;
    for(auto & elt : m_deck) {
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

    
    // for(auto elt : main1) {
    //     std::cout << " MAIN 1" << std::endl;
    //     elt->printCarte();
    // }
    
    // for(auto elt : main2) {
    //     std::cout << " MAIN 2" << std::endl;
    //     elt->printCarte();
    // }
    
    // for(auto elt : main3) {
    //     std::cout << " MAIN 3" << std::endl;
    //     elt->printCarte();
    // }
    
    // for(auto elt : main4) {
    //     std::cout << " MAIN 4" << std::endl;
    //     elt->printCarte();
    // }
}

void Deck::rebuildFromCards(const std::vector<Carte*>& cards)
{
    // Ne pas supprimer les cartes car elles sont réutilisées
    m_deck.clear();

    // Ajouter toutes les cartes dans l'ordre où elles ont été jouées
    for (Carte* carte : cards) {
        m_deck.push_back(carte);
    }
}

void Deck::cutDeck()
{
    if (m_deck.size() <= 1) return;

    // Choisir un point de coupe aléatoire (entre 1 et taille-1)
    int cutPoint = (rand() % (m_deck.size() - 1)) + 1;

    // Créer deux parties
    std::vector<Carte*> topPart(m_deck.begin(), m_deck.begin() + cutPoint);
    std::vector<Carte*> bottomPart(m_deck.begin() + cutPoint, m_deck.end());

    // Placer la partie du haut en dessous
    m_deck.clear();
    m_deck.insert(m_deck.end(), bottomPart.begin(), bottomPart.end());
    m_deck.insert(m_deck.end(), topPart.begin(), topPart.end());
}

void Deck::distribute323(std::vector<Carte*> &main1, std::vector<Carte*> &main2, std::vector<Carte*> &main3, std::vector<Carte*> &main4)
{
    int cardIndex = 0;

    // Premier tour : 3 cartes par joueur
    for (int i = 0; i < 3; i++) {
        main1.push_back(m_deck[cardIndex++]);
        main2.push_back(m_deck[cardIndex++]);
        main3.push_back(m_deck[cardIndex++]);
        main4.push_back(m_deck[cardIndex++]);
    }

    // Deuxième tour : 2 cartes par joueur
    for (int i = 0; i < 2; i++) {
        main1.push_back(m_deck[cardIndex++]);
        main2.push_back(m_deck[cardIndex++]);
        main3.push_back(m_deck[cardIndex++]);
        main4.push_back(m_deck[cardIndex++]);
    }

    // Troisième tour : 3 cartes par joueur
    for (int i = 0; i < 3; i++) {
        main1.push_back(m_deck[cardIndex++]);
        main2.push_back(m_deck[cardIndex++]);
        main3.push_back(m_deck[cardIndex++]);
        main4.push_back(m_deck[cardIndex++]);
    }
}
