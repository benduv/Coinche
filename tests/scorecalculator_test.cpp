#include <gtest/gtest.h>
#include "../server/ScoreCalculator.h"

// Helper pour simplifier les appels
// Paramètres dans l'ordre : pointsT1, pointsT2, contrat, team1HasBid,
//   coinched, surcoinched, capotAnnonce, capotReussi,
//   generaleAnnonce, generaleReussie, capotNonAnnonceT1, capotNonAnnonceT2,
//   beloteT1, beloteT2
static ScoreCalculator::ScoreResult calc(
    int ptsT1, int ptsT2, int contrat, bool t1Bid,
    bool coinche = false, bool surcoinche = false,
    bool capotAnn = false, bool capotOk = false,
    bool genAnn = false, bool genOk = false,
    bool capotNAT1 = false, bool capotNAT2 = false,
    int beloteT1 = 0, int beloteT2 = 0)
{
    return ScoreCalculator::calculateMancheScore(
        ptsT1, ptsT2, contrat, t1Bid,
        coinche, surcoinche,
        capotAnn, capotOk,
        genAnn, genOk,
        capotNAT1, capotNAT2,
        beloteT1, beloteT2);
}

// ========================================
// CONTRAT NORMAL (sans coinche)
// ========================================

TEST(ScoreCalculator, ContratNormal_Reussi) {
    auto r = calc(130, 32, 100, true);
    EXPECT_EQ(r.scoreTeam1, 230); // 100 + 130
    EXPECT_EQ(r.scoreTeam2, 32);
}

TEST(ScoreCalculator, ContratNormal_Echoue) {
    auto r = calc(60, 102, 100, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 260); // 160 + 100
}

TEST(ScoreCalculator, ContratNormal_ReussiTeam2) {
    auto r = calc(62, 100, 90, false);
    EXPECT_EQ(r.scoreTeam1, 62);
    EXPECT_EQ(r.scoreTeam2, 190); // 90 + 100
}

TEST(ScoreCalculator, ContratNormal_EchoueTeam2) {
    auto r = calc(102, 60, 90, false);
    EXPECT_EQ(r.scoreTeam1, 250); // 160 + 90
    EXPECT_EQ(r.scoreTeam2, 0);
}

// ========================================
// COINCHE
// ========================================

TEST(ScoreCalculator, Coinche_Reussi) {
    // 160 + (100×2) = 360
    auto r = calc(130, 32, 100, true, true);
    EXPECT_EQ(r.scoreTeam1, 360);
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreCalculator, Coinche_Echoue) {
    // 160 + (80×2) = 320
    auto r = calc(70, 90, 80, true, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 320);
}

TEST(ScoreCalculator, Coinche_ReussiTeam2) {
    // 160 + (80×2) = 320
    auto r = calc(52, 110, 80, false, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 320);
}

TEST(ScoreCalculator, Coinche_EchoueTeam2) {
    // 160 + (120×2) = 400
    auto r = calc(130, 30, 120, false, true);
    EXPECT_EQ(r.scoreTeam1, 400);
    EXPECT_EQ(r.scoreTeam2, 0);
}

// ========================================
// SURCOINCHE
// ========================================

TEST(ScoreCalculator, Surcoinche_Reussi) {
    // 160 + (100×4) = 560
    auto r = calc(130, 32, 100, true, true, true);
    EXPECT_EQ(r.scoreTeam1, 560);
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreCalculator, Surcoinche_Echoue) {
    // 160 + (100×4) = 560
    auto r = calc(60, 102, 100, true, true, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 560);
}

TEST(ScoreCalculator, Surcoinche_ReussiTeam2) {
    // 160 + (120×4) = 640
    auto r = calc(32, 130, 120, false, true, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 640);
}

TEST(ScoreCalculator, Surcoinche_EchoueTeam2) {
    // 160 + (120×4) = 640
    auto r = calc(130, 30, 120, false, true, true);
    EXPECT_EQ(r.scoreTeam1, 640);
    EXPECT_EQ(r.scoreTeam2, 0);
}

// ========================================
// CAPOT ANNONCE
// ========================================

TEST(ScoreCalculator, CapotAnnonce_Reussi) {
    auto r = calc(162, 0, 250, true, false, false, true, true);
    EXPECT_EQ(r.scoreTeam1, 500); // 250 + 250
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreCalculator, CapotAnnonce_Echoue) {
    auto r = calc(100, 62, 250, true, false, false, true, false);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 410); // 160 + 250
}

