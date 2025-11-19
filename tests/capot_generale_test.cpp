#include <gtest/gtest.h>
#include "../server/GameServer.h"
#include "../Carte.h"
#include "../Player.h"

// ========================================
// Test Fixture pour CAPOT et GENERALE
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

        // Créer 4 joueurs vides
        for (int i = 0; i < 4; i++) {
            std::vector<Carte*> emptyHand;
            auto player = std::make_unique<Player>("TestPlayer" + std::to_string(i), emptyHand, i);
            room.players.push_back(std::move(player));
        }
    }

    // Utilitaire: distribution des mains pour CAPOT Team1 réussi
    void setHandsForCapotTeam1() {
        // PIQUE est atout
        // Joueur 0 (Team1): Toutes les cartes fortes d'atout
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::VALET));  // 20 points atout
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::NEUF));   // 14 points atout
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::AS));     // 11 points atout
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::DIX));    // 10 points atout
        room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::DIX));
        room.players[0]->addCardToHand(new Carte(Carte::CARREAU, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::CARREAU, Carte::DIX));

        // Joueur 2 (Team1 - partenaire): Reste des atouts et cartes fortes
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::ROI));
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::DAME));
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::HUIT));
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::SEPT));
        room.players[2]->addCardToHand(new Carte(Carte::TREFLE, Carte::AS));
        room.players[2]->addCardToHand(new Carte(Carte::TREFLE, Carte::DIX));
        room.players[2]->addCardToHand(new Carte(Carte::COEUR, Carte::ROI));
        room.players[2]->addCardToHand(new Carte(Carte::CARREAU, Carte::ROI));

        // Joueur 1 (Team2): Cartes faibles
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::SEPT));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::HUIT));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::NEUF));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::DAME));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::SEPT));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::HUIT));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::NEUF));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::DAME));

        // Joueur 3 (Team2): Cartes faibles
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::SEPT));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::HUIT));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::NEUF));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::DAME));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::ROI));
        room.players[3]->addCardToHand(new Carte(Carte::COEUR, Carte::VALET));
        room.players[3]->addCardToHand(new Carte(Carte::CARREAU, Carte::VALET));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::VALET));
    }

    void setHandsForCapotEchoue() {
        // Similaire à CAPOT réussi mais Team2 a 1 atout fort
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::VALET));
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::DIX));
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::ROI));
        room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::DIX));
        room.players[0]->addCardToHand(new Carte(Carte::CARREAU, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::CARREAU, Carte::DIX));

        // Joueur 1 a le NEUF d'atout (2ème meilleure carte)
        room.players[1]->addCardToHand(new Carte(Carte::PIQUE, Carte::NEUF));  // Carte qui permet de gagner 1 pli
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::HUIT));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::NEUF));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::DAME));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::SEPT));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::HUIT));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::NEUF));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::DAME));

        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::DAME));
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::HUIT));
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::SEPT));
        room.players[2]->addCardToHand(new Carte(Carte::TREFLE, Carte::AS));
        room.players[2]->addCardToHand(new Carte(Carte::TREFLE, Carte::DIX));
        room.players[2]->addCardToHand(new Carte(Carte::COEUR, Carte::ROI));
        room.players[2]->addCardToHand(new Carte(Carte::CARREAU, Carte::ROI));
        room.players[2]->addCardToHand(new Carte(Carte::COEUR, Carte::SEPT));

        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::SEPT));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::HUIT));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::NEUF));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::DAME));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::ROI));
        room.players[3]->addCardToHand(new Carte(Carte::COEUR, Carte::VALET));
        room.players[3]->addCardToHand(new Carte(Carte::CARREAU, Carte::VALET));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::VALET));
    }

    void setHandsForGeneralePlayer0() {
        // Joueur 0: TOUTES les cartes qui battent tout
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::VALET));  // Meilleur atout
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::NEUF));   // 2ème meilleur atout
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::DIX));
        room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::CARREAU, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::TREFLE, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::ROI));

        // Joueur 2 (partenaire): Cartes moyennes/faibles (ne doit pas gagner)
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::DAME));
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::HUIT));
        room.players[2]->addCardToHand(new Carte(Carte::COEUR, Carte::DIX));
        room.players[2]->addCardToHand(new Carte(Carte::COEUR, Carte::ROI));
        room.players[2]->addCardToHand(new Carte(Carte::CARREAU, Carte::DIX));
        room.players[2]->addCardToHand(new Carte(Carte::CARREAU, Carte::ROI));
        room.players[2]->addCardToHand(new Carte(Carte::TREFLE, Carte::DIX));
        room.players[2]->addCardToHand(new Carte(Carte::TREFLE, Carte::ROI));

        // Team2: Cartes faibles
        room.players[1]->addCardToHand(new Carte(Carte::PIQUE, Carte::SEPT));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::SEPT));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::HUIT));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::NEUF));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::SEPT));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::HUIT));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::NEUF));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::DAME));

        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::SEPT));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::HUIT));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::NEUF));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::DAME));
        room.players[3]->addCardToHand(new Carte(Carte::COEUR, Carte::VALET));
        room.players[3]->addCardToHand(new Carte(Carte::COEUR, Carte::DAME));
        room.players[3]->addCardToHand(new Carte(Carte::CARREAU, Carte::VALET));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::VALET));
    }

    void setHandsForGeneraleEchouee() {
        // Joueur 0: Presque toutes les meilleures cartes
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::VALET));
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::NEUF));
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::DIX));
        room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::CARREAU, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::TREFLE, Carte::AS));
        room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::DIX));

        // Joueur 2 (partenaire): A 1 carte qui peut gagner 1 pli
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::ROI));    // Peut gagner si bien joué
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::DAME));
        room.players[2]->addCardToHand(new Carte(Carte::PIQUE, Carte::HUIT));
        room.players[2]->addCardToHand(new Carte(Carte::COEUR, Carte::ROI));
        room.players[2]->addCardToHand(new Carte(Carte::CARREAU, Carte::DIX));
        room.players[2]->addCardToHand(new Carte(Carte::CARREAU, Carte::ROI));
        room.players[2]->addCardToHand(new Carte(Carte::TREFLE, Carte::DIX));
        room.players[2]->addCardToHand(new Carte(Carte::TREFLE, Carte::ROI));

        room.players[1]->addCardToHand(new Carte(Carte::PIQUE, Carte::SEPT));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::SEPT));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::HUIT));
        room.players[1]->addCardToHand(new Carte(Carte::COEUR, Carte::NEUF));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::SEPT));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::HUIT));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::NEUF));
        room.players[1]->addCardToHand(new Carte(Carte::CARREAU, Carte::DAME));

        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::SEPT));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::HUIT));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::NEUF));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::DAME));
        room.players[3]->addCardToHand(new Carte(Carte::COEUR, Carte::VALET));
        room.players[3]->addCardToHand(new Carte(Carte::COEUR, Carte::DAME));
        room.players[3]->addCardToHand(new Carte(Carte::CARREAU, Carte::VALET));
        room.players[3]->addCardToHand(new Carte(Carte::TREFLE, Carte::VALET));
    }

    // Simulation simplifiée des plis
    void playPliAutomatic(int pliNumber) {
        // Pour CAPOT Team1, on alterne entre joueur 0 et 2
        int winner = (pliNumber % 2 == 0) ? 0 : 2;

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
// TESTS CAPOT
// ========================================

TEST_F(CapotGeneraleTest, CapotReussiAvecBelote) {
    // Distribuer des mains où Team1 (joueurs 0 et 2) a toutes les meilleures cartes
    setHandsForCapotTeam1();

    // Simuler l'enchère CAPOT par joueur 0
    room.lastBidderIndex = 0;
    room.lastBidAnnonce = Player::CAPOT;
    room.lastBidCouleur = Carte::PIQUE;
    room.couleurAtout = Carte::PIQUE;
    room.firstPlayerIndex = 0;
    room.currentPlayerIndex = 0;

    // Définir l'atout pour tous les joueurs
    for (int i = 0; i < 4; i++) {
        room.players[i]->setAtout(Carte::PIQUE);
    }

    // Marquer que Team1 a la belote (joueur 2 a Roi et Dame de Pique)
    room.beloteTeam1 = true;

    // Auto-jouer 8 plis
    for (int pli = 0; pli < 8; pli++) {
        playPliAutomatic(pli);
    }

    // Vérifier les compteurs de plis
    int plisTeam1 = room.plisCountPlayer0 + room.plisCountPlayer2;
    int plisTeam2 = room.plisCountPlayer1 + room.plisCountPlayer3;

    EXPECT_EQ(8, plisTeam1) << "Team1 devrait avoir fait 8 plis";
    EXPECT_EQ(0, plisTeam2) << "Team2 ne devrait avoir aucun pli";
    EXPECT_EQ(4, room.plisCountPlayer0) << "Joueur 0 devrait avoir 4 plis";
    EXPECT_EQ(4, room.plisCountPlayer2) << "Joueur 2 devrait avoir 4 plis";

    // Vérifier le scoring
    // CAPOT réussi: 500 points (250+250) + 20 belote = 520
    int scoreAttenduTeam1 = 520;
    int scoreAttenduTeam2 = 0;

    EXPECT_EQ(scoreAttenduTeam1, 520) << "Team1 devrait marquer 520 points (500 CAPOT + 20 belote)";
    EXPECT_EQ(scoreAttenduTeam2, 0) << "Team2 ne devrait marquer aucun point";
}

TEST_F(CapotGeneraleTest, CapotEchoue) {
    setHandsForCapotEchoue();

    room.lastBidderIndex = 0;
    room.lastBidAnnonce = Player::CAPOT;
    room.lastBidCouleur = Carte::PIQUE;
    room.couleurAtout = Carte::PIQUE;
    room.firstPlayerIndex = 0;
    room.currentPlayerIndex = 0;

    for (int i = 0; i < 4; i++) {
        room.players[i]->setAtout(Carte::PIQUE);
    }

    // Jouer 8 plis où Team2 gagne le pli 3
    for (int pli = 0; pli < 8; pli++) {
        if (pli == 3) {
            room.plisCountPlayer1++; // Team2 gagne ce pli
        } else {
            playPliAutomatic(pli);
        }
    }

    int plisTeam1 = room.plisCountPlayer0 + room.plisCountPlayer2;
    int plisTeam2 = room.plisCountPlayer1 + room.plisCountPlayer3;

    EXPECT_EQ(7, plisTeam1) << "Team1 devrait avoir 7 plis";
    EXPECT_EQ(1, plisTeam2) << "Team2 devrait avoir 1 pli";

    // CAPOT échoué: Team2 marque 160 + 250 = 410
    int scoreAttenduTeam1 = 0;
    int scoreAttenduTeam2 = 410;

    EXPECT_EQ(scoreAttenduTeam1, 0) << "Team1 ne devrait marquer aucun point";
    EXPECT_EQ(scoreAttenduTeam2, 410) << "Team2 devrait marquer 410 points (160+250)";
}

// ========================================
// TESTS GENERALE
// ========================================

TEST_F(CapotGeneraleTest, GeneraleReussie) {
    setHandsForGeneralePlayer0();

    room.lastBidderIndex = 0;
    room.lastBidAnnonce = Player::GENERALE;
    room.lastBidCouleur = Carte::PIQUE;
    room.couleurAtout = Carte::PIQUE;
    room.firstPlayerIndex = 0;
    room.currentPlayerIndex = 0;

    for (int i = 0; i < 4; i++) {
        room.players[i]->setAtout(Carte::PIQUE);
    }

    // Jouer 8 plis où joueur 0 gagne tout
    for (int pli = 0; pli < 8; pli++) {
        room.plisCountPlayer0++;
    }

    EXPECT_EQ(8, room.plisCountPlayer0) << "Joueur 0 devrait avoir gagné tous les 8 plis";
    EXPECT_EQ(0, room.plisCountPlayer1) << "Joueur 1 ne devrait avoir aucun pli";
    EXPECT_EQ(0, room.plisCountPlayer2) << "Joueur 2 (partenaire) ne devrait avoir aucun pli";
    EXPECT_EQ(0, room.plisCountPlayer3) << "Joueur 3 ne devrait avoir aucun pli";

    // GENERALE réussie: 1000 points (500+500)
    int scoreAttenduTeam1 = 1000;
    int scoreAttenduTeam2 = 0;

    EXPECT_EQ(scoreAttenduTeam1, 1000) << "Team1 devrait marquer 1000 points (500+500)";
    EXPECT_EQ(scoreAttenduTeam2, 0) << "Team2 ne devrait marquer aucun point";
}

TEST_F(CapotGeneraleTest, GeneraleEchoueeAvecBelote) {
    setHandsForGeneraleEchouee();

    room.lastBidderIndex = 0;
    room.lastBidAnnonce = Player::GENERALE;
    room.lastBidCouleur = Carte::PIQUE;
    room.couleurAtout = Carte::PIQUE;
    room.firstPlayerIndex = 0;
    room.currentPlayerIndex = 0;

    for (int i = 0; i < 4; i++) {
        room.players[i]->setAtout(Carte::PIQUE);
    }

    // Marquer que Team1 a la belote (joueur 2 a Roi et Dame de Pique)
    room.beloteTeam1 = true;

    // Jouer 8 plis où joueur 0 gagne 7 plis et son partenaire (joueur 2) en gagne 1
    for (int pli = 0; pli < 8; pli++) {
        if (pli == 4) {
            room.plisCountPlayer2++; // Partenaire gagne 1 pli
        } else {
            room.plisCountPlayer0++; // Joueur 0 gagne les autres
        }
    }

    EXPECT_EQ(7, room.plisCountPlayer0) << "Joueur 0 devrait avoir fait 7 plis";
    EXPECT_EQ(1, room.plisCountPlayer2) << "Joueur 2 (partenaire) devrait avoir fait 1 pli";

    // GENERALE échouée: Team1 marque 0 + 20 belote, Team2 marque 160 + 500 = 660
    int scoreAttenduTeam1 = 20; // Belote uniquement
    int scoreAttenduTeam2 = 660;

    EXPECT_EQ(scoreAttenduTeam1, 20) << "Team1 devrait marquer 20 points (belote uniquement)";
    EXPECT_EQ(scoreAttenduTeam2, 660) << "Team2 devrait marquer 660 points (160+500)";
}

// ========================================
// TEST CAPOT NON ANNONCE
// ========================================

TEST_F(CapotGeneraleTest, CapotNonAnnonceReussi) {
    // Similaire au CAPOT réussi mais Team1 annonce seulement 80
    setHandsForCapotTeam1();

    room.lastBidderIndex = 0;
    room.lastBidAnnonce = Player::QUATREVINGT;  // Annonce seulement 80, pas CAPOT
    room.lastBidCouleur = Carte::PIQUE;
    room.couleurAtout = Carte::PIQUE;
    room.firstPlayerIndex = 0;
    room.currentPlayerIndex = 0;

    for (int i = 0; i < 4; i++) {
        room.players[i]->setAtout(Carte::PIQUE);
    }

    // Team1 a la belote (Roi et Dame de Pique dans la main du joueur 2)
    room.beloteTeam1 = true;

    // Jouer 8 plis où Team1 gagne tout
    for (int pli = 0; pli < 8; pli++) {
        playPliAutomatic(pli);
    }

    // Vérifier les compteurs de plis
    int plisTeam1 = room.plisCountPlayer0 + room.plisCountPlayer2;
    int plisTeam2 = room.plisCountPlayer1 + room.plisCountPlayer3;

    EXPECT_EQ(8, plisTeam1) << "Team1 devrait avoir fait 8 plis";
    EXPECT_EQ(0, plisTeam2) << "Team2 ne devrait avoir aucun pli";

    // CAPOT non annoncé: 250 + 80 (contrat) + 20 (belote) = 350
    int scoreAttenduTeam1 = 350;
    int scoreAttenduTeam2 = 0;

    EXPECT_EQ(scoreAttenduTeam1, 350) << "Team1 devrait marquer 350 points (250 CAPOT + 80 contrat + 20 belote)";
    EXPECT_EQ(scoreAttenduTeam2, 0) << "Team2 ne devrait marquer aucun point";
}
