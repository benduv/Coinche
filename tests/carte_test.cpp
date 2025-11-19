#include <gtest/gtest.h>
#include "../Carte.h"

class CarteTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Configuration avant chaque test
    }

    void TearDown() override {
        // Nettoyage après chaque test
    }
};

// ========================================
// Tests des constructeurs
// ========================================

TEST_F(CarteTest, ConstructeurParDefaut) {
    Carte carte;
    // Le constructeur par défaut n'initialise pas les valeurs
    // On vérifie juste qu'il ne crash pas
    EXPECT_NO_THROW(Carte carte2);
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

// ========================================
// Tests des getters/setters
// ========================================

TEST_F(CarteTest, GetCouleur) {
    Carte coeur(Carte::COEUR, Carte::AS);
    Carte pique(Carte::PIQUE, Carte::ROI);
    Carte carreau(Carte::CARREAU, Carte::DAME);
    Carte trefle(Carte::TREFLE, Carte::VALET);

    EXPECT_EQ(coeur.getCouleur(), Carte::COEUR);
    EXPECT_EQ(pique.getCouleur(), Carte::PIQUE);
    EXPECT_EQ(carreau.getCouleur(), Carte::CARREAU);
    EXPECT_EQ(trefle.getCouleur(), Carte::TREFLE);
}

TEST_F(CarteTest, GetChiffre) {
    Carte sept(Carte::COEUR, Carte::SEPT);
    Carte huit(Carte::COEUR, Carte::HUIT);
    Carte neuf(Carte::COEUR, Carte::NEUF);
    Carte dix(Carte::COEUR, Carte::DIX);
    Carte valet(Carte::COEUR, Carte::VALET);
    Carte dame(Carte::COEUR, Carte::DAME);
    Carte roi(Carte::COEUR, Carte::ROI);
    Carte as(Carte::COEUR, Carte::AS);

    EXPECT_EQ(sept.getChiffre(), Carte::SEPT);
    EXPECT_EQ(huit.getChiffre(), Carte::HUIT);
    EXPECT_EQ(neuf.getChiffre(), Carte::NEUF);
    EXPECT_EQ(dix.getChiffre(), Carte::DIX);
    EXPECT_EQ(valet.getChiffre(), Carte::VALET);
    EXPECT_EQ(dame.getChiffre(), Carte::DAME);
    EXPECT_EQ(roi.getChiffre(), Carte::ROI);
    EXPECT_EQ(as.getChiffre(), Carte::AS);
}

// ========================================
// Tests de l'atout
// ========================================

TEST_F(CarteTest, SetAtoutModifieValeur) {
    Carte valet(Carte::PIQUE, Carte::VALET);

    // Valet non-atout vaut 2 points
    valet.setAtout(false);
    EXPECT_EQ(valet.getValeurDeLaCarte(), 2);

    // Valet atout vaut 20 points
    valet.setAtout(true);
    EXPECT_EQ(valet.getValeurDeLaCarte(), 20);
}

TEST_F(CarteTest, SetAtoutModifieOrdre) {
    Carte neuf(Carte::PIQUE, Carte::NEUF);

    // L'ordre change selon atout
    neuf.setAtout(false);
    int ordreNormal = neuf.getOrdreCarteForte();

    neuf.setAtout(true);
    int ordreAtout = neuf.getOrdreCarteForte();

    // Les ordres devraient être différents
    EXPECT_NE(ordreNormal, ordreAtout);
}

// ========================================
// Tests des valeurs en points
// ========================================

TEST_F(CarteTest, GetValeurCarteNormale) {
    // Valeurs normales (non-atout)
    Carte as(Carte::COEUR, Carte::AS);
    Carte dix(Carte::COEUR, Carte::DIX);
    Carte roi(Carte::COEUR, Carte::ROI);
    Carte dame(Carte::COEUR, Carte::DAME);
    Carte valet(Carte::COEUR, Carte::VALET);
    Carte neuf(Carte::COEUR, Carte::NEUF);
    Carte huit(Carte::COEUR, Carte::HUIT);
    Carte sept(Carte::COEUR, Carte::SEPT);

    as.setAtout(false);
    dix.setAtout(false);
    roi.setAtout(false);
    dame.setAtout(false);
    valet.setAtout(false);
    neuf.setAtout(false);
    huit.setAtout(false);
    sept.setAtout(false);

    EXPECT_EQ(as.getValeurDeLaCarte(), 11) << "As devrait valoir 11 points";
    EXPECT_EQ(dix.getValeurDeLaCarte(), 10) << "10 devrait valoir 10 points";
    EXPECT_EQ(roi.getValeurDeLaCarte(), 4) << "Roi devrait valoir 4 points";
    EXPECT_EQ(dame.getValeurDeLaCarte(), 3) << "Dame devrait valoir 3 points";
    EXPECT_EQ(valet.getValeurDeLaCarte(), 2) << "Valet devrait valoir 2 points";
    EXPECT_EQ(neuf.getValeurDeLaCarte(), 0) << "9 devrait valoir 0 points";
    EXPECT_EQ(huit.getValeurDeLaCarte(), 0) << "8 devrait valoir 0 points";
    EXPECT_EQ(sept.getValeurDeLaCarte(), 0) << "7 devrait valoir 0 points";
}

TEST_F(CarteTest, GetValeurCarteAtout) {
    // Valeurs atout
    Carte valet(Carte::PIQUE, Carte::VALET);
    Carte neuf(Carte::PIQUE, Carte::NEUF);
    Carte as(Carte::PIQUE, Carte::AS);
    Carte dix(Carte::PIQUE, Carte::DIX);
    Carte roi(Carte::PIQUE, Carte::ROI);
    Carte dame(Carte::PIQUE, Carte::DAME);
    Carte huit(Carte::PIQUE, Carte::HUIT);
    Carte sept(Carte::PIQUE, Carte::SEPT);

    valet.setAtout(true);
    neuf.setAtout(true);
    as.setAtout(true);
    dix.setAtout(true);
    roi.setAtout(true);
    dame.setAtout(true);
    huit.setAtout(true);
    sept.setAtout(true);

    EXPECT_EQ(valet.getValeurDeLaCarte(), 20) << "Valet d'atout devrait valoir 20 points";
    EXPECT_EQ(neuf.getValeurDeLaCarte(), 14) << "9 d'atout devrait valoir 14 points";
    EXPECT_EQ(as.getValeurDeLaCarte(), 11) << "As d'atout devrait valoir 11 points";
    EXPECT_EQ(dix.getValeurDeLaCarte(), 10) << "10 d'atout devrait valoir 10 points";
    EXPECT_EQ(roi.getValeurDeLaCarte(), 4) << "Roi d'atout devrait valoir 4 points";
    EXPECT_EQ(dame.getValeurDeLaCarte(), 3) << "Dame d'atout devrait valoir 3 points";
    EXPECT_EQ(huit.getValeurDeLaCarte(), 0) << "8 d'atout devrait valoir 0 points";
    EXPECT_EQ(sept.getValeurDeLaCarte(), 0) << "7 d'atout devrait valoir 0 points";
}

// ========================================
// Tests de comparaison
// ========================================

TEST_F(CarteTest, ComparaisonMemeCouleurnon_atout) {
    Carte as(Carte::COEUR, Carte::AS);
    Carte roi(Carte::COEUR, Carte::ROI);

    as.setAtout(false);
    roi.setAtout(false);

    // As bat Roi en non-atout (Roi < As)
    EXPECT_TRUE(roi < as);
    EXPECT_FALSE(as < roi);
}

TEST_F(CarteTest, AtoutBatNonAtout) {
    Carte sept_atout(Carte::PIQUE, Carte::SEPT);
    Carte as_normal(Carte::COEUR, Carte::AS);

    sept_atout.setAtout(true);
    as_normal.setAtout(false);

    // L'As normal est plus faible que le 7 d'atout
    EXPECT_TRUE(as_normal < sept_atout);
    EXPECT_FALSE(sept_atout < as_normal);
}

TEST_F(CarteTest, ComparaisonAtouts) {
    Carte valet(Carte::PIQUE, Carte::VALET);
    Carte neuf(Carte::PIQUE, Carte::NEUF);
    Carte as(Carte::PIQUE, Carte::AS);

    valet.setAtout(true);
    neuf.setAtout(true);
    as.setAtout(true);

    // Valet d'atout > 9 d'atout > As d'atout
    EXPECT_TRUE(as < neuf);
    EXPECT_TRUE(neuf < valet);
    EXPECT_TRUE(as < valet);
}

TEST_F(CarteTest, CouleursDifferentesNonAtout) {
    Carte coeur(Carte::COEUR, Carte::AS);
    Carte pique(Carte::PIQUE, Carte::AS);

    coeur.setAtout(false);
    pique.setAtout(false);

    // Couleurs différentes, non-atout: aucune ne bat l'autre
    EXPECT_FALSE(coeur < pique);
    EXPECT_FALSE(pique < coeur);
}

// ========================================
// Tests d'affichage
// ========================================

TEST_F(CarteTest, PrintCarteNeCrashePas) {
    Carte carte(Carte::PIQUE, Carte::AS);
    // Ce test vérifie juste que printCarte() ne crash pas
    EXPECT_NO_THROW(carte.printCarte());
}
