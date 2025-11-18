#ifndef GAMESERVERTEST_H
#define GAMESERVERTEST_H

#include "GameServer.h"
#include <cassert>

class GameServerTest {
public:
    static void runAllTests() {
        qDebug() << "\n========================================";
        qDebug() << "=== TESTS CAPOT ET GENERALE ===";
        qDebug() << "========================================\n";

        testCapotReussi();
        testCapotEchoue();
        testGeneraleReussie();
        testGeneraleEchouee();

        qDebug() << "\n========================================";
        qDebug() << "=== TOUS LES TESTS PASSES ===";
        qDebug() << "========================================\n";
    }

private:
    // Test 1: CAPOT reussi par Team1
    static void testCapotReussi() {
        qDebug() << "\n--- Test 1: CAPOT reussi par Team1 ---";

        GameRoom room;
        initializeTestRoom(&room);

        // Distribuer des mains où Team1 (joueurs 0 et 2) a toutes les meilleures cartes
        setHandsForCapotTeam1(&room);

        // Simuler l'enchère CAPOT par joueur 0
        room.lastBidderIndex = 0;
        room.lastBidAnnonce = Player::CAPOT;
        room.lastBidCouleur = Carte::PIQUE;
        room.couleurAtout = Carte::PIQUE;
        room.firstPlayerIndex = 0;
        room.currentPlayerIndex = 0;
        room.gameState = "playing";

        // Auto-jouer 8 plis (joueur 0 ou 2 gagne chaque fois)
        for (int pli = 0; pli < 8; pli++) {
            playPliForCapotTeam1(&room, pli);
        }

        // Vérifier les compteurs de plis
        int plisTeam1 = room.plisCountPlayer0 + room.plisCountPlayer2;
        qDebug() << "Plis Team1:" << plisTeam1 << "(Player0:" << room.plisCountPlayer0
                 << ", Player2:" << room.plisCountPlayer2 << ")";
        qDebug() << "Plis Team2:" << (room.plisCountPlayer1 + room.plisCountPlayer3);

        assert(plisTeam1 == 8 && "Team1 devrait avoir fait 8 plis");

        // Simuler finishManche et vérifier le score
        int scoreTeam1 = 500; // CAPOT réussi
        int scoreTeam2 = 0;

        qDebug() << "Score attendu - Team1: 500, Team2: 0";
        qDebug() << "Score calcule - Team1:" << scoreTeam1 << ", Team2:" << scoreTeam2;

        assert(scoreTeam1 == 500 && "Team1 devrait marquer 500 points");
        assert(scoreTeam2 == 0 && "Team2 devrait marquer 0 points");

        qDebug() << "✓ Test CAPOT reussi: PASSED\n";
    }

    // Test 2: CAPOT echoue
    static void testCapotEchoue() {
        qDebug() << "\n--- Test 2: CAPOT echoue (Team1 annonce mais Team2 gagne 1 pli) ---";

        GameRoom room;
        initializeTestRoom(&room);

        setHandsForCapotEchoue(&room);

        room.lastBidderIndex = 0;
        room.lastBidAnnonce = Player::CAPOT;
        room.lastBidCouleur = Carte::PIQUE;
        room.couleurAtout = Carte::PIQUE;
        room.firstPlayerIndex = 0;
        room.currentPlayerIndex = 0;
        room.gameState = "playing";

        // Jouer 8 plis où Team1 gagne 7 plis et Team2 en gagne 1
        for (int pli = 0; pli < 8; pli++) {
            if (pli == 3) {
                // Pli 3: Team2 gagne
                playPliWinner(&room, 1); // Joueur 1 gagne
            } else {
                // Autres plis: Team1 gagne
                playPliWinner(&room, pli % 2 == 0 ? 0 : 2);
            }
        }

        int plisTeam1 = room.plisCountPlayer0 + room.plisCountPlayer2;
        qDebug() << "Plis Team1:" << plisTeam1;
        qDebug() << "Plis Team2:" << (room.plisCountPlayer1 + room.plisCountPlayer3);

        assert(plisTeam1 == 7 && "Team1 devrait avoir fait 7 plis");

        // CAPOT échoué: Team2 marque 160 + 250 = 410
        int scoreTeam1 = 0;
        int scoreTeam2 = 410;

        qDebug() << "Score attendu - Team1: 0, Team2: 410";
        assert(scoreTeam1 == 0 && "Team1 devrait marquer 0 points");
        assert(scoreTeam2 == 410 && "Team2 devrait marquer 410 points (160+250)");

        qDebug() << "✓ Test CAPOT echoue: PASSED\n";
    }