TEST(ScoreCalculator, CapotAnnonce_ReussiCoinche) {
    // 160 + (250×2) = 660
    auto r = calc(162, 0, 250, true, true, false, true, true);
    EXPECT_EQ(r.scoreTeam1, 660);
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreCalculator, CapotAnnonce_EchoueCoinche) {
    // 160 + (250×2) = 660
    auto r = calc(100, 62, 250, true, true, false, true, false);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 660);
}

TEST(ScoreCalculator, CapotAnnonce_ReussiSurcoinche) {
    // 160 + (250×4) = 1160
    auto r = calc(0, 162, 250, false, true, true, true, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 1160);
}

// ========================================
// GENERALE ANNONCEE
// ========================================

TEST(ScoreCalculator, Generale_Reussie) {
    auto r = calc(162, 0, 500, true, false, false, false, false, true, true);
    EXPECT_EQ(r.scoreTeam1, 1000); // 500 + 500
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreCalculator, Generale_Echouee) {
    auto r = calc(20, 142, 500, true, false, false, false, false, true, false);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 660); // 160 + 500
}

TEST(ScoreCalculator, Generale_ReussieCoinche) {
    // 160 + (500×2) = 1160
    auto r = calc(162, 0, 500, true, true, false, false, false, true, true);
    EXPECT_EQ(r.scoreTeam1, 1160);
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreCalculator, Generale_EchoueeSurcoinche) {
    // 160 + (500×4) = 2160
    auto r = calc(130, 30, 500, false, true, true, false, false, true, false);
    EXPECT_EQ(r.scoreTeam1, 2160);
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreCalculator, Generale_ReussieSurcoinche) {
    // 160 + (500×4) = 2160
    auto r = calc(162, 0, 500, true, true, true, false, false, true, true);
    EXPECT_EQ(r.scoreTeam1, 2160);
    EXPECT_EQ(r.scoreTeam2, 0);
}

// ========================================
// CAPOT NON ANNONCE
// ========================================

TEST(ScoreCalculator, CapotNonAnnonce_Team1_Annonce80) {
    // 250 + 80 = 330
    auto r = calc(162, 0, 80, true, false, false, false, false, false, false, true, false);
    EXPECT_EQ(r.scoreTeam1, 330);
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreCalculator, CapotNonAnnonce_Team1_Annonce100) {
    // 250 + 100 = 350
    auto r = calc(162, 0, 100, true, false, false, false, false, false, false, true, false);
    EXPECT_EQ(r.scoreTeam1, 350);
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreCalculator, CapotNonAnnonce_Team2_Annonce90) {
    // 250 + 90 = 340
    auto r = calc(0, 162, 90, false, false, false, false, false, false, false, false, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 340);
}

TEST(ScoreCalculator, CapotNonAnnonce_Coinche) {
    // 250 + (90×2) = 430
    auto r = calc(162, 0, 90, true, true, false, false, false, false, false, true, false);
    EXPECT_EQ(r.scoreTeam1, 430);
    EXPECT_EQ(r.scoreTeam2, 0);
}

// ========================================
// BELOTE — toujours +20 pour l'equipe qui l'a
// ========================================

// Contrat normal réussi + belote annonceur
TEST(ScoreCalculator, Belote_ContratReussi_Annonceur) {
    // 100 + 110 + 20 = 230
    auto r = calc(110, 52, 100, true, false, false, false, false, false, false, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 230);
    EXPECT_EQ(r.scoreTeam2, 52);
}

// Contrat normal réussi + belote défenseur
TEST(ScoreCalculator, Belote_ContratReussi_Defenseur) {
    // Team1: 100 + 110 = 210, Team2: 52 + 20 = 72
    auto r = calc(110, 52, 100, true, false, false, false, false, false, false, false, false, 0, 20);
    EXPECT_EQ(r.scoreTeam1, 210);
    EXPECT_EQ(r.scoreTeam2, 72);
}

// Contrat normal échoué + belote annonceur
TEST(ScoreCalculator, Belote_ContratEchoue_Annonceur) {
    // Team1 garde belote (20), Team2: 160 + 100 = 260
    auto r = calc(60, 102, 100, true, false, false, false, false, false, false, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 20);
    EXPECT_EQ(r.scoreTeam2, 260);
}

