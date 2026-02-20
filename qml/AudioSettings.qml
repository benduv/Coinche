pragma Singleton
import QtQuick
import QtCore

QtObject {
    id: audioSettings

    // État des sons - valeurs locales
    // Le signal onMusicEnabledChanged est généré automatiquement par QML
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
        // Charger les valeurs depuis le stockage
        audioSettings.musicEnabled = persistentStorage.storedMusicEnabled
        audioSettings.effectsEnabled = persistentStorage.storedEffectsEnabled
    }

    // Fonction pour sauvegarder les paramètres
    function saveMusicEnabled(enabled) {
        persistentStorage.storedMusicEnabled = enabled
        audioSettings.musicEnabled = enabled
    }

    function saveEffectsEnabled(enabled) {
        persistentStorage.storedEffectsEnabled = enabled
        audioSettings.effectsEnabled = enabled
    }
}
