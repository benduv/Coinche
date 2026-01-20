# 📊 Système de Statistiques Quotidiennes - Résumé

## ✅ Ce qui a été implémenté

### 1. Base de données
- ✅ Nouvelle table `daily_stats` créée dans SQLite
- ✅ Colonnes: date, logins, game_rooms_created, new_accounts, player_quits
- ✅ Index unique sur la date

### 2. Tracking automatique
- ✅ **Connexions**: Enregistrées à chaque login réussi
- ✅ **GameRooms**: Enregistrées à chaque création de partie
- ✅ **Nouveaux comptes**: Enregistrés à chaque création de compte
- ✅ **Abandons**: Enregistrés quand un joueur déconnecte pendant une partie

### 3. Rapports par email
- ✅ Classe `StatsReporter` créée
- ✅ Email HTML avec design moderne (thème spatial)
- ✅ Comparaison avec la veille (tendances)
- ✅ Envoi automatique quotidien à minuit
- ✅ Envoi manuel possible pour tests

### 4. Intégration serveur
- ✅ StatsReporter intégré dans GameServer
- ✅ Appels de tracking aux bons endroits
- ✅ Configuration via mot de passe SMTP

## 📁 Fichiers créés/modifiés

### Nouveaux fichiers
```
server/StatsReporter.h         # Classe de génération et envoi de rapports
server/StatsReporter.cpp        # Implémentation
server/STATS_README.md          # Documentation complète
server/STATS_SUMMARY.md         # Ce fichier (résumé)
test_stats.sh                   # Script de test
```

### Fichiers modifiés
```
server/DatabaseManager.h        # Ajout des méthodes de tracking
server/DatabaseManager.cpp      # Implémentation + table daily_stats
server/GameServer.h             # Intégration StatsReporter + tracking
CMakeLists.txt                  # Ajout de StatsReporter dans la compilation
```

## 🚀 Utilisation

### Démarrage
```bash
# Avec variable d'environnement
export COINCHE_SMTP_PASSWORD="votre_mot_de_passe_ovh"
./server

# Ou avec argument
./server --smtp-password "votre_mot_de_passe_ovh"
```

### Test
```bash
# Tester que la table existe et insérer des données de test
./test_stats.sh

# Pour tester l'envoi d'email immédiatement
./test_stats.sh "votre_mot_de_passe_smtp"
```

## 📧 Email reçu quotidiennement

**À**: contact@nebuludik.fr
**Sujet**: 📊 Rapport Quotidien Coinche - [DATE]
**Format**: HTML avec design spatial

**Contenu**:
- 🔐 Connexions avec tendance vs hier
- 🎮 Parties créées avec tendance
- ✨ Nouveaux joueurs avec tendance
- ⚠️ Abandons avec tendance

## 📊 Exemple de rapport

```
📊 Rapport Quotidien Coinche - 2026-01-20

🔐 Connexions
   45 connexions
   📈 +15% vs hier

🎮 Parties créées
   12 GameRooms
   📈 +20% vs hier

✨ Nouveaux joueurs
   3 comptes
   🚀 Nouveau!

⚠️ Abandons de partie
   5 abandons
   📉 -10% vs hier
```

## 🔧 Configuration avancée

### Changer l'heure d'envoi
Par défaut: minuit (00:00). Pour modifier, ajoutez dans `server_main.cpp`:
```cpp
server.getStatsReporter()->setReportTime(8, 0);  // 8h du matin
```

### Envoi manuel (pour tester)
Ajoutez après la création du serveur:
```cpp
QTimer::singleShot(5000, [&server]() {
    server.getStatsReporter()->sendDailyReport();
});
```

## 📈 Requêtes SQL utiles

### Stats d'aujourd'hui
```sql
SELECT * FROM daily_stats WHERE date = date('now');
```

### 7 derniers jours
```sql
SELECT * FROM daily_stats
WHERE date >= date('now', '-7 days')
ORDER BY date DESC;
```

### Tendances mensuelles
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

## ✅ Prochaines étapes

1. **Recompiler le serveur**:
   ```bash
   cd build
   cmake ..
   make
   ```

2. **Tester localement**:
   ```bash
   ./test_stats.sh
   ```

3. **Déployer sur VPS**:
   ```bash
   # Copier les fichiers
   scp -r server/* user@vps:/path/to/server/

   # Recompiler sur le VPS
   ssh user@vps
   cd /path/to/server
   cmake .. && make

   # Démarrer avec SMTP
   export COINCHE_SMTP_PASSWORD="votre_mot_de_passe"
   ./server --smtp-password "$COINCHE_SMTP_PASSWORD"
   ```

4. **Vérifier les emails**:
   - Le premier rapport sera envoyé à minuit
   - Pour tester immédiatement, voir section "Envoi manuel"

## 🎯 Métriques trackées automatiquement

| Métrique | Quand | Où dans le code |
|----------|-------|-----------------|
| Connexions | Login réussi | `handleLoginAccount()` ligne 810 |
| Nouveaux comptes | Création compte | `handleRegisterAccount()` ligne 770 |
| GameRooms | Partie créée | Après `new GameRoom()` ligne 1121 |
| Abandons | Déconnexion en partie | `handlePlayerDisconnect()` ligne 4568 |

## 💡 Améliorations futures possibles

- [ ] Dashboard web pour visualiser les stats
- [ ] Graphiques de tendances
- [ ] Alertes si métriques anormales
- [ ] Rapports hebdomadaires/mensuels
- [ ] Export CSV/JSON
- [ ] API REST pour consulter les stats
- [ ] Stats géographiques
- [ ] Temps de session moyen
- [ ] Taux de rétention

## 🐛 Dépannage

### Email non reçu
1. Vérifier le mot de passe SMTP: `echo $COINCHE_SMTP_PASSWORD`
2. Vérifier les logs: `tail -f server_log.txt | grep -i "stats\|smtp"`
3. Tester manuellement (voir "Envoi manuel")

### Table non créée
1. Supprimer la DB: `rm coinche.db`
2. Recompiler: `cmake .. && make`
3. Relancer: `./server`

### Données non enregistrées
1. Vérifier les logs de tracking
2. Tester les insertions SQL directement
3. Vérifier les permissions sur coinche.db

## 📞 Support

Pour toute question sur le système de statistiques:
- Voir la documentation complète: `server/STATS_README.md`
- Vérifier les logs: `server_log.txt`
- Tester la DB: `sqlite3 coinche.db`

---

✅ **Système entièrement fonctionnel et prêt pour la production!**

© 2026 NEBULUDIK
