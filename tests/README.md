# Tests CAPOT et GENERALE

Ce dossier contient les tests fonctionnels pour vérifier le bon fonctionnement des règles spéciales CAPOT et GENERALE.

Ces tests utilisent **Google Test**, le framework de test standard en C++.

## Structure

- `capot_generale_test.cpp` - Tests Google Test pour CAPOT et GENERALE
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

## Compilation et exécution

### Compilation

```bash
# Depuis le répertoire racine du projet
mkdir build
cd build
cmake ..
cmake --build . --target test_capot_generale

# Ou compiler tous les tests
cmake --build . --target coinche_tests test_capot_generale
```

### Exécution des tests

```bash
# Exécuter tous les tests CAPOT/GENERALE
cd build/tests
./test_capot_generale          # Linux/Mac
.\test_capot_generale.exe      # Windows

# Exécuter un test spécifique
./test_capot_generale --gtest_filter=CapotGeneraleTest.CapotReussiAvecBelote

# Exécuter tous les tests CAPOT uniquement
./test_capot_generale --gtest_filter=*Capot*

# Exécuter tous les tests GENERALE uniquement
./test_capot_generale --gtest_filter=*Generale*

# Afficher la liste des tests
./test_capot_generale --gtest_list_tests

# Mode verbose
./test_capot_generale --gtest_print_time=1
```

### Intégration avec CTest

Les tests sont automatiquement découverts par CTest :

```bash
cd build
ctest -R test_capot_generale    # Exécuter ces tests via CTest
ctest --verbose                 # Mode verbose
```

## Exemple de sortie

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

## Prochaines étapes

- [ ] Ajouter des tests d'intégration avec vraie simulation de plis
- [ ] Tester avec de vrais clients WebSocket
- [ ] Ajouter des tests pour la Belote combinée avec CAPOT/GENERALE
- [ ] Tester les cas limites (disconnect pendant GENERALE, etc.)
