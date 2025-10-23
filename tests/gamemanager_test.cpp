#include <gtest/gtest.h>
#include "../GameManager.h"
#include "../Deck.h"
#include <set>
#include <sstream>
#include <iostream>

// Helper struct pour comparer les cartes
struct CarteCompare {
    bool operator()(const Carte* a, const Carte* b) const {
        if (a->getCouleur() != b->getCouleur())
            return static_cast<int>(a->getCouleur()) < static_cast<int>(b->getCouleur());
        return static_cast<int>(a->getChiffre()) < static_cast<int>(b->getChiffre());
    }
};

class GameManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Créer des joueurs pour le test
        Deck deck = Deck(/*Carte::CARREAU*/);
        deck.shuffleDeck();

        std::vector<Carte*> main1, main2, main3, main4;
        deck.distribute(main1, main2, main3, main4);
        
        // Créer les joueurs avec unique_ptr
        playersPtr.push_back(std::make_unique<Player>("Joueur1", main1, 0));
        playersPtr.push_back(std::make_unique<Player>("Joueur2", main2, 1));
        playersPtr.push_back(std::make_unique<Player>("Joueur3", main3, 2));
        playersPtr.push_back(std::make_unique<Player>("Joueur4", main4, 3));

        // Debug : afficher les mains
        // std::cout << "---- PRINT MAIN1 ----" << std::endl;
        // playersPtr[0]->printMain();
        // std::cout << "---- PRINT MAIN2 ----" << std::endl;
        // playersPtr[1]->printMain();
        // std::cout << "---- PRINT MAIN3 ----" << std::endl;
        // playersPtr[2]->printMain();
        // std::cout << "---- PRINT MAIN4 ----" << std::endl;
        // playersPtr[3]->printMain();

        deck.setAtout(Carte::COEUR);

        // Créer les références pour GameManager
        std::vector<std::reference_wrapper<std::unique_ptr<Player>>> playerRefs;
        for (auto& player : playersPtr) {
            playerRefs.push_back(std::ref(player));
        }
        
        gameManager = new GameManager(playerRefs, Carte::COEUR, 0);
    }

    void TearDown() override {
        delete gameManager;
        playersPtr.clear();  // Libère les joueurs
    }

    std::vector<std::unique_ptr<Player>> playersPtr;  // Maintenant membre de la classe
    GameManager* gameManager;
};

TEST_F(GameManagerTest, ConstructeurInitialiseCorrectement) {
    EXPECT_NE(gameManager, nullptr);
}

// TEST_F(GameManagerTest, RunTurnExecuteCorrectement) {
//     // Test que runTurn s'exécute sans erreur
//     EXPECT_NO_THROW(gameManager->runTurn());

//     // Vérifier que toutes les cartes sont uniques après le tour
//     std::set<const Carte*, CarteCompare> cartesJouees;
    
//     // Vérifier toutes les cartes dans les plis des joueurs
//     for (const auto& player : playersPtr) {
//         std::cout << "Vérification des cartes pour le joueur: " << player->getName() << std::endl;
//         // Afficher les cartes restantes dans la main du joueur
//         std::cout << "Cartes restantes dans la main:" << std::endl;
//         player->printMain();
        
//         // Vérifier que chaque carte dans les plis est unique
//         for (const auto& pli : player->getPlis()) {
//             for (const Carte* carte : pli) {
//                 if (carte != nullptr) {
//                     std::cout << "Carte trouvée dans le pli: ";
//                     carte->printCarte();
//                     auto result = cartesJouees.insert(carte);
//                     EXPECT_TRUE(result.second) << "Carte dupliquée détectée dans les plis";
//                 }
//             }
//         }
//     }
// }

// TEST_F(GameManagerTest, RunTurnJoueCarteAtoutDemandee) {
//     std::cerr << "\n=== Début du test RunTurnJoueCarteAtoutDemandee ===\n" << std::endl;
    
