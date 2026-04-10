import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

// Panneau d'annonces Belote unifié (tours 1 et 2).
Rectangle {
    id: root
    color: "#000000"
    opacity: 0.92
    radius: 10
    border.color: "#FFD700"
    border.width: parent ? parent.width * 0.002 : 2

    // Dimensions relatives (comme AnnoncesPanel.qml)
    property real w: width
    property real h: height

    // Timer d'annonce
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
            if (root.timeRemaining > 0) {
                root.timeRemaining--
                if (root.timeRemaining === 4 && !root.alertSoundPlayed) {
                    root.alertSoundPlayed = true
                    if (AudioSettings.effectsEnabled && Qt.application.state === Qt.ApplicationActive) {
                        alertSound.stop()
                        alertSound.play()
                    }
                }
            } else {
                stop()
            }
        }
    }

    Connections {
        target: gameModel
        function onBiddingPlayerChanged() {
            root.timeRemaining = root.maxTime
            root.alertSoundPlayed = false
            if (root.visible) bidTimer.restart()
        }
        function onBeloteBidRoundChanged() {
            root.timeRemaining = root.maxTime
            root.alertSoundPlayed = false
            if (root.visible) bidTimer.restart()
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

    // Suits : COEUR=3, TREFLE=4, CARREAU=5, PIQUE=6
    readonly property var suits: [
        { value: 3, icon: "qrc:/resources/heart-svgrepo-com.svg",   color: "#E60000" },
        { value: 4, icon: "qrc:/resources/clover-svgrepo-com.svg",  color: "#cccccc" },
        { value: 5, icon: "qrc:/resources/diamond-svgrepo-com.svg", color: "#E60000" },
        { value: 6, icon: "qrc:/resources/spade-svgrepo-com.svg",   color: "#cccccc" }
    ]

    property var availableSuits: {
        var result = []
        for (var i = 0; i < suits.length; i++) {
            if (suits[i].value !== gameModel.retourneeSuit)
                result.push(suits[i])
        }
        // Alterner rouge/noir/rouge ou noir/rouge/noir
        // Retournée rouge (Coeur=3, Carreau=5) → il reste 2 noirs + 1 rouge → noir/rouge/noir
        // Retournée noire (Trèfle=4, Pique=6) → il reste 2 rouges + 1 noir → rouge/noir/rouge
        if (result.length === 3) {
            var reds = []
            var blacks = []
            for (var j = 0; j < result.length; j++) {
                if (result[j].color === "#E60000")
                    reds.push(result[j])
                else
                    blacks.push(result[j])
            }
            if (reds.length === 2 && blacks.length === 1)
                result = [reds[0], blacks[0], reds[1]]
            else if (blacks.length === 2 && reds.length === 1)
                result = [blacks[0], reds[0], blacks[1]]
        }
        return result
    }

    property bool isMyTurn: gameModel.biddingPlayer === gameModel.myPosition
    property bool isRound1: gameModel.beloteBidRound === 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: w * 0.04
        spacing: h * 0.025

        // ========= Titre =========
        Row {
            Layout.alignment: Qt.AlignHCenter
            spacing: 0

            Text {
                text: root.isMyTurn
                      ? "A vous d'annoncez !"
                      : (gameModel.getPlayerName(gameModel.biddingPlayer) + " annonce ")
                font.pixelSize: h * 0.10
                font.bold: true
                color: "#FFD700"
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                visible: !root.isMyTurn
                text: dotsAnim.dots
                font.pixelSize: h * 0.10
                font.bold: true
                color: "#FFD700"
                verticalAlignment: Text.AlignVCenter
                width: h * 0.25  // largeur fixe pour 3 points
            }

            QtObject {
                id: dotsAnim
                property string dots: ""
            }

            Timer {
                running: !root.isMyTurn && root.visible
                repeat: true
                interval: 500
                onTriggered: {
                    if (dotsAnim.dots === "") dotsAnim.dots = "."
                    else if (dotsAnim.dots === ".") dotsAnim.dots = ".."
                    else if (dotsAnim.dots === "..") dotsAnim.dots = "..."
                    else dotsAnim.dots = ""
                }
            }
        }

        // ========= Jauge de temps =========
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: h * 0.05
            color: "#333333"
            radius: height / 2
            border.color: "#FFD700"
            border.width: 2

            Rectangle {
                width: parent.width * (root.timeRemaining / root.maxTime)
                height: parent.height
                radius: parent.radius
                color: {
                    if (root.timeRemaining <= 5) return "#ff3333"
                    if (root.timeRemaining <= 10) return "#ffaa00"
                    return "#00cc00"
                }
                Behavior on width { NumberAnimation { duration: 300 } }
                Behavior on color { ColorAnimation { duration: 300 } }
            }
        }

        // ========= Carte retournée + contrôles =========
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: w * 0.04

            // --- Carte retournée (gauche) ---
            Item {
                Layout.preferredWidth:  h * 0.42
                Layout.preferredHeight: h * 0.66
                Layout.alignment: Qt.AlignVCenter

                Card {
                    anchors.fill: parent
                    value: gameModel.retourneeValue >= 0 ? gameModel.retourneeValue : 7
                    suit:  gameModel.retourneeSuit  >= 0 ? gameModel.retourneeSuit  : 3
                    faceUp: true
                    cardRatio: 1.0
                }
            }

            // --- Contrôles (droite) ---
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                // ---- Tour 1 : Prendre / Passer ----
                Column {
                    anchors.centerIn: parent
                    spacing: h * 0.04
                    visible: root.isRound1

                    AppButton {
                        width: w * 0.28
                        height: h * 0.22
                        enabled: root.isMyTurn

                        background: Rectangle {
                            radius: 8
                            color: parent.enabled
                                   ? (parent.down ? "#007700" : (parent.hovered ? "#00aa00" : "#005500"))
                                   : "#2a2a2a"
                            border.color: parent.enabled ? "#FFD700" : "#555555"
                            border.width: 2
                            opacity: parent.enabled ? 1.0 : 0.5
                        }
                        contentItem: Text {
                            text: "Prendre"
                            font.pixelSize: h * 0.08
                            font.bold: true
                            color: parent.enabled ? "white" : "#777777"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            bidTimer.stop()
                            gameModel.prendreBid(gameModel.retourneeSuit)
                        }
                    }

                    AppButton {
                        width: w * 0.28
                        height: h * 0.22
                        enabled: root.isMyTurn

                        background: Rectangle {
                            radius: 8
                            color: parent.enabled
                                   ? (parent.down ? "#444444" : (parent.hovered ? "#666666" : "#333333"))
                                   : "#1a1a1a"
                            border.color: parent.enabled ? "#888888" : "#444444"
                            border.width: 2
                            opacity: parent.enabled ? 1.0 : 0.5
                        }
                        contentItem: Text {
                            text: "Passer"
                            font.pixelSize: h * 0.08
                            font.bold: true
                            color: parent.enabled ? "#cccccc" : "#555555"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            bidTimer.stop()
                            gameModel.passBeloteBid()
                        }
                    }
                }

                // ---- Tour 2 : 3 couleurs + Passer ----
                Column {
                    anchors.centerIn: parent
                    spacing: h * 0.04
                    visible: !root.isRound1

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: w * 0.02

                        Repeater {
                            model: root.availableSuits

                            AppButton {
                                width:  h * 0.32
                                height: h * 0.32
                                enabled: root.isMyTurn

                                background: Rectangle {
                                    radius: 8
                                    color: parent.enabled
                                           ? (parent.down ? "#333333" : (parent.hovered ? "#555555" : "lightgrey"))
                                           : "#666666"
                                    border.color: parent.enabled ? modelData.color : "#444444"
                                    border.width: 2
                                    opacity: parent.enabled ? 1.0 : 0.4
                                }
                                contentItem: Image {
                                    source: modelData.icon
                                    fillMode: Image.PreserveAspectFit
                                    anchors.fill: parent
                                    anchors.margins: parent.width * 0.2
                                    opacity: parent.enabled ? 1.0 : 0.4
                                }
                                onClicked: {
                                    bidTimer.stop()
                                    gameModel.prendreBid(modelData.value)
                                }
                            }
                        }
                    }

                    AppButton {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: w * 0.28
                        height: h * 0.22
                        enabled: root.isMyTurn

                        background: Rectangle {
                            radius: 8
                            color: parent.enabled
                                   ? (parent.down ? "#444444" : (parent.hovered ? "#666666" : "#333333"))
                                   : "#1a1a1a"
                            border.color: parent.enabled ? "#888888" : "#444444"
                            border.width: 2
                            opacity: parent.enabled ? 1.0 : 0.5
                        }
                        contentItem: Text {
                            text: "Passer"
                            font.pixelSize: h * 0.07
                            font.bold: true
                            color: parent.enabled ? "#cccccc" : "#555555"
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
    }
}
