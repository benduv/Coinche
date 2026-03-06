#include <gtest/gtest.h>
#include "../server/GameServer.h"
#include "../server/ScoreCalculator.h"
#include "../Carte.h"
#include "../Player.h"

// ========================================
// Test Fixture pour COINCHE et SURCOINCHE
// ========================================
class CoincheTest : public ::testing::Test {
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
        room.coinched = false;
        room.surcoinched = false;

        // Créer 4 joueurs vides
        for (int i = 0; i < 4; i++) {
            std::vector<Carte*> emptyHand;
            auto player = std::make_unique<Player>("TestPlayer" + std::to_string(i), emptyHand, i);
            room.players.push_back(std::move(player));
        }
    }

    // Utilitaire: distribution des mains où Team1 peut réussir son contrat
    void setHandsForCoincheReussi() {
        // PIQUE est atout
        // Joueur 0 (Team1): Bonnes cartes d'atout
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::VALET));  // 20 points atout
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::NEUF));   // 14 points atout
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::AS));     // 11 points atout
        room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::DIX));
        room.players[0]->addCardToHand(new Carte(Carte::CARREAU, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::CARREAU, Carte::DIX));
        room.players[0]->addCardToHand(new Carte(Carte::TREFLE, Carte::AS));

        // Joueur 2 (Team1 - partenaire): Reste des atouts et cartes moyennes
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::DIX));
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::ROI));
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::DAME));
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::HUIT));
        room.players[2]->addCardToHand(new Carte(Carte::TREFLE, Carte::DIX));
        room.players[2]->addCardToHand(new Carte(Carte::COEUR, Carte::ROI));
        room.players[2]->addCardToHand(new Carte(Carte::CARREAU, Carte::ROI));
        room.players[2]->addCardToHand(new Carte(Carte::TREFLE, Carte::ROI));

        // Joueur 1 (Team2): Cartes faibles
        room.players[1]->addCardToHand(new Carte(Carte::PIQUE, Carte::SEPT));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::SEPT));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::HUIT));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::NEUF));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::DAME));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::SEPT));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::HUIT));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::NEUF));

        // Joueur 3 (Team2): Cartes faibles
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::SEPT));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::HUIT));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::NEUF));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::DAME));
        room.players[3]->addCardToHand(new Carte(Carte::COEUR, Carte::VALET));
        room.players[3]->addCardToHand(new Carte(Carte::CARREAU, Carte::VALET));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::VALET));
        room.players[3]->addCardToHand(new Carte(Carte::CARREAU, Carte::DAME));
    }

    void setHandsForCoincheEchoue() {
        // Team1 a des cartes faibles, ne peut pas réussir son contrat
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::SEPT));
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::HUIT));
        room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::SEPT));
        room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::HUIT));
        room.players[0]->addCardToHand(new Carte(Carte::CARREAU, Carte::SEPT));
        room.players[0]->addCardToHand(new Carte(Carte::CARREAU, Carte::HUIT));
        room.players[0]->addCardToHand(new Carte(Carte::TREFLE, Carte::SEPT));
        room.players[0]->addCardToHand(new Carte(Carte::TREFLE, Carte::HUIT));

        room.players[2]->addCardToHand(new Carte(Carte::COEUR, Carte::NEUF));
        room.players[2]->addCardToHand(new Carte(Carte::COEUR, Carte::DAME));
        room.players[2]->addCardToHand(new Carte(Carte::CARREAU, Carte::NEUF));
        room.players[2]->addCardToHand(new Carte(Carte::CARREAU, Carte::DAME));
        room.players[2]->addCardToHand(new Carte(Carte::TREFLE, Carte::NEUF));
        room.players[2]->addCardToHand(new Carte(Carte::TREFLE, Carte::DAME));
        room.players[2]->addCardToHand(new Carte(Carte::COEUR, Carte::VALET));
        room.players[2]->addCardToHand(new Carte(Carte::CARREAU, Carte::VALET));

        // Team2 a toutes les bonnes cartes
        room.players[1]->addCardToHand(new Carte(Carte::PIQUE, Carte::VALET));
        room.players[1]->addCardToHand(new Carte(Carte::PIQUE, Carte::NEUF));
        room.players[1]->addCardToHand(new Carte(Carte::PIQUE, Carte::AS));
        room.players[1]->addCardToHand(new Carte(Carte::PIQUE, Carte::DIX));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::AS));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::DIX));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::AS));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::DIX));

        room.players[3]->addCardToHand(new Carte(Carte::PIQUE, Carte::ROI));
        room.players[3]->addCardToHand(new Carte(Carte::PIQUE, Carte::DAME));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::AS));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::DIX));
        room.players[3]->addCardToHand(new Carte(Carte::COEUR, Carte::ROI));
        room.players[3]->addCardToHand(new Carte(Carte::CARREAU, Carte::ROI));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::ROI));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::VALET));
    }

    // Simulation simplifiée des plis
    void playPliAutomatic(int pliNumber, bool team1Wins) {
        int winner;
        if (team1Wins) {
            winner = (pliNumber % 2 == 0) ? 0 : 2;
        } else {
            winner = (pliNumber % 2 == 0) ? 1 : 3;
        }

        switch (winner) {
            case 0: room.plisCountPlayer0++; break;
            case 1: room.plisCountPlayer1++; break;
            case 2: room.plisCountPlayer2++; break;
            case 3: room.plisCountPlayer3++; break;
        }

        room.currentPlayerIndex = winner;
    }
};

