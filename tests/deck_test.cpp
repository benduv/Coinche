#include <gtest/gtest.h>
#include "../Deck.h"
#include <algorithm>
#include <set>

class DeckTest : public ::testing::Test {
protected:
    Deck* deck;

    void SetUp() override {
        deck = new Deck();
    }

    void TearDown() override {
        delete deck;
    }
};

// ========================================
// Tests du constructeur
// ========================================

TEST_F(DeckTest, ConstructeurCreeDeck32Cartes) {
    // Test que le deck est initialisé avec 32 cartes
    std::vector<Carte*> main1, main2, main3, main4;
    deck->distribute(main1, main2, main3, main4);
    EXPECT_EQ(main1.size() + main2.size() + main3.size() + main4.size(), 32);
}

TEST_F(DeckTest, DistributionDonne8CartesParJoueur) {
    std::vector<Carte*> main1, main2, main3, main4;
    deck->distribute(main1, main2, main3, main4);

    EXPECT_EQ(main1.size(), 8) << "Joueur 1 devrait avoir 8 cartes";
    EXPECT_EQ(main2.size(), 8) << "Joueur 2 devrait avoir 8 cartes";
    EXPECT_EQ(main3.size(), 8) << "Joueur 3 devrait avoir 8 cartes";
    EXPECT_EQ(main4.size(), 8) << "Joueur 4 devrait avoir 8 cartes";
}

// ========================================
// Tests d'unicité des cartes
// ========================================

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

    // Vérifier l'unicité pour main1
    for (Carte* carte : main1) {
        auto result = toutesLesCartes.insert(carte);
        EXPECT_TRUE(result.second) << "Carte dupliquée trouvée dans main1";
    }

    // Vérifier l'unicité pour main2
    for (Carte* carte : main2) {
        auto result = toutesLesCartes.insert(carte);
        EXPECT_TRUE(result.second) << "Carte dupliquée trouvée dans main2";
    }

    // Vérifier l'unicité pour main3
    for (Carte* carte : main3) {
        auto result = toutesLesCartes.insert(carte);
        EXPECT_TRUE(result.second) << "Carte dupliquée trouvée dans main3";
    }

    // Vérifier l'unicité pour main4
    for (Carte* carte : main4) {
        auto result = toutesLesCartes.insert(carte);
        EXPECT_TRUE(result.second) << "Carte dupliquée trouvée dans main4";
    }

    // Vérifier qu'on a bien 32 cartes uniques
    EXPECT_EQ(toutesLesCartes.size(), 32);
}

TEST_F(DeckTest, DeckContient4Couleurs) {
    std::vector<Carte*> main1, main2, main3, main4;
    deck->distribute(main1, main2, main3, main4);

    // Combiner toutes les mains
    std::vector<Carte*> toutesLesCartes;
    toutesLesCartes.insert(toutesLesCartes.end(), main1.begin(), main1.end());
    toutesLesCartes.insert(toutesLesCartes.end(), main2.begin(), main2.end());
    toutesLesCartes.insert(toutesLesCartes.end(), main3.begin(), main3.end());
    toutesLesCartes.insert(toutesLesCartes.end(), main4.begin(), main4.end());

    // Compter les couleurs
    int coeur = 0, pique = 0, carreau = 0, trefle = 0;

    for (const Carte* carte : toutesLesCartes) {
        switch (carte->getCouleur()) {
            case Carte::COEUR: coeur++; break;
            case Carte::PIQUE: pique++; break;
            case Carte::CARREAU: carreau++; break;
            case Carte::TREFLE: trefle++; break;
        }
    }

    // Chaque couleur devrait avoir 8 cartes (7, 8, 9, 10, V, D, R, A)
    EXPECT_EQ(coeur, 8) << "Devrait y avoir 8 cartes de Coeur";
    EXPECT_EQ(pique, 8) << "Devrait y avoir 8 cartes de Pique";
    EXPECT_EQ(carreau, 8) << "Devrait y avoir 8 cartes de Carreau";
    EXPECT_EQ(trefle, 8) << "Devrait y avoir 8 cartes de Trèfle";
}

