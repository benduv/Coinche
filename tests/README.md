# Tests CAPOT et GENERALE

Ce dossier contient les tests fonctionnels pour vérifier le bon fonctionnement des règles spéciales CAPOT et GENERALE.

## Structure

- `GameServerTest.h` - Classe de test contenant tous les scénarios de test
- `test_capot_generale.cpp` - Point d'entrée pour exécuter les tests

## Tests implémentés

### Test 1: CAPOT réussi
- **Scénario**: Team1 annonce CAPOT et fait tous les 8 plis
- **Mains**: Team1 reçoit tous les atouts forts (Valet, 9, As, 10, etc.)
- **Résultat attendu**: Team1 marque 500 points (250+250), Team2 marque 0

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
- **Mains**: Joueur 2 a une carte forte qui lui permet de gagner 1 pli
- **Résultat attendu**: Team1 marque 0, Team2 marque 660 points (160+500)

## Compilation

### Avec CMake (recommandé)

La configuration CMake est déjà incluse dans le projet. Pour compiler et exécuter:

```bash
# Depuis le répertoire racine du projet
mkdir build
cd build
cmake ..
cmake --build . --target test_capot_generale

# Exécuter les tests
./test_capot_generale

# Ou sur Windows
.\test_capot_generale.exe
```

Le fichier `tests/CMakeLists.txt` contient la configuration suivante:

```cmake
add_executable(test_capot_generale
    test_capot_generale.cpp
    GameServerTest.h
)

target_include_directories(test_capot_generale PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_capot_generale PRIVATE
    coinche_common
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
)
```

### Avec qmake (alternatif)

Si vous utilisez qmake plutôt que CMake, créez un fichier `tests.pro` :

```qmake
QT += core network websockets
CONFIG += console c++17
CONFIG -= app_bundle

SOURCES += test_capot_generale.cpp
HEADERS += GameServerTest.h

INCLUDEPATH += ..
INCLUDEPATH += ../server

# Lier avec les sources communes
SOURCES += ../Carte.cpp ../Deck.cpp ../Player.cpp
HEADERS += ../Carte.h ../Deck.h ../Player.h
```

Puis compilez:

```bash
cd tests
qmake tests.pro
make
./test_capot_generale
```

## Exemple de sortie

```
==========================================
Lancement des tests fonctionnels CAPOT/GENERALE
==========================================

========================================
=== TESTS CAPOT ET GENERALE ===
========================================

--- Test 1: CAPOT reussi par Team1 ---
Configuration des mains pour CAPOT Team1 reussi
Mains distribuees: Team1 a tous les atouts forts
Simulation de 8 plis...
Pli 1 - Joueur 0 commence
  -> Gagnant: Joueur 0
Pli 2 - Joueur 0 commence
  -> Gagnant: Joueur 2
...
Plis Team1: 8 (Player0: 4, Player2: 4)
Plis Team2: 0 (Player1: 0, Player3: 0)
Score attendu - Team1: 500, Team2: 0
✓ Test CAPOT reussi: PASSED

--- Test 2: CAPOT echoue ---
...
✓ Test CAPOT echoue: PASSED

--- Test 3: GENERALE reussie ---
...
✓ Test GENERALE reussie: PASSED

--- Test 4: GENERALE echouee ---
...
✓ Test GENERALE echouee: PASSED

========================================
=== TOUS LES TESTS PASSES ===
========================================

==========================================
SUCCES: Tous les tests sont passes!
==========================================
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
