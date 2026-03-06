#include <gtest/gtest.h>
#include "../server/ScoreCalculator.h"
#include "../server/GameServer.h"
#include "../Carte.h"
#include "../Player.h"

// ========================================
// Test Fixture pour vérifier les compteurs de plis
// (logique de jeu indépendante du scoring)
// ========================================
class CapotGeneraleTest : public ::testing::Test {
protected:
    GameRoom room;

    void SetUp() override {
        room.roomId = 999;
        room.gameState = "playing";
        room.plisCountPlayer0 = 0;
        room.plisCountPlayer1 = 0;
        room.plisCountPlayer2 = 0;
        room.plisCountPlayer3 = 0;
        room.scoreTeam1 = 0;
        room.scoreTeam2 = 0;
        room.scoreMancheTeam1 = 0;
        room.scoreMancheTeam2 = 0;
        room.beloteTeam1 = false;
        room.beloteTeam2 = false;

        for (int i = 0; i < 4; i++) {
            std::vector<Carte*> emptyHand;
            auto player = std::make_unique<Player>("TestPlayer" + std::to_string(i), emptyHand, i);
            room.players.push_back(std::move(player));
        }
    }

    void playPliAutomatic(int pliNumber) {
        int winner = (pliNumber % 2 == 0) ? 0 : 2;
        switch (winner) {
            case 0: room.plisCountPlayer0++; break;
            case 2: room.plisCountPlayer2++; break;
        }
        room.currentPlayerIndex = winner;
    }
};

// ========================================
// TESTS COMPTEURS DE PLIS (logique de jeu)
// ========================================

TEST_F(CapotGeneraleTest, CapotTeam1_TousLesPlis) {
    for (int pli = 0; pli < 8; pli++) {
        playPliAutomatic(pli);
    }
    int plisTeam1 = room.plisCountPlayer0 + room.plisCountPlayer2;
    int plisTeam2 = room.plisCountPlayer1 + room.plisCountPlayer3;

    EXPECT_EQ(8, plisTeam1) << "Team1 devrait avoir fait 8 plis";
    EXPECT_EQ(0, plisTeam2) << "Team2 ne devrait avoir aucun pli";
    EXPECT_EQ(4, room.plisCountPlayer0);
    EXPECT_EQ(4, room.plisCountPlayer2);
}

TEST_F(CapotGeneraleTest, CapotEchoue_Team2GagneUnPli) {
    for (int pli = 0; pli < 8; pli++) {
        if (pli == 3) {
            room.plisCountPlayer1++;
        } else {
            playPliAutomatic(pli);
        }
    }
    int plisTeam1 = room.plisCountPlayer0 + room.plisCountPlayer2;
    int plisTeam2 = room.plisCountPlayer1 + room.plisCountPlayer3;

    EXPECT_EQ(7, plisTeam1);
    EXPECT_EQ(1, plisTeam2);
}

TEST_F(CapotGeneraleTest, Generale_JoueurSeulGagneTout) {
    for (int pli = 0; pli < 8; pli++) {
        room.plisCountPlayer0++;
    }
    EXPECT_EQ(8, room.plisCountPlayer0) << "Joueur 0 devrait avoir tous les plis";
    EXPECT_EQ(0, room.plisCountPlayer2) << "Partenaire ne doit avoir aucun pli";
}

TEST_F(CapotGeneraleTest, GeneraleEchouee_PartenaireGagneUnPli) {
    for (int pli = 0; pli < 8; pli++) {
        if (pli == 4) {
            room.plisCountPlayer2++;
        } else {
            room.plisCountPlayer0++;
        }
    }
    EXPECT_EQ(7, room.plisCountPlayer0);
    EXPECT_EQ(1, room.plisCountPlayer2);
}

// ========================================
// TESTS SCORE — CAPOT ANNONCE
// Appels directs à ScoreCalculator::calculateMancheScore()
// ========================================