//     // Ici on construit des mains contrôlées pour tester le comportement
//     // Main du joueur 0: couleur demandée = COEUR, il a une carte COEUR et une carte PIQUE
//     std::vector<Carte*> m1 = {
//         new Carte(Carte::COEUR, Carte::AS),    // carte 0 : la carte qu'on veut jouer
//         new Carte(Carte::PIQUE, Carte::ROI)    // carte 1
//     };
//     // Joueur 2: n'a pas de COEUR, a un CARREAU et un TREFLE
//     std::vector<Carte*> m2 = {
//         new Carte(Carte::CARREAU, Carte::DAME), // carte 0
//         new Carte(Carte::TREFLE, Carte::VALET)  // carte 1
//     };
//     // Joueur 3: a COEUR et CARREAU - on testera le refus d'une mauvaise saisie
//     std::vector<Carte*> m3 = {
//         new Carte(Carte::COEUR, Carte::ROI),    // carte 0 : la bonne carte à jouer
//         new Carte(Carte::CARREAU, Carte::NEUF)  // carte 1 : celle qu'on va essayer de jouer (mauvais choix)
//     };
//     // Joueur 4: TREFLE et PIQUE
//     std::vector<Carte*> m4 = {
//         new Carte(Carte::TREFLE, Carte::AS),    // carte 0
//         new Carte(Carte::PIQUE, Carte::DIX)     // carte 1
//     };

//     std::cerr << "Cartes créées" << std::endl;

//     // Remplacer les mains des players existants
//     std::cerr << "Nombre de joueurs avant clear: " << playersPtr.size() << std::endl;
    
//     // On ne devrait pas appeler explicitement le destructeur, clear() s'en charge
//     playersPtr.clear();
    
//     std::cerr << "Nombre de joueurs après clear: " << playersPtr.size() << std::endl;
//     std::cerr << "Anciens joueurs nettoyés" << std::endl;

//     playersPtr.push_back(std::make_unique<Player>("J1", m1, 0));
//     playersPtr.push_back(std::make_unique<Player>("J2", m2, 1));
//     playersPtr.push_back(std::make_unique<Player>("J3", m3, 2));
//     playersPtr.push_back(std::make_unique<Player>("J4", m4, 3));

//     std::cerr << "Nouveaux joueurs créés" << std::endl;

//     // Re-créer GameManager avec ces joueurs
//     std::vector<std::reference_wrapper<std::unique_ptr<Player>>> playerRefs;
//     for (auto& p : playersPtr) playerRefs.push_back(std::ref(p));
//     delete gameManager;
//     gameManager = new GameManager(playerRefs, Carte::COEUR, 0);

//     std::cerr << "GameManager recréé" << std::endl;

//     // Simuler des saisies utilisateur pour les joueurs:
//     // - Premier joueur (index 0) va jouer carte 0 (COEUR) -> OK
//     // - Deuxième joueur (index 1) n'a pas de COEUR, il peut jouer n'importe quoi -> on choisit 0
//     // - Troisième joueur (index 2) a COEUR mais on simule d'abord une mauvaise saisie (choix 1 qui est NEUF carreau)
//     //   puis une saisie correcte (choix 0 qui est ROI de coeur)
//     // - Quatrième joueur (index 3) joue 0

//     std::istringstream inputStream("0\n0\n1\n0\n0\n");
//     std::streambuf* oldCin = std::cin.rdbuf(inputStream.rdbuf());

//     std::ostringstream outputCapture;
//     std::streambuf* oldCout = std::cout.rdbuf(outputCapture.rdbuf());

//     std::cerr << "Flux redirigés" << std::endl;

//     // Exécuter un seul tour (runTurn réalise 8 plis, ici on se contente de l'exécution complète)
//     try {
//         std::cerr << "Début de runTurn()" << std::endl;
//         gameManager->runTurn();
//         std::cerr << "Fin de runTurn()" << std::endl;
//     } catch (const std::exception& e) {
//         std::cerr << "Exception dans runTurn(): " << e.what() << std::endl;
//         throw;
//     } catch (...) {
//         std::cerr << "Exception inconnue dans runTurn()" << std::endl;
//         throw;
//     }

//     std::string out = outputCapture.str();

//     // Restaurer les flux avant d'afficher quoi que ce soit
//     std::cin.rdbuf(oldCin);
//     std::cout.rdbuf(oldCout);

