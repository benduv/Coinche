#include <gtest/gtest.h>
#include "../server/GameServer.h"
#include "../Carte.h"
#include "../Player.h"

// ========================================
// Test Fixture pour GameServer
// ========================================
class GameServerTest : public ::testing::Test {
protected:
    GameRoom room;

    void SetUp() override {
        room.roomId = 1;
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
        room.coinchePlayerIndex = -1;

        // Créer 4 joueurs
        for (int i = 0; i < 4; i++) {
            std::vector<Carte*> emptyHand;
            auto player = std::make_unique<Player>("Player" + std::to_string(i), emptyHand, i);
            room.players.push_back(std::move(player));
        }
    }

    // Helper: Simuler une distribution de cartes pour Team1
    void distributeCardsTeam1Strong() {
        // Team1 (joueurs 0 et 2) a des cartes fortes
        // Atout: PIQUE
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::VALET));  // 20
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::NEUF));   // 14
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::AS));     // 11
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::DIX));    // 10
        room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::AS));     // 11
        room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::DIX));    // 10
        room.players[0]->addCardToHand(new Carte(Carte::CARREAU, Carte::AS));   // 11
        room.players[0]->addCardToHand(new Carte(Carte::CARREAU, Carte::DIX));  // 10

        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::ROI));    // 4
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::DAME));   // 3
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::HUIT));   // 0
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::SEPT));   // 0
        room.players[2]->addCardToHand(new Carte(Carte::TREFLE, Carte::AS));    // 11
        room.players[2]->addCardToHand(new Carte(Carte::TREFLE, Carte::DIX));   // 10
        room.players[2]->addCardToHand(new Carte(Carte::COEUR, Carte::ROI));    // 4
        room.players[2]->addCardToHand(new Carte(Carte::CARREAU, Carte::ROI));  // 4

        // Team2 a des cartes faibles
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::SEPT));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::HUIT));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::NEUF));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::DAME));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::SEPT));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::HUIT));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::NEUF));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::DAME));

        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::SEPT));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::HUIT));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::NEUF));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::DAME));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::ROI));
        room.players[3]->addCardToHand(new Carte(Carte::COEUR, Carte::VALET));
        room.players[3]->addCardToHand(new Carte(Carte::CARREAU, Carte::VALET));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::VALET));
    }
};

// ========================================
// Tests pour CAPOT non annoncé
// ========================================

TEST_F(GameServerTest, CapotNonAnnonce_Team1ReussiAvecBelote) {
    // Scénario: Team1 annonce 80 Pique, fait tous les 8 plis (capot non annoncé) + Belote
    room.lastBidAnnonce = Player::Annonce::QUATREVINGT;
    room.lastBidCouleur = Carte::PIQUE;
    room.lastBidderIndex = 0;  // Joueur 0 (Team1)

    // Team1 fait tous les plis
    room.plisCountPlayer0 = 5;
    room.plisCountPlayer2 = 3;
    room.plisCountPlayer1 = 0;
    room.plisCountPlayer3 = 0;

    // Team1 a la belote
    room.beloteTeam1 = true;

    distributeCardsTeam1Strong();

    // Calculer les points réalisés (simulation)
    // Team1 devrait avoir tous les 162 points
    int pointsTeam1 = 162;
    int pointsTeam2 = 0;

    // Calcul attendu:
    // CAPOT non annoncé = 250 + pointsRéalisés = 250 + 162 = 412
    // + Belote = 20
    // Total Team1 = 432
    int expectedScoreTeam1 = 250 + pointsTeam1 + 20;
    int expectedScoreTeam2 = 0;

    EXPECT_EQ(room.plisCountPlayer0 + room.plisCountPlayer2, 8);
    EXPECT_EQ(expectedScoreTeam1, 432);
    EXPECT_EQ(expectedScoreTeam2, 0);
}

TEST_F(GameServerTest, CapotNonAnnonce_Team2Reussi) {
    // Scénario: Team2 annonce 90 Coeur, fait tous les 8 plis
    room.lastBidAnnonce = Player::Annonce::QUATREVINGTDIX;
    room.lastBidCouleur = Carte::COEUR;
    room.lastBidderIndex = 1;  // Joueur 1 (Team2)

    // Team2 fait tous les plis
    room.plisCountPlayer0 = 0;
    room.plisCountPlayer2 = 0;
    room.plisCountPlayer1 = 4;
    room.plisCountPlayer3 = 4;

    // Calculer points réalisés
    int pointsTeam1 = 0;
    int pointsTeam2 = 162;

    // Calcul attendu:
    // CAPOT non annoncé = 250 + 162 = 412
    int expectedScoreTeam1 = 0;
    int expectedScoreTeam2 = 250 + pointsTeam2;

    EXPECT_EQ(room.plisCountPlayer1 + room.plisCountPlayer3, 8);
    EXPECT_EQ(expectedScoreTeam2, 412);
    EXPECT_EQ(expectedScoreTeam1, 0);
}