TEST(ScoreCapotTest, CapotAnnonceReussi) {
    // CAPOT annoncé réussi → 500 (250+250), pointsRealises ignorés
    auto r = ScoreCalculator::calculateMancheScore(
        162, 0,   // pointsRealises (ignorés pour capot annoncé)
        250,      // valeurContrat = Player::getContractValue(CAPOT)
        true,     // team1HasBid
        false, false,
        true,  true,   // isCapotAnnonce, capotReussi
        false, false,
        false, false
    );
    EXPECT_EQ(r.scoreTeam1, 500) << "CAPOT annoncé réussi = 250+250 = 500";
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreCapotTest, CapotAnnonceEchoue) {
    // CAPOT annoncé échoué → adversaire marque 160+250 = 410
    auto r = ScoreCalculator::calculateMancheScore(
        100, 62,
        250,
        true,
        false, false,
        true,  false,  // isCapotAnnonce, capotReussi=false
        false, false,
        false, false
    );
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 410) << "CAPOT annoncé échoué → adversaire marque 160+250 = 410";
}

TEST(ScoreCapotTest, CapotAnnonceReussiCoinche) {
    // CAPOT coinché réussi → 250 + (250×2) = 750
    auto r = ScoreCalculator::calculateMancheScore(
        162, 0,
        250,
        true,
        true, false,  // coinched
        true, true,
        false, false,
        false, false
    );
    EXPECT_EQ(r.scoreTeam1, 750) << "CAPOT coinché réussi = 250+(250*2) = 750";
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreCapotTest, CapotAnnonceEchoueCoinche) {
    // CAPOT coinché échoué → adversaire marque 160 + (250×2) = 660
    auto r = ScoreCalculator::calculateMancheScore(
        100, 62,
        250,
        true,
        true, false,  // coinched
        true, false,
        false, false,
        false, false
    );
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 660) << "CAPOT coinché échoué = 160+(250*2) = 660";
}

// ========================================
// TESTS SCORE — GENERALE ANNONCEE
// ========================================

TEST(ScoreGeneraleTest, GeneraleReussie) {
    // GENERALE réussie → 1000 (500+500)
    auto r = ScoreCalculator::calculateMancheScore(
        162, 0,
        500,      // valeurContrat = Player::getContractValue(GENERALE)
        true,
        false, false,
        false, false,
        true, true,   // isGeneraleAnnonce, generaleReussie
        false, false
    );
    EXPECT_EQ(r.scoreTeam1, 1000) << "GENERALE réussie = 500+500 = 1000";
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreGeneraleTest, GeneraleEchouee) {
    // GENERALE échouée → adversaire marque 160+500 = 660, annonceur marque 0
    // Note: la belote de l'annonceur (si elle était dans pointsRealises) est ignorée
    auto r = ScoreCalculator::calculateMancheScore(
        20, 142,  // Team1 a belote (20pts) mais pas tous les plis
        500,
        true,
        false, false,
        false, false,
        true, false,  // isGeneraleAnnonce, generaleReussie=false
        false, false
    );
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 660) << "GENERALE échouée → adversaire marque 160+500 = 660";
}

TEST(ScoreGeneraleTest, GeneraleReussieSurcoinche) {
    // GENERALE surcoinchée réussie → 500 + (500×4) = 2500
    auto r = ScoreCalculator::calculateMancheScore(
        162, 0,
        500,
        true,
        true, true,   // coinched, surcoinched
        false, false,
        true, true,
        false, false
    );
    EXPECT_EQ(r.scoreTeam1, 2500) << "GENERALE surcoinchée réussie = 500+(500*4) = 2500";
    EXPECT_EQ(r.scoreTeam2, 0);
}

// ========================================
// TESTS SCORE — CAPOT NON ANNONCE
// ========================================

