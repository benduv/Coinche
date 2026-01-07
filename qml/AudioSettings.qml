pragma Singleton
import QtQuick

QtObject {
    id: audioSettings

    // État des sons
    property bool musicEnabled: true
    property bool effectsEnabled: true

    // Fonction pour sauvegarder les paramètres
    function saveMusicEnabled(enabled) {
        musicEnabled = enabled
        console.log("Musique:", enabled ? "activee" : "désactivee")
    }

    function saveEffectsEnabled(enabled) {
        effectsEnabled = enabled
        console.log("Effets sonores:", enabled ? "actives" : "désactives")
    }
}
