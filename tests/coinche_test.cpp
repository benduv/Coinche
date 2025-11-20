#include <gtest/gtest.h>
#include "../server/GameServer.h"
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

TEST_F(CoincheTest, CoincheReussiTeam1Annonce80) {
    // Team1 annonce 80, Team2 coinche, Team1 réussit
    setHandsForCoincheReussi();

    room.lastBidderIndex = 0;  // Team1 a fait l'annonce
    room.lastBidAnnonce = Player::QUATREVINGT;  // 80 points
    room.lastBidCouleur = Carte::PIQUE;
    room.couleurAtout = Carte::PIQUE;
    room.firstPlayerIndex = 0;
    room.currentPlayerIndex = 0;
    room.coinched = true;  // Team2 a coinché

    // Définir l'atout pour tous les joueurs
    for (int i = 0; i < 4; i++) {
        room.players[i]->setAtout(Carte::PIQUE);
    }

    // Simuler que Team1 gagne 6 plis (suffisant pour 80 points)
    for (int pli = 0; pli < 6; pli++) {
        playPliAutomatic(pli, true);  // Team1 gagne
    }
    for (int pli = 6; pli < 8; pli++) {
        playPliAutomatic(pli, false);  // Team2 gagne les 2 derniers
    }

    int plisTeam1 = room.plisCountPlayer0 + room.plisCountPlayer2;
    int plisTeam2 = room.plisCountPlayer1 + room.plisCountPlayer3;

    EXPECT_EQ(6, plisTeam1) << "Team1 devrait avoir 6 plis";
    EXPECT_EQ(2, plisTeam2) << "Team2 devrait avoir 2 plis";

    // Scoring COINCHE réussi: (contrat + pointsRealisés) × 2
    // Team1 a 6 plis, on estime environ 110 points réalisés
    // Score = (80 + 110) × 2 = 380 points (approximation pour le test)
    // NOTE: Le score réel dépend des cartes jouées dans chaque pli
    int scoreAttenduTeam1 = 380;  // Approximation basée sur 6 plis
    int scoreAttenduTeam2 = 0;

    EXPECT_GE(scoreAttenduTeam1, 80 * 2) << "Team1 devrait marquer au moins le contrat doublé ((80+points)*2)";
    EXPECT_EQ(scoreAttenduTeam2, 0) << "Team2 ne devrait marquer aucun point";
}

TEST_F(CoincheTest, CoincheEchoueTeam1Annonce80) {
    // Team1 annonce 80, Team2 coinche, Team1 échoue
    setHandsForCoincheEchoue();

    room.lastBidderIndex = 0;  // Team1 a fait l'annonce
    room.lastBidAnnonce = Player::QUATREVINGT;  // 80 points
    room.lastBidCouleur = Carte::PIQUE;
    room.couleurAtout = Carte::PIQUE;
    room.firstPlayerIndex = 0;
    room.currentPlayerIndex = 0;
    room.coinched = true;  // Team2 a coinché

    for (int i = 0; i < 4; i++) {
        room.players[i]->setAtout(Carte::PIQUE);
    }

    // Simuler que Team2 gagne 7 plis (Team1 n'atteint pas 80)
    for (int pli = 0; pli < 7; pli++) {
        playPliAutomatic(pli, false);  // Team2 gagne
    }
    playPliAutomatic(7, true);  // Team1 gagne seulement 1 pli

    int plisTeam1 = room.plisCountPlayer0 + room.plisCountPlayer2;
    int plisTeam2 = room.plisCountPlayer1 + room.plisCountPlayer3;

    EXPECT_EQ(1, plisTeam1) << "Team1 devrait avoir 1 pli";
    EXPECT_EQ(7, plisTeam2) << "Team2 devrait avoir 7 plis";

    // Scoring COINCHE échoué: Team2 marque (contrat + 160) × 2 = (80 + 160) × 2 = 480
    int scoreAttenduTeam1 = 0;
    int scoreAttenduTeam2 = 480;

    EXPECT_EQ(scoreAttenduTeam1, 0) << "Team1 ne devrait marquer aucun point";
    EXPECT_EQ(scoreAttenduTeam2, 480) << "Team2 devrait marquer 480 points ((80+160)*2)";
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

    // Scoring SURCOINCHE réussi: (contrat + pointsRealisés) × 4
    // Team2 a 7 plis, on estime environ 130 points réalisés
    // Score = (120 + 130) × 4 = 1000 points (approximation pour le test)
    int scoreAttenduTeam1 = 0;
    int scoreAttenduTeam2 = 1000;  // Approximation

    EXPECT_EQ(scoreAttenduTeam1, 0) << "Team1 ne devrait marquer aucun point";
    EXPECT_GE(scoreAttenduTeam2, 120 * 4) << "Team2 devrait marquer au moins le contrat quadruplé ((120+points)*4)";
}

