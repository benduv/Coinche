import QtQuick

QtObject {
    id: config

    // Configuration du serveur
    // Changez cette valeur selon votre environnement :
    // - "localhost" : Développement local (Windows/Mac/Linux) - WS non sécurisé
    // - "emulator" : Émulateur Android - WS non sécurisé
    // - "local-network" : Tests sur réseau local (PC hotspot) - WS non sécurisé
    // - "remote" : Serveur VPS distant en production - WSS sécurisé (SSL/TLS)
    property string environment: "remote"  // Configuré pour le VPS distant

    // URLs pour chaque environnement
    // Note: Les environnements de développement utilisent WS (non sécurisé)
    //       La production utilise WSS (sécurisé avec SSL/TLS)
    readonly property var serverUrls: ({
        "localhost": "ws://localhost:1234",
        "emulator": "ws://10.0.2.2:1234",
        "local-network": "ws://172.20.10.13:1234",  // IP du PC sur le réseau local
        "remote": "wss://game.nebuludik.fr"})  // VPS OVH via nginx reverse proxy (port 443 par défaut)    

    // URL active selon l'environnement
    readonly property string serverUrl: serverUrls[environment]

    // Fonction pour obtenir l'URL du serveur
    function getServerUrl() {
        console.log("Environnement:", environment)
        console.log("URL du serveur:", serverUrl)
        return serverUrl
    }
}