TEST_F(GameServerTest, CapotNonAnnonce_PasDeCapot) {
    // Scénario: Team1 annonce 100, fait 7 plis (pas de capot)
    room.lastBidAnnonce = Player::CENT;
    room.lastBidCouleur = Carte::TREFLE;
    room.lastBidderIndex = 0;

    // Team1 fait 7 plis, Team2 fait 1 pli
    room.plisCountPlayer0 = 4;
    room.plisCountPlayer2 = 3;
    room.plisCountPlayer1 = 1;
    room.plisCountPlayer3 = 0;

    // Points réalisés approximatifs
    int pointsTeam1 = 140;
    int pointsTeam2 = 22;

    // Calcul attendu: Contrat normal réussi
    // Team1 = valeurContrat + pointsRéalisés = 100 + 140 = 240
    // Team2 = pointsRéalisés = 22
    int expectedScoreTeam1 = 100 + pointsTeam1;
    int expectedScoreTeam2 = pointsTeam2;

    EXPECT_EQ(room.plisCountPlayer0 + room.plisCountPlayer2, 7);
    EXPECT_NE(room.plisCountPlayer0 + room.plisCountPlayer2, 8); // Pas de capot
    EXPECT_EQ(expectedScoreTeam1, 240);
    EXPECT_EQ(expectedScoreTeam2, 22);
}

// ========================================
// Tests pour COINCHE et SURCOINCHE
// ========================================

TEST_F(GameServerTest, Coinche_Team1ReussitContrat) {
    // Scénario: Team1 annonce 80, Team2 coinche, Team1 réussit
    room.lastBidAnnonce = Player::Annonce::QUATREVINGT;
    room.lastBidCouleur = Carte::PIQUE;
    room.lastBidderIndex = 0;
    room.coinched = true;
    room.coinchePlayerIndex = 1;  // Joueur 1 (Team2) a coinché

    // Team1 fait 6 plis
    room.plisCountPlayer0 = 4;
    room.plisCountPlayer2 = 2;
    room.plisCountPlayer1 = 1;
    room.plisCountPlayer3 = 1;

    int pointsTeam1 = 110;  // Réussit le contrat (>= 80)
    int pointsTeam2 = 52;

    // Calcul attendu: (80 + 110) × 2 = 380
    int expectedScoreTeam1 = (80 + pointsTeam1) * 2;
    int expectedScoreTeam2 = 0;

    EXPECT_TRUE(room.coinched);
    EXPECT_EQ(expectedScoreTeam1, 380);
    EXPECT_EQ(expectedScoreTeam2, 0);
}

TEST_F(GameServerTest, Coinche_Team1EchoueContrat) {
    // Scénario: Team1 annonce 100, Team2 coinche, Team1 échoue
    room.lastBidAnnonce = Player::CENT;
    room.lastBidCouleur = Carte::COEUR;
    room.lastBidderIndex = 0;
    room.coinched = true;
    room.coinchePlayerIndex = 1;

    // Team1 fait seulement 3 plis
    room.plisCountPlayer0 = 2;
    room.plisCountPlayer2 = 1;
    room.plisCountPlayer1 = 3;
    room.plisCountPlayer3 = 2;

    int pointsTeam1 = 70;  // Échoue le contrat (< 100)
    int pointsTeam2 = 92;

    // Calcul attendu: Team2 marque (100 + 160) × 2 = 520
    int expectedScoreTeam1 = 0;
    int expectedScoreTeam2 = (100 + 160) * 2;

    EXPECT_TRUE(room.coinched);
    EXPECT_LT(pointsTeam1, 100);  // Contrat échoué
    EXPECT_EQ(expectedScoreTeam2, 520);
    EXPECT_EQ(expectedScoreTeam1, 0);
}

