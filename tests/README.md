# Tests CAPOT, GENERALE et COINCHE

Ce dossier contient les tests fonctionnels pour vérifier le bon fonctionnement des règles spéciales CAPOT, GENERALE, COINCHE et SURCOINCHE.

Ces tests utilisent **Google Test**, le framework de test standard en C++.

## Structure

- `capot_generale_test.cpp` - Tests Google Test pour CAPOT et GENERALE
- `coinche_test.cpp` - Tests Google Test pour COINCHE et SURCOINCHE
- `carte_test.cpp`, `deck_test.cpp`, `player_test.cpp` - Tests unitaires existants

## Tests implémentés

### Test 1: CAPOT réussi
- **Scénario**: Team1 annonce CAPOT et fait tous les 8 plis
- **Mains**: Team1 reçoit tous les atouts forts (Valet, 9, As, 10, etc.) + Belote (Roi et Dame)
- **Résultat attendu**: Team1 marque 520 points (250+250+20 belote), Team2 marque 0

### Test 2: CAPOT échoué
- **Scénario**: Team1 annonce CAPOT mais Team2 gagne 1 pli
- **Mains**: Team2 a le 9 d'atout pour remporter 1 pli
- **Résultat attendu**: Team1 marque 0, Team2 marque 410 points (160+250)

### Test 3: GENERALE réussie
- **Scénario**: Joueur 0 annonce GENERALE et fait tous les 8 plis seul
- **Mains**: Joueur 0 a toutes les meilleures cartes, son partenaire (Joueur 2) a des cartes moyennes
- **Résultat attendu**: Team1 marque 1000 points (500+500), Team2 marque 0

### Test 4: GENERALE échouée
- **Scénario**: Joueur 0 annonce GENERALE mais son partenaire (Joueur 2) gagne 1 pli
- **Mains**: Joueur 2 a une carte forte qui lui permet de gagner 1 pli + Belote (Roi et Dame)
- **Résultat attendu**: Team1 marque 20 points (belote uniquement), Team2 marque 660 points (160+500)

### Test 5: CAPOT non annoncé réussi
- **Scénario**: Team1 annonce 80 mais fait tous les 8 plis (CAPOT non annoncé)
- **Mains**: Team1 reçoit tous les atouts forts + Belote (Roi et Dame)
- **Résultat attendu**: Team1 marque 350 points (250 CAPOT + 80 contrat + 20 belote), Team2 marque 0

### Test 6: COINCHE réussi (Team1 annonce 80)
- **Scénario**: Team1 annonce 80, Team2 coinche, Team1 réussit son contrat
- **Mains**: Team1 a de bonnes cartes d'atout et gagne 6 plis (environ 110 points réalisés)
- **Résultat attendu**: Team1 marque environ 380 points ((80 + 110) × 2), Team2 marque 0

### Test 7: COINCHE échoué (Team1 annonce 80)
- **Scénario**: Team1 annonce 80, Team2 coinche, Team1 échoue son contrat
- **Mains**: Team2 a les meilleures cartes et Team1 ne peut atteindre 80 points
- **Résultat attendu**: Team1 marque 0, Team2 marque 480 points ((80 + 160) × 2)

### Test 8: SURCOINCHE réussi (Team2 annonce 120)
- **Scénario**: Team2 annonce 120, Team1 coinche, Team2 surcoinche, Team2 réussit
- **Mains**: Team2 a les meilleures cartes et gagne 7 plis (environ 130 points réalisés)
- **Résultat attendu**: Team1 marque 0, Team2 marque environ 1000 points ((120 + 130) × 4)

### Test 9: SURCOINCHE échoué (Team2 annonce 120)
- **Scénario**: Team2 annonce 120, Team1 coinche, Team2 surcoinche, Team2 échoue
- **Mains**: Team1 a les meilleures cartes et Team2 ne peut atteindre 120 points
- **Résultat attendu**: Team1 marque 1120 points ((120 + 160) × 4), Team2 marque 0

