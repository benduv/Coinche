# Guide de déploiement sur VPS OVH

## 📋 Prérequis

Vous avez reçu de OVH:
- ✉️ Email avec l'**adresse IP** du VPS
- 🔑 **Mot de passe root** (à changer immédiatement)

## 🚀 Étape 1: Première connexion

### Depuis Windows (PowerShell ou CMD)

```bash
ssh root@VOTRE-IP-VPS
# Entrez le mot de passe reçu par email
```

### Changer le mot de passe root (IMPORTANT!)

```bash
passwd
# Entrez un nouveau mot de passe fort
```

## 🔧 Étape 2: Installation des dépendances

```bash
# Mise à jour du système
apt update && apt upgrade -y

# Installation des outils de compilation
apt install -y build-essential cmake git nano wget curl

# Installation de Qt 6 et dépendances
apt install -y qt6-base-dev qt6-websockets-dev libqt6sql6-sqlite libqt6core6

# Vérification
qmake6 --version
```

## 👤 Étape 3: Création d'un utilisateur dédié

```bash
# Créer l'utilisateur 'coinche'
useradd -m -s /bin/bash coinche
passwd coinche
# Choisir un mot de passe

# Ajouter aux sudoers (optionnel)
usermod -aG sudo coinche

# Créer le répertoire de logs
mkdir -p /var/log/coinche
chown coinche:coinche /var/log/coinche
```

## 📦 Étape 4: Transfert du code source

### Option A: Via Git (Recommandé si vous avez un repo)

```bash
su - coinche
git clone https://github.com/VOTRE-USERNAME/Coinche.git
cd Coinche
```

### Option B: Via SCP depuis votre PC Windows

Ouvrez PowerShell sur votre PC:

```powershell
# Compresser le dossier server
Compress-Archive -Path "C:\Users\33672\projects\Coinche\server" -DestinationPath "C:\Users\33672\coinche-server.zip"

# Transférer vers le VPS
scp "C:\Users\33672\coinche-server.zip" coinche@VOTRE-IP-VPS:/home/coinche/

# Sur le VPS, décompresser
ssh coinche@VOTRE-IP-VPS
cd /home/coinche
apt install -y unzip
unzip coinche-server.zip
```

### Option C: Via WinSCP (Interface graphique)

