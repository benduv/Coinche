pragma Singleton
import QtQuick
import QtCore

QtObject {
    id: audioSettings

    // Signaux pour notifier les changements (plus fiable sur Android)
    signal musicEnabledChanged()
    signal effectsEnabledChanged()

    // État des sons - valeurs locales
    property bool musicEnabled: true
    property bool effectsEnabled: true

    // QSettings pour persister les préférences audio
    property Settings storage: Settings {
        id: persistentStorage
        category: "Audio"

        // Ces propriétés sont automatiquement persistées par Qt
        property bool storedMusicEnabled: true
        property bool storedEffectsEnabled: true
    }

    // Charger les préférences au démarrage
    Component.onCompleted: {
        console.log("AudioSettings - Chargement des préférences...")
        console.log("AudioSettings - Valeurs stockées: Musique:", persistentStorage.storedMusicEnabled, "Effets:", persistentStorage.storedEffectsEnabled)

        // Charger les valeurs depuis le stockage
        audioSettings.musicEnabled = persistentStorage.storedMusicEnabled
        audioSettings.effectsEnabled = persistentStorage.storedEffectsEnabled

        console.log("AudioSettings - Valeurs chargées: Musique:", audioSettings.musicEnabled, "Effets:", audioSettings.effectsEnabled)
    }

    // Fonction pour sauvegarder les paramètres
    function saveMusicEnabled(enabled) {
        console.log("AudioSettings.saveMusicEnabled appelé avec:", enabled)
        persistentStorage.storedMusicEnabled = enabled
        audioSettings.musicEnabled = enabled
        audioSettings.musicEnabledChanged()
        console.log("Musique:", enabled ? "activee" : "désactivee", "(sauvegardé)")
    }

    function saveEffectsEnabled(enabled) {
        console.log("AudioSettings.saveEffectsEnabled appelé avec:", enabled)
        persistentStorage.storedEffectsEnabled = enabled
        audioSettings.effectsEnabled = enabled
        audioSettings.effectsEnabledChanged()
        console.log("Effets sonores:", enabled ? "actives" : "désactives", "(sauvegardé)")
    }
}
