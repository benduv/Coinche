# Monitoring du Serveur - Guide Pratique

## 🎯 Métriques Clés à Surveiller

### 1. Parties Actives

**Commande rapide:**
```bash
# Sur le VPS
echo "SELECT COUNT(*) as active_rooms FROM (SELECT DISTINCT room_id FROM some_table);" | sqlite3 coinche.db
```

**Ou via logs:**
```bash
tail -f server_log.txt | grep "GameRoom créée"
```

### 2. Utilisation RAM

```bash
# Total utilisé par le serveur
ps aux | grep server | awk '{print $6/1024 " MB"}'

# RAM système disponible
free -h
```

**Seuil d'alerte:** > 6 GB utilisés (75% du VPS)

### 3. Utilisation CPU

```bash
# CPU du processus serveur
top -p $(pgrep server) -b -n 1 | tail -1

# Ou avec monitoring continu
htop
```

**Seuil d'alerte:** > 300% (3 cores sur 4)

### 4. Connexions Réseau

```bash
# Nombre de connexions WebSocket actives
netstat -an | grep :8080 | grep ESTABLISHED | wc -l

# Bande passante
iftop -i eth0
```

### 5. Base de Données

```bash
# Taille de la DB
ls -lh coinche.db

# Nombre d'entrées dans les tables critiques
sqlite3 coinche.db "SELECT
    (SELECT COUNT(*) FROM users) as total_users,
    (SELECT COUNT(*) FROM daily_stats) as days_tracked,
    (SELECT COUNT(*) FROM user_sessions) as total_sessions;"
```

## 📊 Dashboard Simple (Script Bash)

Créer un fichier `monitor.sh`:

```bash
#!/bin/bash

echo "======================================"
echo "📊 Coinche Server Status"
echo "======================================"
echo ""

# Processus
if pgrep -x "server" > /dev/null; then
    echo "✅ Serveur: ACTIF"
    PID=$(pgrep server)

    # RAM
    RAM_MB=$(ps aux | grep $PID | awk '{print $6/1024}' | head -1)
    echo "💾 RAM utilisée: ${RAM_MB} MB"

    # CPU
    CPU=$(ps aux | grep $PID | awk '{print $3}' | head -1)
    echo "🔥 CPU utilisée: ${CPU}%"

    # Uptime
    UPTIME=$(ps -p $PID -o etime= | tr -d ' ')
    echo "⏱️  Uptime: ${UPTIME}"
else
    echo "❌ Serveur: INACTIF"
fi

echo ""
echo "🌐 Connexions réseau:"
CONNECTIONS=$(netstat -an | grep :8080 | grep ESTABLISHED | wc -l)
echo "   WebSocket actives: ${CONNECTIONS}"

echo ""
echo "💾 Base de données:"
DB_SIZE=$(ls -lh coinche.db 2>/dev/null | awk '{print $5}')
echo "   Taille: ${DB_SIZE}"

USERS=$(sqlite3 coinche.db "SELECT COUNT(*) FROM users;" 2>/dev/null)
echo "   Utilisateurs: ${USERS}"

SESSIONS_TODAY=$(sqlite3 coinche.db "SELECT session_count FROM daily_stats WHERE date = date('now');" 2>/dev/null)
echo "   Sessions aujourd'hui: ${SESSIONS_TODAY:-0}"

echo ""
echo "🖥️  Système:"
echo "   $(free -h | grep Mem | awk '{print "RAM: "$3"/"$2" ("$3/$2*100"%)"}')"
echo "   $(df -h / | tail -1 | awk '{print "Disk: "$3"/"$2" ("$5")"}')"

echo ""
echo "======================================"
```

**Usage:**
```bash
chmod +x monitor.sh
./monitor.sh
```

## 🚨 Alertes Automatiques

### Script d'Alerte (alert.sh)