//     // Maintenant on peut afficher la sortie capturée
//     std::cout << "\n=== Début de la sortie capturée (RunTurnJoueCarteAtoutDemandee) ===\n" << out << "\n=== Fin de la sortie capturée ===\n";
    
//     // Message exact qui devrait apparaître (provient de Player.cpp)
//     const std::string expectedMsg = "Vous avez la couleur demandee, veuillez selectionner une carte de cette couleur";
//     bool foundMsg = out.find(expectedMsg) != std::string::npos;
    
//     EXPECT_TRUE(foundMsg) << "Le message attendu '" << expectedMsg << "' n'a pas été trouvé dans la sortie:\n" << out;

//     // Cleanup: les cartes sont allouées et sont gérées par les Player (le destructor doit les delete)
//     // On supprime gameManager, playersPtr sera nettoyé par TearDown

// }

// TEST_F(GameManagerTest, RunTurnJoueCarteNonAtoutDemandee) {
//     std::cerr << "\n=== Debut du test RunTurnJoueCarteNonAtoutDemandee ===\n" << std::endl;
    
//     // Ici on construit des mains contrôlées pour tester le comportement
//     // Main du joueur 0: joue CARREAU pour commencer
//     std::vector<Carte*> m1 = {
//         new Carte(Carte::CARREAU, Carte::AS),    // carte 0 : la carte qu'on va jouer (CARREAU)
//         new Carte(Carte::TREFLE, Carte::ROI)     // carte 1
//     };
//     // Joueur 2: n'a pas de CARREAU, peut jouer n'importe quoi
//     std::vector<Carte*> m2 = {
//         new Carte(Carte::PIQUE, Carte::DAME),    // carte 0
//         new Carte(Carte::TREFLE, Carte::VALET)   // carte 1
//     };
//     // Joueur 3: a CARREAU mais essaie d'abord de jouer autre chose
//     std::vector<Carte*> m3 = {
//         new Carte(Carte::TREFLE, Carte::DAME),   // carte 0 : mauvais choix (sera refusé)
//         new Carte(Carte::CARREAU, Carte::NEUF)   // carte 1 : la bonne carte à jouer
//     };
//     // Joueur 4: n'a pas de CARREAU, peut jouer n'importe quoi
//     std::vector<Carte*> m4 = {
//         new Carte(Carte::TREFLE, Carte::AS),     // carte 0
//         new Carte(Carte::PIQUE, Carte::DIX)      // carte 1
//     };
    
//     // On ne devrait pas appeler explicitement le destructeur, clear() s'en charge
//     playersPtr.clear();

//     playersPtr.push_back(std::make_unique<Player>("J1", m1, 0));
//     playersPtr.push_back(std::make_unique<Player>("J2", m2, 1));
//     playersPtr.push_back(std::make_unique<Player>("J3", m3, 2));
//     playersPtr.push_back(std::make_unique<Player>("J4", m4, 3));

//     // Re-créer GameManager avec ces joueurs
//     std::vector<std::reference_wrapper<std::unique_ptr<Player>>> playerRefs;
//     for (auto& p : playersPtr) playerRefs.push_back(std::ref(p));
//     delete gameManager;
//     gameManager = new GameManager(playerRefs, Carte::COEUR, 0);

//     // Simuler des saisies utilisateur pour les joueurs:
//     // - Premier joueur (index 0) joue carte 0 (AS de CARREAU) pour établir la couleur demandée
//     // - Deuxième joueur (index 1) n'a pas de CARREAU, peut jouer n'importe quoi -> joue carte 0
//     // - Troisième joueur (index 2) a CARREAU (carte 1) mais essaie d'abord de jouer TREFLE (carte 0) -> refusé
//     //   puis joue correctement CARREAU (carte 1)
//     // - Quatrième joueur (index 3) n'a pas de CARREAU, joue carte 0

//     std::istringstream inputStream("0\n0\n0\n1\n0\n");
//     std::streambuf* oldCin = std::cin.rdbuf(inputStream.rdbuf());

