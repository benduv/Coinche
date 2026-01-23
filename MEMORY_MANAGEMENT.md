# Analyse de la Gestion Mémoire

## Vue d'ensemble

Ce document analyse la gestion mémoire dans le projet Coinche et documente les améliorations apportées pour prévenir les fuites mémoire.

## Structures avec Allocation Dynamique

### 1. PlayerConnection (server/GameServer.h:28-36)

**Allocation**: Pointeurs bruts `PlayerConnection*` stockés dans `QMap<QString, PlayerConnection*> m_connections`

**Libération**:
- ✅ **onDisconnected()** (ligne 360): `delete it.value()` quand un joueur se déconnecte
- ✅ **Destructeur ~GameServer()** (ligne 273): `qDeleteAll(m_connections.values())`

**Verdict**: ✅ Gestion correcte - Pas de fuite mémoire

---

### 2. GameRoom (server/GameServer.h:39-166)

**Allocation**: Pointeurs bruts `GameRoom*` stockés dans `QMap<int, GameRoom*> m_gameRooms`

**Ressources internes**:
- `QTimer* turnTimeout` (ligne 73)
- `QTimer* bidTimeout` (ligne 77)
- `QTimer* surcoincheTimer` (ligne 67)
- `std::vector<std::unique_ptr<Player>> players` (ligne 48) - ✅ Smart pointers
- `std::vector<std::pair<int, Carte*>>` - Pointeurs vers cartes gérées par Deck

**Libération**:
- ✅ **handleForfeit()** (ligne 2653): `delete room` quand tous les joueurs quittent
- ✅ **Destructeur ~GameServer()** (ligne 271): `qDeleteAll(m_gameRooms.values())`
- ✅ **Destructeur ~GameRoom()** (ligne 145-163): Nettoie automatiquement tous les timers

**Améliorations apportées**:
1. Ajout d'un destructeur `~GameRoom()` qui nettoie automatiquement les 3 timers
2. Simplification de `handleForfeit()` - le nettoyage des timers est maintenant automatique
3. Protection contre les fuites si une GameRoom est supprimée ailleurs dans le code

**Verdict**: ✅ Gestion correcte après améliorations - Pas de fuite mémoire

---

### 3. PrivateLobby (server/GameServer.h:169-175)

**Allocation**: Pointeurs bruts `PrivateLobby*` stockés dans `QMap<QString, PrivateLobby*> m_privateLobbies`

**Ressources internes**: Aucun pointeur - uniquement des QString et QList

**Libération**:
- ✅ **handleStartLobbyGame()** (ligne 4679): `delete lobby` quand une partie démarre
- ✅ **handleLeaveLobby()** (ligne 4714): `delete lobby` quand le lobby devient vide
- ⚠️ **Destructeur ~GameServer()**: MANQUAIT - Ajouté (ligne 278)

**Améliorations apportées**:
1. Ajout de `qDeleteAll(m_privateLobbies.values())` dans le destructeur
2. Ajout de `m_privateLobbies.clear()` pour nettoyer la map

**Verdict**: ✅ Gestion correcte après améliorations - Pas de fuite mémoire

---

## Pointeurs Qt avec Parent

Ces objets sont automatiquement nettoyés par Qt quand leur parent est détruit:

- `m_server` (QWebSocketServer) - parent: this
- `m_matchmakingTimer` (QTimer) - parent: this
- `m_countdownTimer` (QTimer) - parent: this
- `m_dbManager` (DatabaseManager) - parent: this
- `m_smtpClient` (SmtpClient) - parent: this
- `m_statsReporter` (StatsReporter) - parent: this

**Verdict**: ✅ Gestion automatique par Qt

---

## Utilisation de Smart Pointers

### Endroits où des smart pointers SONT utilisés:

1. ✅ **GameRoom::players** (ligne 48): `std::vector<std::unique_ptr<Player>>`
   - Les joueurs sont automatiquement libérés quand la GameRoom est détruite

### Endroits où des smart pointers POURRAIENT être utilisés:

1. ⚠️ **m_connections**: Pourrait utiliser `QMap<QString, std::unique_ptr<PlayerConnection>>`
   - Avantage: Libération automatique, impossible d'oublier un `delete`
   - Inconvénient: Nécessite des changements dans tout le code