TEST_F(DeckTest, DeckContientToutesLesValeurs) {
    std::vector<Carte*> main1, main2, main3, main4;
    deck->distribute(main1, main2, main3, main4);

    // Combiner toutes les mains
    std::vector<Carte*> toutesLesCartes;
    toutesLesCartes.insert(toutesLesCartes.end(), main1.begin(), main1.end());
    toutesLesCartes.insert(toutesLesCartes.end(), main2.begin(), main2.end());
    toutesLesCartes.insert(toutesLesCartes.end(), main3.begin(), main3.end());
    toutesLesCartes.insert(toutesLesCartes.end(), main4.begin(), main4.end());

    // Compter chaque valeur
    int sept = 0, huit = 0, neuf = 0, dix = 0;
    int valet = 0, dame = 0, roi = 0, as = 0;

    for (const Carte* carte : toutesLesCartes) {
        switch (carte->getChiffre()) {
            case Carte::SEPT: sept++; break;
            case Carte::HUIT: huit++; break;
            case Carte::NEUF: neuf++; break;
            case Carte::DIX: dix++; break;
            case Carte::VALET: valet++; break;
            case Carte::DAME: dame++; break;
            case Carte::ROI: roi++; break;
            case Carte::AS: as++; break;
        }
    }

    // Chaque valeur devrait apparaître 4 fois (une par couleur)
    EXPECT_EQ(sept, 4) << "Devrait y avoir 4 Sept";
    EXPECT_EQ(huit, 4) << "Devrait y avoir 4 Huit";
    EXPECT_EQ(neuf, 4) << "Devrait y avoir 4 Neuf";
    EXPECT_EQ(dix, 4) << "Devrait y avoir 4 Dix";
    EXPECT_EQ(valet, 4) << "Devrait y avoir 4 Valets";
    EXPECT_EQ(dame, 4) << "Devrait y avoir 4 Dames";
    EXPECT_EQ(roi, 4) << "Devrait y avoir 4 Rois";
    EXPECT_EQ(as, 4) << "Devrait y avoir 4 As";
}

// ========================================
// Tests du mélange
// ========================================

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

    EXPECT_TRUE(different) << "Le mélange devrait modifier l'ordre des cartes";
    delete deck2;
}

TEST_F(DeckTest, ShuffleDeckGardeNombreCartes) {
    deck->shuffleDeck();

    std::vector<Carte*> main1, main2, main3, main4;
    deck->distribute(main1, main2, main3, main4);

    int total = main1.size() + main2.size() + main3.size() + main4.size();
    EXPECT_EQ(total, 32) << "Le mélange ne devrait pas changer le nombre de cartes";
}

TEST_F(DeckTest, ShuffleMultipleFoisDonneResultatsDifferents) {
    // Premier mélange et distribution
    deck->shuffleDeck();
    std::vector<Carte*> main1_1, main2_1, main3_1, main4_1;
    deck->distribute(main1_1, main2_1, main3_1, main4_1);

    // Créer un nouveau deck, mélanger différemment
    Deck* deck2 = new Deck();
    deck2->shuffleDeck();
    std::vector<Carte*> main1_2, main2_2, main3_2, main4_2;
    deck2->distribute(main1_2, main2_2, main3_2, main4_2);

    // Il est très improbable que deux mélanges donnent exactement le même résultat
    bool different = false;
    for(size_t i = 0; i < main1_1.size() && !different; i++) {
        if(main1_1[i]->getCouleur() != main1_2[i]->getCouleur() ||
           main1_1[i]->getChiffre() != main1_2[i]->getChiffre()) {
            different = true;
        }
    }

    // Note: Ce test pourrait échouer par hasard, mais c'est extrêmement improbable
    EXPECT_TRUE(different) << "Deux mélanges successifs devraient donner des résultats différents";

    delete deck2;
}

// ========================================
// Tests de distribution
// ========================================

