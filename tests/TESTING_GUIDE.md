# Guide de test pour GameServer

Ce guide explique comment écrire et exécuter des tests pour le serveur de jeu Coinche.

## Vue d'ensemble

Le projet utilise **Google Test** (gtest) comme framework de test. Les tests sont organisés en plusieurs suites :

1. **test_gameserver** - Tests unitaires pour GameServer (logique de scoring, règles du jeu)
2. **test_capot_generale** - Tests fonctionnels pour CAPOT et GENERALE
3. **test_coinche** - Tests fonctionnels pour COINCHE et SURCOINCHE

## Structure d'un test

### Exemple de test simple

```cpp
TEST_F(GameServerTest, CapotNonAnnonce_Team1Reussi) {
    // Arrange - Préparer le contexte
    room.lastBidAnnonce = Player::QUATRE_VINGTS;
    room.lastBidCouleur = Carte::PIQUE;
    room.plisCountPlayer0 = 5;
    room.plisCountPlayer2 = 3;

    // Act - (Pas nécessaire ici, on teste la logique de calcul)
    int totalPlis = room.plisCountPlayer0 + room.plisCountPlayer2;

    // Assert - Vérifier les résultats
    EXPECT_EQ(totalPlis, 8);
}
```

### Fixture de test

La classe `GameServerTest` est une fixture qui initialise un `GameRoom` avant chaque test :

```cpp
class GameServerTest : public ::testing::Test {
protected:
    GameRoom room;

    void SetUp() override {
        // Initialisation avant chaque test
        room.roomId = 1;
        room.gameState = "playing";
        // ... autres initialisations
    }
};
```

## Exécution des tests

### Méthode 1 : Scripts automatiques

**Windows:**
```bash
cd tests
run_tests.bat              # Tous les tests
run_tests.bat gameserver   # Tests GameServer uniquement
run_tests.bat capot        # Tests CAPOT/GENERALE
run_tests.bat coinche      # Tests COINCHE/SURCOINCHE
```

**Linux/Mac:**
```bash
cd tests
chmod +x run_tests.sh
./run_tests.sh              # Tous les tests
./run_tests.sh gameserver   # Tests GameServer uniquement
./run_tests.sh capot        # Tests CAPOT/GENERALE
./run_tests.sh coinche      # Tests COINCHE/SURCOINCHE
```

### Méthode 2 : CMake/CTest

```bash
mkdir build && cd build
cmake ..
cmake --build .

# Exécuter tous les tests
ctest

# Exécuter un test spécifique
ctest -R test_gameserver --verbose

# Exécuter avec plus de détails
ctest --output-on-failure
```

### Méthode 3 : Exécution manuelle

```bash
cd build/tests

# Windows
test_gameserver.exe
test_capot_generale.exe
test_coinche.exe

# Linux/Mac
./test_gameserver
./test_capot_generale
./test_coinche
```

### Méthode 4 : Filtres Google Test

```bash
# Exécuter un test spécifique
./test_gameserver --gtest_filter=GameServerTest.CapotNonAnnonce_Team1ReussiAvecBelote

# Exécuter tous les tests de CAPOT non annoncé
./test_gameserver --gtest_filter=*CapotNonAnnonce*

# Exécuter tous les tests SURCOINCHE
./test_gameserver --gtest_filter=*Surcoinche*

# Lister tous les tests disponibles
./test_gameserver --gtest_list_tests
```

## Écrire de nouveaux tests

### Étape 1 : Identifier ce qu'on veut tester

Exemples :
- Nouvelle règle de scoring
- Cas limite (ex: belote + capot coinché)
- Détection d'erreur (ex: joueur déconnecté)

### Étape 2 : Créer le test

```cpp
TEST_F(GameServerTest, MonNouveauTest) {
    // 1. Arrange - Configurer le contexte
    room.lastBidAnnonce = Player::CENT;
    room.coinched = true;

    // 2. Act - Exécuter l'action à tester
    // (Peut être un appel de fonction, ou juste une configuration)

    // 3. Assert - Vérifier les résultats attendus
    EXPECT_TRUE(room.coinched);
    EXPECT_EQ(room.lastBidAnnonce, Player::CENT);
}
```

### Étape 3 : Utiliser les assertions appropriées

```cpp
// Égalité
EXPECT_EQ(actual, expected);
EXPECT_NE(actual, expected);

// Comparaisons
EXPECT_LT(val1, val2);  // Less Than
EXPECT_LE(val1, val2);  // Less or Equal
EXPECT_GT(val1, val2);  // Greater Than
EXPECT_GE(val1, val2);  // Greater or Equal

// Booléens
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

// Assertions qui arrêtent le test en cas d'échec
ASSERT_EQ(actual, expected);
ASSERT_TRUE(condition);
```

### Étape 4 : Ajouter des helpers si nécessaire

