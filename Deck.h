#ifndef DECK_H
#define DECK_H

#include <vector>
#include "Carte.h"

 class Deck {
    public:
        Deck(/*Carte::Couleur atoutCouleur*/);
        ~Deck();

        void printDeck();
        void shuffleDeck();
        void resetDeck();
        int size() const { return m_deck.size(); }
        Carte* drawCard();
        void setAtout(Carte::Couleur atoutCouleur);
        void distribute(std::vector<Carte*> &main1, std::vector<Carte*> &main2, std::vector<Carte*> &main3, std::vector<Carte*> &main4);

    private:
        std::vector<Carte *> m_deck;
        //Carte::Couleur m_atoutCouleur;
 };

#endif