// ========================================
// TESTS COINCHE
// ========================================

TEST(ScoreContratTest, CoincheReussiTeam1Annonce80) {
    // Team1 annonce 80, coinchée, réussit avec 110 pts → 160 + (80×2) = 320
    auto r = ScoreCalculator::calculateMancheScore(
        110, 52, 80, true, true, false,
        false, false, false, false, false, false
    );
    EXPECT_EQ(r.scoreTeam1, 320) << "Coinche réussie (annonce 80) = 160+(80×2) = 320";
    EXPECT_EQ(r.scoreTeam2, 0);
}

TEST(ScoreContratTest, ContratReussiSansBelote) {
    // Team1 annonce 100, réalise 110 pts, Team2 a 52 pts → 100+110=210, adversaire 52
    auto r = ScoreCalculator::calculateMancheScore(
        110, 52, 100, true, false, false,
        false, false, false, false, false, false
    );
    EXPECT_EQ(r.scoreTeam1, 210) << "Contrat 100 réussi (110pts) = 100+110 = 210";
    EXPECT_EQ(r.scoreTeam2, 52)  << "Team2 garde ses 52 points";
}

TEST(ScoreContratTest, ContratReussiAvecBeloteAnnonceur) {
    // Team1 annonce 100, réalise 100 pts + 20 belote = 120 → 100+120=220, adversaire 60
    auto r = ScoreCalculator::calculateMancheScore(
        120, 60, 100, true, false, false,
        false, false, false, false, false, false
    );
    EXPECT_EQ(r.scoreTeam1, 220) << "Contrat 100 réussi avec belote (120pts) = 100+120 = 220";
    EXPECT_EQ(r.scoreTeam2, 60)  << "Team2 garde ses 60 points";
}

TEST(ScoreContratTest, ContratReussiAvecBeloteAdversaire) {
    // Team1 annonce 100, réalise 110 pts. Team2 a 52 pts de cartes (162-110) + 20 belote = 72
    auto r = ScoreCalculator::calculateMancheScore(
        110, 72, 100, true, false, false,
        false, false, false, false, false, false
    );
    EXPECT_EQ(r.scoreTeam1, 210) << "Contrat 100 réussi = 100+110 = 210";
    EXPECT_EQ(r.scoreTeam2, 72)  << "Team2 garde ses 72 points (52 cartes + 20 belote)";
}

TEST(ScoreContratTest, ContratEchoueSansBelote) {
    // Team1 annonce 100, réalise seulement 70 pts → 0, Team2 marque 160+100=260
    auto r = ScoreCalculator::calculateMancheScore(
        70, 92, 100, true, false, false,
        false, false, false, false, false, false
    );
    EXPECT_EQ(r.scoreTeam1, 0)   << "Team1 ne marque rien";
    EXPECT_EQ(r.scoreTeam2, 260) << "Contrat 100 échoué → adversaire marque 160+100 = 260";
}

TEST(ScoreContratTest, ContratEchoueAvecBeloteAnnonceur) {
    // Team1 annonce 100, a belote (+20) mais réalise 80+20=100 pts : contrat juste atteint
    auto r = ScoreCalculator::calculateMancheScore(
        100, 62, 100, true, false, false,
        false, false, false, false, false, false
    );
    EXPECT_EQ(r.scoreTeam1, 200) << "Contrat 100 atteint pile (100pts avec belote) = 100+100 = 200";
    EXPECT_EQ(r.scoreTeam2, 62);
}

