#include <gtest/gtest.h>
#include "../server/GameServer.h"
#include "../Carte.h"
#include "../Player.h"

// ========================================
// Test Fixture pour COINCHE et SURCOINCHE
// (tests de logique de jeu avec compteurs de plis)
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

        for (int i = 0; i < 4; i++) {
            std::vector<Carte*> emptyHand;
            auto player = std::make_unique<Player>("TestPlayer" + std::to_string(i), emptyHand, i);
            room.players.push_back(std::move(player));
        }
    }

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
// TESTS COMPTEURS DE PLIS COINCHE
// ========================================

TEST_F(CoincheTest, Team1GagneTousLesPlis) {
    for (int pli = 0; pli < 8; pli++) {
        playPliAutomatic(pli, true);
    }
    int plisTeam1 = room.plisCountPlayer0 + room.plisCountPlayer2;
    int plisTeam2 = room.plisCountPlayer1 + room.plisCountPlayer3;
    EXPECT_EQ(8, plisTeam1);
    EXPECT_EQ(0, plisTeam2);
}

TEST_F(CoincheTest, Team2Gagne7PlisTeam1Gagne1) {
    for (int pli = 0; pli < 7; pli++) {
        playPliAutomatic(pli, false);
    }
    playPliAutomatic(7, true);

    int plisTeam1 = room.plisCountPlayer0 + room.plisCountPlayer2;
    int plisTeam2 = room.plisCountPlayer1 + room.plisCountPlayer3;
    EXPECT_EQ(1, plisTeam1);
    EXPECT_EQ(7, plisTeam2);
}
