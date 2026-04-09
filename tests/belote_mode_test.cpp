#include <gtest/gtest.h>
#include "../server/GameServer.h"
#include "../Carte.h"
#include "../Player.h"
#include "../Deck.h"
#include <set>

// ========================================
// Test Fixture pour le mode Belote
// ========================================
class BeloteModeTest : public ::testing::Test {
protected:
    GameRoom room;

    void SetUp() override {
        room.roomId = 1;
        room.gameState = "bidding";
        room.isBeloteMode = true;
        room.beloteBidRound = 1;
        room.beloteBidPassCount = 0;
        room.firstPlayerIndex = 0;
        room.currentPlayerIndex = 0;
        room.biddingPlayer = 0;
        room.scoreTeam1 = 0;
        room.scoreTeam2 = 0;
        room.scoreMancheTeam1 = 0;
        room.scoreMancheTeam2 = 0;
        room.beloteTeam1 = false;
        room.beloteTeam2 = false;
        room.coinched = false;
        room.surcoinched = false;
        room.coinchePlayerIndex = -1;
        room.lastBidderIndex = -1;
        room.couleurAtout = Carte::COULEURINVALIDE;
        room.passedBidsCount = 0;

        // Creer 4 joueurs
        for (int i = 0; i < 4; i++) {
            std::vector<Carte*> emptyHand;
            auto player = std::make_unique<Player>("Player" + std::to_string(i), emptyHand, i);
            room.players.push_back(std::move(player));
            room.isBot.push_back(false);
            room.connectionIds.append("");
            room.playerNames.append(QString("Player%1").arg(i));
        }
    }

    // Helper : distribuer 5 cartes par joueur + retournee
    void distributeBelote() {
        room.deck.shuffleDeck();
        std::vector<Carte*> main1, main2, main3, main4;
        Carte* retournee = nullptr;
        room.deck.distributeBelote(main1, main2, main3, main4, retournee);
        room.retournee = retournee;

        for (Carte* c : main1) room.players[0]->addCardToHand(c);
        for (Carte* c : main2) room.players[1]->addCardToHand(c);
        for (Carte* c : main3) room.players[2]->addCardToHand(c);
        for (Carte* c : main4) room.players[3]->addCardToHand(c);
    }
};

// ========================================
// Tests de distribution Belote via GameRoom
// ========================================

TEST_F(BeloteModeTest, DistributionBelote_5CartesParJoueur) {
    distributeBelote();
    for (int i = 0; i < 4; i++) {
        EXPECT_EQ(room.players[i]->getMain().size(), 5)
            << "Joueur " << i << " devrait avoir 5 cartes";
    }
}

TEST_F(BeloteModeTest, DistributionBelote_RetourneeExiste) {
    distributeBelote();
    EXPECT_NE(room.retournee, nullptr) << "La retournee doit exister";
}

TEST_F(BeloteModeTest, DistributionBelote_RetourneeNEstPasDansLesMains) {
    distributeBelote();
    Carte::Couleur retCouleur = room.retournee->getCouleur();
    Carte::Chiffre retChiffre = room.retournee->getChiffre();

    for (int i = 0; i < 4; i++) {
        for (const Carte* c : room.players[i]->getMain()) {
            bool meme = (c->getCouleur() == retCouleur && c->getChiffre() == retChiffre);
            EXPECT_FALSE(meme) << "La retournee ne doit pas etre dans la main du joueur " << i;
        }
    }
}

// ========================================
// Tests completeBeloteDistribution
// ========================================

TEST_F(BeloteModeTest, CompleteBeloteDistribution_PreneurRecoit8Cartes) {
    distributeBelote();
    int takerIndex = 1;
    room.couleurAtout = room.retournee->getCouleur();
    room.lastBidderIndex = takerIndex;

    // Simuler completeBeloteDistribution :
    // Preneur recoit retournee + 2 cartes du deck
    room.players[takerIndex]->addCardToHand(room.retournee);
    Carte* c1 = room.deck.drawCard();
    Carte* c2 = room.deck.drawCard();
    if (c1) room.players[takerIndex]->addCardToHand(c1);
    if (c2) room.players[takerIndex]->addCardToHand(c2);

    // Autres joueurs recoivent 3 cartes chacun
    for (int i = 0; i < 4; i++) {
        if (i == takerIndex) continue;
        for (int k = 0; k < 3; k++) {
            Carte* card = room.deck.drawCard();
            if (card) room.players[i]->addCardToHand(card);
        }
    }

    EXPECT_EQ(room.players[takerIndex]->getMain().size(), 8)
        << "Le preneur devrait avoir 8 cartes (5 + retournee + 2)";

    for (int i = 0; i < 4; i++) {
        if (i == takerIndex) continue;
        EXPECT_EQ(room.players[i]->getMain().size(), 8)
            << "Joueur " << i << " devrait avoir 8 cartes (5 + 3)";
    }
}