//     std::ostringstream outputCapture;
//     std::streambuf* oldCout = std::cout.rdbuf(outputCapture.rdbuf());

//     // Exécuter un seul tour (runTurn réalise 8 plis, ici on se contente de l'exécution complète)
//     try {
//         std::cerr << "\nDébut de runTurn()" << std::endl;
//         gameManager->runTurn();
//         std::cerr << "Fin de runTurn()" << std::endl;
//     } catch (const std::exception& e) {
//         std::cerr << "Exception dans runTurn(): " << e.what() << std::endl;
//         std::cerr << "Stack trace ou contexte additionnel:" << std::endl;
//         std::cerr << "- Nombre de joueurs: " << playersPtr.size() << std::endl;
//         std::cerr << "- État des mains après exception:" << std::endl;
//         for (size_t i = 0; i < playersPtr.size(); ++i) {
//             std::cerr << "  Joueur " << i << ": " << playersPtr[i]->getMain().size() << " cartes" << std::endl;
//         }
//         throw;
//     } catch (...) {
//         std::cerr << "Exception inconnue dans runTurn()" << std::endl;
//         std::cerr << "Erreur fatale non identifiée" << std::endl;
//         throw;
//     }

//     std::string out = outputCapture.str();

//     // Restaurer les flux avant d'afficher quoi que ce soit
//     std::cin.rdbuf(oldCin);
//     std::cout.rdbuf(oldCout);

//     // Maintenant on peut afficher la sortie capturée
//     std::cout << "\n=== Debut de la sortie capturee (RunTurnJoueCarteAtoutDemandee) ===\n" << out << "\n=== Fin de la sortie capturee ===\n";
    
//     // Message exact qui devrait apparaître (provient de Player.cpp)
//     const std::string expectedMsg = "Vous avez la couleur demandee, veuillez selectionner une carte de cette couleur";
//     bool foundMsg = out.find(expectedMsg) != std::string::npos;
    
//     EXPECT_TRUE(foundMsg) << "Le message attendu '" << expectedMsg << "' n'a pas ete trouve dans la sortie:\n" << out;

//     // Cleanup: les cartes sont allouées et sont gérées par les Player (le destructor doit les delete)
//     // On supprime gameManager, playersPtr sera nettoyé par TearDown
// }

// TEST_F(GameManagerTest, RunTurnJoueAtoutSiPasCarteDemandee) {
//     std::cerr << "\n=== Début du test RunTurnJoueAtoutSiPasCarteDemandee ===\n" << std::endl;
    
//     // Ici on construit des mains contrôlées pour tester le comportement
//     // Main du joueur 0: commence par jouer PIQUE
//     std::vector<Carte*> m1 = {
//         new Carte(Carte::PIQUE, Carte::ROI),     // carte 0 : la carte qu'on va jouer pour commencer
//         new Carte(Carte::TREFLE, Carte::AS)      // carte 1
//     };
//     // Joueur 2: pas de PIQUE mais a de l'atout (COEUR)
//     std::vector<Carte*> m2 = {
//         new Carte(Carte::COEUR, Carte::DAME),    // carte 0 : doit jouer l'atout
//         new Carte(Carte::TREFLE, Carte::VALET)   // carte 1
//     };
//     // Joueur 3: a PIQUE et COEUR - devrait jouer PIQUE
//     std::vector<Carte*> m3 = {
//         new Carte(Carte::PIQUE, Carte::VALET),   // carte 0 : doit jouer cette carte (a la couleur demandée)
//         new Carte(Carte::COEUR, Carte::NEUF)     // carte 1
//     };
//     // Joueur 4: pas de PIQUE mais a de l'atout
//     std::vector<Carte*> m4 = {
//         new Carte(Carte::COEUR, Carte::DIX),     // carte 0 : doit jouer l'atout
//         new Carte(Carte::TREFLE, Carte::ROI)     // carte 1
//     };
    
//     // On ne devrait pas appeler explicitement le destructeur, clear() s'en charge
//     playersPtr.clear();

