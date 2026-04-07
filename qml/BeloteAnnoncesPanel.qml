import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

Rectangle {
    id: root
    anchors.fill: parent
    color: "#000000"
    opacity: 0.92
    radius: 10
    border.color: "#FFD700"
    border.width: parent ? parent.width * 0.002 : 2

    property real w: width
    property real h: height

    // Visible pendant la phase d'enchères Belote côté joueur local
    visible: gameModel.isBeloteMode &&
             gameModel.biddingPhase &&
             gameModel.distributionPhase === 0 &&
             gameModel.biddingPlayer === gameModel.myPosition

    // Timer de 20 secondes
    property int timeRemaining: 20
    property int maxTime: 20
    property bool alertSoundPlayed: false

    MediaPlayer {
        id: alertSound
        source: "qrc:/resources/sons/342299__kruud__clock-128-bpm-half-speed.wav"
        audioOutput: AudioOutput {}
    }

    Timer {
        id: bidTimer
        interval: 1000
        repeat: true
        running: root.visible
        onTriggered: {
            if (timeRemaining > 0) {
                timeRemaining--
                if (timeRemaining === 4 && !root.alertSoundPlayed) {
                    root.alertSoundPlayed = true
                    if (AudioSettings.effectsEnabled && Qt.application.state === Qt.ApplicationActive) {
                        alertSound.stop()
                        alertSound.play()
                    }
                }
            } else {
                // Temps écoulé : passer automatiquement
                bidTimer.stop()
                gameModel.passBeloteBid()
            }
        }
    }

    onVisibleChanged: {
        if (visible) {
            timeRemaining = maxTime
            alertSoundPlayed = false
            bidTimer.restart()
        } else {
            bidTimer.stop()
            alertSound.stop()
        }
    }

    // Suit data : COEUR=3, TREFLE=4, CARREAU=5, PIQUE=6
    readonly property var suits: [
        { value: 3, icon: "qrc:/resources/heart-svgrepo-com.svg",   color: "#E60000" },
        { value: 4, icon: "qrc:/resources/clover-svgrepo-com.svg",  color: "#000000" },
        { value: 5, icon: "qrc:/resources/diamond-svgrepo-com.svg", color: "#E60000" },
        { value: 6, icon: "qrc:/resources/spade-svgrepo-com.svg",   color: "#000000" }
    ]

    // Couleurs disponibles au tour 2 (toutes sauf la retournée)
    property var availableSuits: {
        var result = []
        for (var i = 0; i < suits.length; i++) {
            if (suits[i].value !== gameModel.retourneeSuit) {
                result.push(suits[i])
            }
        }
        return result
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: w * 0.04
        spacing: h * 0.03

        // ========= Titre =========
        Text {
            text: gameModel.beloteBidRound === 1 ? "Voulez-vous prendre ?" : "Choisissez votre couleur"
            font.pixelSize: h * 0.08
            font.bold: true
            color: "#FFD700"
            Layout.alignment: Qt.AlignHCenter
        }

        // ========= Jauge de temps =========
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: h * 0.04
            color: "#333333"
            radius: height / 2
            border.color: "#FFD700"
            border.width: 2

            Rectangle {
                width: parent.width * (timeRemaining / maxTime)
                height: parent.height
                radius: parent.radius
                color: {
                    if (timeRemaining <= 5) return "#ff3333"
                    if (timeRemaining <= 10) return "#ffaa00"
                    return "#00cc00"
                }
                Behavior on width { NumberAnimation { duration: 300 } }
                Behavior on color { ColorAnimation { duration: 300 } }
            }
        }

        // ========= Tour 1 : Prendre / Passer =========
        RowLayout {
            visible: gameModel.beloteBidRound === 1
            Layout.alignment: Qt.AlignHCenter
            spacing: w * 0.06

            // Bouton Prendre (retournée)
            AppButton {
                Layout.preferredWidth: w * 0.35
                Layout.preferredHeight: h * 0.22

                background: Rectangle {
                    radius: 10
                    color: parent.down ? "#008800" : (parent.hovered ? "#00bb00" : "#006600")
                    border.color: "#FFD700"
                    border.width: 3
                }

                contentItem: ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 4

                    Image {
                        source: {
                            for (var i = 0; i < root.suits.length; i++) {
                                if (root.suits[i].value === gameModel.retourneeSuit)
                                    return root.suits[i].icon
                            }
                            return ""
                        }
                        width: h * 0.08
                        height: width
                        Layout.alignment: Qt.AlignHCenter
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        text: "Prendre"
                        font.pixelSize: h * 0.07
                        font.bold: true
                        color: "white"
                        Layout.alignment: Qt.AlignHCenter
                    }
                }

                onClicked: {
                    bidTimer.stop()
                    gameModel.prendreBid(gameModel.retourneeSuit)
                }
            }

            // Bouton Passer
            AppButton {
                Layout.preferredWidth: w * 0.25
                Layout.preferredHeight: h * 0.22

                background: Rectangle {
                    radius: 10
                    color: parent.down ? "#555555" : (parent.hovered ? "#777777" : "#444444")
                    border.color: "#888888"
                    border.width: 2
                }

                contentItem: Text {
                    text: "Passer"
                    font.pixelSize: h * 0.07
                    font.bold: true
                    color: "#cccccc"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    bidTimer.stop()
                    gameModel.passBeloteBid()
                }
            }
        }

        // ========= Tour 2 : 3 couleurs + Passer =========
        ColumnLayout {
            visible: gameModel.beloteBidRound === 2
            Layout.alignment: Qt.AlignHCenter
            spacing: h * 0.02

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: w * 0.04

                Repeater {
                    model: root.availableSuits

                    AppButton {
                        Layout.preferredWidth: w * 0.22
                        Layout.preferredHeight: h * 0.22

                        background: Rectangle {
                            radius: 10
                            color: parent.down ? Qt.darker(modelData.color === "#000000" ? "#555555" : modelData.color, 1.3)
                                              : (parent.hovered ? Qt.lighter(modelData.color === "#000000" ? "#555555" : modelData.color, 1.5)
                                                                : (modelData.color === "#000000" ? "#333333" : Qt.darker(modelData.color, 1.2)))
                            border.color: modelData.color === "#000000" ? "#888888" : modelData.color
                            border.width: 3
                        }

                        contentItem: ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 2

                            Image {
                                source: modelData.icon
                                width: h * 0.08
                                height: width
                                Layout.alignment: Qt.AlignHCenter
                                fillMode: Image.PreserveAspectFit
                            }
                        }

                        onClicked: {
                            bidTimer.stop()
                            gameModel.prendreBid(modelData.value)
                        }
                    }
                }
            }

            // Bouton Passer (tour 2)
            AppButton {
                Layout.preferredWidth: w * 0.25
                Layout.preferredHeight: h * 0.15
                Layout.alignment: Qt.AlignHCenter

                background: Rectangle {
                    radius: 10
                    color: parent.down ? "#555555" : (parent.hovered ? "#777777" : "#444444")
                    border.color: "#888888"
                    border.width: 2
                }

                contentItem: Text {
                    text: "Passer"
                    font.pixelSize: h * 0.07
                    font.bold: true
                    color: "#cccccc"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    bidTimer.stop()
                    gameModel.passBeloteBid()
                }
            }
        }
    }
}
