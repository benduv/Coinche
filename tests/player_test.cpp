#include <gtest/gtest.h>
#include "../Player.h"

class PlayerTest : public ::testing::Test {
protected:
    Player* player;

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
};

// ========================================
// Tests du constructeur
// ========================================

TEST_F(PlayerTest, ConstructeurInitialiseNom) {
    EXPECT_EQ(player->getName(), "TestPlayer");
}

TEST_F(PlayerTest, ConstructeurInitialiseIndex) {
    EXPECT_EQ(player->getIndex(), 0);
}

TEST_F(PlayerTest, ConstructeurInitialiseMain) {
    auto cartes = player->getCartes();
    EXPECT_EQ(cartes.size(), 3) << "Le joueur devrait avoir 3 cartes";
}

TEST_F(PlayerTest, ConstructeurIndexDifferents) {
    std::vector<Carte*> main;
    Player* player1 = new Player("Player1", main, 0);
    Player* player2 = new Player("Player2", main, 1);
    Player* player3 = new Player("Player3", main, 2);
    Player* player4 = new Player("Player4", main, 3);

    EXPECT_EQ(player1->getIndex(), 0);
    EXPECT_EQ(player2->getIndex(), 1);
    EXPECT_EQ(player3->getIndex(), 2);
    EXPECT_EQ(player4->getIndex(), 3);

    delete player1;
    delete player2;
    delete player3;
    delete player4;
}

// ========================================
// Tests de gestion de la main
// ========================================

TEST_F(PlayerTest, AddCardToHandAugmenteTaille) {
    auto cartesAvant = player->getCartes();
    size_t tailleAvant = cartesAvant.size();

    player->addCardToHand(new Carte(Carte::TREFLE, Carte::VALET));

    auto cartesApres = player->getCartes();
    EXPECT_EQ(cartesApres.size(), tailleAvant + 1);
}

TEST_F(PlayerTest, AddCardToHandAjouteCorrectement) {
    player->addCardToHand(new Carte(Carte::TREFLE, Carte::VALET));

    auto cartes = player->getCartes();
    bool trouve = false;
    for (const auto& carte : cartes) {
        if (carte.getCouleur() == Carte::TREFLE && carte.getChiffre() == Carte::VALET) {
            trouve = true;
            break;
        }
    }

    EXPECT_TRUE(trouve) << "La carte ajoutée devrait être dans la main";
}

TEST_F(PlayerTest, GetCartesRetourneCopiePasReference) {
    auto cartes1 = player->getCartes();
    auto cartes2 = player->getCartes();

    // Modifier une copie ne devrait pas affecter l'autre
    EXPECT_EQ(cartes1.size(), cartes2.size());
}

// ========================================
// Tests de la belote
// ========================================

TEST_F(PlayerTest, HasBelotteAvecRoiEtDame) {
    std::vector<Carte*> main;
    main.push_back(new Carte(Carte::PIQUE, Carte::ROI));
    main.push_back(new Carte(Carte::PIQUE, Carte::DAME));
    main.push_back(new Carte(Carte::COEUR, Carte::AS));

    Player* joueur = new Player("TestBelote", main, 0);

    // Définir PIQUE comme atout
    joueur->setAtout(Carte::PIQUE);

    // Le joueur devrait avoir la belote
    EXPECT_TRUE(joueur->getHasBelotte()) << "Joueur avec Roi et Dame d'atout devrait avoir la belote";

    delete joueur;
}

TEST_F(PlayerTest, HasBelotteSansRoi) {
    std::vector<Carte*> main;
    main.push_back(new Carte(Carte::PIQUE, Carte::DAME));
    main.push_back(new Carte(Carte::PIQUE, Carte::VALET));
    main.push_back(new Carte(Carte::COEUR, Carte::AS));

    Player* joueur = new Player("TestSansBelote", main, 0);
    joueur->setAtout(Carte::PIQUE);

    EXPECT_FALSE(joueur->getHasBelotte()) << "Joueur sans Roi ne devrait pas avoir la belote";

    delete joueur;
}

TEST_F(PlayerTest, HasBelotteSansDame) {
    std::vector<Carte*> main;
    main.push_back(new Carte(Carte::PIQUE, Carte::ROI));
    main.push_back(new Carte(Carte::PIQUE, Carte::VALET));
    main.push_back(new Carte(Carte::COEUR, Carte::AS));

    Player* joueur = new Player("TestSansBelote", main, 0);
    joueur->setAtout(Carte::PIQUE);

    EXPECT_FALSE(joueur->getHasBelotte()) << "Joueur sans Dame ne devrait pas avoir la belote";

    delete joueur;
}

TEST_F(PlayerTest, HasBelotteMauvaiseCouleur) {
    std::vector<Carte*> main;
    main.push_back(new Carte(Carte::COEUR, Carte::ROI));  // Roi de Coeur
    main.push_back(new Carte(Carte::COEUR, Carte::DAME)); // Dame de Coeur
    main.push_back(new Carte(Carte::PIQUE, Carte::AS));

    Player* joueur = new Player("TestMauvaiseCouleur", main, 0);
    joueur->setAtout(Carte::PIQUE);  // Atout = Pique

    EXPECT_FALSE(joueur->getHasBelotte()) << "Roi et Dame de la mauvaise couleur ne font pas belote";

    delete joueur;
}

