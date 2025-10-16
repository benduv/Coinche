#include <iostream>
#include "Deck.h"
#include "Carte.h"
#include "Player.h"
#include "GameManager.h"


int main() {
    //Carte carte1 = Carte(Carte::PIQUE, Carte::ROI);
    //std::cout << "Carte : " << carte1.getCouleur() << " , " << carte1.getChiffre() << std::endl;
    Deck deck = Deck(/*Carte::CARREAU*/);
    deck.shuffleDeck();
    deck.printDeck();

    std::vector<Carte*> main1, main2, main3, main4;
    deck.distribute(main1, main2, main3, main4);
    Player player1 = Player("Joueur1", main1, 0);
    Player player2 = Player("Joueur2", main2, 1);
    Player player3 = Player("Joueur3", main3, 2);
    Player player4 = Player("Joueur4", main4, 3);

    std::vector<Player> players;
    players.push_back(player1);
    players.push_back(player2);
    players.push_back(player3);
    players.push_back(player4);

    int incr = 0;
    Carte::Couleur couleurAnnonce;
    while(incr != 10) {

        int idxFirstPlayers = 0;
        for (auto & elt : players) {
            if(elt.getIndex() != 0) {
                idxFirstPlayers++;
            } else 
                break;
        }

        Player::Annonce annoncePrec = Player::ANNONCEINVALIDE;

        Carte::Couleur couleurAnnonce = Carte::CARREAU;

        int cpt = 0;
        int i = idxFirstPlayers;
        int joueurAyantLaPlusGrandeAnnonce = 0;
        Player::Annonce annonceCour = Player::ANNONCEINVALIDE;
        while (cpt < 3) { // Tant que tous les autres joueurs n'ont pas passé
            players[i%4].annonce(annoncePrec, couleurAnnonce);
            players[i%4].getAnnonce(annonceCour);
            if(annonceCour == Player::PASSE)
                cpt++;
            else {
                joueurAyantLaPlusGrandeAnnonce=i%4;
                cpt = 0;
            }
            i++;
        }

        players[joueurAyantLaPlusGrandeAnnonce%4].printAnnonce();

        players[joueurAyantLaPlusGrandeAnnonce%4].getCouleurAnnonce(couleurAnnonce);
        deck.setAtout(couleurAnnonce);
        deck.printDeck();

        //player1.printMain();

        // At the end of the turn, change the index of players so that index 0 is the first player
        for (auto & elt : players) {
            elt.setIndex((elt.getIndex() - 1) % 4);
        }

        GameManager gameManager = GameManager(players, couleurAnnonce, idxFirstPlayers);
        gameManager.runTurn();
    }
    return 0;
}