2. ⚠️ **m_gameRooms**: Pourrait utiliser `QMap<int, std::unique_ptr<GameRoom>>`
   - Avantage: Libération automatique, impossible d'oublier un `delete`
   - Inconvénient: Nécessite des changements dans tout le code

3. ⚠️ **m_privateLobbies**: Pourrait utiliser `QMap<QString, std::unique_ptr<PrivateLobby>>`
   - Avantage: Libération automatique
   - Inconvénient: Nécessite des changements dans tout le code

**Recommandation**: La gestion actuelle avec pointeurs bruts + destructeurs est correcte et sûre. Une migration vers smart pointers serait une amélioration de qualité de code mais n'est pas urgente.

---

## Cartes (Carte*)

Les pointeurs `Carte*` sont gérés par la classe `Deck`:
- Le `Deck` crée les cartes lors de l'initialisation
- Le `Deck` possède les cartes et les libère dans son destructeur
- Les `GameRoom` stockent seulement des pointeurs vers ces cartes (ne les possèdent pas)

**Verdict**: ✅ Gestion correcte - Ownership clair

---

## Résumé des Améliorations

### Avant:
- ❌ Les timers de GameRoom n'étaient nettoyés que manuellement dans handleForfeit()
- ❌ Les PrivateLobby n'étaient jamais nettoyés à la fermeture du serveur
- ⚠️ Risque de fuites si une GameRoom était supprimée ailleurs sans nettoyer les timers

### Après:
- ✅ Destructeur `~GameRoom()` nettoie automatiquement tous les timers
- ✅ Destructeur `~GameServer()` nettoie tous les lobbies privés
- ✅ Code simplifié dans handleForfeit()
- ✅ Protection contre les fuites même si du code supprime une GameRoom ailleurs

---

## Outils de Détection de Fuites Mémoire

### Pour le serveur (C++):

1. **Valgrind** (Linux/macOS):
   ```bash
   valgrind --leak-check=full --show-leak-kinds=all ./server.exe
   ```

2. **Dr. Memory** (Windows):
   ```bash
   drmemory -- ./server.exe
   ```

3. **AddressSanitizer** (Tous OS):
   Ajouter au CMakeLists.txt:
   ```cmake
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
   ```

### Pour le client (Qt/QML):

Qt fournit déjà un système de détection des objets QObject non supprimés. Pour l'activer:

```bash
set QT_FATAL_WARNINGS=1
./coinche.exe
```

---

## Bonnes Pratiques Suivies

1. ✅ **RAII (Resource Acquisition Is Initialization)**:
   - Les destructeurs libèrent automatiquement les ressources

2. ✅ **Ownership clair**:
   - Les maps possèdent leurs objets via pointeurs
   - Les Deck possèdent leurs cartes

3. ✅ **Libération symétrique**:
   - Chaque `new` a un `delete` correspondant
   - Chaque allocation a une libération dans le destructeur

4. ✅ **Utilisation de smart pointers pour les collections**:
   - `std::unique_ptr<Player>` dans GameRoom::players

---

## Recommandations Futures

1. **Migration progressive vers smart pointers**:
   - Commencer par les nouvelles fonctionnalités
   - Migrer les structures existantes quand elles sont modifiées

2. **Tests réguliers avec outils de détection**:
   - Exécuter Valgrind/Dr. Memory en CI/CD
   - Tester avec AddressSanitizer pendant le développement

3. **Documentation de l'ownership**:
   - Commenter quel objet "possède" quel pointeur
   - Clarifier qui est responsable de la libération

---

## Conclusion

✅ **Aucune fuite mémoire détectée** dans l'état actuel du code après les améliorations.

Le code utilise une combinaison saine de:
- Pointeurs bruts avec gestion manuelle (mais systématique)
- Smart pointers pour les collections complexes
- Système de parent/child de Qt pour les objets Qt

Les améliorations apportées garantissent que:
1. Tous les timers sont nettoyés automatiquement
2. Tous les lobbies sont nettoyés à la fermeture
3. Le code est robuste même en cas de modifications futures