// Contrat normal échoué + belote défenseur
TEST(ScoreCalculator, Belote_ContratEchoue_Defenseur) {
    // Team1: 0, Team2: 160 + 100 + 20 = 280
    auto r = calc(60, 102, 100, true, false, false, false, false, false, false, false, false, 0, 20);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 280);
}

// Contrat réussi GRACE à la belote (90 + 20 = 110 >= 100)
TEST(ScoreCalculator, Belote_ContratReussiGraceABelote) {
    // 100 + 90 + 20 = 210
    auto r = calc(90, 72, 100, true, false, false, false, false, false, false, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 210);
    EXPECT_EQ(r.scoreTeam2, 72);
}

// Contrat échoué MALGRE la belote (70 + 20 = 90 < 100)
TEST(ScoreCalculator, Belote_ContratEchoueMalgreBelote) {
    auto r = calc(70, 92, 100, true, false, false, false, false, false, false, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 20);
    EXPECT_EQ(r.scoreTeam2, 260);
}

// Capot non annoncé + belote annonceur → 250 + 100 + 20 = 370
TEST(ScoreCalculator, Belote_CapotNonAnnonce_Annonceur) {
    auto r = calc(162, 0, 100, true, false, false, false, false, false, false, true, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 370);
    EXPECT_EQ(r.scoreTeam2, 0);
}

// Capot non annoncé + belote adversaire → Team1=350, Team2=20
TEST(ScoreCalculator, Belote_CapotNonAnnonce_Adversaire) {
    auto r = calc(162, 0, 100, true, false, false, false, false, false, false, true, false, 0, 20);
    EXPECT_EQ(r.scoreTeam1, 350);
    EXPECT_EQ(r.scoreTeam2, 20);
}

// Capot annoncé réussi + belote → 500 + 20 = 520
TEST(ScoreCalculator, Belote_CapotAnnonceReussi) {
    auto r = calc(162, 0, 250, true, false, false, true, true, false, false, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 520);
    EXPECT_EQ(r.scoreTeam2, 0);
}

// Capot annoncé échoué + belote annonceur → Team1=20, Team2=410
TEST(ScoreCalculator, Belote_CapotAnnonceEchoue_Annonceur) {
    auto r = calc(100, 62, 250, true, false, false, true, false, false, false, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 20);
    EXPECT_EQ(r.scoreTeam2, 410);
}

// Generale réussie + belote → 1000 + 20 = 1020
TEST(ScoreCalculator, Belote_GeneraleReussie) {
    auto r = calc(162, 0, 500, true, false, false, false, false, true, true, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 1020);
    EXPECT_EQ(r.scoreTeam2, 0);
}

// Generale échouée + belote annonceur → Team1=20, Team2=660
TEST(ScoreCalculator, Belote_GeneraleEchouee_Annonceur) {
    auto r = calc(20, 142, 500, true, false, false, false, false, true, false, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 20);
    EXPECT_EQ(r.scoreTeam2, 660);
}

// Coinche réussi + belote → 160 + (100×2) + 20 = 380
TEST(ScoreCalculator, Belote_CoincheReussi) {
    auto r = calc(130, 32, 100, true, true, false, false, false, false, false, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 380);
    EXPECT_EQ(r.scoreTeam2, 0);
}

// Coinche échoué + belote annonceur → Team1=20, Team2=360
TEST(ScoreCalculator, Belote_CoincheEchoue_Annonceur) {
    auto r = calc(60, 102, 100, true, true, false, false, false, false, false, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 20);
    EXPECT_EQ(r.scoreTeam2, 360);
}

// Coinche réussi grâce à la belote (90+20=110 >= 100)
TEST(ScoreCalculator, Belote_CoincheReussiGraceABelote) {
    // 160 + (100×2) + 20 = 380
    auto r = calc(90, 72, 100, true, true, false, false, false, false, false, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 380);
    EXPECT_EQ(r.scoreTeam2, 0);
}

// ========================================
// CAPOT ANNONCE — chemins manquants
// ========================================

// Capot annoncé réussi par team2 (sans coinche)
TEST(ScoreCalculator, CapotAnnonce_ReussiTeam2) {
    auto r = calc(0, 162, 250, false, false, false, true, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 500);
}