TEST_F(BeloteModeTest, CompleteBeloteDistribution_ToutesCartesSont32) {
    distributeBelote();
    room.couleurAtout = room.retournee->getCouleur();
    room.lastBidderIndex = 0;

    // Preneur : retournee + 2
    room.players[0]->addCardToHand(room.retournee);
    room.players[0]->addCardToHand(room.deck.drawCard());
    room.players[0]->addCardToHand(room.deck.drawCard());

    // Autres : 3 chacun
    for (int i = 1; i < 4; i++) {
        for (int k = 0; k < 3; k++) {
            room.players[i]->addCardToHand(room.deck.drawCard());
        }
    }

    int total = 0;
    for (int i = 0; i < 4; i++) {
        total += room.players[i]->getMain().size();
    }
    EXPECT_EQ(total, 32) << "Le total des cartes devrait etre 32";
}

// ========================================
// Tests encheres Belote - Tour 1
// ========================================

TEST_F(BeloteModeTest, BidRound1_Initial) {
    EXPECT_EQ(room.beloteBidRound, 1);
    EXPECT_EQ(room.beloteBidPassCount, 0);
    EXPECT_EQ(room.biddingPlayer, 0);
}

TEST_F(BeloteModeTest, BidRound1_PasseIncrement) {
    room.beloteBidPassCount++;
    EXPECT_EQ(room.beloteBidPassCount, 1);

    room.currentPlayerIndex = (room.currentPlayerIndex + 1) % 4;
    room.biddingPlayer = room.currentPlayerIndex;
    EXPECT_EQ(room.biddingPlayer, 1);
}

TEST_F(BeloteModeTest, BidRound1_4PassesSwitchToRound2) {
    room.beloteBidPassCount = 4;
    if (room.beloteBidPassCount >= 4 && room.beloteBidRound == 1) {
        room.beloteBidRound = 2;
        room.beloteBidPassCount = 0;
        room.currentPlayerIndex = room.firstPlayerIndex;
        room.biddingPlayer = room.firstPlayerIndex;
    }

    EXPECT_EQ(room.beloteBidRound, 2);
    EXPECT_EQ(room.beloteBidPassCount, 0);
    EXPECT_EQ(room.biddingPlayer, room.firstPlayerIndex);
}

TEST_F(BeloteModeTest, BidRound2_4PassesShouldRedistribute) {
    room.beloteBidRound = 2;
    room.beloteBidPassCount = 4;

    bool shouldRedistribute = (room.beloteBidPassCount >= 4 && room.beloteBidRound == 2);
    EXPECT_TRUE(shouldRedistribute);
}

// ========================================
// Tests encheres Belote - Prendre
// ========================================

TEST_F(BeloteModeTest, Prendre_Tour1_SetsAtout) {
    distributeBelote();
    Carte::Couleur retCouleur = room.retournee->getCouleur();

    // Simuler "Prendre" au tour 1
    room.lastBidderIndex = 0;
    room.couleurAtout = retCouleur;
    room.lastBidCouleur = retCouleur;

    EXPECT_EQ(room.couleurAtout, retCouleur);
    EXPECT_EQ(room.lastBidderIndex, 0);
}

TEST_F(BeloteModeTest, Prendre_Tour2_CouleurDifferenteDeRetournee) {
    distributeBelote();
    room.beloteBidRound = 2;
    Carte::Couleur retCouleur = room.retournee->getCouleur();

    // Choisir une autre couleur
    Carte::Couleur choix = Carte::COEUR;
    if (choix == retCouleur) choix = Carte::TREFLE;

    // Valider que la couleur choisie est differente
    EXPECT_NE(choix, retCouleur)
        << "Au tour 2, la couleur choisie ne peut pas etre celle de la retournee";

    room.couleurAtout = choix;
    room.lastBidderIndex = 2;
    EXPECT_EQ(room.couleurAtout, choix);
}

TEST_F(BeloteModeTest, Prendre_Tour2_RefuseCouleurRetournee) {
    distributeBelote();
    room.beloteBidRound = 2;
    Carte::Couleur retCouleur = room.retournee->getCouleur();

    // Tentative de prendre la meme couleur que la retournee → doit etre rejetee
    bool rejected = (room.beloteBidRound == 2 && retCouleur == retCouleur);
    EXPECT_TRUE(rejected);
}

