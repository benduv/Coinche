#include <iostream>
#include "Deck.h"
#include "Carte.h"
#include "Player.h"
#include "GameManager.h"
#include <memory>


int main() {
    //Carte carte1 = Carte(Carte::PIQUE, Carte::ROI);
    //std::cout << "Carte : " << carte1.getCouleur() << " , " << carte1.getChiffre() << std::endl;
    Deck deck = Deck(/*Carte::CARREAU*/);
    deck.shuffleDeck();
    deck.printDeck();

    std::vector<Carte*> main1, main2, main3, main4;
    deck.distribute(main1, main2, main3, main4);
    // Utilisation de unique_ptr pour la gestion automatique de la mémoire
    std::vector<std::unique_ptr<Player>> playersPtr;
    playersPtr.push_back(std::make_unique<Player>("Joueur1", main1, 0));
    playersPtr.push_back(std::make_unique<Player>("Joueur2", main2, 1));
    playersPtr.push_back(std::make_unique<Player>("Joueur3", main3, 2));
    playersPtr.push_back(std::make_unique<Player>("Joueur4", main4, 3));

    int incr = 0;
    Carte::Couleur couleurAnnonce;
    while(incr != 10) {

        int idxFirstPlayers = 0;
        for (const auto& player : playersPtr) {
            if(player->getIndex() != 0) {
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
            playersPtr[i%4]->annonce(annoncePrec, couleurAnnonce);
            playersPtr[i%4]->getAnnonce(annonceCour);
            if(annonceCour == Player::PASSE)
                cpt++;
            else {
                joueurAyantLaPlusGrandeAnnonce=i%4;
                cpt = 0;
            }
            i++;
        }

        playersPtr[joueurAyantLaPlusGrandeAnnonce%4]->printAnnonce();

        playersPtr[joueurAyantLaPlusGrandeAnnonce%4]->getCouleurAnnonce(couleurAnnonce);
        deck.setAtout(couleurAnnonce);
        deck.printDeck();

        //player1.printMain();

        // At the end of the turn, change the index of players so that index 0 is the first player
        for (auto& player : playersPtr) {
            player->setIndex((player->getIndex() - 1) % 4);
        }

        // Créer un vecteur de références aux unique_ptr pour GameManager
        std::vector<std::reference_wrapper<std::unique_ptr<Player>>> playerRefs;
        for (auto& player : playersPtr) {
            playerRefs.push_back(std::ref(player));
        }

        GameManager gameManager = GameManager(playerRefs, couleurAnnonce, idxFirstPlayers);
        gameManager.runTurn();
    }
    return 0;
}