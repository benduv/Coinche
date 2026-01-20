# Système de Statistiques Quotidiennes - Coinche Beta

## Vue d'ensemble

Le système de statistiques quotidiennes enregistre automatiquement les événements clés sur votre serveur et vous envoie un rapport quotidien par email à `contact@nebuludik.fr`.

## Métriques suivies

Le système track automatiquement:
- 🔐 **Connexions**: Nombre de connexions réussies par jour
- 🎮 **Parties créées**: Nombre de GameRooms créées
- ✨ **Nouveaux joueurs**: Nombre de créations de nouveaux comptes
- ⚠️  **Abandons**: Nombre de joueurs qui quittent pendant une partie

## Architecture

### Fichiers créés

1. **DatabaseManager.h/cpp** (modifié)
   - Table `daily_stats` ajoutée à la base de données
   - Méthodes de tracking: `recordLogin()`, `recordGameRoomCreated()`, etc.
   - Méthodes de lecture: `getDailyStats()`, `getYesterdayStats()`

2. **StatsReporter.h/cpp** (nouveau)
   - Génère et envoie le rapport quotidien par email
   - Timer automatique (par défaut: minuit)
   - Génération HTML avec graphiques et tendances

3. **GameServer.h** (modifié)
   - Intégration de StatsReporter
   - Appels de tracking aux points clés:
     - Login réussi → `recordLogin()`
     - Création compte → `recordNewAccount()`
     - GameRoom créée → `recordGameRoomCreated()`
     - Déconnexion en partie → `recordPlayerQuit()`

## Configuration

### 1. Variables d'environnement (recommandé)

```bash
export COINCHE_SMTP_PASSWORD="votre_mot_de_passe_ovh"
```

### 2. Arguments en ligne de commande

```bash
./server --smtp-password "votre_mot_de_passe_ovh"
```

## Envoi du rapport

### Automatique (par défaut)

Le rapport est envoyé automatiquement **tous les jours à minuit (00:00)**.

### Manuel (pour tester)

Vous pouvez déclencher un envoi immédiat pour tester:

```cpp
// Dans server_main.cpp, après la création du serveur:
QTimer::singleShot(5000, &server, [&server]() {
    // Obtenir le StatsReporter du serveur
    server.getStatsReporter()->sendDailyReport();
});
```

## Format du rapport email

Le rapport est envoyé en HTML avec:
- **Design moderne** inspiré du thème spatial de votre jeu
- **Cartes visuelles** pour chaque métrique
- **Tendances** avec icônes (📈 hausse, 📉 baisse, → stable)
- **Comparaison** avec la veille

Exemple de rapport:

```
📊 Rapport Quotidien Coinche
2026-01-20

🔐 Connexions: 45
   📈 +15% vs hier

🎮 Parties créées: 12
   📈 +20% vs hier

✨ Nouveaux joueurs: 3
   🚀 Nouveau!

⚠️  Abandons: 5
   📉 -10% vs hier
```

## Base de données

### Structure de la table daily_stats

```sql
CREATE TABLE daily_stats (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    date TEXT UNIQUE NOT NULL,
    logins INTEGER DEFAULT 0,
    game_rooms_created INTEGER DEFAULT 0,
    new_accounts INTEGER DEFAULT 0,
    player_quits INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### Requêtes utiles

Voir les stats d'aujourd'hui:
```sql
SELECT * FROM daily_stats WHERE date = date('now');
```

Voir les stats des 7 derniers jours:
```sql
SELECT * FROM daily_stats
WHERE date >= date('now', '-7 days')
ORDER BY date DESC;
```

Tendances mensuelles:
```sql
SELECT
    date,
    logins,
    game_rooms_created,
    new_accounts,
    player_quits
FROM daily_stats
WHERE date >= date('now', 'start of month')
ORDER BY date ASC;
```

## Dépannage

### Le rapport n'est pas envoyé

1. **Vérifier le mot de passe SMTP**:
   ```bash
   echo $COINCHE_SMTP_PASSWORD
   ```

2. **Vérifier les logs du serveur**:
   ```bash
   tail -f server_log.txt | grep -i "stats\|smtp\|email"
   ```

3. **Tester manuellement** (voir section "Envoi manuel")

### Changer l'heure d'envoi

Par défaut: minuit (00:00). Pour changer:

```cpp
// Dans server_main.cpp, après création du serveur:
server.getStatsReporter()->setReportTime(8, 0);  // 8h00 du matin
```

### Vérifier que les événements sont trackés

```bash
# Dans le dossier du serveur
sqlite3 coinche.db "SELECT * FROM daily_stats ORDER BY date DESC LIMIT 5;"
```

## Améliorations futures possibles

- 📊 Dashboard web pour visualiser les stats
- 📈 Graphiques de tendances sur 7/30 jours
- 🔔 Alertes si métriques anormales (ex: trop d'abandons)
- 📧 Rapports hebdomadaires/mensuels
- 🌍 Stats géographiques (pays des joueurs)
- ⏱️  Temps de session moyen
- 🎯 Taux de rétention (J1, J7, J30)

## Support

Si vous rencontrez des problèmes avec le système de statistiques, vérifiez:
1. Les logs du serveur: `server_log.txt`
2. La table daily_stats existe: `sqlite3 coinche.db ".tables"`
3. Le mot de passe SMTP est configuré
4. Les événements sont bien enregistrés (voir logs)

---

© 2026 NEBULUDIK - Système de statistiques v1.0