//     playersPtr.push_back(std::make_unique<Player>("J1", m1, 0));
//     playersPtr.push_back(std::make_unique<Player>("J2", m2, 1));
//     playersPtr.push_back(std::make_unique<Player>("J3", m3, 2));
//     playersPtr.push_back(std::make_unique<Player>("J4", m4, 3));

//     // Re-créer GameManager avec ces joueurs
//     std::vector<std::reference_wrapper<std::unique_ptr<Player>>> playerRefs;
//     for (auto& p : playersPtr) playerRefs.push_back(std::ref(p));
//     delete gameManager;
//     gameManager = new GameManager(playerRefs, Carte::COEUR, 0);

//     // Simuler des saisies utilisateur pour les joueurs:
//     // - Premier joueur (index 0) joue carte 0 (ROI de PIQUE) pour établir la couleur demandée
//     // - Deuxième joueur (index 1) n'a pas de PIQUE mais a de l'atout -> doit jouer DAME de COEUR (carte 0) mais joue d'abord carte 1 (mauvais choix) puis carte 0

//     std::istringstream inputStream("0\n1\n0\n");
//     std::streambuf* oldCin = std::cin.rdbuf(inputStream.rdbuf());

//     std::ostringstream outputCapture;
//     std::streambuf* oldCout = std::cout.rdbuf(outputCapture.rdbuf());

//     // Exécuter un seul tour (runTurn réalise 8 plis, ici on se contente de l'exécution complète)
//     try {
//         std::cerr << "\nDébut de runTurn()" << std::endl;
//         gameManager->runTurn();
//         std::cerr << "Fin de runTurn()" << std::endl;
//     } catch (const std::exception& e) {
//         std::cerr << "Exception dans runTurn(): " << e.what() << std::endl;
//         std::cerr << "Stack trace ou contexte additionnel:" << std::endl;
//         std::cerr << "- Nombre de joueurs: " << playersPtr.size() << std::endl;
//         std::cerr << "- État des mains après exception:" << std::endl;
//         for (size_t i = 0; i < playersPtr.size(); ++i) {
//             std::cerr << "  Joueur " << i << ": " << playersPtr[i]->getMain().size() << " cartes" << std::endl;
//         }
//         throw;
//     } catch (...) {
//         std::cerr << "Exception inconnue dans runTurn()" << std::endl;
//         std::cerr << "Erreur fatale non identifiée" << std::endl;
//         throw;
//     }

//     std::string out = outputCapture.str();

//     // Restaurer les flux avant d'afficher quoi que ce soit
//     std::cin.rdbuf(oldCin);
//     std::cout.rdbuf(oldCout);

//     // Maintenant on peut afficher la sortie capturée
//     std::cout << "\n=== Debut de la sortie capturee (RunTurnJoueAtoutSiPasCarteDemandee) ===\n" << out << "\n=== Fin de la sortie capturee ===\n";
    
//     // Message exact qui devrait apparaître (provient de Player.cpp)
//     const std::string expectedMsg = "Vous avez de l'atout, veuillez selectionner une carte de cette couleur...";
//     bool foundMsg = out.find(expectedMsg) != std::string::npos;
    
//     EXPECT_TRUE(foundMsg) << "Le message attendu '" << expectedMsg << "' n'a pas ete trouve dans la sortie:\n" << out;
//     // Cleanup: les cartes sont allouées et sont gérées par les Player (le destructor doit les delete)
//     // On supprime gameManager, playersPtr sera nettoyé par TearDown

// }