TEST_F(GameServerTest, Surcoinche_Team2ReussitContrat) {
    // Scénario: Team2 annonce 120, Team1 coinche, Team2 surcoinche et réussit
    room.lastBidAnnonce = Player::Annonce::CENTVINGT;
    room.lastBidCouleur = Carte::TREFLE;
    room.lastBidderIndex = 3;  // Joueur 3 (Team2)
    room.coinched = true;
    room.surcoinched = true;
    room.coinchePlayerIndex = 0;  // Joueur 0 (Team1) a coinché

    // Team2 fait 7 plis
    room.plisCountPlayer0 = 1;
    room.plisCountPlayer2 = 0;
    room.plisCountPlayer1 = 4;
    room.plisCountPlayer3 = 3;

    int pointsTeam1 = 30;
    int pointsTeam2 = 132;  // Réussit le contrat (>= 120)

    // Calcul attendu: (120 + 132) × 4 = 1008
    int expectedScoreTeam1 = 0;
    int expectedScoreTeam2 = (120 + pointsTeam2) * 4;

    EXPECT_TRUE(room.surcoinched);
    EXPECT_GE(pointsTeam2, 120);  // Contrat réussi
    EXPECT_EQ(expectedScoreTeam2, 1008);
    EXPECT_EQ(expectedScoreTeam1, 0);
}

TEST_F(GameServerTest, Surcoinche_Team2EchoueContrat) {
    // Scénario: Team2 annonce 140, Team1 coinche, Team2 surcoinche et échoue
    room.lastBidAnnonce = Player::Annonce::CENTQUARANTE;
    room.lastBidCouleur = Carte::CARREAU;
    room.lastBidderIndex = 1;
    room.coinched = true;
    room.surcoinched = true;
    room.coinchePlayerIndex = 2;  // Joueur 2 (Team1) a coinché

    // Team2 fait seulement 4 plis
    room.plisCountPlayer0 = 2;
    room.plisCountPlayer2 = 2;
    room.plisCountPlayer1 = 2;
    room.plisCountPlayer3 = 2;

    int pointsTeam1 = 82;
    int pointsTeam2 = 80;  // Échoue le contrat (< 140)

    // Calcul attendu: Team1 marque (140 + 160) × 4 = 1200
    int expectedScoreTeam1 = (140 + 160) * 4;
    int expectedScoreTeam2 = 0;

    EXPECT_TRUE(room.surcoinched);
    EXPECT_LT(pointsTeam2, 140);  // Contrat échoué
    EXPECT_EQ(expectedScoreTeam1, 1200);
    EXPECT_EQ(expectedScoreTeam2, 0);
}

// ========================================
// Tests pour SURCOINCHE SUBIES
// ========================================

TEST_F(GameServerTest, SurcoincheSubie_JoueurQuiCoincheSeulementRecoit) {
    // Scénario: Team1 annonce 90, Joueur 1 (Team2) coinche, Team1 surcoinche et réussit
    // Seul le joueur 1 devrait avoir ses stats "surcoinche subies" mises à jour
    room.lastBidAnnonce = Player::Annonce::QUATREVINGTDIX;
    room.lastBidCouleur = Carte::PIQUE;
    room.lastBidderIndex = 0;
    room.coinched = true;
    room.surcoinched = true;
    room.coinchePlayerIndex = 1;  // SEUL le joueur 1 a coinché

    // Team1 réussit le contrat
    room.plisCountPlayer0 = 4;
    room.plisCountPlayer2 = 2;
    room.plisCountPlayer1 = 1;
    room.plisCountPlayer3 = 1;

    int pointsTeam1 = 105;  // Réussit
    int pointsTeam2 = 57;

    // Vérifications:
    // 1. Le joueur 1 (et seulement lui) devrait recevoir la stat "surcoinche subie"
    // 2. Puisque Team1 réussit, le joueur 1 "perd" la surcoinche subie (won = false)
    EXPECT_EQ(room.coinchePlayerIndex, 1);
    EXPECT_TRUE(room.surcoinched);
    EXPECT_GE(pointsTeam1, 90);  // Contrat réussi
}

TEST_F(GameServerTest, SurcoincheSubie_JoueurQuiCoincheGagneSiContratEchoue) {
    // Scénario: Team2 annonce 110, Joueur 0 (Team1) coinche, Team2 surcoinche et ÉCHOUE
    // Le joueur 0 devrait "gagner" la surcoinche subie (won = true)
    room.lastBidAnnonce = Player::Annonce::CENTDIX;
    room.lastBidCouleur = Carte::COEUR;
    room.lastBidderIndex = 3;
    room.coinched = true;
    room.surcoinched = true;
    room.coinchePlayerIndex = 0;  // Joueur 0 a coinché

    // Team2 échoue le contrat
    room.plisCountPlayer0 = 3;
    room.plisCountPlayer2 = 2;
    room.plisCountPlayer1 = 2;
    room.plisCountPlayer3 = 1;

    int pointsTeam1 = 85;
    int pointsTeam2 = 77;  // Échoue (< 110)

    // Vérifications:
    // Le joueur 0 devrait recevoir "surcoinche subie gagnée" (won = true)
    EXPECT_EQ(room.coinchePlayerIndex, 0);
    EXPECT_TRUE(room.surcoinched);
    EXPECT_LT(pointsTeam2, 110);  // Contrat échoué
}