```bash
#!/bin/bash

# Seuils
MAX_RAM_PERCENT=75
MAX_CPU_PERCENT=300
MAX_DISK_PERCENT=80

# Email pour alertes
ADMIN_EMAIL="contact@nebuludik.fr"

# Vérifier RAM
RAM_USED=$(free | grep Mem | awk '{print $3/$2 * 100}')
if (( $(echo "$RAM_USED > $MAX_RAM_PERCENT" | bc -l) )); then
    echo "⚠️ ALERTE: RAM à ${RAM_USED}%" | mail -s "Coinche Server - RAM Alert" $ADMIN_EMAIL
fi

# Vérifier CPU
CPU_USED=$(ps aux | grep server | awk '{sum+=$3} END {print sum}')
if (( $(echo "$CPU_USED > $MAX_CPU_PERCENT" | bc -l) )); then
    echo "⚠️ ALERTE: CPU à ${CPU_USED}%" | mail -s "Coinche Server - CPU Alert" $ADMIN_EMAIL
fi

# Vérifier Disk
DISK_USED=$(df / | tail -1 | awk '{print $5}' | sed 's/%//')
if [ $DISK_USED -gt $MAX_DISK_PERCENT ]; then
    echo "⚠️ ALERTE: Disk à ${DISK_USED}%" | mail -s "Coinche Server - Disk Alert" $ADMIN_EMAIL
fi

# Vérifier que le serveur tourne
if ! pgrep -x "server" > /dev/null; then
    echo "🔴 ALERTE CRITIQUE: Le serveur ne répond pas!" | mail -s "Coinche Server - DOWN" $ADMIN_EMAIL
fi
```

**Automatiser avec cron:**
```bash
# Éditer crontab
crontab -e

# Ajouter (check toutes les 5 minutes)
*/5 * * * * /root/alert.sh >> /var/log/coinche-alerts.log 2>&1
```

## 📈 Métriques de Performance

### Créer un log de performance

Ajouter dans le serveur (optionnel):

```cpp
// Dans GameServer.h - méthode appelée toutes les minutes
void logPerformanceMetrics() {
    static QElapsedTimer timer;
    static qint64 lastLog = 0;

    if (!timer.isValid()) {
        timer.start();
        lastLog = QDateTime::currentMSecsSinceEpoch();
        return;
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - lastLog > 60000) {  // 1 minute
        qInfo() << "📊 METRICS:"
                << "Rooms:" << m_gameRooms.size()
                << "Connections:" << m_connections.size()
                << "Queue:" << m_matchmakingQueue.size();
        lastLog = now;
    }
}
```

### Parser les logs

```bash
# Extraire métriques des logs
grep "📊 METRICS" server_log.txt | tail -20

# Parties max atteintes
grep "📊 METRICS" server_log.txt | grep -oP 'Rooms:\s*\K\d+' | sort -n | tail -1
```

## 🎯 Objectifs de Performance

### Latence

**Mesurer:**
```bash
# Depuis un client
ping votre-vps.com

# WebSocket roundtrip
# (implémenter un "ping" message dans le serveur)
```

**Objectifs:**
- Ping < 50ms: ✅ Excellent
- Ping 50-100ms: ✅ Bon
- Ping > 100ms: ⚠️ Vérifier réseau

### Throughput

**Parties par heure:**
```sql
SELECT
    date,
    game_rooms_created,
    game_rooms_created / 24.0 as parties_per_hour
FROM daily_stats
ORDER BY date DESC
LIMIT 7;
```

## 🔧 Optimisations Préventives

### 1. Rotation des Logs

```bash
# /etc/logrotate.d/coinche-server
/var/log/coinche/*.log {
    daily
    rotate 7
    compress
    missingok
    notifempty
}
```

### 2. Backup Automatique

```bash
#!/bin/bash
# backup.sh

DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_DIR="/backup/coinche"

# Backup DB
cp coinche.db $BACKUP_DIR/coinche_${DATE}.db
gzip $BACKUP_DIR/coinche_${DATE}.db

# Garder seulement les 7 derniers jours
find $BACKUP_DIR -name "coinche_*.db.gz" -mtime +7 -delete

echo "✅ Backup créé: coinche_${DATE}.db.gz"
```