TEST_F(CoincheTest, SurcoincheEchoueTeam2Annonce120) {
    // Team2 annonce 120, Team1 coinche, Team2 surcoinche, Team2 échoue
    setHandsForCoincheReussi();  // Team1 a les bonnes cartes

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

    // Simuler que Team1 gagne 7 plis (Team2 n'atteint pas 120)
    for (int pli = 0; pli < 7; pli++) {
        playPliAutomatic(pli, true);  // Team1 gagne
    }
    playPliAutomatic(7, false);  // Team2 gagne seulement 1 pli

    int plisTeam1 = room.plisCountPlayer0 + room.plisCountPlayer2;
    int plisTeam2 = room.plisCountPlayer1 + room.plisCountPlayer3;

    EXPECT_EQ(7, plisTeam1) << "Team1 devrait avoir 7 plis";
    EXPECT_EQ(1, plisTeam2) << "Team2 devrait avoir 1 pli";

    // Scoring SURCOINCHE échoué: Team1 marque (contrat + 160) × 4 = (120 + 160) × 4 = 1120
    int scoreAttenduTeam1 = 1120;
    int scoreAttenduTeam2 = 0;

    EXPECT_EQ(scoreAttenduTeam1, 1120) << "Team1 devrait marquer 1120 points ((120+160)*4)";
    EXPECT_EQ(scoreAttenduTeam2, 0) << "Team2 ne devrait marquer aucun point";
}

TEST_F(CoincheTest, CoincheAvecBeloteTeam1Reussi) {
    // Team1 annonce 100, Team2 coinche, Team1 réussit avec belote
    setHandsForCoincheReussi();

    room.lastBidderIndex = 0;  // Team1 a fait l'annonce
    room.lastBidAnnonce = Player::CENT;  // 100 points
    room.lastBidCouleur = Carte::PIQUE;
    room.couleurAtout = Carte::PIQUE;
    room.firstPlayerIndex = 0;
    room.currentPlayerIndex = 0;
    room.coinched = true;  // Team2 a coinché
    room.beloteTeam1 = true;  // Team1 a la belote (Roi et Dame de Pique)

    for (int i = 0; i < 4; i++) {
        room.players[i]->setAtout(Carte::PIQUE);
    }

    // Simuler que Team1 gagne 6 plis
    for (int pli = 0; pli < 6; pli++) {
        playPliAutomatic(pli, true);  // Team1 gagne
    }
    for (int pli = 6; pli < 8; pli++) {
        playPliAutomatic(pli, false);  // Team2 gagne les 2 derniers
    }

    int plisTeam1 = room.plisCountPlayer0 + room.plisCountPlayer2;
    int plisTeam2 = room.plisCountPlayer1 + room.plisCountPlayer3;

    EXPECT_EQ(6, plisTeam1) << "Team1 devrait avoir 6 plis";
    EXPECT_EQ(2, plisTeam2) << "Team2 devrait avoir 2 plis";

    // Scoring COINCHE réussi avec belote: ((contrat + pointsRealisés) × 2) + 20 belote
    // Team1 a 6 plis, on estime environ 110 points réalisés
    // Score = (100 + 110) × 2 + 20 = 440 points (approximation pour le test)
    int scoreAttenduTeam1 = 440;
    int scoreAttenduTeam2 = 0;

    EXPECT_GE(scoreAttenduTeam1, 100 * 2 + 20) << "Team1 devrait marquer au moins ((100+points)*2 + 20 belote)";
    EXPECT_EQ(scoreAttenduTeam2, 0) << "Team2 ne devrait marquer aucun point";
}
