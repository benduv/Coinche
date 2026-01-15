pragma Singleton
import QtQuick
import QtCore

QtObject {
    id: audioSettings

    // QSettings pour persister les préférences audio
    property Settings settings: Settings {
        category: "Audio"
        property bool musicEnabled: true
        property bool effectsEnabled: true
    }

    // État des sons (liés aux settings persistants)
    property bool musicEnabled: settings.musicEnabled
    property bool effectsEnabled: settings.effectsEnabled

    // Fonction pour sauvegarder les paramètres (persiste automatiquement)
    function saveMusicEnabled(enabled) {
        settings.musicEnabled = enabled
        musicEnabled = enabled
        console.log("Musique:", enabled ? "activee" : "désactivee", "(sauvegardé)")
    }

    function saveEffectsEnabled(enabled) {
        settings.effectsEnabled = enabled
        effectsEnabled = enabled
        console.log("Effets sonores:", enabled ? "actives" : "désactives", "(sauvegardé)")
    }

    Component.onCompleted: {
        // Charger les préférences sauvegardées au démarrage
        musicEnabled = settings.musicEnabled
        effectsEnabled = settings.effectsEnabled
        console.log("Préférences audio chargées - Musique:", musicEnabled, "Effets:", effectsEnabled)
    }
}
