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
        void rebuildFromCards(const std::vector<Carte*>& cards);  // Reconstruit le deck à partir de cartes existantes
        void cutDeck();  // Coupe le deck (prend une partie aléatoire et la place en dessous)
        int size() const { return m_deck.size(); }
        Carte* drawCard();
        void setAtout(Carte::Couleur atoutCouleur);
        void distribute(std::vector<Carte*> &main1, std::vector<Carte*> &main2, std::vector<Carte*> &main3, std::vector<Carte*> &main4);
        void distribute323(std::vector<Carte*> &main1, std::vector<Carte*> &main2, std::vector<Carte*> &main3, std::vector<Carte*> &main4);  // Distribution 3-2-3

    private:
        std::vector<Carte *> m_deck;
        //Carte::Couleur m_atoutCouleur;
 };

#endif