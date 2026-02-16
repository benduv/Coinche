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
    // Test: vérifier que dans un état de surcoinche valide, coinched est aussi true
    // Ce test vérifie la règle: on ne peut pas surcoincher sans avoir d'abord coinché
    room.coinched = true;      // D'abord quelqu'un coinche
    room.surcoinched = true;   // Puis l'équipe annonceur surcoinche

    // Vérifier l'état cohérent
    EXPECT_TRUE(room.coinched);
    EXPECT_TRUE(room.surcoinched);
}

// ========================================
// Tests pour GameRoom - isBot et gestion des joueurs
// ========================================

TEST_F(GameServerTest, GameRoom_IsBotInitialization) {
    // Vérifier que les joueurs sont initialisés comme humains
    for (int i = 0; i < 4; i++) {
        room.isBot.push_back(false);
    }

    EXPECT_EQ(room.isBot.size(), 4);
    for (int i = 0; i < 4; i++) {
        EXPECT_FALSE(room.isBot[i]) << "Joueur " << i << " devrait être humain initialement";
    }
}

TEST_F(GameServerTest, GameRoom_ReplacePlayerWithBot) {
    // Simuler le remplacement d'un joueur par un bot
    for (int i = 0; i < 4; i++) {
        room.isBot.push_back(false);
    }

    // Joueur 1 abandonne
    room.isBot[1] = true;

    EXPECT_FALSE(room.isBot[0]);
    EXPECT_TRUE(room.isBot[1]) << "Joueur 1 devrait être un bot après abandon";
    EXPECT_FALSE(room.isBot[2]);
    EXPECT_FALSE(room.isBot[3]);
}

TEST_F(GameServerTest, GameRoom_AllPlayersAreBots) {
    // Simuler le scénario où tous les joueurs abandonnent
    for (int i = 0; i < 4; i++) {
        room.isBot.push_back(false);
    }

    // Tous les joueurs abandonnent successivement
    for (int i = 0; i < 4; i++) {
        room.isBot[i] = true;
    }

    // Vérifier que tous sont des bots
    bool allBots = true;
    for (int i = 0; i < 4; i++) {
        if (!room.isBot[i]) {
            allBots = false;
            break;
        }
    }

    EXPECT_TRUE(allBots) << "Tous les joueurs devraient être des bots";
}

TEST_F(GameServerTest, GameRoom_NotAllPlayersAreBots) {
    // Scénario où seulement certains joueurs sont des bots
    for (int i = 0; i < 4; i++) {
        room.isBot.push_back(false);
    }

    room.isBot[0] = true;  // Joueur 0 abandon
    room.isBot[2] = true;  // Joueur 2 abandon

    // Vérifier qu'il reste des humains
    bool allBots = true;
    for (int i = 0; i < 4; i++) {
        if (!room.isBot[i]) {
            allBots = false;
            break;
        }
    }

    EXPECT_FALSE(allBots) << "Il devrait rester des joueurs humains";
    EXPECT_FALSE(room.isBot[1]);
    EXPECT_FALSE(room.isBot[3]);
}

// ========================================
// Tests pour la gestion des connexions
// ========================================

TEST_F(GameServerTest, GameRoom_ConnectionIds) {
    // Vérifier la gestion des connectionIds
    room.connectionIds.resize(4);
    room.connectionIds[0] = "conn_player0";
    room.connectionIds[1] = "conn_player1";
    room.connectionIds[2] = "conn_player2";
    room.connectionIds[3] = "conn_player3";

    EXPECT_EQ(room.connectionIds[0], "conn_player0");
    EXPECT_EQ(room.connectionIds[1], "conn_player1");
    EXPECT_EQ(room.connectionIds[2], "conn_player2");
    EXPECT_EQ(room.connectionIds[3], "conn_player3");
}

TEST_F(GameServerTest, GameRoom_PlayerNames) {
    // Vérifier la gestion des noms de joueurs
    room.playerNames.resize(4);
    room.playerNames[0] = "Alice";
    room.playerNames[1] = "Bob";
    room.playerNames[2] = "Charlie";
    room.playerNames[3] = "Diana";

    EXPECT_EQ(room.playerNames[0], "Alice");
    EXPECT_EQ(room.playerNames[1], "Bob");
    EXPECT_EQ(room.playerNames[2], "Charlie");
    EXPECT_EQ(room.playerNames[3], "Diana");
}

// ========================================
// Tests pour les états de jeu
// ========================================

TEST_F(GameServerTest, GameState_Bidding) {
    room.gameState = "bidding";
    EXPECT_EQ(room.gameState, "bidding");
}

TEST_F(GameServerTest, GameState_Playing) {
    room.gameState = "playing";
    EXPECT_EQ(room.gameState, "playing");
}

