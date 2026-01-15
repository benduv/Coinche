pragma Singleton
import QtQuick
import QtCore

QtObject {
    id: audioSettings

    // QSettings pour persister les préférences audio
    // Les propriétés déclarées ici sont automatiquement sauvegardées/restaurées
    property Settings storage: Settings {
        id: persistentStorage
        category: "Audio"

        // Ces propriétés sont automatiquement persistées par Qt
        property bool musicEnabled: true
        property bool effectsEnabled: true
    }

    // État des sons - liés aux valeurs persistées
    property bool musicEnabled: persistentStorage.musicEnabled
    property bool effectsEnabled: persistentStorage.effectsEnabled

    // Charger les préférences au démarrage
    Component.onCompleted: {
        console.log("AudioSettings chargé - Musique:", persistentStorage.musicEnabled, "Effets:", persistentStorage.effectsEnabled)
    }

    // Fonction pour sauvegarder les paramètres
    function saveMusicEnabled(enabled) {
        persistentStorage.musicEnabled = enabled
        audioSettings.musicEnabled = enabled
        console.log("Musique:", enabled ? "activee" : "désactivee", "(sauvegardé)")
    }

    function saveEffectsEnabled(enabled) {
        persistentStorage.effectsEnabled = enabled
        audioSettings.effectsEnabled = enabled
        console.log("Effets sonores:", enabled ? "actives" : "désactives", "(sauvegardé)")
    }
}