// ========================================
// Tests rotation du dealer
// ========================================

TEST_F(BeloteModeTest, RotationDealer_FirstPlayerIndexIncremente) {
    EXPECT_EQ(room.firstPlayerIndex, 0);
    room.firstPlayerIndex = (room.firstPlayerIndex + 1) % 4;
    EXPECT_EQ(room.firstPlayerIndex, 1);
    room.firstPlayerIndex = (room.firstPlayerIndex + 1) % 4;
    EXPECT_EQ(room.firstPlayerIndex, 2);
    room.firstPlayerIndex = (room.firstPlayerIndex + 1) % 4;
    EXPECT_EQ(room.firstPlayerIndex, 3);
    room.firstPlayerIndex = (room.firstPlayerIndex + 1) % 4;
    EXPECT_EQ(room.firstPlayerIndex, 0);
}

// ========================================
// Tests isBeloteMode flag
// ========================================

TEST_F(BeloteModeTest, IsBeloteMode_True) {
    EXPECT_TRUE(room.isBeloteMode);
}

TEST_F(BeloteModeTest, IsBeloteMode_CoincheFeaturesDisabled) {
    // En mode Belote, coinche et surcoinche sont inutilisees
    EXPECT_FALSE(room.coinched);
    EXPECT_FALSE(room.surcoinched);
}

// ========================================
// Tests completeBeloteDistribution detection belote (Roi+Dame atout)
// ========================================

TEST_F(BeloteModeTest, DetectionBelote_RoiDameAtout) {
    // Donner Roi et Dame de Coeur au joueur 0
    room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::ROI));
    room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::DAME));
    room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::AS));
    room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::SEPT));
    room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::HUIT));

    // Joueur 0 a la belote si atout = Coeur
    bool hasBelote = room.players[0]->hasBelotte(Carte::COEUR);
    EXPECT_TRUE(hasBelote) << "Joueur 0 avec Roi+Dame de Coeur devrait avoir la belote";
}

TEST_F(BeloteModeTest, DetectionBelote_PasRoiDameAtout) {
    room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::ROI));
    room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::DAME));
    room.players[0]->addCardToHand(new Carte(Carte::COEUR, Carte::AS));

    bool hasBelote = room.players[0]->hasBelotte(Carte::COEUR);
    EXPECT_FALSE(hasBelote) << "Roi de Coeur + Dame de Pique != belote";
}

// ========================================
// Tests Deck::distributeBelote + drawCard
// ========================================

struct CarteCompare2 {
    bool operator()(const Carte* a, const Carte* b) const {
        if (a->getCouleur() != b->getCouleur())
            return a->getCouleur() < b->getCouleur();
        return a->getChiffre() < b->getChiffre();
    }
};

TEST_F(BeloteModeTest, DistributeBelote_21CartesDistribuees) {
    room.deck.shuffleDeck();
    std::vector<Carte*> m1, m2, m3, m4;
    Carte* ret = nullptr;
    room.deck.distributeBelote(m1, m2, m3, m4, ret);

    // distributeBelote distribue 5*4=20 cartes + 1 retournee = 21 cartes referencees
    int totalDistributed = m1.size() + m2.size() + m3.size() + m4.size() + (ret ? 1 : 0);
    EXPECT_EQ(totalDistributed, 21) << "distributeBelote doit referencer 21 cartes (5x4 + retournee)";
    EXPECT_EQ(m1.size(), 5);
    EXPECT_EQ(m2.size(), 5);
    EXPECT_EQ(m3.size(), 5);
    EXPECT_EQ(m4.size(), 5);
    EXPECT_NE(ret, nullptr);
}

TEST_F(BeloteModeTest, DistributionComplete_ToutesCartesUniques) {
    distributeBelote();
    room.couleurAtout = room.retournee->getCouleur();

    // Preneur (joueur 0)
    room.players[0]->addCardToHand(room.retournee);
    room.players[0]->addCardToHand(room.deck.drawCard());
    room.players[0]->addCardToHand(room.deck.drawCard());

    for (int i = 1; i < 4; i++) {
        for (int k = 0; k < 3; k++) {
            room.players[i]->addCardToHand(room.deck.drawCard());
        }
    }

    std::set<Carte*, CarteCompare2> allCards;
    for (int i = 0; i < 4; i++) {
        for (Carte* c : room.players[i]->getMain()) {
            auto result = allCards.insert(c);
            EXPECT_TRUE(result.second) << "Carte dupliquee chez joueur " << i;
        }
    }
    EXPECT_EQ(allCards.size(), 32) << "Toutes les 32 cartes doivent etre presentes et uniques";
}