TEST(ScoreContratTest, ContratEchoueAvecBeloteMaisInsuffisante) {
    // Team1 annonce 100, a belote (+20) mais 75 pts de cartes + 20 = 95 < 100 → échec
    // La belote (20) est toujours rendue à Team1 même sur contrat échoué
    auto r = ScoreCalculator::calculateMancheScore(
        95, 87, 100, true, false, false,
        false, false, false, false, false, false,
        20, 0  // beloteTeam1=20
    );
    EXPECT_EQ(r.scoreTeam1, 20)  << "Team1 garde sa belote (20) malgré l'échec";
    EXPECT_EQ(r.scoreTeam2, 260) << "Adversaire marque 160+100 = 260";
}

TEST(ScoreContratTest, ContratReussiTeam2SansBelote) {
    // Team2 annonce 90, réalise 100 pts, Team1 a 62 pts → Team2 marque 90+100=190, Team1 garde 62
    auto r = ScoreCalculator::calculateMancheScore(
        62, 100, 90, false, false, false,
        false, false, false, false, false, false
    );
    EXPECT_EQ(r.scoreTeam1, 62)  << "Team1 garde ses 62 points";
    EXPECT_EQ(r.scoreTeam2, 190) << "Contrat 90 réussi (100pts) = 90+100 = 190";
}

TEST(ScoreContratTest, ContratEchoueTeam2) {
    // Team2 annonce 90, réalise 60 pts → 0, Team1 marque 160+90=250
    auto r = ScoreCalculator::calculateMancheScore(
        102, 60, 90, false, false, false,
        false, false, false, false, false, false
    );
    EXPECT_EQ(r.scoreTeam1, 250) << "Contrat 90 échoué → adversaire marque 160+90 = 250";
    EXPECT_EQ(r.scoreTeam2, 0)   << "Team2 ne marque rien";
}

TEST_F(CoincheTest, CoincheEchoueTeam1Annonce80) {
    // Team1 annonce 80, Team2 coinche, Team1 échoue
    int valeurContrat = 80;
    int pointsRealisesTeam1 = 70;  // Team1 n'atteint pas 80
    int pointsRealisesTeam2 = 90;
    bool team1HasBid = true;
    bool coinched = true;
    bool surcoinched = false;

    // Appeler la fonction de calcul
    auto result = ScoreCalculator::calculateMancheScore(
        pointsRealisesTeam1,
        pointsRealisesTeam2,
        valeurContrat,
        team1HasBid,
        coinched,
        surcoinched,
        false,  // isCapotAnnonce
        false,  // capotReussi
        false,  // isGeneraleAnnonce
        false,  // generaleReussie
        false,  // capotNonAnnonceTeam1
        false   // capotNonAnnonceTeam2
    );

    // Scoring COINCHE échoué: Team2 marque 160 + (contrat × 2) = 160 + (80 × 2) = 320
    EXPECT_EQ(result.scoreTeam1, 0) << "Team1 ne devrait marquer aucun point";
    EXPECT_EQ(result.scoreTeam2, 320) << "Team2 devrait marquer 320 points (160+(80×2))";
}

TEST_F(CoincheTest, SurcoincheReussiTeam2Annonce120) {
    // Team2 annonce 120, Team1 coinche, Team2 surcoinche, Team2 réussit
    setHandsForCoincheEchoue();  // Team2 a les bonnes cartes

    room.lastBidderIndex = 1;  // Team2 a fait l'annonce
    room.lastBidAnnonce = Player::CENTVINGT;  // 120 points
    room.lastBidCouleur = Carte::PIQUE;
    room.couleurAtout = Carte::PIQUE;
    room.firstPlayerIndex = 0;
    room.currentPlayerIndex = 0;
    room.coinched = true;  // Team1 a coinché
    room.surcoinched = true;  // Team2 a surcoinché

    for (int i = 0; i < 4; i++) {
        room.players[i]->setAtout(Carte::PIQUE);
    }

    // Simuler que Team2 gagne 7 plis (suffisant pour 120 points)
    for (int pli = 0; pli < 7; pli++) {
        playPliAutomatic(pli, false);  // Team2 gagne
    }
    playPliAutomatic(7, true);  // Team1 gagne 1 pli

    int plisTeam1 = room.plisCountPlayer0 + room.plisCountPlayer2;
    int plisTeam2 = room.plisCountPlayer1 + room.plisCountPlayer3;

    EXPECT_EQ(1, plisTeam1) << "Team1 devrait avoir 1 pli";
    EXPECT_EQ(7, plisTeam2) << "Team2 devrait avoir 7 plis";

    // Scoring SURCOINCHE réussi: 160 + (contrat × 4) = 160 + (120 × 4) = 640
    auto result = ScoreCalculator::calculateMancheScore(
        32, 130,  // Team2 a 7 plis ≈ 130 pts, Team1 ≈ 32 pts
        120,      // valeurContrat
        false,    // team2HasBid
        true, true,  // coinched, surcoinched
        false, false, false, false, false, false
    );
    EXPECT_EQ(result.scoreTeam1, 0)   << "Team1 ne devrait marquer aucun point";
    EXPECT_EQ(result.scoreTeam2, 640) << "Surcoinche réussie = 160+(120×4) = 640";
}