// Capot annoncé échoué par team2 (sans coinche)
TEST(ScoreCalculator, CapotAnnonce_EchoueTeam2) {
    auto r = calc(62, 100, 250, false, false, false, true, false);
    EXPECT_EQ(r.scoreTeam1, 410);
    EXPECT_EQ(r.scoreTeam2, 0);
}

// Capot annoncé échoué + surcoinche (team1 annonce)
// 160 + (250×4) = 1160 pour team2
TEST(ScoreCalculator, CapotAnnonce_EchoueSurcoinche) {
    auto r = calc(100, 62, 250, true, true, true, true, false);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 1160);
}

// Capot annoncé échoué + coinche (team2 annonce)
// 160 + (250×2) = 660 pour team1
TEST(ScoreCalculator, CapotAnnonce_EchoueCoinche_Team2) {
    auto r = calc(62, 100, 250, false, true, false, true, false);
    EXPECT_EQ(r.scoreTeam1, 660);
    EXPECT_EQ(r.scoreTeam2, 0);
}

// ========================================
// GENERALE — chemins manquants
// ========================================

// Générale réussie par team2 (sans coinche)
TEST(ScoreCalculator, Generale_ReussieTeam2) {
    auto r = calc(0, 162, 500, false, false, false, false, false, true, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 1000);
}

// Générale échouée + coinche (team1 annonce)
// 160 + (500×2) = 1160 pour team2
TEST(ScoreCalculator, Generale_EchoueeCoinche) {
    auto r = calc(20, 142, 500, true, true, false, false, false, true, false);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 1160);
}

// ========================================
// CAPOT NON ANNONCE + SURCOINCHE
// ========================================

// Capot non annoncé + surcoinche: 250 + (90×4) = 610
TEST(ScoreCalculator, CapotNonAnnonce_Surcoinche) {
    auto r = calc(162, 0, 90, true, true, true, false, false, false, false, true, false);
    EXPECT_EQ(r.scoreTeam1, 610);
    EXPECT_EQ(r.scoreTeam2, 0);
}

// Capot non annoncé + surcoinche (team2 annonce): 250 + (80×4) = 570
TEST(ScoreCalculator, CapotNonAnnonce_Surcoinche_Team2) {
    auto r = calc(0, 162, 80, false, true, true, false, false, false, false, false, true);
    EXPECT_EQ(r.scoreTeam1, 0);
    EXPECT_EQ(r.scoreTeam2, 570);
}

// ========================================
// SURCOINCHE + BELOTE
// ========================================

// Surcoinche réussi + belote annonceur: 160 + (100×4) + 20 = 580
TEST(ScoreCalculator, Belote_SurcoinchReussi_Annonceur) {
    auto r = calc(130, 32, 100, true, true, true, false, false, false, false, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 580);
    EXPECT_EQ(r.scoreTeam2, 0);
}

// Surcoinche échoué + belote annonceur: Team1=20, Team2=560
TEST(ScoreCalculator, Belote_SurcoinchEchoue_Annonceur) {
    auto r = calc(60, 102, 100, true, true, true, false, false, false, false, false, false, 20, 0);
    EXPECT_EQ(r.scoreTeam1, 20);
    EXPECT_EQ(r.scoreTeam2, 560);
}

// Coinche réussi + belote défenseur: 160 + (100×2) = 360, Team2=20
TEST(ScoreCalculator, Belote_CoincheReussi_Defenseur) {
    auto r = calc(130, 32, 100, true, true, false, false, false, false, false, false, false, 0, 20);
    EXPECT_EQ(r.scoreTeam1, 360);
    EXPECT_EQ(r.scoreTeam2, 20);
}

// ========================================
// SEUIL EXACT DU CONTRAT
// ========================================

// Contrat normal: points == valeurContrat → réussi (condition >=)
TEST(ScoreCalculator, ContratNormal_SeuilExact) {
    auto r = calc(100, 62, 100, true);
    EXPECT_EQ(r.scoreTeam1, 200); // 100 + 100
    EXPECT_EQ(r.scoreTeam2, 62);
}

// Coinche: points == valeurContrat → réussi
TEST(ScoreCalculator, Coinche_SeuilExact) {
    auto r = calc(100, 62, 100, true, true);
    EXPECT_EQ(r.scoreTeam1, 360); // 160 + (100×2)
    EXPECT_EQ(r.scoreTeam2, 0);
}