```cpp
class GameServerTest : public ::testing::Test {
protected:
    // ... SetUp() ...

    // Helper pour distribuer des cartes
    void distributeCardsForCapot() {
        room.players[0]->addCardToHand(new Carte(Carte::PIQUE, Carte::VALET));
        // ... autres cartes
    }

    // Helper pour calculer les points
    int calculateTeamPoints(int team) {
        if (team == 1) {
            return room.plisCountPlayer0 * 20 + room.plisCountPlayer2 * 20;
        }
        return room.plisCountPlayer1 * 20 + room.plisCountPlayer3 * 20;
    }
};
```

## Tests de régression

Lorsqu'un bug est corrigé, créez un test pour s'assurer qu'il ne revienne pas :

```cpp
// Exemple : Bug #42 - Capot non annoncé ne donnait pas le bon score
TEST_F(GameServerTest, Regression_Bug42_CapotNonAnnonceScore) {
    // Configuration qui reproduisait le bug
    room.lastBidAnnonce = Player::QUATRE_VINGTS;
    room.plisCountPlayer0 = 8;

    int pointsTeam1 = 162;

    // Le bug donnait: 250 + 80 = 330
    // Le fix correct: 250 + 162 = 412
    int expectedScore = 250 + pointsTeam1;

    EXPECT_EQ(expectedScore, 412);
    EXPECT_NE(expectedScore, 330);  // S'assurer que l'ancien bug ne revient pas
}
```

## Bonnes pratiques

### 1. Noms de tests descriptifs

✅ **BON:**
```cpp
TEST_F(GameServerTest, Coinche_Team1EchoueContrat_Team2MarqueDoubleScore)
```

❌ **MAUVAIS:**
```cpp
TEST_F(GameServerTest, Test1)
TEST_F(GameServerTest, CoincheTest)
```

### 2. Tests indépendants

Chaque test doit pouvoir s'exécuter seul, dans n'importe quel ordre.

✅ **BON:**
```cpp
TEST_F(GameServerTest, Test1) {
    room.scoreTeam1 = 0;  // Réinitialiser
    room.scoreTeam1 += 100;
    EXPECT_EQ(room.scoreTeam1, 100);
}

TEST_F(GameServerTest, Test2) {
    room.scoreTeam1 = 0;  // Réinitialiser
    room.scoreTeam1 += 200;
    EXPECT_EQ(room.scoreTeam1, 200);
}
```

❌ **MAUVAIS:**
```cpp
TEST_F(GameServerTest, Test1) {
    room.scoreTeam1 = 100;  // Test2 dépend de cette valeur
}

TEST_F(GameServerTest, Test2) {
    room.scoreTeam1 += 200;  // Dépend de Test1 !
    EXPECT_EQ(room.scoreTeam1, 300);
}
```

### 3. Un test, une chose

Chaque test doit vérifier UNE seule fonctionnalité.

✅ **BON:**
```cpp
TEST_F(GameServerTest, CapotNonAnnonce_ScoreCalculation) {
    // Teste seulement le calcul du score
    EXPECT_EQ(score, 412);
}

TEST_F(GameServerTest, CapotNonAnnonce_PlisCount) {
    // Teste seulement le compte des plis
    EXPECT_EQ(totalPlis, 8);
}
```

❌ **MAUVAIS:**
```cpp
TEST_F(GameServerTest, CapotTests) {
    // Teste trop de choses à la fois
    EXPECT_EQ(score, 412);
    EXPECT_EQ(totalPlis, 8);
    EXPECT_TRUE(capotDetected);
    EXPECT_FALSE(generaleDetected);
    // ...
}
```

### 4. Messages d'assertion explicites

```cpp
// Ajouter un message personnalisé
EXPECT_EQ(score, 412) << "Score capot non annoncé incorrect pour Team1";
EXPECT_TRUE(room.coinched) << "La partie devrait être coinchée";
```

## Debugging des tests

### Afficher des valeurs pendant le test

```cpp
TEST_F(GameServerTest, Debug) {
    std::cout << "Score Team1: " << room.scoreTeam1 << std::endl;
    std::cout << "Plis Player0: " << room.plisCountPlayer0 << std::endl;

    EXPECT_EQ(room.scoreTeam1, 412);
}
```

### Exécuter un seul test en mode verbose

```bash
./test_gameserver --gtest_filter=GameServerTest.MonTest --gtest_print_time=1
```

### Répéter un test qui échoue parfois

```bash
# Répéter 100 fois pour détecter un bug intermittent
./test_gameserver --gtest_filter=MonTest --gtest_repeat=100
```

## Couverture de code

Pour vérifier quelles parties du code sont testées :

```bash
# Avec gcov/lcov (Linux)
cmake -DCMAKE_BUILD_TYPE=Coverage ..
make
make test
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

## Ressources

- [Documentation Google Test](https://google.github.io/googletest/)
- [Assertions Google Test](https://google.github.io/googletest/reference/assertions.html)
- [FAQ Google Test](https://google.github.io/googletest/faq.html)

## Conclusion

Les tests sont essentiels pour garantir la qualité du code. Prenez l'habitude de :

1. ✅ Écrire un test avant de corriger un bug
2. ✅ Écrire des tests pour chaque nouvelle fonctionnalité
3. ✅ Exécuter tous les tests avant chaque commit
4. ✅ Maintenir les tests à jour avec le code

Bon testing ! 🎯
