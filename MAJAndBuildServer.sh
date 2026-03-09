#!/bin/bash

if [ "$1" == "test" ]; then
    echo "==> Déploiement serveur TEST"
    cd ~/TestCoinche/Coinche
    git pull origin addVersionCheck   # Nom de branche a changer!
    mkdir -p build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make server -j$(nproc)
    sudo systemctl restart test-coinche
else
    if [ "$(pwd)" != "/home/coinche/Coinche" ]; then
        echo "Vous devez etre dans /home/coinche/Coinche pour mettre a jour le server de prod, ou bien lancer avec arg test"
        exit 1
    fi
    echo "==> Déploiement serveur PROD (main)"
    git pull origin main
    mkdir -p build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make server -j$(nproc)
    sudo systemctl restart coinche-server
fi
