#include <gtest/gtest.h>
#include "../Deck.h"
#include <algorithm>

class DeckTest : public ::testing::Test {
protected:
    void SetUp() override {
        deck = new Deck();
    }

    void TearDown() override {
        delete deck;
    }

    Deck* deck;
};

TEST_F(DeckTest, ConstructeurCreeDeck32Cartes) {
    // Test que le deck est initialisé avec 32 cartes
    std::vector<Carte*> main1, main2, main3, main4;
    deck->distribute(main1, main2, main3, main4);
    EXPECT_EQ(main1.size() + main2.size() + main3.size() + main4.size(), 32);
}

TEST_F(DeckTest, DistributionDonne8CartesParJoueur) {
    std::vector<Carte*> main1, main2, main3, main4;
    deck->distribute(main1, main2, main3, main4);
    
    EXPECT_EQ(main1.size(), 8);
    EXPECT_EQ(main2.size(), 8);
    EXPECT_EQ(main3.size(), 8);
    EXPECT_EQ(main4.size(), 8);
}

// Helper struct pour comparer les cartes
struct CarteCompare {
    bool operator()(const Carte* a, const Carte* b) const {
        if (a->getCouleur() != b->getCouleur())
            return a->getCouleur() < b->getCouleur();
        return a->getChiffre() < b->getChiffre();
    }
};

TEST_F(DeckTest, CartesUniques) {
    std::vector<Carte*> main1, main2, main3, main4;
    deck->shuffleDeck();
    deck->distribute(main1, main2, main3, main4);
    
    // Créer un set qui va contenir toutes les cartes
    std::set<Carte*, CarteCompare> toutesLesCartes;
    
    // Vérifier l'unicité pour chaque main
    for (Carte* carte : main1) {
        // Le second élément du pair indique si l'insertion a réussi
        auto result = toutesLesCartes.insert(carte);
        EXPECT_TRUE(result.second) << "Carte dupliquée trouvée dans main1 : " 
            << "Couleur=" << carte->getCouleur() 
            << ", Chiffre=" << carte->getChiffre();
    }
    
    for (Carte* carte : main2) {
        auto result = toutesLesCartes.insert(carte);
        EXPECT_TRUE(result.second) << "Carte dupliquée trouvée dans main2 : "
            << "Couleur=" << carte->getCouleur() 
            << ", Chiffre=" << carte->getChiffre();
    }
    
    for (Carte* carte : main3) {
        auto result = toutesLesCartes.insert(carte);
        EXPECT_TRUE(result.second) << "Carte dupliquée trouvée dans main3 : "
            << "Couleur=" << carte->getCouleur() 
            << ", Chiffre=" << carte->getChiffre();
    }
    
    for (Carte* carte : main4) {
        auto result = toutesLesCartes.insert(carte);
        EXPECT_TRUE(result.second) << "Carte dupliquée trouvée dans main4 : "
            << "Couleur=" << carte->getCouleur() 
            << ", Chiffre=" << carte->getChiffre();
    }
    
    // Vérifier qu'on a bien 32 cartes uniques
    EXPECT_EQ(toutesLesCartes.size(), 32);
}

TEST_F(DeckTest, ShuffleDeckModifieOrdre) {
    // Créer deux decks identiques
    Deck* deck2 = new Deck();
    
    // Mélanger seulement le premier
    deck->shuffleDeck();
    
    // Distribuer les cartes des deux decks
    std::vector<Carte*> main1_1, main2_1, main3_1, main4_1;
    std::vector<Carte*> main1_2, main2_2, main3_2, main4_2;
    
    deck->distribute(main1_1, main2_1, main3_1, main4_1);
    deck2->distribute(main1_2, main2_2, main3_2, main4_2);
    
    // Vérifier que les cartes ne sont pas dans le même ordre
    bool different = false;
    for(size_t i = 0; i < main1_1.size(); i++) {
        if(main1_1[i]->getCouleur() != main1_2[i]->getCouleur() ||
           main1_1[i]->getChiffre() != main1_2[i]->getChiffre()) {
            different = true;
            break;
        }
    }
    
    EXPECT_TRUE(different);
    delete deck2;
}

// TEST_F(DeckTest, SetAtoutChangeAtout) {
//     deck->setAtout(Carte::COEUR);
//     // Note: Comme l'atout est commenté dans le code actuel,
//     // ce test devra être modifié une fois que la fonctionnalité sera implémentée
//     // EXPECT_EQ(deck->getAtout(), Carte::COEUR);
// }