TEST_F(CoincheTest, SurcoincheEchoueTeam2Annonce120) {
    // Team2 annonce 120, Team1 coinche, Team2 surcoinche, Team2 échoue
    int valeurContrat = 120;
    int pointsRealisesTeam1 = 130;
    int pointsRealisesTeam2 = 30;  // Team2 n'atteint pas 120
    bool team1HasBid = false;  // Team2 a annoncé
    bool coinched = true;
    bool surcoinched = true;

    // Appeler la fonction de calcul
    auto result = ScoreCalculator::calculateMancheScore(
        pointsRealisesTeam1,
        pointsRealisesTeam2,
        valeurContrat,
        team1HasBid,
        coinched,
        surcoinched,
        false,  // isCapotAnnonce
        false,  // capotReussi
        false,  // isGeneraleAnnonce
        false,  // generaleReussie
        false,  // capotNonAnnonceTeam1
        false   // capotNonAnnonceTeam2
    );

    // Scoring SURCOINCHE échoué: Team1 marque 160 + (contrat × 4) = 160 + (120 × 4) = 640
    EXPECT_EQ(result.scoreTeam1, 640) << "Team1 devrait marquer 640 points (160+(120×4))";
    EXPECT_EQ(result.scoreTeam2, 0) << "Team2 ne devrait marquer aucun point";
}

TEST_F(CoincheTest, CoincheAvecBeloteTeam1Reussi) {
    // Team1 annonce 100, Team2 coinche, Team1 réussit avec belote
    int valeurContrat = 100;
    int pointsRealisesTeam1 = 120 + 20;  // 120 points + 20 belote = 140
    int pointsRealisesTeam2 = 40;
    bool team1HasBid = true;
    bool coinched = true;
    bool surcoinched = false;

    // Appeler la fonction de calcul
    auto result = ScoreCalculator::calculateMancheScore(
        pointsRealisesTeam1,
        pointsRealisesTeam2,
        valeurContrat,
        team1HasBid,
        coinched,
        surcoinched,
        false,  // isCapotAnnonce
        false,  // capotReussi
        false,  // isGeneraleAnnonce
        false,  // generaleReussie
        false,  // capotNonAnnonceTeam1
        false   // capotNonAnnonceTeam2
    );

    // Scoring COINCHE réussi: 160 + (contrat × 2) = 360
    // Note: La belote (20 points) est déjà incluse dans pointsRealisesTeam1
    // qui est utilisé uniquement pour vérifier si le contrat est réussi (140 >= 100)
    EXPECT_EQ(result.scoreTeam1, 360) << "Team1 devrait marquer 360 points (160+(100×2))";
    EXPECT_EQ(result.scoreTeam2, 0) << "Team2 ne devrait marquer aucun point";
}

// ========================================
// TESTS CAPOT et GENERALE COINCHES
// ========================================

TEST_F(CoincheTest, CapotAnnonceReussiCoinche) {
    // Team1 annonce CAPOT coinché et réussit
    int valeurContrat = 250;  // CAPOT
    bool team1HasBid = true;
    bool coinched = true;
    bool surcoinched = false;
    bool isCapotAnnonce = true;
    bool capotReussi = true;

    // Appeler la fonction de calcul
    auto result = ScoreCalculator::calculateMancheScore(
        160,  // pointsRealisesTeam1 (non utilisé pour capot annoncé)
        0,    // pointsRealisesTeam2
        valeurContrat,
        team1HasBid,
        coinched,
        surcoinched,
        isCapotAnnonce,
        capotReussi,
        false,  // isGeneraleAnnonce
        false,  // generaleReussie
        false,  // capotNonAnnonceTeam1
        false   // capotNonAnnonceTeam2
    );

    // Scoring CAPOT annoncé réussi coinché: 250 + (250 × 2) = 750
    EXPECT_EQ(result.scoreTeam1, 750) << "Team1 devrait marquer 750 points (250+(250×2))";
    EXPECT_EQ(result.scoreTeam2, 0) << "Team2 ne devrait marquer aucun point";
}

