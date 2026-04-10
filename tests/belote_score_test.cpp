#include <gtest/gtest.h>
#include "../server/ScoreCalculator.h"

// Helper pour simplifier les appels
static ScoreCalculator::ScoreResult calcBelote(
    int ptsT1, int ptsT2, bool t1Bid,
    bool capotT1 = false, bool capotT2 = false,
    int beloteT1 = 0, int beloteT2 = 0)
{
    return ScoreCalculator::calculateBeloteMancheScore(
        ptsT1, ptsT2, t1Bid,
        capotT1, capotT2,
        beloteT1, beloteT2);
}

// ========================================
// CONTRAT REUSSI (> 81 pts)
// ========================================

TEST(BeloteScore, ContratReussi_Team1) {
    // Team1 prend, fait 100 pts → les deux équipes marquent leurs points réels
    auto r = calcBelote(100, 62, true);
    EXPECT_EQ(r.scoreTeam1, 100);
    EXPECT_EQ(r.scoreTeam2, 62);
}

TEST(BeloteScore, ContratReussi_Team2) {
    // Team2 prend, fait 90 pts → les deux équipes marquent leurs points réels
    auto r = calcBelote(72, 90, false);
    EXPECT_EQ(r.scoreTeam1, 72);
    EXPECT_EQ(r.scoreTeam2, 90);
}

TEST(BeloteScore, ContratReussiJuste82_Team1) {
    // Team1 prend, fait exactement 82 pts → réussi (> 81)
    auto r = calcBelote(82, 80, true);
    EXPECT_EQ(r.scoreTeam1, 82);
    EXPECT_EQ(r.scoreTeam2, 80);
}

TEST(BeloteScore, ContratEchoueJuste81_Team1) {
    // Team1 prend, fait exactement 81 pts → chute (pas > 81)
    auto r = calcBelote(81, 81, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 162);
}

// ========================================
// CHUTE (<= 81 pts)
// ========================================

TEST(BeloteScore, Chute_Team1) {
    // Team1 prend, fait 60 pts → chute : Team1=0, Team2=162
    auto r = calcBelote(60, 102, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 162);
}

TEST(BeloteScore, Chute_Team2) {
    // Team2 prend, fait 70 pts → chute : Team1=162, Team2=0
    auto r = calcBelote(92, 70, false);
    EXPECT_EQ(r.scoreTeam1, 162);
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(BeloteScore, Chute_80pts_Team1) {
    // Team1 prend, fait 80 pts (en dessous de 81) → chute
    auto r = calcBelote(80, 82, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 162);
}

// ========================================
// CAPOT par le preneur (250 pts)
// ========================================

TEST(BeloteScore, Capot_ByTaker_Team1) {
    // Team1 prend et fait tous les plis → 250 pts pour Team1
    auto r = calcBelote(162, 0, true, true, false);
    EXPECT_EQ(r.scoreTeam1, 250);
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(BeloteScore, Capot_ByTaker_Team2) {
    // Team2 prend et fait tous les plis → 250 pts pour Team2
    auto r = calcBelote(0, 162, false, false, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 250);
}

// ========================================
// CONTRE-CAPOT (le défenseur fait tous les plis)
// ========================================

TEST(BeloteScore, ContreCapot_DefenseurTeam2_WhenTeam1Bid) {
    // Team1 prend, Team2 (défenseur) fait tous les plis → 250 pts pour Team2
    auto r = calcBelote(0, 162, true, false, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 250);
}

TEST(BeloteScore, ContreCapot_DefenseurTeam1_WhenTeam2Bid) {
    // Team2 prend, Team1 (défenseur) fait tous les plis → 250 pts pour Team1
    auto r = calcBelote(162, 0, false, true, false);
    EXPECT_EQ(r.scoreTeam1, 250);
    EXPECT_EQ(r.scoreTeam2, 0);
}

// ========================================
// BELOTE — toujours +20 pour l'équipe qui l'a
// ========================================

TEST(BeloteScore, Belote_ContratReussi_Preneur) {
    // Team1 prend (100 pts), a la belote (20 pts) → Team1=100+20=120, Team2=62
    auto r = calcBelote(100, 62, true, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 120);
    EXPECT_EQ(r.scoreTeam2, 62);
}

TEST(BeloteScore, Belote_ContratReussi_Defenseur) {
    // Team1 prend (90 pts), Team2 (défenseur) a la belote → Team1=90, Team2=72+20=92
    auto r = calcBelote(90, 72, true, false, false, 0, 20);
    EXPECT_EQ(r.scoreTeam1, 90);
    EXPECT_EQ(r.scoreTeam2, 92);
}

TEST(BeloteScore, Belote_ContratReussiGraceABelote) {
    // Team1 prend 62 pts + belote 20 = 82 > 81 → réussi
    auto r = calcBelote(62, 100, true, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 62 + 20); // 82
    EXPECT_EQ(r.scoreTeam2, 100);
}

TEST(BeloteScore, Belote_ChuteJuste81AvecBelote) {
    // Team1 prend 61 pts + belote 20 = 81 → chute (pas > 81)
    auto r = calcBelote(61, 101, true, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 20); // chute mais garde belote
    EXPECT_EQ(r.scoreTeam2, 162);
}

TEST(BeloteScore, Belote_Chute_PreneurGardeBelote) {
    // Team1 prend 60 pts + belote 20 = 80 <= 81 → chute quand même
    // Team1 garde sa belote (20), Team2 marque 162
    auto r = calcBelote(60, 102, true, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 20); // chute mais garde belote
    EXPECT_EQ(r.scoreTeam2, 162);
}

TEST(BeloteScore, Belote_Chute_DefenseurABelote) {
    // Team1 prend, chute, Team2 (défenseur) a la belote
    auto r = calcBelote(60, 102, true, false, false, 0, 20);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 162 + 20); // 182
}

TEST(BeloteScore, Belote_Capot_PreneurAvecBelote) {
    // Team1 prend et fait capot + a la belote → 250 + 20 = 270
    auto r = calcBelote(162, 0, true, true, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 270);
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(BeloteScore, Belote_ContreCapot_DefenseurAvecBelote) {
    // Team1 prend, Team2 fait contre-capot + a la belote → 250 + 20 = 270
    auto r = calcBelote(0, 162, true, false, true, 0, 20);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 270);
}