TEST_F(GameManagerTest, RunTurnJoueAtoutPlusFortSiPossible) {
    std::cerr << "\n=== Début du test RunTurnJoueAtoutPlusFortSiPossible ===\n" << std::endl;
    
    // Ici on construit des mains contrôlées pour tester le comportement
    // Main du joueur 0: commence par jouer PIQUE
    Carte *asCoeur = new Carte(Carte::COEUR, Carte::AS);
    asCoeur->setAtout(true); // Marquer comme atout
    Carte *asTrefle = new Carte(Carte::TREFLE, Carte::AS);
    std::vector<Carte*> m1 = {
        asCoeur,     // carte 0 : la carte qu'on va jouer pour commencer
        asTrefle         // carte 1
    };
    // Joueur 2: pas de PIQUE mais a de l'atout (COEUR)
    Carte *dameCoeur = new Carte(Carte::COEUR, Carte::DAME);
    dameCoeur->setAtout(true); // Marquer comme atout
    Carte *valetTrefle = new Carte(Carte::TREFLE, Carte::VALET);
    std::vector<Carte*> m2 = {
        dameCoeur,   // carte 0 : doit jouer l'atout
        valetTrefle   // carte 1
    };
    // Joueur 3: a 2 COEUR - devrait jouer la plus forte car la dame est jouée par le joueur 2
    Carte *roiCoeur = new Carte(Carte::COEUR, Carte::ROI);
    roiCoeur->setAtout(true); // Marquer comme atout
    Carte *neufCoeur = new Carte(Carte::COEUR, Carte::NEUF);
    neufCoeur->setAtout(true); // Marquer comme atout
    std::vector<Carte*> m3 = {
        roiCoeur,   // carte 0
        neufCoeur    // carte 1 doit jouer cette carte (atout plus fort)
    };
    // Joueur 4: pas de PIQUE mais a de l'atout
    Carte *dixCoeur = new Carte(Carte::COEUR, Carte::DIX);
    dixCoeur->setAtout(true); // Marquer comme atout
    Carte *valetCoeur = new Carte(Carte::COEUR, Carte::VALET);
    valetCoeur->setAtout(true); // Marquer comme atout
    std::vector<Carte*> m4 = {
        dixCoeur,    // carte 0
        valetCoeur   // carte 1 : doit jouer cette carte (atout plus fort)
    };
    
    // On ne devrait pas appeler explicitement le destructeur, clear() s'en charge
    playersPtr.clear();

    playersPtr.push_back(std::make_unique<Player>("J1", m1, 0));
    playersPtr.push_back(std::make_unique<Player>("J2", m2, 1));
    playersPtr.push_back(std::make_unique<Player>("J3", m3, 2));
    playersPtr.push_back(std::make_unique<Player>("J4", m4, 3));

    // Re-créer GameManager avec ces joueurs
    std::vector<std::reference_wrapper<std::unique_ptr<Player>>> playerRefs;
    for (auto& p : playersPtr) playerRefs.push_back(std::ref(p));
    delete gameManager;
    gameManager = new GameManager(playerRefs, Carte::COEUR, 0);

    // Simuler des saisies utilisateur pour les joueurs:
    // - Premier joueur (index 0) joue carte 0 (ROI de PIQUE) pour établir la couleur demandée
    // - Deuxième joueur (index 1) n'a pas de PIQUE mais a de l'atout -> doit jouer DAME de COEUR (carte 0) mais joue d'abord carte 1 (mauvais choix) puis carte 0

    std::istringstream inputStream("0\n0\n0\n1\n0\n1\n");
    //std::istringstream inputStream("0\n0\n0\n1\n1\n");
    std::streambuf* oldCin = std::cin.rdbuf(inputStream.rdbuf());

    std::ostringstream outputCapture;
    std::streambuf* oldCout = std::cout.rdbuf(outputCapture.rdbuf());

    // Exécuter un seul tour (runTurn réalise 8 plis, ici on se contente de l'exécution complète)
    try {
        std::cerr << "\nDébut de runTurn()" << std::endl;
        gameManager->runTurn();
        std::cerr << "Fin de runTurn()" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception dans runTurn(): " << e.what() << std::endl;
        std::cerr << "Stack trace ou contexte additionnel:" << std::endl;
        std::cerr << "- Nombre de joueurs: " << playersPtr.size() << std::endl;
        std::cerr << "- État des mains après exception:" << std::endl;
        for (size_t i = 0; i < playersPtr.size(); ++i) {
            std::cerr << "  Joueur " << i << ": " << playersPtr[i]->getMain().size() << " cartes" << std::endl;
        }
        throw;
    } catch (...) {
        std::cerr << "Exception inconnue dans runTurn()" << std::endl;
        std::cerr << "Erreur fatale non identifiée" << std::endl;
        throw;
    }

    std::string out = outputCapture.str();

    // Restaurer les flux avant d'afficher quoi que ce soit
    std::cin.rdbuf(oldCin);
    std::cout.rdbuf(oldCout);

    // Maintenant on peut afficher la sortie capturée
    std::cout << "\n=== Debut de la sortie capturee (RunTurnJoueAtoutPlusFortSiPossible) ===\n" << out << "\n=== Fin de la sortie capturee ===\n";
    
    // Message exact qui devrait apparaître (provient de Player.cpp)
    const std::string atoutFortMsg = "Vous avez un atout plus fort que l'atout joue precedemment,";    

    // Compter les occurrences du message d'atout fort
    size_t count = 0;
    size_t pos = 0;
    while ((pos = out.find(atoutFortMsg, pos)) != std::string::npos) {
        count++;
        pos += atoutFortMsg.length();
    }

    // Vérifier le message d'atout obligatoire
    bool foundMsg = out.find(atoutFortMsg) != std::string::npos;
    EXPECT_TRUE(foundMsg) << "Le message attendu '" << atoutFortMsg << "' n'a pas ete trouve dans la sortie:\n" << out;
    
    // Vérifier que le message d'atout fort apparaît 2 fois
    EXPECT_EQ(count, 2) << "Le message '" << atoutFortMsg << "' apparait " << count << " fois, n'etait pas prevu (expected 2). Sortie:\n" << out;
}

