#include <gtest/gtest.h>
#include "../Player.h"

class PlayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::vector<Carte*> main;
        // Créer quelques cartes pour la main initiale
        main.push_back(new Carte(Carte::COEUR, Carte::AS));
        main.push_back(new Carte(Carte::PIQUE, Carte::ROI));
        main.push_back(new Carte(Carte::CARREAU, Carte::DAME));
        player = new Player("TestPlayer", main, 0);
    }

    void TearDown() override {
        delete player;
    }

    Player* player;
};

TEST_F(PlayerTest, ConstructeurInitialiseCorrectement) {
    EXPECT_EQ(player->getName(), "TestPlayer");
    EXPECT_EQ(player->getIndex(), 0);
}

// TEST_F(PlayerTest, PlayCarteRetourneUneCarte) {
//     Carte* carte = player->playCarte();
//     EXPECT_NE(carte, nullptr);
// }

// TEST_F(PlayerTest, PlayCarteAvecCouleur) {
//     Carte* carteAtout = new Carte(Carte::COEUR, Carte::AS);
//     Carte* carte = player->playCarte(Carte::CARREAU, Carte::COEUR, carteAtout);
//     EXPECT_NE(carte, nullptr);
//     delete carteAtout;
// }

// TEST_F(PlayerTest, AnnonceTest) {
//     Player::Annonce annoncePrec = Player::PASSE;
//     Carte::Couleur couleurAnnonce = Carte::COEUR;
    
//     player->annonce(annoncePrec, couleurAnnonce);
    
//     Player::Annonce nouvelleAnnonce;
//     player->getAnnonce(nouvelleAnnonce);
    
//     // L'annonce doit être valide (différente de ANNONCEINVALIDE)
//     EXPECT_NE(nouvelleAnnonce, Player::ANNONCEINVALIDE);
// }

// TEST_F(PlayerTest, GetSetIndex) {
//     int newIndex = 2;
//     player->setIndex(newIndex);
//     EXPECT_EQ(player->getIndex(), newIndex);
// }

// TEST_F(PlayerTest, AjoutPli) {
//     std::array<Carte*, 4> pli = {
//         new Carte(Carte::COEUR, Carte::AS),
//         new Carte(Carte::PIQUE, Carte::ROI),
//         new Carte(Carte::CARREAU, Carte::DAME),
//         new Carte(Carte::TREFLE, Carte::VALET)
//     };
    
//     EXPECT_NO_THROW(player->addPli(pli));
    
//     // Nettoyage
//     for (Carte* carte : pli) {
//         delete carte;
//     }
// }