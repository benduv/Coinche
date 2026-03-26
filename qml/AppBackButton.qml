import QtQuick
import QtQuick.Controls
import QtMultimedia

Button {
    SoundEffect {
        id: wooshSound
        source: "qrc:/resources/sons/742832__sadiquecat__woosh-metal-tea-strainer-1.wav"
    }

    onPressed: {
        if (AudioSettings.effectsEnabled && Qt.application.state === Qt.ApplicationActive) {
            wooshSound.play()
        }
    }
}
