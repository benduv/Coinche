import QtQuick

QtObject {
    id: config

    // Configuration du serveur
    // Changez cette valeur selon votre environnement :
    // - "localhost" : Développement local (Windows/Mac/Linux)
    // - "emulator" : Émulateur Android
    // - "production" : Serveur de production
    property string environment: "localhost"

    // URLs pour chaque environnement
    readonly property var serverUrls: ({
        "localhost": "ws://localhost:1234",
        "emulator": "ws://10.0.2.2:1234",
        "production": "ws://VOTRE_IP_SERVEUR:1234"  // À MODIFIER avec votre IP ou domaine
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