1. Téléchargez [WinSCP](https://winscp.net/eng/download.php)
2. Connectez-vous avec:
   - Host: VOTRE-IP-VPS
   - Username: coinche
   - Password: mot de passe coinche
3. Glissez-déposez le dossier `server` vers `/home/coinche/`

## 🔨 Étape 5: Compilation du serveur

```bash
su - coinche
cd ~/server
mkdir build && cd build

# Configuration CMake
cmake ..

# Compilation (utilise tous les cores disponibles)
make -j$(nproc)

# Vérification
ls -lh GameServer
# Vous devriez voir le binaire GameServer
```

### Test rapide (optionnel)

```bash
./GameServer
# Le serveur démarre, vous verrez:
# [2026-01-XX XX:XX:XX.XXX] INFO: ========================================
# [2026-01-XX XX:XX:XX.XXX] INFO: Serveur de jeu démarre...
# [2026-01-XX XX:XX:XX.XXX] INFO: Mode verbeux: DESACTIVE
# ...

# Ctrl+C pour arrêter
```

## 🔥 Étape 6: Configuration du pare-feu

```bash
# Retour en root
exit

# Installation et configuration UFW
apt install -y ufw

# Autoriser SSH (IMPORTANT!)
ufw allow 22/tcp

# Autoriser le port du serveur de jeu
ufw allow 1234/tcp

# Activer le pare-feu
ufw enable

# Vérifier
ufw status
# Devrait afficher:
# Status: active
# To                         Action      From
# --                         ------      ----
# 22/tcp                     ALLOW       Anywhere
# 1234/tcp                   ALLOW       Anywhere
```

## ⚙️ Étape 7: Configuration du service systemd

### Créer le fichier de service

```bash
nano /etc/systemd/system/coinche-server.service
```

Collez ce contenu:

```ini
[Unit]
Description=Coinche Game Server
After=network.target

[Service]
Type=simple
User=coinche
WorkingDirectory=/home/coinche/server/build
ExecStart=/home/coinche/server/build/GameServer
Restart=always
RestartSec=10

# Logs
StandardOutput=append:/var/log/coinche/server.log
StandardError=append:/var/log/coinche/error.log

# Limites de sécurité
PrivateTmp=yes
NoNewPrivileges=true

[Install]
WantedBy=multi-user.target
```

Sauvegardez: `Ctrl+O`, `Entrée`, `Ctrl+X`

### Activer et démarrer le service

```bash
# Recharger systemd
systemctl daemon-reload

# Activer le service au démarrage
systemctl enable coinche-server

# Démarrer le service
systemctl start coinche-server

# Vérifier le statut
systemctl status coinche-server
```

Vous devriez voir:
```
● coinche-server.service - Coinche Game Server
   Loaded: loaded (/etc/systemd/system/coinche-server.service; enabled)
   Active: active (running) since ...
```

## 🔍 Étape 8: Vérification

### Vérifier que le serveur écoute

```bash
netstat -tlnp | grep 1234
# ou
ss -tlnp | grep 1234
```

Vous devriez voir:
```
tcp6  0  0  :::1234  :::*  LISTEN  [PID]/GameServer
```

### Voir les logs en temps réel

```bash
journalctl -u coinche-server -f
# ou
tail -f /var/log/coinche/server.log
```

### Test de connexion depuis votre PC

```bash
# Sur votre PC Windows (PowerShell)
Test-NetConnection -ComputerName VOTRE-IP-VPS -Port 1234
```

Ou via un navigateur web, installez une extension WebSocket (comme "Simple WebSocket Client") et testez:
```
ws://VOTRE-IP-VPS:1234
```

## 📱 Étape 9: Configuration du client

### Mettre à jour Config.qml

Éditez `c:\Users\33672\projects\Coinche\qml\Config.qml`:

```qml
property string environment: "remote"  // Changez de "localhost" à "remote"

readonly property var serverUrls: ({
    "localhost": "ws://localhost:1234",
    "emulator": "ws://10.0.2.2:1234",
    "local-network": "ws://172.20.10.13:1234",
    "remote": "ws://VOTRE-IP-VPS:1234"  // Remplacez par votre vraie IP
})
```

### Recompiler l'application

- **Desktop**: Recompilez et testez
- **Android**: Recompilez l'APK et redéployez sur le téléphone

## 🛠️ Maintenance quotidienne

### Redémarrer le serveur

```bash
systemctl restart coinche-server
```

### Arrêter le serveur

```bash
systemctl stop coinche-server
```

### Voir les logs

```bash
# Logs systemd (dernières 100 lignes)
journalctl -u coinche-server -n 100

# Logs en temps réel
journalctl -u coinche-server -f

# Fichiers de logs directs
tail -f /var/log/coinche/server.log
tail -f /var/log/coinche/error.log
```

### Nettoyer les logs

```bash
# Nettoyer le fichier de log
truncate -s 0 /var/log/coinche/server.log

# Ou limiter la taille des logs systemd
journalctl --vacuum-size=50M
```

### Voir l'utilisation des ressources

```bash
# Installer htop
apt install -y htop

# Lancer
htop
# Cherchez le processus GameServer
# Appuyez sur F10 pour quitter
```

## 🔄 Mise à jour du serveur

### Si vous utilisez Git

```bash
su - coinche
cd ~/Coinche
git pull
cd server/build
make -j$(nproc)
exit

# Redémarrer le service
systemctl restart coinche-server
```

### Si vous utilisez SCP

```bash
# Sur votre PC, transférez le nouveau code
scp -r "C:\Users\33672\projects\Coinche\server" coinche@VOTRE-IP-VPS:/home/coinche/server-new

# Sur le VPS
ssh coinche@VOTRE-IP-VPS
mv ~/server ~/server-backup
mv ~/server-new ~/server
cd ~/server
mkdir build && cd build
cmake ..
make -j$(nproc)
exit

# Redémarrer
systemctl restart coinche-server
```

## 🔒 Sécurité supplémentaire (Recommandé)

### 1. Désactiver l'authentification par mot de passe SSH

```bash
# Générer une clé SSH sur votre PC Windows (PowerShell)
ssh-keygen -t ed25519 -C "votre-email@example.com"
# Appuyez sur Entrée pour accepter l'emplacement par défaut

# Copier la clé publique vers le VPS
type $env:USERPROFILE\.ssh\id_ed25519.pub | ssh root@VOTRE-IP-VPS "mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys"

# Sur le VPS, désactiver l'authentification par mot de passe
nano /etc/ssh/sshd_config

# Changez ces lignes:
# PasswordAuthentication no
# PubkeyAuthentication yes

# Redémarrer SSH
systemctl restart sshd
```

### 2. Installer Fail2Ban (protection contre brute-force)

```bash
apt install -y fail2ban

# Configuration de base
cat > /etc/fail2ban/jail.local <<EOF
[DEFAULT]
bantime = 3600
findtime = 600
maxretry = 5

[sshd]
enabled = true
port = 22
logpath = /var/log/auth.log
EOF

# Démarrer et activer
systemctl enable fail2ban
systemctl start fail2ban

# Vérifier
fail2ban-client status
```

### 3. Activer les mises à jour automatiques de sécurité

```bash
apt install -y unattended-upgrades
dpkg-reconfigure -plow unattended-upgrades
# Sélectionnez "Yes"
```

## 🐛 Dépannage

### Le serveur ne démarre pas

```bash
# Voir les erreurs
journalctl -u coinche-server -n 50 --no-pager

# Tester manuellement
su - coinche
cd ~/server/build
./GameServer
# Regardez les erreurs affichées
```

### Le port n'est pas accessible

```bash
# Vérifier le pare-feu
ufw status

# Vérifier que le serveur écoute
netstat -tlnp | grep 1234

# Tester localement
telnet localhost 1234
```

### Problèmes de mémoire

```bash
# Voir l'utilisation mémoire
free -h

# Voir les processus qui consomment le plus
top
# Appuyez sur M pour trier par mémoire
```

## 📊 Monitoring (Optionnel mais utile)

### Script de monitoring simple

```bash
# Créer un script de monitoring
nano /home/coinche/monitor.sh
```

Contenu:

```bash
#!/bin/bash
echo "=== Monitoring Coinche Server ==="
echo "Date: $(date)"
echo ""
echo "Service status:"
systemctl status coinche-server --no-pager | head -n 5
echo ""
echo "Memory usage:"
free -h
echo ""
echo "CPU load:"
uptime
echo ""
echo "Disk usage:"
df -h /
echo ""
echo "Active connections:"
netstat -an | grep :1234 | grep ESTABLISHED | wc -l
```

```bash
# Rendre exécutable
chmod +x /home/coinche/monitor.sh

# Exécuter
/home/coinche/monitor.sh
```

## 📈 Optimisations avancées (Si besoin)

### Activer le mode verbeux temporairement (debug)

```bash
# Éditer le service
nano /etc/systemd/system/coinche-server.service

# Modifier ExecStart:
ExecStart=/home/coinche/server/build/GameServer --verbose

# Recharger et redémarrer
systemctl daemon-reload
systemctl restart coinche-server

# N'oubliez pas de le désactiver après debug!
```

### Limiter l'utilisation CPU (si nécessaire)

```bash
# Éditer le service
nano /etc/systemd/system/coinche-server.service

# Ajouter sous [Service]:
CPUQuota=200%

# Recharger
systemctl daemon-reload
systemctl restart coinche-server
```

## ✅ Checklist finale

- [ ] Serveur compilé et fonctionne
- [ ] Service systemd activé
- [ ] Pare-feu configuré (ports 22 et 1234)
- [ ] Connexion testée depuis PC/téléphone
- [ ] Config.qml mis à jour avec l'IP du VPS
- [ ] Application recompilée
- [ ] Sécurité SSH configurée (clés + fail2ban)
- [ ] Backups configurés (automatiques OVH)

## 🆘 Support

En cas de problème:

1. **Vérifier les logs**: `journalctl -u coinche-server -f`
2. **Vérifier le pare-feu**: `ufw status`
3. **Tester le port**: `telnet VOTRE-IP-VPS 1234`
4. **Ressources**: `htop` ou `free -h`

---

**🎮 Bon jeu et bon déploiement!**
