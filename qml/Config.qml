import QtQuick

QtObject {
    id: config

    // Configuration du serveur
    // Changez cette valeur selon votre environnement :
    // - "localhost" : Développement local (Windows/Mac/Linux)
    // - "emulator" : Émulateur Android
    // - "local-network" : Tests sur réseau local (PC hotspot)
    // - "remote" : Serveur VPS distant en production
    property string environment: "localhost"

    // URLs pour chaque environnement
    readonly property var serverUrls: ({
        "localhost": "ws://localhost:1234",
        "emulator": "ws://10.0.2.2:1234",
        "local-network": "ws://172.20.10.13:1234",  // IP du PC sur le réseau local
        "remote": "ws://VOTRE-IP-VPS:1234"  // À remplacer par l'IP de votre VPS
    })

    // URL active selon l'environnement
    readonly property string serverUrl: serverUrls[environment]

    // Fonction pour obtenir l'URL du serveur
    function getServerUrl() {
        console.log("Environnement:", environment)
        console.log("URL du serveur:", serverUrl)
        return serverUrl
    }
}
