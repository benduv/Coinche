# Checklist - Déploiement serveur de production

## ✅ Fichiers créés

- [x] `server/server.pro` - Fichier de compilation du serveur seul
- [x] `server/deploy/README_DEPLOY.md` - Documentation complète
- [x] `server/deploy/GUIDE_RAPIDE.md` - Guide pas-à-pas
- [x] `server/deploy/deploy.sh` - Script de déploiement automatique
- [x] `qml/Config.qml` - Configuration centralisée des URLs
- [x] Modification de `MainMenu.qml` pour utiliser Config.qml

## 📝 À faire AVANT le déploiement

### 1. Créer un compte hébergeur
- [ ] Choisir un hébergeur (DigitalOcean recommandé)
- [ ] Créer un compte
- [ ] Créer un serveur (Droplet/VPS) Ubuntu 22.04
- [ ] Noter l'adresse IP du serveur : `___________________`

### 2. Préparer la connexion SSH
- [ ] Générer une clé SSH (si pas déjà fait)
  ```bash
  ssh-keygen -t rsa -b 4096
  ```
- [ ] Ajouter la clé SSH au serveur
- [ ] Tester la connexion :
  ```bash
  ssh root@VOTRE_IP
  ```

### 3. Configurer le serveur
- [ ] Se connecter au serveur
- [ ] Installer les dépendances :
  ```bash
  sudo apt update && sudo apt upgrade -y
  sudo apt install -y build-essential qt6-base-dev qt6-websockets-dev libqt6sql6-sqlite
  ```
- [ ] Vérifier Qt : `qmake6 --version`

### 4. Déployer le code
- [ ] Option A : Utiliser le script `deploy.sh`
- [ ] Option B : Copier manuellement les fichiers
- [ ] Compiler sur le serveur : `qmake6 server.pro && make`
- [ ] Tester le serveur : `./coinche_server`

### 5. Configurer le pare-feu
- [ ] Ouvrir le port 1234 :
  ```bash
  sudo ufw allow 1234/tcp
  sudo ufw allow 22/tcp
  sudo ufw enable
  ```
- [ ] Vérifier : `sudo ufw status`

### 6. Tester la connexion réseau
- [ ] Depuis votre PC, tester :
  ```bash
  Test-NetConnection VOTRE_IP -Port 1234  # Windows
  nc -zv VOTRE_IP 1234                    # Linux/Mac
  ```

### 7. Configurer le client
- [ ] Ouvrir `qml/Config.qml`
- [ ] Ligne 11 : Changer `environment: "production"`
- [ ] Ligne 16 : Remplacer `VOTRE_IP_SERVEUR` par l'IP réelle
  ```qml
  "production": "ws://192.168.1.100:1234"  // Exemple
  ```
- [ ] Recompiler l'application client

### 8. Installer le service systemd (optionnel mais recommandé)
- [ ] Créer le fichier `/etc/systemd/system/coinche-server.service`
- [ ] Copier le contenu du template (voir README_DEPLOY.md)
- [ ] Activer le service :
  ```bash
  sudo systemctl daemon-reload
  sudo systemctl enable coinche-server
  sudo systemctl start coinche-server
  ```

### 9. Tests finaux
- [ ] Lancer le client sur votre PC
- [ ] Vérifier la connexion au serveur
- [ ] Créer un compte de test
- [ ] Lancer une partie de test
- [ ] Demander à un ami de tester depuis un autre réseau

### 10. Monitoring et logs
- [ ] Vérifier les logs : `tail -f /root/coinche-server/server_log.txt`
- [ ] Vérifier que le serveur redémarre bien : `sudo systemctl status coinche-server`
- [ ] Configurer des alertes (optionnel)

## 🎯 Configuration pour différents scénarios

### Développement local (Windows)
```qml
// Config.qml
property string environment: "localhost"
```

### Test sur émulateur Android
```qml
// Config.qml
property string environment: "emulator"
```

### Production (amis qui testent)
```qml
// Config.qml
property string environment: "production"
"production": "ws://VOTRE_IP:1234"
```

## 🔧 Commandes de maintenance

### Voir les logs en temps réel
```bash
ssh root@VOTRE_IP 'tail -f /root/coinche-server/server_log.txt'
```

### Redémarrer le serveur
```bash
ssh root@VOTRE_IP 'sudo systemctl restart coinche-server'
```

### Arrêter le serveur
```bash
ssh root@VOTRE_IP 'sudo systemctl stop coinche-server'
```

### Mettre à jour le serveur
```bash
# Sur votre PC
cd c:/Users/33672/projects/Coinche/server
./deploy/deploy.sh root@VOTRE_IP

# Puis redémarrer
ssh root@VOTRE_IP 'sudo systemctl restart coinche-server'
```

### Sauvegarder la base de données
```bash
scp root@VOTRE_IP:/root/coinche-server/coinche.db ./coinche_backup_$(date +%Y%m%d).db
```

## 📊 Vérifications après déploiement

- [ ] Le serveur démarre sans erreur
- [ ] Le port 1234 est accessible depuis l'extérieur
- [ ] Les clients peuvent se connecter
- [ ] La création de compte fonctionne
- [ ] Le matchmaking fonctionne
- [ ] Une partie peut être jouée du début à la fin
- [ ] Les statistiques sont sauvegardées
- [ ] La reconnexion fonctionne
- [ ] Les lobbies privés fonctionnent

## 💰 Budget estimé

- Serveur DigitalOcean Basic : 4-6€/mois
- (Optionnel) Nom de domaine : 10-15€/an
- (Optionnel) Monitoring : 0€ (Grafana gratuit)

**Total minimal : ~5€/mois**

## 📞 En cas de problème

1. Vérifier les logs du serveur
2. Vérifier la connexion réseau (ping, nc)
3. Vérifier le pare-feu
4. Vérifier que le service est démarré
5. Redémarrer le service
6. Recompiler si nécessaire

## 🚀 Prochaines améliorations

- [ ] Configurer SSL/TLS pour WSS
- [ ] Ajouter un nom de domaine
- [ ] Mettre en place des backups automatiques
- [ ] Ajouter du monitoring (Grafana)
- [ ] Configurer des alertes email
- [ ] Optimiser les performances
- [ ] Ajouter un load balancer (si beaucoup d'utilisateurs)
