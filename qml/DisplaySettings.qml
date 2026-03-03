pragma Singleton
import QtQuick
import QtCore

QtObject {
    id: displaySettings

    // Tri des cartes : true = fortes à gauche, false = fortes à droite (défaut)
    property bool strongCardsLeft: false

    // QSettings pour persister les préférences d'affichage
    property Settings storage: Settings {
        id: persistentStorage
        category: "Display"

        property bool storedStrongCardsLeft: false
    }

    // Charger les préférences au démarrage
    Component.onCompleted: {
        displaySettings.strongCardsLeft = persistentStorage.storedStrongCardsLeft
    }

    function saveStrongCardsLeft(enabled) {
        persistentStorage.storedStrongCardsLeft = enabled
        displaySettings.strongCardsLeft = enabled
    }
}