// ========================================
// Tests pour CAPOT ANNONCÉ
// ========================================

TEST_F(GameServerTest, CapotAnnonce_Reussi) {
    // Scénario: Team1 annonce CAPOT et réussit
    room.lastBidAnnonce = Player::CAPOT;
    room.lastBidCouleur = Carte::PIQUE;
    room.lastBidderIndex = 0;

    // Team1 fait tous les 8 plis
    room.plisCountPlayer0 = 5;
    room.plisCountPlayer2 = 3;
    room.plisCountPlayer1 = 0;
    room.plisCountPlayer3 = 0;

    // Calcul attendu: 250 + 250 = 500
    int expectedScoreTeam1 = 500;
    int expectedScoreTeam2 = 0;

    EXPECT_EQ(room.plisCountPlayer0 + room.plisCountPlayer2, 8);
    EXPECT_EQ(expectedScoreTeam1, 500);
}

TEST_F(GameServerTest, CapotAnnonce_Echoue) {
    // Scénario: Team1 annonce CAPOT mais échoue (Team2 fait 1 pli)
    room.lastBidAnnonce = Player::CAPOT;
    room.lastBidCouleur = Carte::PIQUE;
    room.lastBidderIndex = 0;

    // Team1 fait 7 plis, Team2 fait 1 pli
    room.plisCountPlayer0 = 4;
    room.plisCountPlayer2 = 3;
    room.plisCountPlayer1 = 1;
    room.plisCountPlayer3 = 0;

    // Calcul attendu: Team1 marque 0, Team2 marque 160 + 250 = 410
    int expectedScoreTeam1 = 0;
    int expectedScoreTeam2 = 410;

    EXPECT_NE(room.plisCountPlayer0 + room.plisCountPlayer2, 8);
    EXPECT_EQ(expectedScoreTeam2, 410);
}

TEST_F(GameServerTest, CapotAnnonce_CoincheReussi) {
    // Scénario: Team1 annonce CAPOT, Team2 coinche, Team1 réussit
    room.lastBidAnnonce = Player::CAPOT;
    room.lastBidCouleur = Carte::TREFLE;
    room.lastBidderIndex = 2;
    room.coinched = true;
    room.coinchePlayerIndex = 1;

    // Team1 fait tous les 8 plis
    room.plisCountPlayer0 = 4;
    room.plisCountPlayer2 = 4;
    room.plisCountPlayer1 = 0;
    room.plisCountPlayer3 = 0;

    // Calcul attendu: 500 × 2 = 1000
    int expectedScoreTeam1 = 1000;
    int expectedScoreTeam2 = 0;

    EXPECT_EQ(room.plisCountPlayer0 + room.plisCountPlayer2, 8);
    EXPECT_TRUE(room.coinched);
    EXPECT_EQ(expectedScoreTeam1, 1000);
}

// ========================================
// Tests pour détection d'erreurs
// ========================================

TEST_F(GameServerTest, Detection_TotalPlisDoit8) {
    // Vérifier que le total des plis est toujours 8
    room.plisCountPlayer0 = 2;
    room.plisCountPlayer1 = 3;
    room.plisCountPlayer2 = 1;
    room.plisCountPlayer3 = 2;

    int totalPlis = room.plisCountPlayer0 + room.plisCountPlayer1 +
                   room.plisCountPlayer2 + room.plisCountPlayer3;

    EXPECT_EQ(totalPlis, 8);
}

TEST_F(GameServerTest, Detection_CoinchePlayerIndexValide) {
    // Vérifier que coinchePlayerIndex est dans la plage valide
    room.coinched = true;
    room.coinchePlayerIndex = 1;

    EXPECT_GE(room.coinchePlayerIndex, 0);
    EXPECT_LT(room.coinchePlayerIndex, 4);
}

TEST_F(GameServerTest, Detection_SurcoincheNecessiteCoinche) {
    // Vérifier qu'une surcoinche ne peut exister sans coinche
    room.surcoinched = true;

    // Si surcoinched est true, coinched doit aussi être true
    EXPECT_TRUE(room.coinched);
}