TEST(ScoreCapotNonAnnonceTest, CapotNonAnnonceTeam1_Annonce80) {
    // Annonce 80, capot réalisé → 250 + 80 = 330
    // pointsRealisesTeam1 (172 = toutes cartes + dernier pli) ignoré par la formule
    auto r = ScoreCalculator::calculateMancheScore(
        172, 0,
        80,   // valeurContrat (QUATREVINGT)
        true,
        false, false,
        false, false,
        false, false,
        true,  false  // capotNonAnnonceTeam1
    );
    EXPECT_EQ(r.scoreTeam1, 330) << "Capot non annoncé (annonce 80) = 250+80 = 330";
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreCapotNonAnnonceTest, CapotNonAnnonceTeam1_Annonce100) {
    // Cas rapporté : annonce 100 coeur, capot réalisé → 250 + 100 = 350 (pas 412)
    auto r = ScoreCalculator::calculateMancheScore(
        172, 0,
        100,  // valeurContrat (CENT)
        true,
        false, false,
        false, false,
        false, false,
        true,  false
    );
    EXPECT_EQ(r.scoreTeam1, 350) << "Capot non annoncé (annonce 100) = 250+100 = 350";
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreCapotNonAnnonceTest, CapotNonAnnonceTeam2_Annonce90) {
    // Team2 annonce 90, capot → 250 + 90 = 340
    auto r = ScoreCalculator::calculateMancheScore(
        0, 172,
        90,   // valeurContrat (QUATREVINGTDIX)
        false, // team2HasBid
        false, false,
        false, false,
        false, false,
        false, true   // capotNonAnnonceTeam2
    );
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 340) << "Capot non annoncé Team2 (annonce 90) = 250+90 = 340";
}

// ========================================
// TESTS SCORE — CONTRAT NORMAL
// ========================================

TEST(ScoreContratNormalTest, ContratReussi) {
    // Team1 annonce 100, réalise 130 pts → 100 + 130 = 230, Team2 garde ses 32 pts
    auto r = ScoreCalculator::calculateMancheScore(
        130, 32,
        100,
        true,
        false, false,
        false, false,
        false, false,
        false, false
    );
    EXPECT_EQ(r.scoreTeam1, 230) << "Contrat 100 réussi (130pts) = 100+130 = 230";
    EXPECT_EQ(r.scoreTeam2, 32);
}

TEST(ScoreContratNormalTest, ContratEchoue) {
    // Team1 annonce 100, réalise 60 pts → 0, Team2 marque 160+100 = 260
    auto r = ScoreCalculator::calculateMancheScore(
        60, 102,
        100,
        true,
        false, false,
        false, false,
        false, false,
        false, false
    );
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 260) << "Contrat 100 échoué → adversaire marque 160+100 = 260";
}

TEST(ScoreContratNormalTest, ContratReussiCoinche) {
    // Team1 annonce 100, coinchée, réussit → 160 + (100×2) = 360
    auto r = ScoreCalculator::calculateMancheScore(
        130, 32,
        100,
        true,
        true, false,  // coinched
        false, false,
        false, false,
        false, false
    );
    EXPECT_EQ(r.scoreTeam1, 360) << "Contrat 100 coinché réussi = 160+(100*2) = 360";
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreContratNormalTest, ContratEchoueCoinche) {
    // Team1 annonce 100, coinchée, échoue → 0, Team2 marque 160 + (100×2) = 360
    auto r = ScoreCalculator::calculateMancheScore(
        60, 102,
        100,
        true,
        true, false,
        false, false,
        false, false,
        false, false
    );
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 360) << "Contrat 100 coinché échoué → adversaire marque 160+200 = 360";
}

TEST(ScoreContratNormalTest, ContratReussiSurcoinche) {
    // Team1 annonce 100, surcoinchée, réussit → 160 + (100×4) = 560
    auto r = ScoreCalculator::calculateMancheScore(
        130, 32,
        100,
        true,
        true, true,   // coinched, surcoinched
        false, false,
        false, false,
        false, false
    );
    EXPECT_EQ(r.scoreTeam1, 560) << "Contrat 100 surcoinché réussi = 160+(100*4) = 560";
    EXPECT_EQ(r.scoreTeam2, 0);
}