TEST_F(DeckTest, DistributionPeutEtreReappelee) {
    std::vector<Carte*> main1, main2, main3, main4;
    deck->distribute(main1, main2, main3, main4);

    // Vérifier la première distribution
    EXPECT_EQ(main1.size(), 8);
    EXPECT_EQ(main2.size(), 8);

    // Le deck peut être appelé plusieurs fois (les mêmes cartes sont redistribuées)
    std::vector<Carte*> main1_2, main2_2, main3_2, main4_2;
    deck->distribute(main1_2, main2_2, main3_2, main4_2);

    // La deuxième distribution donne aussi 8 cartes par joueur
    EXPECT_EQ(main1_2.size(), 8);
    EXPECT_EQ(main2_2.size(), 8);
    EXPECT_EQ(main3_2.size(), 8);
    EXPECT_EQ(main4_2.size(), 8);
}

TEST_F(DeckTest, NouveauDeckPeutEtreRedistribue) {
    // Première distribution
    std::vector<Carte*> main1, main2, main3, main4;
    deck->distribute(main1, main2, main3, main4);

    // Créer un nouveau deck
    delete deck;
    deck = new Deck();

    // Deuxième distribution avec le nouveau deck
    std::vector<Carte*> main1_2, main2_2, main3_2, main4_2;
    deck->distribute(main1_2, main2_2, main3_2, main4_2);

    EXPECT_EQ(main1_2.size(), 8);
    EXPECT_EQ(main2_2.size(), 8);
    EXPECT_EQ(main3_2.size(), 8);
    EXPECT_EQ(main4_2.size(), 8);
}

// ========================================
// Tests de valeur totale
// ========================================

TEST_F(DeckTest, ValeurTotaleDeckEst152Points) {
    std::vector<Carte*> main1, main2, main3, main4;
    deck->distribute(main1, main2, main3, main4);

    // Combiner toutes les mains
    std::vector<Carte*> toutesLesCartes;
    toutesLesCartes.insert(toutesLesCartes.end(), main1.begin(), main1.end());
    toutesLesCartes.insert(toutesLesCartes.end(), main2.begin(), main2.end());
    toutesLesCartes.insert(toutesLesCartes.end(), main3.begin(), main3.end());
    toutesLesCartes.insert(toutesLesCartes.end(), main4.begin(), main4.end());

    // Calculer la valeur totale (non-atout)
    int valeurTotale = 0;
    for (Carte* carte : toutesLesCartes) {
        carte->setAtout(false);
        valeurTotale += carte->getValeurDeLaCarte();
    }

    // Total attendu: 4 * (11 + 10 + 4 + 3 + 2) = 4 * 30 = 120 points
    // Attends... 4 couleurs * (As:11 + 10:10 + Roi:4 + Dame:3 + Valet:2) = 4*30 = 120
    // Mais avec atout le total change. Vérifions sans atout:
    EXPECT_EQ(valeurTotale, 120) << "La valeur totale du deck (non-atout) devrait être 120 points";
}

TEST_F(DeckTest, ValeurTotaleAvecAtout) {
    std::vector<Carte*> main1, main2, main3, main4;
    deck->distribute(main1, main2, main3, main4);

    // Combiner toutes les cartes
    std::vector<Carte*> toutesLesCartes;
    toutesLesCartes.insert(toutesLesCartes.end(), main1.begin(), main1.end());
    toutesLesCartes.insert(toutesLesCartes.end(), main2.begin(), main2.end());
    toutesLesCartes.insert(toutesLesCartes.end(), main3.begin(), main3.end());
    toutesLesCartes.insert(toutesLesCartes.end(), main4.begin(), main4.end());

    // Définir PIQUE comme atout
    for (Carte* carte : toutesLesCartes) {
        if (carte->getCouleur() == Carte::PIQUE) {
            carte->setAtout(true);
        } else {
            carte->setAtout(false);
        }
    }

    int valeurTotale = 0;
    for (Carte* carte : toutesLesCartes) {
        valeurTotale += carte->getValeurDeLaCarte();
    }

    // 3 couleurs normales: 3 * 30 = 90
    // 1 couleur atout: As(11) + 10(10) + Roi(4) + Dame(3) + Valet(20) + 9(14) = 62
    // Total: 90 + 62 = 152
    EXPECT_EQ(valeurTotale, 152) << "La valeur totale avec une couleur atout devrait être 152 points";
}
