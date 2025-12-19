#!/bin/bash

# Script de déploiement du serveur Coinche
# Usage: ./deploy.sh <user@serveur>

set -e  # Arrêter en cas d'erreur

# Vérifier les arguments
if [ $# -eq 0 ]; then
    echo "Usage: ./deploy.sh user@serveur.com"
    echo "Exemple: ./deploy.sh ubuntu@192.168.1.100"
    exit 1
fi

SERVER=$1
REMOTE_DIR="/home/$(echo $SERVER | cut -d'@' -f1)/coinche-server"

echo "=========================================="
echo "Déploiement du serveur Coinche"
echo "Serveur cible: $SERVER"
echo "Répertoire distant: $REMOTE_DIR"
echo "=========================================="

# 1. Créer le répertoire sur le serveur
echo "Création du répertoire sur le serveur..."
ssh $SERVER "mkdir -p $REMOTE_DIR/server"

# 2. Copier les fichiers serveur
echo "Copie des fichiers serveur..."
scp server_main.cpp $SERVER:$REMOTE_DIR/server/
scp GameServer.h $SERVER:$REMOTE_DIR/server/
scp DatabaseManager.h $SERVER:$REMOTE_DIR/server/
scp DatabaseManager.cpp $SERVER:$REMOTE_DIR/server/
scp server.pro $SERVER:$REMOTE_DIR/

# 3. Copier les classes partagées
echo "Copie des classes partagées..."
scp ../Player.h ../Player.cpp $SERVER:$REMOTE_DIR/
scp ../Deck.h ../Deck.cpp $SERVER:$REMOTE_DIR/
scp ../Carte.h ../Carte.cpp $SERVER:$REMOTE_DIR/
scp ../GameModel.h ../GameModel.cpp $SERVER:$REMOTE_DIR/

# 4. Copier la base de données
echo "Copie de la base de données..."
scp ../coinche.db $SERVER:$REMOTE_DIR/

# 5. Compiler sur le serveur
echo "Compilation sur le serveur..."
ssh $SERVER << 'EOF'
cd ~/coinche-server
qmake6 server.pro
make -j$(nproc)
EOF

# 6. Créer le service systemd (optionnel)
echo "Voulez-vous installer le service systemd ? (y/n)"
read -r response
if [[ "$response" =~ ^([yY][eE][sS]|[yY])$ ]]; then
    echo "Installation du service systemd..."

    # Créer le fichier service localement
    cat > /tmp/coinche-server.service << EOF
[Unit]
Description=Coinche Game Server
After=network.target

[Service]
Type=simple
User=$(echo $SERVER | cut -d'@' -f1)
WorkingDirectory=$REMOTE_DIR
ExecStart=$REMOTE_DIR/coinche_server
Restart=always
RestartSec=10

StandardOutput=append:$REMOTE_DIR/server_log.txt
StandardError=append:$REMOTE_DIR/server_error.txt

[Install]
WantedBy=multi-user.target
EOF

    # Copier et installer le service
    scp /tmp/coinche-server.service $SERVER:/tmp/
    ssh $SERVER << EOF
sudo mv /tmp/coinche-server.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable coinche-server
sudo systemctl start coinche-server
sudo systemctl status coinche-server
EOF
    rm /tmp/coinche-server.service
fi

echo "=========================================="
echo "Déploiement terminé !"
echo "Pour voir les logs:"
echo "  ssh $SERVER 'tail -f $REMOTE_DIR/server_log.txt'"
echo "Pour redémarrer le serveur:"
echo "  ssh $SERVER 'sudo systemctl restart coinche-server'"
echo "=========================================="
