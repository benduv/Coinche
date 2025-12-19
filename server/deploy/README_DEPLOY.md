# Guide de déploiement du serveur Coinche

## Prérequis sur le serveur

### Installation des dépendances (Ubuntu/Debian)

```bash
# Mettre à jour le système
sudo apt update
sudo apt upgrade -y

# Installer Qt et dépendances
sudo apt install -y build-essential git cmake
sudo apt install -y qt6-base-dev qt6-websockets-dev libqt6sql6-sqlite

# Vérifier l'installation
qmake6 --version
```

## Déploiement

### 1. Copier les fichiers sur le serveur

```bash
# Depuis votre machine locale
scp -r server/ user@votre-serveur.com:/home/user/coinche-server/
scp coinche.db user@votre-serveur.com:/home/user/coinche-server/
```

### 2. Compiler le serveur

```bash
# Sur le serveur
cd /home/user/coinche-server
qmake6 server.pro
make

# Vérifier que l'exécutable est créé
ls -lh coinche_server
```

### 3. Configurer le pare-feu

```bash
# Ouvrir le port 1234 (ou votre port choisi)
sudo ufw allow 1234/tcp
sudo ufw enable
sudo ufw status
```

### 4. Créer un service systemd (pour démarrage automatique)

Créer le fichier `/etc/systemd/system/coinche-server.service` :

```ini
[Unit]
Description=Coinche Game Server
After=network.target

[Service]
Type=simple
User=user
WorkingDirectory=/home/user/coinche-server
ExecStart=/home/user/coinche-server/coinche_server
Restart=always
RestartSec=10

# Logs
StandardOutput=append:/home/user/coinche-server/server_log.txt
StandardError=append:/home/user/coinche-server/server_error.txt

[Install]
WantedBy=multi-user.target
```

Activer et démarrer le service :

```bash
sudo systemctl daemon-reload
sudo systemctl enable coinche-server
sudo systemctl start coinche-server
sudo systemctl status coinche-server
```

### 5. Vérifier les logs

```bash
# Logs du serveur
tail -f /home/user/coinche-server/server_log.txt

# Logs système
sudo journalctl -u coinche-server -f
```

## Configuration du client

Dans `MainMenu.qml`, modifier l'URL de connexion :

```qml
// Remplacer
networkManager.connectToServer("ws://10.0.2.2:1234")

// Par (remplacer par votre IP ou domaine)
networkManager.connectToServer("ws://VOTRE_IP_SERVEUR:1234")
```

## Commandes utiles

```bash
# Arrêter le serveur
sudo systemctl stop coinche-server

# Redémarrer le serveur
sudo systemctl restart coinche-server

# Voir les logs en temps réel
tail -f /home/user/coinche-server/server_log.txt

# Vérifier que le serveur écoute sur le port
sudo netstat -tlnp | grep 1234
# ou
sudo ss -tlnp | grep 1234
```

## Sécurité (À faire plus tard pour WSS)

Pour une sécurité maximale, vous devrez :
1. Configurer un certificat SSL/TLS
2. Modifier le serveur pour utiliser QWebSocketServer::SecureMode
3. Utiliser wss:// au lieu de ws:// dans le client

## Hébergeurs recommandés

- **DigitalOcean** : 4-6€/mois (Simple, documentation excellente)
- **Vultr** : 5€/mois (Similaire à DigitalOcean)
- **OVH** : 3-5€/mois (Européen, RGPD-compliant)
- **AWS EC2** : Free tier 1 an, puis ~10€/mois

## Test rapide depuis votre machine

Pour tester avant de déployer :

```bash
# Compiler
qmake server.pro
make

# Lancer
./coinche_server

# Le serveur devrait afficher :
# Serveur demarre sur le port 1234
```