TEST_F(GameManagerTest, RunTurnDefausseSiPartenaireTiensLaMain) {
    std::cerr << "\n=== Début du test RunTurnDefausseSiPartenaireTiensLaMain ===\n" << std::endl;
    
    // Ici on construit des mains contrôlées pour tester le comportement
    // Main du joueur 0: commence par jouer PIQUE
    Carte *asCoeur = new Carte(Carte::COEUR, Carte::AS);
    asCoeur->setAtout(true); // Marquer comme atout
    Carte *asTrefle = new Carte(Carte::TREFLE, Carte::AS);
    std::vector<Carte*> m1 = {
        asCoeur,     // carte 0
        asTrefle         // carte 1 : la carte qu'on va jouer pour commencer
    };
    // Joueur 2: pas de PIQUE mais a de l'atout (COEUR)
    Carte *dameCoeur = new Carte(Carte::COEUR, Carte::DAME);
    dameCoeur->setAtout(true); // Marquer comme atout
    Carte *valetPique = new Carte(Carte::PIQUE, Carte::VALET);
    std::vector<Carte*> m2 = {
        dameCoeur,   // carte 0 : doit jouer l'atout
        valetPique   // carte 1
    };
    // Joueur 3: a un COEUR - doit la jouer
    Carte *huitCoeur = new Carte(Carte::COEUR, Carte::HUIT);
    huitCoeur->setAtout(true); // Marquer comme atout
    Carte *neufCarreau = new Carte(Carte::CARREAU, Carte::NEUF);
    std::vector<Carte*> m3 = {
        huitCoeur,   // carte 0, doit la jouer
        neufCarreau
    };
    // Joueur 4: pas de TREFLE, mais a de l'atout, mais peut se defausser car son partenaire tient la main
    Carte *dixCoeur = new Carte(Carte::COEUR, Carte::DIX);
    dixCoeur->setAtout(true); // Marquer comme atout
    Carte *valetCarreau = new Carte(Carte::CARREAU, Carte::VALET);
    std::vector<Carte*> m4 = {
        dixCoeur,    // carte 0: peut la jouer mais peut se defausser de l'autre carte
        valetCarreau   // carte 1 : peut se defausser de cette carte
    };
    
    // On ne devrait pas appeler explicitement le destructeur, clear() s'en charge
    playersPtr.clear();

    playersPtr.push_back(std::make_unique<Player>("J1", m1, 0));
    playersPtr.push_back(std::make_unique<Player>("J2", m2, 1));
    playersPtr.push_back(std::make_unique<Player>("J3", m3, 2));
    playersPtr.push_back(std::make_unique<Player>("J4", m4, 3));

    // Re-créer GameManager avec ces joueurs
    std::vector<std::reference_wrapper<std::unique_ptr<Player>>> playerRefs;
    for (auto& p : playersPtr) playerRefs.push_back(std::ref(p));
    delete gameManager;
    gameManager = new GameManager(playerRefs, Carte::COEUR, 0);

    // Simuler des saisies utilisateur pour les joueurs:
    // - Premier joueur (index 0) joue carte 1 (As de TREFLE) pour établir la couleur demandée
    // - Deuxième joueur (index 1) n'a pas de Trefle mais a de l'atout -> doit jouer DAME de COEUR (carte 0)
    // - Troisieme joueur (index 2) joue carte 0 (huit de COEUR) car n'a pas de TREFLE et que son partenaire ne tient pas la main
    // - Quatrieme joueur (index 3) n'a pas de Trefle mais a de l'atout -> peut se defausser car son partenaire (joueur 2) tient la main

    std::istringstream inputStream("0\n0\n0\n1\n");
    //std::istringstream inputStream("0\n0\n0\n1\n1\n");
    std::streambuf* oldCin = std::cin.rdbuf(inputStream.rdbuf());

    std::ostringstream outputCapture;
    std::streambuf* oldCout = std::cout.rdbuf(outputCapture.rdbuf());

    // Exécuter un seul tour (runTurn réalise 8 plis, ici on se contente de l'exécution complète)
    try {
        std::cerr << "\nDébut de runTurn()" << std::endl;
        gameManager->runTurn();
        std::cerr << "Fin de runTurn()" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception dans runTurn(): " << e.what() << std::endl;
        std::cerr << "Stack trace ou contexte additionnel:" << std::endl;
        std::cerr << "- Nombre de joueurs: " << playersPtr.size() << std::endl;
        std::cerr << "- État des mains après exception:" << std::endl;
        for (size_t i = 0; i < playersPtr.size(); ++i) {
            std::cerr << "  Joueur " << i << ": " << playersPtr[i]->getMain().size() << " cartes" << std::endl;
        }
        throw;
    } catch (...) {
        std::cerr << "Exception inconnue dans runTurn()" << std::endl;
        std::cerr << "Erreur fatale non identifiée" << std::endl;
        throw;
    }

    std::string out = outputCapture.str();

    // Restaurer les flux avant d'afficher quoi que ce soit
    std::cin.rdbuf(oldCin);
    std::cout.rdbuf(oldCout);

    // Maintenant on peut afficher la sortie capturée
    std::cout << "\n=== Debut de la sortie capturee (RunTurnDefausseSiPartenaireTiensLaMain) ===\n" << out << "\n=== Fin de la sortie capturee ===\n";
    
    // Message exact qui devrait ne pas apparaître (provient de Player.cpp)
    const std::string expectedMsg = "Vous avez de l'atout, veuillez selectionner une carte de cette couleur...";
    bool foundMsg = out.find(expectedMsg) != std::string::npos;

    EXPECT_FALSE(foundMsg) << "Le message attendu '" << expectedMsg << "a ete trouve dans la sortie:\n" << out;

}




// // Test de gestion des tours
// TEST_F(GameManagerTest, GestionTourJoueurs) {
//     // Vérifier que chaque joueur joue dans l'ordre
//     // Note: Ceci nécessitera peut-être d'ajouter des getters dans GameManager
//     gameManager->runTurn();
//     // EXPECT_EQ(gameManager->getCurrentPlayer(), 1);  // Le prochain joueur devrait être 1
// }

// // Test de la détermination du gagnant d'un pli
// TEST_F(GameManagerTest, DeterminationGagnantPli) {
//     // Simuler un pli et vérifier que le gagnant est correctement déterminé
//     gameManager->runTurn();
//     // EXPECT_TRUE(gameManager->determineWinner() >= 0 && gameManager->determineWinner() < 4);
// }

// // Test de la gestion de l'atout
// TEST_F(GameManagerTest, GestionAtout) {
//     // Vérifier que les règles de l'atout sont correctement appliquées
//     // EXPECT_EQ(gameManager->getCouleurAtout(), Carte::COEUR);
//}