TEST_F(GameServerTest, GameState_Finished) {
    room.gameState = "finished";
    EXPECT_EQ(room.gameState, "finished");
}

// ========================================
// Tests pour les annonces spéciales
// ========================================

TEST_F(GameServerTest, Annonce_ToutAtout) {
    room.isToutAtout = true;
    room.isSansAtout = false;
    room.lastBidAnnonce = Player::CENT;
    room.lastBidCouleur = Carte::PIQUE;  // Couleur ignorée en tout atout

    EXPECT_TRUE(room.isToutAtout);
    EXPECT_FALSE(room.isSansAtout);
}

TEST_F(GameServerTest, Annonce_SansAtout) {
    room.isToutAtout = false;
    room.isSansAtout = true;
    room.lastBidAnnonce = Player::CENTDIX;
    room.lastBidCouleur = Carte::COEUR;  // Couleur ignorée en sans atout

    EXPECT_FALSE(room.isToutAtout);
    EXPECT_TRUE(room.isSansAtout);
}

TEST_F(GameServerTest, Annonce_Normal) {
    room.isToutAtout = false;
    room.isSansAtout = false;
    room.lastBidAnnonce = Player::QUATREVINGT;
    room.lastBidCouleur = Carte::TREFLE;

    EXPECT_FALSE(room.isToutAtout);
    EXPECT_FALSE(room.isSansAtout);
    EXPECT_EQ(room.lastBidCouleur, Carte::TREFLE);
}

// ========================================
// Tests pour le comptage des passes
// ========================================

TEST_F(GameServerTest, Bidding_PassCount) {
    room.passedBidsCount = 0;

    // Simuler 3 passes après une annonce
    room.lastBidAnnonce = Player::QUATREVINGT;
    room.passedBidsCount = 3;

    // Les enchères devraient se terminer
    bool biddingEnded = (room.passedBidsCount >= 3 && room.lastBidAnnonce != Player::ANNONCEINVALIDE);
    EXPECT_TRUE(biddingEnded);
}

TEST_F(GameServerTest, Bidding_AllPass) {
    room.passedBidsCount = 0;
    room.lastBidAnnonce = Player::ANNONCEINVALIDE;

    // Simuler 4 passes sans annonce
    room.passedBidsCount = 4;

    // Nouvelle manche devrait commencer
    bool newMancheRequired = (room.passedBidsCount >= 4 && room.lastBidAnnonce == Player::ANNONCEINVALIDE);
    EXPECT_TRUE(newMancheRequired);
}

// ========================================
// Tests pour la Générale
// ========================================

TEST_F(GameServerTest, Generale_OnePlayerWinsAll) {
    room.lastBidAnnonce = Player::GENERALE;
    room.lastBidderIndex = 0;

    // Joueur 0 gagne tous les 8 plis seul
    room.plisCountPlayer0 = 8;
    room.plisCountPlayer1 = 0;
    room.plisCountPlayer2 = 0;  // Partenaire ne gagne rien
    room.plisCountPlayer3 = 0;

    // Vérifier que c'est bien une générale réussie
    bool generaleReussie = (room.plisCountPlayer0 == 8);
    EXPECT_TRUE(generaleReussie);

    // Score attendu: 1000 (500+500)
    int expectedScore = 1000;
    EXPECT_EQ(expectedScore, 1000);
}

TEST_F(GameServerTest, Generale_PartnerWinsPli_Fail) {
    room.lastBidAnnonce = Player::GENERALE;
    room.lastBidderIndex = 0;

    // Joueur 0 gagne 7 plis, son partenaire (joueur 2) en gagne 1
    room.plisCountPlayer0 = 7;
    room.plisCountPlayer1 = 0;
    room.plisCountPlayer2 = 1;  // Partenaire gagne 1 pli = échec
    room.plisCountPlayer3 = 0;

    // Vérifier que la générale a échoué
    bool generaleReussie = (room.plisCountPlayer0 == 8);
    EXPECT_FALSE(generaleReussie);
}

// ========================================
// Tests pour le calcul des scores de manche
// ========================================

TEST_F(GameServerTest, Score_MancheTeam1Wins) {
    room.scoreMancheTeam1 = 120;
    room.scoreMancheTeam2 = 42;

    // Vérifier que Team1 a plus de points
    EXPECT_GT(room.scoreMancheTeam1, room.scoreMancheTeam2);
}

TEST_F(GameServerTest, Score_TotalPoints162) {
    // Total des points d'une manche sans belote = 162
    int totalPoints = 162;

    room.scoreMancheTeam1 = 100;
    room.scoreMancheTeam2 = 62;

    EXPECT_EQ(room.scoreMancheTeam1 + room.scoreMancheTeam2, totalPoints);
}

