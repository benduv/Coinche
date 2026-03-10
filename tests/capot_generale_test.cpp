#include <gtest/gtest.h>
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