### Test 10: COINCHE avec Belote (Team1 annonce 100)
- **Scénario**: Team1 annonce 100, Team2 coinche, Team1 réussit avec belote
- **Mains**: Team1 a de bonnes cartes + Belote (Roi et Dame de l'atout), environ 110 points réalisés
- **Résultat attendu**: Team1 marque environ 440 points ((100 + 110) × 2 + 20 belote), Team2 marque 0

## Compilation et exécution

### Compilation

```bash
# Depuis le répertoire racine du projet
mkdir build
cd build
cmake ..
cmake --build . --target test_capot_generale
cmake --build . --target test_coinche

# Ou compiler tous les tests
cmake --build . --target coinche_tests test_capot_generale test_coinche
```

### Exécution des tests

```bash
# Exécuter tous les tests CAPOT/GENERALE
cd build/tests
./test_capot_generale          # Linux/Mac
.\test_capot_generale.exe      # Windows

# Exécuter tous les tests COINCHE/SURCOINCHE
./test_coinche                 # Linux/Mac
.\test_coinche.exe             # Windows

# Exécuter un test spécifique
./test_capot_generale --gtest_filter=CapotGeneraleTest.CapotReussiAvecBelote
./test_coinche --gtest_filter=CoincheTest.CoincheReussiTeam1Annonce80

# Exécuter tous les tests CAPOT uniquement
./test_capot_generale --gtest_filter=*Capot*

# Exécuter tous les tests GENERALE uniquement
./test_capot_generale --gtest_filter=*Generale*

# Exécuter tous les tests COINCHE uniquement (sans SURCOINCHE)
./test_coinche --gtest_filter=*Coinche*

# Exécuter tous les tests SURCOINCHE uniquement
./test_coinche --gtest_filter=*Surcoinche*

# Afficher la liste des tests
./test_capot_generale --gtest_list_tests
./test_coinche --gtest_list_tests

# Mode verbose
./test_capot_generale --gtest_print_time=1
./test_coinche --gtest_print_time=1
```

### Intégration avec CTest

Les tests sont automatiquement découverts par CTest :

```bash
cd build
ctest -R test_capot_generale    # Exécuter tests CAPOT/GENERALE via CTest
ctest -R test_coinche           # Exécuter tests COINCHE/SURCOINCHE via CTest
ctest                           # Exécuter tous les tests
ctest --verbose                 # Mode verbose
```

## Exemples de sortie

### Tests CAPOT/GENERALE

```
Running main() from gtest_main.cc
[==========] Running 5 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 5 tests from CapotGeneraleTest
[ RUN      ] CapotGeneraleTest.CapotReussiAvecBelote
Le joueur TestPlayer2 a la belote!
[       OK ] CapotGeneraleTest.CapotReussiAvecBelote (0 ms)
[ RUN      ] CapotGeneraleTest.CapotEchoue
[       OK ] CapotGeneraleTest.CapotEchoue (0 ms)
[ RUN      ] CapotGeneraleTest.GeneraleReussie
[       OK ] CapotGeneraleTest.GeneraleReussie (0 ms)
[ RUN      ] CapotGeneraleTest.GeneraleEchoueeAvecBelote
Le joueur TestPlayer2 a la belote!
[       OK ] CapotGeneraleTest.GeneraleEchoueeAvecBelote (0 ms)
[ RUN      ] CapotGeneraleTest.CapotNonAnnonceReussi
Le joueur TestPlayer2 a la belote!
[       OK ] CapotGeneraleTest.CapotNonAnnonceReussi (0 ms)
[----------] 5 tests from CapotGeneraleTest (0 ms total)

[----------] Global test environment tear-down
[==========] 5 tests from 1 test suite ran. (0 ms total)
[  PASSED  ] 5 tests.
```

### Tests COINCHE/SURCOINCHE

```
Running main() from gtest_main.cc
[==========] Running 5 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 5 tests from CoincheTest
[ RUN      ] CoincheTest.CoincheReussiTeam1Annonce80
Le joueur TestPlayer2 a la belote!
[       OK ] CoincheTest.CoincheReussiTeam1Annonce80 (0 ms)
[ RUN      ] CoincheTest.CoincheEchoueTeam1Annonce80
Le joueur TestPlayer3 a la belote!
[       OK ] CoincheTest.CoincheEchoueTeam1Annonce80 (0 ms)
[ RUN      ] CoincheTest.SurcoincheReussiTeam2Annonce120
Le joueur TestPlayer3 a la belote!
[       OK ] CoincheTest.SurcoincheReussiTeam2Annonce120 (0 ms)
[ RUN      ] CoincheTest.SurcoincheEchoueTeam2Annonce120
Le joueur TestPlayer2 a la belote!
[       OK ] CoincheTest.SurcoincheEchoueTeam2Annonce120 (0 ms)
[ RUN      ] CoincheTest.CoincheAvecBeloteTeam1Reussi
Le joueur TestPlayer2 a la belote!
[       OK ] CoincheTest.CoincheAvecBeloteTeam1Reussi (0 ms)
[----------] 5 tests from CoincheTest (0 ms total)

[----------] Global test environment tear-down
[==========] 5 tests from 1 test suite ran. (0 ms total)
[  PASSED  ] 5 tests.
```

### Exemple avec filtre

```bash
$ ./test_capot_generale --gtest_filter=*Capot*
Note: Google Test filter = *Capot*
[==========] Running 2 tests from 1 test suite.
[ RUN      ] CapotGeneraleTest.CapotReussiAvecBelote
[       OK ] CapotGeneraleTest.CapotReussiAvecBelote (0 ms)
[ RUN      ] CapotGeneraleTest.CapotEchoue
[       OK ] CapotGeneraleTest.CapotEchoue (0 ms)
[  PASSED  ] 2 tests.
```

## Notes d'implémentation

Les tests utilisent des **compteurs de plis simplifiés** plutôt que de jouer réellement chaque carte. Cela permet de :

1. Tester rapidement la logique de scoring
2. Vérifier les conditions CAPOT/GENERALE
3. S'assurer que les compteurs par joueur fonctionnent correctement

Pour des tests plus complets avec simulation de jeu réelle, voir `tests/integration/` (à venir).

## Règles de scoring COINCHE/SURCOINCHE

### COINCHE
Lorsqu'un joueur de l'équipe adverse annonce "Coinche", la phase d'enchères se termine immédiatement.
- Si l'équipe qui a fait l'enchère **réussit** son contrat : elle marque **(valeur du contrat + points réalisés) × 2**
- Si l'équipe qui a fait l'enchère **échoue** son contrat : l'équipe adverse marque **(valeur du contrat + 160) × 2**

### SURCOINCHE
Lorsqu'un joueur de l'équipe qui a fait l'enchère répond par "Surcoinche" à un "Coinche":
- Si l'équipe qui a fait l'enchère **réussit** son contrat : elle marque **(valeur du contrat + points réalisés) × 4**
- Si l'équipe qui a fait l'enchère **échoue** son contrat : l'équipe adverse marque **(valeur du contrat + 160) × 4**

### CAPOT et GENERALE coinchés/surcoinchés
Les annonces CAPOT et GENERALE peuvent aussi être coinchées/surcoinchées:
- **CAPOT coinché réussi**: 500 × 2 = 1000 points
- **CAPOT surcoinché réussi**: 500 × 4 = 2000 points
- **CAPOT coinché échoué**: (160 + 250) × 2 = 820 points pour l'équipe adverse
- **CAPOT surcoinché échoué**: (160 + 250) × 4 = 1640 points pour l'équipe adverse
- **GENERALE coinché réussi**: 1000 × 2 = 2000 points
- **GENERALE surcoinché réussi**: 1000 × 4 = 4000 points
- **GENERALE coinché échoué**: (160 + 500) × 2 = 1320 points pour l'équipe adverse
- **GENERALE surcoinché échoué**: (160 + 500) × 4 = 2640 points pour l'équipe adverse

### Combinaison avec Belote
Les 20 points de Belote s'ajoutent au score de l'équipe qui l'a, même en cas de COINCHE/SURCOINCHE.

## Prochaines étapes

- [ ] Ajouter des tests d'intégration avec vraie simulation de plis
- [ ] Tester avec de vrais clients WebSocket
- [ ] Ajouter des tests pour la Belote combinée avec CAPOT/GENERALE
- [ ] Tester les cas limites (disconnect pendant GENERALE, etc.)
- [ ] Ajouter un bouton SURCOINCHE dans l'UI (actuellement seul COINCHE est disponible dans l'UI)
