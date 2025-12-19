# Guide rapide - Déploiement en production

## Étape 1 : Choisir un hébergeur

Je recommande **DigitalOcean** pour débuter (simple et pas cher) :

1. Créer un compte sur https://www.digitalocean.com
2. Créer un Droplet (serveur virtuel) :
   - **Image** : Ubuntu 22.04 LTS
   - **Plan** : Basic (4-6€/mois suffit pour commencer)
   - **Région** : Choisir la plus proche de vous (Paris, Frankfurt, Amsterdam)
   - **Authentification** : SSH key (recommandé) ou mot de passe

3. Notez l'**adresse IP** de votre serveur (ex: 192.168.1.100)

## Étape 2 : Se connecter au serveur

```bash
# Depuis Git Bash ou PowerShell
ssh root@VOTRE_IP
# Ou si vous avez créé un utilisateur :
ssh ubuntu@VOTRE_IP
```

## Étape 3 : Installer les dépendances

```bash
# Une fois connecté au serveur
sudo apt update
sudo apt upgrade -y
sudo apt install -y build-essential qt6-base-dev qt6-websockets-dev libqt6sql6-sqlite git

# Vérifier
qmake6 --version
```

## Étape 4 : Déployer le serveur

### Option A : Déploiement manuel

```bash
# Sur votre PC, dans le dossier server/
cd c:/Users/33672/projects/Coinche/server

# Copier les fichiers (remplacer root@VOTRE_IP par vos infos)
scp -r . root@VOTRE_IP:/root/coinche-server/
scp ../coinche.db root@VOTRE_IP:/root/coinche-server/
scp ../Player.* ../Deck.* ../Carte.* ../GameModel.* root@VOTRE_IP:/root/coinche-server/

# Sur le serveur
ssh root@VOTRE_IP
cd /root/coinche-server
qmake6 server.pro
make
./coinche_server
```

### Option B : Avec le script automatique (Linux/Mac/Git Bash)

```bash
# Rendre le script exécutable
chmod +x deploy/deploy.sh

# Lancer le déploiement
./deploy/deploy.sh root@VOTRE_IP
```

## Étape 5 : Configurer le pare-feu

```bash
# Sur le serveur
sudo ufw allow 1234/tcp
sudo ufw allow 22/tcp  # SSH
sudo ufw enable
```

## Étape 6 : Tester la connexion

Sur votre PC, testez la connexion :

```bash
# Windows PowerShell
Test-NetConnection VOTRE_IP -Port 1234

# Git Bash / Linux / Mac
nc -zv VOTRE_IP 1234
# ou
telnet VOTRE_IP 1234
```

## Étape 7 : Configurer le client

### Dans Config.qml (ligne 11)

```qml
property string environment: "production"  // Changer de "emulator" à "production"
```

### Dans Config.qml (ligne 16)

```qml
"production": "ws://VOTRE_IP:1234"  // Remplacer VOTRE_IP
```

Exemple si votre IP est 192.168.1.100 :
```qml
"production": "ws://192.168.1.100:1234"
```

## Étape 8 : Tester avec vos amis

1. Compilez et lancez l'application sur votre PC
2. Demandez à vos amis de faire pareil (ils doivent aussi modifier Config.qml avec votre IP)
3. Lancez une partie !

## Commandes utiles

### Voir les logs du serveur
```bash
ssh root@VOTRE_IP 'tail -f /root/coinche-server/server_log.txt'
```

### Redémarrer le serveur
```bash
ssh root@VOTRE_IP
cd /root/coinche-server
killall coinche_server
./coinche_server &
```

### Installer comme service (démarrage automatique)
```bash
# Créer le fichier service
sudo nano /etc/systemd/system/coinche-server.service
```

Copier ce contenu :
```ini
[Unit]
Description=Coinche Game Server
After=network.target

[Service]
Type=simple
User=root
WorkingDirectory=/root/coinche-server
ExecStart=/root/coinche-server/coinche_server
Restart=always
RestartSec=10

StandardOutput=append:/root/coinche-server/server_log.txt
StandardError=append:/root/coinche-server/server_error.txt

[Install]
WantedBy=multi-user.target
```

Puis :
```bash
sudo systemctl daemon-reload
sudo systemctl enable coinche-server
sudo systemctl start coinche-server
sudo systemctl status coinche-server
```

## Problèmes courants

### Le client ne se connecte pas
1. Vérifier que le serveur est démarré : `ssh root@VOTRE_IP 'ps aux | grep coinche_server'`
2. Vérifier le pare-feu : `ssh root@VOTRE_IP 'sudo ufw status'`
3. Vérifier que le port est ouvert : `nc -zv VOTRE_IP 1234`
4. Vérifier les logs : `ssh root@VOTRE_IP 'tail -f /root/coinche-server/server_log.txt'`

### Le serveur crash
Voir les logs d'erreur :
```bash
ssh root@VOTRE_IP 'tail -f /root/coinche-server/server_error.txt'
```

### Base de données corrompue
Remplacer par une nouvelle :
```bash
scp ../coinche.db root@VOTRE_IP:/root/coinche-server/
```

## Prochaines étapes (optionnel)

1. **Nom de domaine** : Acheter un nom de domaine (ex: coinche.com) et le pointer vers votre IP
2. **SSL/TLS** : Configurer HTTPS/WSS pour sécuriser les connexions
3. **Monitoring** : Installer un système de surveillance (Grafana, Prometheus)
4. **Backup** : Automatiser les sauvegardes de la base de données

## Coûts estimés

- **Serveur DigitalOcean** : 4-6€/mois
- **Nom de domaine** : 10-15€/an (optionnel)
- **Certificat SSL** : Gratuit avec Let's Encrypt

**Total : environ 5€/mois pour commencer**
