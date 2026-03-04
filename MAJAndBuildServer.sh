#!/bin/bash

if [ "$1" == "test" ]; then
    echo "==> Déploiement serveur TEST (v0.2.0)"
    cd ~/TestCoinche/Coinche
    git pull origin v0.2.0   # Nom de branche a changer!
    mkdir -p build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make server -j$(nproc)
    sudo systemctl restart test-coinche
else
    echo "==> Déploiement serveur PROD (main)"
    #cd ~/Coinche ne pas aller automatiquement dans le dossier de l'application, pour eviter de faire l'erreur de couper le server de prod en testant le script
    git pull origin main
    mkdir -p build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make server -j$(nproc)
    sudo systemctl restart coinche-server
fi