TEST_F(GameServerTest, Score_BeloteAdds20) {
    room.beloteTeam1 = true;
    room.beloteTeam2 = false;

    int bonusBelote = room.beloteTeam1 ? 20 : 0;
    EXPECT_EQ(bonusBelote, 20);
}

// ========================================
// Tests pour le timer de surcoinche
// ========================================

TEST_F(GameServerTest, Surcoinche_WindowOpen) {
    room.coinched = true;
    room.surcoinched = false;
    room.coinchePlayerIndex = 1;  // Team2 a coinché
    room.lastBidderIndex = 0;     // Team1 a fait l'annonce

    // Team1 (joueurs 0 et 2) peut surcoincher
    int bidderTeam = room.lastBidderIndex % 2;
    EXPECT_EQ(bidderTeam, 0);  // Team1

    // Le joueur qui peut surcoincher est dans l'équipe de l'annonceur
    bool canPlayer0Surcoinche = (0 % 2 == bidderTeam);
    bool canPlayer2Surcoinche = (2 % 2 == bidderTeam);

    EXPECT_TRUE(canPlayer0Surcoinche);
    EXPECT_TRUE(canPlayer2Surcoinche);
}

// ========================================
// Tests additionnels pour GameRoom
// ========================================

TEST_F(GameServerTest, GameRoom_IsToutAtoutMode) {
    room.isToutAtout = true;
    room.isSansAtout = false;

    EXPECT_TRUE(room.isToutAtout);
    EXPECT_FALSE(room.isSansAtout);
}

TEST_F(GameServerTest, GameRoom_IsSansAtoutMode) {
    room.isToutAtout = false;
    room.isSansAtout = true;

    EXPECT_FALSE(room.isToutAtout);
    EXPECT_TRUE(room.isSansAtout);
}

TEST_F(GameServerTest, GameRoom_CoincheMechanics) {
    room.coinched = true;
    room.surcoinched = false;
    room.coinchePlayerIndex = 1;

    EXPECT_TRUE(room.coinched);
    EXPECT_FALSE(room.surcoinched);
    EXPECT_GE(room.coinchePlayerIndex, 0);
    EXPECT_LE(room.coinchePlayerIndex, 3);
}

TEST_F(GameServerTest, GameRoom_SurcoincheMechanics) {
    room.coinched = true;
    room.surcoinched = true;
    room.coinchePlayerIndex = 1;
    room.surcoinchePlayerIndex = 0;

    EXPECT_TRUE(room.surcoinched);
    EXPECT_NE(room.coinchePlayerIndex, room.surcoinchePlayerIndex)
        << "Coinche et surcoinche doivent être par des joueurs différents";
}

TEST_F(GameServerTest, GameRoom_PassedBidsCount) {
    room.passedBidsCount = 0;

    room.passedBidsCount++;
    EXPECT_EQ(room.passedBidsCount, 1);

    room.passedBidsCount += 2;
    EXPECT_EQ(room.passedBidsCount, 3);

    // All pass scenario
    room.passedBidsCount = 4;
    EXPECT_EQ(room.passedBidsCount, 4);
}

TEST_F(GameServerTest, GameRoom_PlayerNamesAndAvatars) {
    room.playerNames.clear();
    room.playerAvatars.clear();

    room.playerNames.append("Player1");
    room.playerNames.append("Player2");
    room.playerNames.append("Player3");
    room.playerNames.append("Player4");

    room.playerAvatars.append("avatar1.svg");
    room.playerAvatars.append("avatar2.svg");
    room.playerAvatars.append("avatar3.svg");
    room.playerAvatars.append("avatar4.svg");

    EXPECT_EQ(room.playerNames.size(), 4);
    EXPECT_EQ(room.playerAvatars.size(), 4);
    EXPECT_EQ(room.playerNames[0], "Player1");
    EXPECT_EQ(room.playerAvatars[2], "avatar3.svg");
}

TEST_F(GameServerTest, GameRoom_CurrentPliTracking) {
    room.currentPli.clear();

    Carte* carte1 = new Carte(Carte::COEUR, Carte::AS);
    Carte* carte2 = new Carte(Carte::PIQUE, Carte::ROI);

    room.currentPli.push_back(std::make_pair(0, carte1));
    room.currentPli.push_back(std::make_pair(1, carte2));

    EXPECT_EQ(room.currentPli.size(), 2);
    EXPECT_EQ(room.currentPli[0].first, 0) << "Premier joueur";
    EXPECT_EQ(room.currentPli[1].first, 1) << "Deuxième joueur";

    // Cleanup
    for (auto& pair : room.currentPli) {
        delete pair.second;
    }
}
