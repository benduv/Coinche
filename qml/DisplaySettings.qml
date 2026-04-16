pragma Singleton
import QtQuick
import QtCore

QtObject {
    id: displaySettings

    // Tri des cartes : true = fortes à gauche, false = fortes à droite (défaut)
    property bool strongCardsLeft: false

    // Sens de jeu : true = antihoraire (défaut), false = horaire
    property bool antiClockwisePlay: true

    // QSettings pour persister les préférences d'affichage
    property Settings storage: Settings {
        id: persistentStorage
        category: "Display"

        property bool storedStrongCardsLeft: false
        property bool storedAntiClockwisePlay: true
    }

    // Charger les préférences au démarrage
    Component.onCompleted: {
        displaySettings.strongCardsLeft = persistentStorage.storedStrongCardsLeft
        displaySettings.antiClockwisePlay = persistentStorage.storedAntiClockwisePlay
    }

    function saveStrongCardsLeft(enabled) {
        persistentStorage.storedStrongCardsLeft = enabled
        displaySettings.strongCardsLeft = enabled
    }

    function saveAntiClockwisePlay(enabled) {
        persistentStorage.storedAntiClockwisePlay = enabled
        displaySettings.antiClockwisePlay = enabled
    }
}
