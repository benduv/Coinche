#include <gtest/gtest.h>
#include "../Carte.h"

class CarteTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Code de configuration qui sera exécuté avant chaque test
    }

    void TearDown() override {
        // Code de nettoyage qui sera exécuté après chaque test
    }
};

TEST_F(CarteTest, ConstructeurParDefaut) {
    Carte carte;
    // Vérifiez que les valeurs par défaut sont correctement initialisées
    EXPECT_EQ(carte.getCouleur(), Carte::COEUR);  // Assumant que COEUR est la valeur par défaut
}

TEST_F(CarteTest, ConstructeurAvecParametres) {
    Carte carte(Carte::PIQUE, Carte::AS);
    EXPECT_EQ(carte.getCouleur(), Carte::PIQUE);
    EXPECT_EQ(carte.getChiffre(), Carte::AS);
}

TEST_F(CarteTest, ConstructeurCopie) {
    Carte carte1(Carte::CARREAU, Carte::ROI);
    Carte carte2(carte1);
    EXPECT_EQ(carte2.getCouleur(), Carte::CARREAU);
    EXPECT_EQ(carte2.getChiffre(), Carte::ROI);
}