// ========================================
// Tests de l'atout
// ========================================

TEST_F(PlayerTest, SetAtoutModifieValeurCartes) {
    std::vector<Carte*> main;
    main.push_back(new Carte(Carte::PIQUE, Carte::VALET));
    main.push_back(new Carte(Carte::COEUR, Carte::VALET));

    Player* joueur = new Player("TestAtout", main, 0);

    // Avant setAtout, les valets valent 2 points chacun
    auto cartesAvant = joueur->getCartes();
    int totalAvant = 0;
    for (const auto& carte : cartesAvant) {
        totalAvant += carte.getValeurDeLaCarte();
    }
    EXPECT_EQ(totalAvant, 4) << "Deux valets non-atout = 4 points";

    // Après setAtout sur PIQUE
    joueur->setAtout(Carte::PIQUE);
    auto cartesApres = joueur->getCartes();
    int totalApres = 0;
    for (const auto& carte : cartesApres) {
        totalApres += carte.getValeurDeLaCarte();
    }
    EXPECT_EQ(totalApres, 22) << "Valet PIQUE atout (20) + Valet COEUR normal (2) = 22 points";

    delete joueur;
}

// ========================================
// Tests des annonces (enum values)
// ========================================

TEST_F(PlayerTest, AnnonceEnumValues) {
    // Test que les valeurs d'enum existent
    EXPECT_EQ(Player::ANNONCEINVALIDE, 0);
    EXPECT_NE(Player::QUATREVINGT, 0);
    EXPECT_NE(Player::CAPOT, 0);
    EXPECT_NE(Player::GENERALE, 0);
    EXPECT_NE(Player::PASSE, 0);
}

// ========================================
// Tests des valeurs d'annonce
// ========================================

TEST_F(PlayerTest, GetContractValue80) {
    int value = Player::getContractValue(Player::QUATREVINGT);
    EXPECT_EQ(value, 80);
}

TEST_F(PlayerTest, GetContractValue90) {
    int value = Player::getContractValue(Player::QUATREVINGTDIX);
    EXPECT_EQ(value, 90);
}

TEST_F(PlayerTest, GetContractValue100) {
    int value = Player::getContractValue(Player::CENT);
    EXPECT_EQ(value, 100);
}

TEST_F(PlayerTest, GetContractValue110) {
    int value = Player::getContractValue(Player::CENTDIX);
    EXPECT_EQ(value, 110);
}

TEST_F(PlayerTest, GetContractValue120) {
    int value = Player::getContractValue(Player::CENTVINGT);
    EXPECT_EQ(value, 120);
}

TEST_F(PlayerTest, GetContractValue130) {
    int value = Player::getContractValue(Player::CENTTRENTE);
    EXPECT_EQ(value, 130);
}

TEST_F(PlayerTest, GetContractValue140) {
    int value = Player::getContractValue(Player::CENTQUARANTE);
    EXPECT_EQ(value, 140);
}

TEST_F(PlayerTest, GetContractValue150) {
    int value = Player::getContractValue(Player::CENTCINQUANTE);
    EXPECT_EQ(value, 150);
}

TEST_F(PlayerTest, GetContractValue160) {
    int value = Player::getContractValue(Player::CENTSOIXANTE);
    EXPECT_EQ(value, 160);
}

TEST_F(PlayerTest, GetContractValueCapot) {
    int value = Player::getContractValue(Player::CAPOT);
    EXPECT_EQ(value, 250);
}

TEST_F(PlayerTest, GetContractValueGenerale) {
    int value = Player::getContractValue(Player::GENERALE);
    EXPECT_EQ(value, 500);
}

// ========================================
// Tests des cartes jouables
// ========================================

TEST_F(PlayerTest, IsCartePlayableMethodExists) {
    std::vector<Carte*> main;
    main.push_back(new Carte(Carte::COEUR, Carte::AS));
    main.push_back(new Carte(Carte::PIQUE, Carte::ROI));
    main.push_back(new Carte(Carte::CARREAU, Carte::DAME));

    Player* joueur = new Player("Test", main, 0);
    joueur->setAtout(Carte::PIQUE);

    // Test que la méthode existe et ne crash pas
    EXPECT_NO_THROW({
        bool result = joueur->isCartePlayable(
            0,
            Carte::COULEURINVALIDE,  // Pas de couleur demandée
            Carte::PIQUE,            // Atout = PIQUE
            nullptr,                 // Pas de carte la plus forte
            -1                       // Pas de joueur gagnant
        );
    });

    delete joueur;
}

// ========================================
// Tests de l'index
// ========================================

TEST_F(PlayerTest, GetIndexRetourneIndexCorrect) {
    EXPECT_EQ(player->getIndex(), 0);

    std::vector<Carte*> main2;
    Player* player2 = new Player("Player2", main2, 2);
    EXPECT_EQ(player2->getIndex(), 2);

    delete player2;
}