**Automatiser:**
```bash
# Crontab - backup quotidien à 3h du matin
0 3 * * * /root/backup.sh >> /var/log/coinche-backup.log 2>&1
```

### 3. Redémarrage Automatique en Cas de Crash

**Systemd Service** (`/etc/systemd/system/coinche-server.service`):

```ini
[Unit]
Description=Coinche Game Server
After=network.target

[Service]
Type=simple
User=coinche
WorkingDirectory=/opt/coinche
ExecStart=/opt/coinche/server --smtp-password "VOTRE_PASSWORD"
Restart=always
RestartSec=10
StandardOutput=append:/var/log/coinche/server.log
StandardError=append:/var/log/coinche/error.log

[Install]
WantedBy=multi-user.target
```

**Activer:**
```bash
systemctl enable coinche-server
systemctl start coinche-server
systemctl status coinche-server
```

## 📊 Dashboard Web Simple (Bonus)

Si tu veux un dashboard visuel, simple script PHP:

```php
<?php
// dashboard.php - À mettre sur un serveur web

$db = new SQLite3('/opt/coinche/coinche.db');

// Stats du jour
$today = $db->querySingle("SELECT * FROM daily_stats WHERE date = date('now')", true);

// Serveur running?
$serverRunning = shell_exec('pgrep server') ? true : false;

// RAM
$ram = shell_exec("ps aux | grep server | awk '{print \$6/1024}'");

?>
<!DOCTYPE html>
<html>
<head>
    <title>Coinche Server Dashboard</title>
    <meta http-equiv="refresh" content="10">
    <style>
        body { font-family: Arial; background: #0a0a2e; color: #fff; padding: 20px; }
        .metric { background: #16213e; padding: 20px; margin: 10px; border-radius: 10px; }
        .value { font-size: 48px; font-weight: bold; color: #FFD700; }
    </style>
</head>
<body>
    <h1>🎮 Coinche Server Status</h1>

    <div class="metric">
        <h3>Serveur</h3>
        <div class="value"><?= $serverRunning ? '✅ ACTIF' : '❌ INACTIF' ?></div>
    </div>

    <div class="metric">
        <h3>Connexions Aujourd'hui</h3>
        <div class="value"><?= $today['logins'] ?? 0 ?></div>
    </div>

    <div class="metric">
        <h3>Parties Créées</h3>
        <div class="value"><?= $today['game_rooms_created'] ?? 0 ?></div>
    </div>

    <div class="metric">
        <h3>RAM Utilisée</h3>
        <div class="value"><?= round($ram) ?> MB</div>
    </div>
</body>
</html>
```

## 🎯 Checklist de Production

Avant le lancement public:

- [ ] Monitoring script installé (`monitor.sh`)
- [ ] Alertes configurées (`alert.sh` + cron)
- [ ] Logs rotatés (logrotate)
- [ ] Backup quotidien (backup.sh + cron)
- [ ] Systemd service activé
- [ ] Firewall configuré (port 8080)
- [ ] SSL/TLS configuré (wss://)
- [ ] Dashboard accessible (optionnel)

## 🔍 Troubleshooting

### Serveur lent?

1. **Vérifier CPU:**
   ```bash
   top -p $(pgrep server)
   ```

2. **Vérifier RAM:**
   ```bash
   ps aux | grep server
   ```

3. **Vérifier DB locks:**
   ```bash
   sqlite3 coinche.db "PRAGMA wal_checkpoint;"
   ```

### Trop de connexions?

```bash
# Limiter dans GameServer.h
const int MAX_CONCURRENT_GAMES = 500;
```

### DB trop grosse?

```bash
# Nettoyer vieilles sessions
sqlite3 coinche.db "DELETE FROM user_sessions WHERE date(login_time) < date('now', '-90 days');"
sqlite3 coinche.db "VACUUM;"
```

---

**Ton VPS de 4 vCPU / 8 GB RAM peut facilement gérer 500+ parties simultanées!** 🚀

Pour la beta, tu es **largement surdimensionné**. Profites-en pour te concentrer sur les fonctionnalités plutôt que sur les performances.
