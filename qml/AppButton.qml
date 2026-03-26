import QtQuick
import QtQuick.Controls
import QtMultimedia

Button {
    SoundEffect {
        id: clickSound
        source: "qrc:/resources/sons/146721__leszek_szary__menu-click.wav"
    }

    onPressed: {
        if (AudioSettings.effectsEnabled && Qt.application.state === Qt.ApplicationActive) {
            clickSound.play()
        }
    }
}