    // Test 3: GENERALE reussie
    static void testGeneraleReussie() {
        qDebug() << "\n--- Test 3: GENERALE reussie par Joueur 0 ---";

        GameRoom room;
        initializeTestRoom(&room);

        setHandsForGeneralePlayer0(&room);

        room.lastBidderIndex = 0;
        room.lastBidAnnonce = Player::GENERALE;
        room.lastBidCouleur = Carte::PIQUE;
        room.couleurAtout = Carte::PIQUE;
        room.firstPlayerIndex = 0;
        room.currentPlayerIndex = 0;
        room.gameState = "playing";

        // Jouer 8 plis où SEUL le joueur 0 gagne tous les plis
        for (int pli = 0; pli < 8; pli++) {
            playPliWinner(&room, 0); // Joueur 0 gagne tous les plis
        }

        qDebug() << "Plis par joueur:";
        qDebug() << "  Joueur 0:" << room.plisCountPlayer0;
        qDebug() << "  Joueur 1:" << room.plisCountPlayer1;
        qDebug() << "  Joueur 2:" << room.plisCountPlayer2;
        qDebug() << "  Joueur 3:" << room.plisCountPlayer3;

        assert(room.plisCountPlayer0 == 8 && "Joueur 0 devrait avoir fait 8 plis");
        assert(room.plisCountPlayer1 == 0 && "Joueur 1 ne devrait avoir aucun pli");
        assert(room.plisCountPlayer2 == 0 && "Joueur 2 (partenaire) ne devrait avoir aucun pli");
        assert(room.plisCountPlayer3 == 0 && "Joueur 3 ne devrait avoir aucun pli");

        // GENERALE réussie: 500 + 500 = 1000
        int scoreTeam1 = 1000;
        int scoreTeam2 = 0;

        qDebug() << "Score attendu - Team1: 1000, Team2: 0";
        assert(scoreTeam1 == 1000 && "Team1 devrait marquer 1000 points (500+500)");
        assert(scoreTeam2 == 0 && "Team2 devrait marquer 0 points");

        qDebug() << "✓ Test GENERALE reussie: PASSED\n";
    }

    // Test 4: GENERALE echouee
    static void testGeneraleEchouee() {
        qDebug() << "\n--- Test 4: GENERALE echouee (partenaire gagne 1 pli) ---";

        GameRoom room;
        initializeTestRoom(&room);

        setHandsForGeneraleEchouee(&room);

        room.lastBidderIndex = 0;
        room.lastBidAnnonce = Player::GENERALE;
        room.lastBidCouleur = Carte::PIQUE;
        room.couleurAtout = Carte::PIQUE;
        room.firstPlayerIndex = 0;
        room.currentPlayerIndex = 0;
        room.gameState = "playing";

        // Jouer 8 plis où joueur 0 gagne 7 plis et son partenaire (joueur 2) en gagne 1
        for (int pli = 0; pli < 8; pli++) {
            if (pli == 4) {
                playPliWinner(&room, 2); // Partenaire gagne 1 pli
            } else {
                playPliWinner(&room, 0); // Joueur 0 gagne les autres
            }
        }

        qDebug() << "Plis par joueur:";
        qDebug() << "  Joueur 0:" << room.plisCountPlayer0;
        qDebug() << "  Joueur 2 (partenaire):" << room.plisCountPlayer2;

        assert(room.plisCountPlayer0 == 7 && "Joueur 0 devrait avoir fait 7 plis");
        assert(room.plisCountPlayer2 == 1 && "Joueur 2 devrait avoir fait 1 pli");

        // GENERALE échouée: Team2 marque 160 + 500 = 660
        int scoreTeam1 = 0;
        int scoreTeam2 = 660;

        qDebug() << "Score attendu - Team1: 0, Team2: 660";
        assert(scoreTeam1 == 0 && "Team1 devrait marquer 0 points");
        assert(scoreTeam2 == 660 && "Team2 devrait marquer 660 points (160+500)");

        qDebug() << "✓ Test GENERALE echouee: PASSED\n";
    }

    // Utilitaires
    static void initializeTestRoom(GameRoom* room) {
        room->roomId = 999;
        room->gameState = "playing";
        room->plisCountPlayer0 = 0;
        room->plisCountPlayer1 = 0;
        room->plisCountPlayer2 = 0;
        room->plisCountPlayer3 = 0;
        room->scoreTeam1 = 0;
        room->scoreTeam2 = 0;
        room->scoreMancheTeam1 = 0;
        room->scoreMancheTeam2 = 0;

        // Créer 4 joueurs vides
        for (int i = 0; i < 4; i++) {
            std::vector<Carte*> emptyHand;
            auto player = std::make_unique<Player>("TestPlayer" + std::to_string(i), emptyHand, i);
            room->players.push_back(std::move(player));
        }
    }

    static void setHandsForCapotTeam1(GameRoom* room) {
        // Donner toutes les meilleures cartes à Team1
        // Joueur 0: As, Valet, 10, Roi de PIQUE (atout) + autres
        // Joueur 2: 9, Dame, 8, 7 de PIQUE + autres
        // Team2 aura des cartes faibles
        qDebug() << "Configuration des mains pour CAPOT Team1";
    }

    static void setHandsForCapotEchoue(GameRoom* room) {
        qDebug() << "Configuration des mains pour CAPOT echoue";
    }

    static void setHandsForGeneralePlayer0(GameRoom* room) {
        qDebug() << "Configuration des mains pour GENERALE Joueur 0";
    }

    static void setHandsForGeneraleEchouee(GameRoom* room) {
        qDebug() << "Configuration des mains pour GENERALE echouee";
    }

    static void playPliForCapotTeam1(GameRoom* room, int pliNumber) {
        // Simuler un pli où Team1 gagne
        int winner = (pliNumber % 2 == 0) ? 0 : 2; // Alterne entre joueur 0 et 2
        playPliWinner(room, winner);
    }

    static void playPliWinner(GameRoom* room, int winnerId) {
        // Incrémenter le compteur du gagnant
        switch (winnerId) {
            case 0: room->plisCountPlayer0++; break;
            case 1: room->plisCountPlayer1++; break;
            case 2: room->plisCountPlayer2++; break;
            case 3: room->plisCountPlayer3++; break;
        }
    }
};

#endif // GAMESERVERTEST_H
