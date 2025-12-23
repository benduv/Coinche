import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: settingsRoot
    anchors.fill: parent
    color: "#1a1a1a"

    // Ratio responsive
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    signal backToMenu()

    // Synchroniser avec les paramètres globaux
    Component.onCompleted: {
        musicEnabled = AudioSettings.musicEnabled
        effectsEnabled = AudioSettings.effectsEnabled
    }

    // Propriétés pour l'état des sons
    property bool musicEnabled: AudioSettings.musicEnabled
    property bool effectsEnabled: AudioSettings.effectsEnabled

    // Watcher pour mettre à jour les paramètres globaux
    onMusicEnabledChanged: {
        AudioSettings.saveMusicEnabled(musicEnabled)
    }

    onEffectsEnabledChanged: {
        AudioSettings.saveEffectsEnabled(effectsEnabled)
    }

    // Bouton retour
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 40 * minRatio
        width: 100 * minRatio
        height: 100 * minRatio
        color: "transparent"
        z: 100

        Image {
            anchors.fill: parent
            source: "qrc:/resources/back-square-svgrepo-com.svg"
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                settingsRoot.backToMenu()
            }
        }
    }

    // Titre
    Text {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 50 * minRatio
        text: "RÉGLAGES"
        font.pixelSize: 60 * minRatio
        font.bold: true
        color: "#FFD700"
    }

    // Contenu des réglages
    ScrollView {
        anchors.centerIn: parent
        width: parent.width * 0.8
        height: parent.height * 0.7
        clip: true

        Column {
            width: parent.width
            spacing: 30 * settingsRoot.minRatio

            // Section Audio
            Rectangle {
                width: parent.width
                height: audioColumn.height + 40 * settingsRoot.minRatio
                color: "#2a2a2a"
                radius: 10 * settingsRoot.minRatio
                border.color: "#FFD700"
                border.width: 2 * settingsRoot.minRatio

                Column {
                    id: audioColumn
                    anchors.centerIn: parent
                    width: parent.width - 40 * settingsRoot.minRatio
                    spacing: 20 * settingsRoot.minRatio

                    Text {
                        text: "🔊 AUDIO"
                        font.pixelSize: 40 * settingsRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
                    }

                    // Musique ON/OFF
                    Row {
                        width: parent.width
                        spacing: 20 * settingsRoot.minRatio

                        Text {
                            text: "Musique:"
                            font.pixelSize: 30 * settingsRoot.minRatio
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width * 0.5
                        }

                        Button {
                            id: musicToggleButton
                            width: 150 * settingsRoot.minRatio
                            height: 60 * settingsRoot.minRatio
                            anchors.verticalCenter: parent.verticalCenter

                            background: Rectangle {
                                color: settingsRoot.musicEnabled ? "#00aa00" : "#aa0000"
                                radius: 10 * settingsRoot.minRatio
                                border.color: "#FFD700"
                                border.width: 2 * settingsRoot.minRatio
                            }

                            contentItem: Text {
                                text: settingsRoot.musicEnabled ? "ON" : "OFF"
                                font.pixelSize: 30 * settingsRoot.minRatio
                                font.bold: true
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                settingsRoot.musicEnabled = !settingsRoot.musicEnabled
                            }
                        }
                    }

                    // Effets sonores ON/OFF
                    Row {
                        width: parent.width
                        spacing: 20 * settingsRoot.minRatio

                        Text {
                            text: "Effets sonores:"
                            font.pixelSize: 30 * settingsRoot.minRatio
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width * 0.5
                        }

                        Button {
                            id: effectsToggleButton
                            width: 150 * settingsRoot.minRatio
                            height: 60 * settingsRoot.minRatio
                            anchors.verticalCenter: parent.verticalCenter

                            background: Rectangle {
                                color: settingsRoot.effectsEnabled ? "#00aa00" : "#aa0000"
                                radius: 10 * settingsRoot.minRatio
                                border.color: "#FFD700"
                                border.width: 2 * settingsRoot.minRatio
                            }

                            contentItem: Text {
                                text: settingsRoot.effectsEnabled ? "ON" : "OFF"
                                font.pixelSize: 30 * settingsRoot.minRatio
                                font.bold: true
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                settingsRoot.effectsEnabled = !settingsRoot.effectsEnabled
                            }
                        }
                    }
                }
            }

            // Section Affichage (pour futures options)
            Rectangle {
                width: parent.width
                height: displayColumn.height + 40 * settingsRoot.minRatio
                color: "#2a2a2a"
                radius: 10 * settingsRoot.minRatio
                border.color: "#FFD700"
                border.width: 2 * settingsRoot.minRatio

                Column {
                    id: displayColumn
                    anchors.centerIn: parent
                    width: parent.width - 40 * settingsRoot.minRatio
                    spacing: 20 * settingsRoot.minRatio

                    Text {
                        text: "🎨 AFFICHAGE"
                        font.pixelSize: 40 * settingsRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
                    }

                    Text {
                        text: "(Options d'affichage à venir...)"
                        font.pixelSize: 25 * settingsRoot.minRatio
                        color: "#888888"
                        font.italic: true
                    }
                }
            }
        }
    }
}