TEST_F(CoincheTest, CapotAnnonceEchoueCoinche) {
    // Team1 annonce CAPOT coinché et échoue
    auto result = ScoreCalculator::calculateMancheScore(
        130, 30, 250, true, true, false,
        true, false,  // isCapotAnnonce, capotEchoue
        false, false, false, false
    );

    // Scoring CAPOT annoncé échoué coinché: Team2 marque 160 + (250 × 2) = 660
    EXPECT_EQ(result.scoreTeam1, 0) << "Team1 ne devrait marquer aucun point";
    EXPECT_EQ(result.scoreTeam2, 660) << "Team2 devrait marquer 660 points (160+(250×2))";
}

TEST_F(CoincheTest, CapotAnnonceReussiSurcoinche) {
    // Team2 annonce CAPOT surcoinché et réussit
    auto result = ScoreCalculator::calculateMancheScore(
        0, 160, 250, false, true, true,
        true, true,  // isCapotAnnonce, capotReussi
        false, false, false, false
    );

    // Scoring CAPOT annoncé réussi surcoinché: 250 + (250 × 4) = 1250
    EXPECT_EQ(result.scoreTeam1, 0) << "Team1 ne devrait marquer aucun point";
    EXPECT_EQ(result.scoreTeam2, 1250) << "Team2 devrait marquer 1250 points (250+(250×4))";
}

TEST_F(CoincheTest, GeneraleAnnonceeReussieCoinche) {
    // Joueur 0 (Team1) annonce GENERALE coinchée et réussit
    auto result = ScoreCalculator::calculateMancheScore(
        160, 0, 500, true, true, false,
        false, false,
        true, true,  // isGeneraleAnnonce, generaleReussie
        false, false
    );

    // Scoring GENERALE annoncée réussie coinchée: 500 + (500 × 2) = 1500
    EXPECT_EQ(result.scoreTeam1, 1500) << "Team1 devrait marquer 1500 points (500+(500×2))";
    EXPECT_EQ(result.scoreTeam2, 0) << "Team2 ne devrait marquer aucun point";
}

TEST_F(CoincheTest, GeneraleAnnonceeEchoueeSurcoinche) {
    // Joueur 1 (Team2) annonce GENERALE surcoinchée et échoue
    auto result = ScoreCalculator::calculateMancheScore(
        130, 30, 500, false, true, true,
        false, false,
        true, false,  // isGeneraleAnnonce, generaleEchouee
        false, false
    );

    // Scoring GENERALE annoncée échouée surcoinchée: Team1 marque 160 + (500 × 4) = 2160
    EXPECT_EQ(result.scoreTeam1, 2160) << "Team1 devrait marquer 2160 points (160+(500×4))";
    EXPECT_EQ(result.scoreTeam2, 0) << "Team2 ne devrait marquer aucun point";
}

TEST_F(CoincheTest, CapotNonAnnonceCoinche) {
    // Team1 annonce 90 coinché, réussit et fait capot non annoncé
    auto result = ScoreCalculator::calculateMancheScore(
        160, 0, 90, true, true, false,
        false, false, false, false,
        true, false  // capotNonAnnonceTeam1
    );

    // Scoring CAPOT non annoncé coinché: 250 + (90 × 2) = 430
    EXPECT_EQ(result.scoreTeam1, 430) << "Team1 devrait marquer 430 points (250+(90×2))";
    EXPECT_EQ(result.scoreTeam2, 0) << "Team2 ne devrait marquer aucun point";
}

TEST_F(CoincheTest, BeloteCompteeUneFoisSeulement) {
    // Test que la belote est incluse dans pointsRealises et n'est pas ajoutée après
    // Team1 annonce 100, réussit avec 100 points + 20 belote = 120 points réalisés
    int valeurContrat = 100;
    int pointsRealisesTeam1 = 100 + 20;  // 100 points plis + 20 belote
    int pointsRealisesTeam2 = 60;

    auto result = ScoreCalculator::calculateMancheScore(
        pointsRealisesTeam1,
        pointsRealisesTeam2,
        valeurContrat,
        true,   // team1HasBid
        false,  // coinched
        false,  // surcoinched
        false, false, false, false, false, false
    );

    // Scoring normal réussi: valeurContrat + pointsRealisés = 100 + 120 = 220
    // La belote est déjà incluse dans les 120 points
    EXPECT_EQ(result.scoreTeam1, 220) << "Team1 devrait marquer 220 points (100+120, belote incluse)";
    EXPECT_EQ(result.scoreTeam2, 60) << "Team2 devrait marquer 60 points";
}
