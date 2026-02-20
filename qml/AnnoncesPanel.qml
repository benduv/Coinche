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

    // Dimensions relatives pour adaptation
    property real w: width
    property real h: height

    // Timer pour l'annonce
    property int timeRemaining: 20
    property int maxTime: 20

    // Son d'alerte quand le temps est presque écoulé
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
                // Jouer le son d'alerte à 4 secondes restantes (une seule fois)
                if (timeRemaining === 4 && !root.alertSoundPlayed) {
                    root.alertSoundPlayed = true
                    if (AudioSettings.effectsEnabled && Qt.application.state === Qt.ApplicationActive) {
                        alertSound.stop()
                        alertSound.play()
                    }
                }
            } else {
                // Temps ecoule, fermer la popup si ouverte et laisser le bot annoncer automatiquement
                if (suitSelector.visible) {
                    suitSelector.close()
                }
                //gameModel.passBid()
                stop()
            }
        }
    }

    // Reinitialiser le timer quand le panneau devient visible
    onVisibleChanged: {
        if (visible) {
            timeRemaining = maxTime
            alertSoundPlayed = false
            bidTimer.restart()
        } else {
            bidTimer.stop()
            alertSound.stop()
            // Fermer le suitSelector si ouvert (au cas où le serveur timeout avant le client)
            if (suitSelector.visible) {
                suitSelector.close()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: w * 0.04
        spacing: h * 0.02

        // ========= Titre =========
        Text {
            text: "Phase d'annonces"
            font.pixelSize: h * 0.08
            font.bold: true
            color: "#FFD700"
            Layout.alignment: Qt.AlignHCenter
        }

        // ========= Jauge de temps =========
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: h * 0.12
            spacing: h * 0.01

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

                    Behavior on width {
                        NumberAnimation { duration: 300 }
                    }

                    Behavior on color {
                        ColorAnimation { duration: 300 }
                    }
                }
            }
        }

        // ========= Choix des annonces =========
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: h * 0.02

            // ---- Navigation avec fleches ----
            property int currentBidIndex: {
                // Commencer a l'annonce suivant la derniere annonce faite
                var lastBid = gameModel.lastBidValue
                var startIndex = 0
                for (var i = 0; i < allBids.length; i++) {
                    if (allBids[i].value <= lastBid) {
                        startIndex = i + 1
                    }
                }
                return startIndex
            }
            property var allBids: [
                { value: 1, label: "80" },
                { value: 2, label: "90" },
                { value: 3, label: "100" },
                { value: 4, label: "110" },
                { value: 5, label: "120" },
                { value: 6, label: "130" },
                { value: 7, label: "140" },
                { value: 8, label: "150" },
                { value: 9, label: "160" },
                { value: 10, label: "Capot" },
                { value: 11, label: "Générale" }
            ]

            Row {
                Layout.alignment: Qt.AlignHCenter
                spacing: w * 0.02

                // Fleche gauche
                Button {
                    width: w * 0.15
                    height: h * 0.25
                    enabled: parent.parent.currentBidIndex > 0

                    background: Rectangle {
                        color: parent.enabled
                               ? (parent.down ? "#888888" : (parent.hovered ? "#777777" : "#999999"))
                               : "#222222"
                        radius: 6
                        border.color: parent.enabled ? "#FFD700" : "#333333"
                        border.width: 2
                    }

                    contentItem: Image {
                        source: "qrc:/resources/left-arrow-next-svgrepo-com.svg"
                        fillMode: Image.PreserveAspectFit
                        sourceSize.width: parent.width * 0.5
                        sourceSize.height: parent.height * 0.5
                        opacity: parent.enabled ? 1.0 : 0.3
                        anchors.centerIn: parent
                    }

                    onClicked: {
                        if (parent.parent.currentBidIndex > 0) {
                            parent.parent.currentBidIndex = parent.parent.currentBidIndex - 3
                        }
                    }
                }

                // Boutons d'annonces (3 visibles)
                Repeater {
                    model: 3
                    BidButton {
                        property var bidData: {
                            var idx = parent.parent.currentBidIndex + index
                            return idx < parent.parent.allBids.length ? parent.parent.allBids[idx] : null
                        }
                        text: bidData ? bidData.label : ""
                        bidValue: bidData ? bidData.value : 0
                        width: w * 0.15
                        height: h * 0.25
                        visible: bidData !== null
                        enabled: bidData && bidValue > gameModel.lastBidValue
                        onClicked: showSuitSelector(bidValue)
                    }
                }

                // Fleche droite
                Button {
                    width: w * 0.15
                    height: h * 0.25
                    enabled: parent.parent.currentBidIndex + 3 < parent.parent.allBids.length

                    background: Rectangle {
                        color: parent.enabled
                               ? (parent.down ? "#888888" : (parent.hovered ? "#777777" : "#999999"))
                               : "#222222"
                        radius: 6
                        border.color: parent.enabled ? "#FFD700" : "#333333"
                        border.width: 3
                    }

                    contentItem: Image {
                        source: "qrc:/resources/right-arrow-next-svgrepo-com.svg"
                        fillMode: Image.PreserveAspectFit
                        sourceSize.width: parent.width * 0.5
                        sourceSize.height: parent.height * 0.5
                        opacity: parent.enabled ? 1.0 : 0.3
                        anchors.centerIn: parent
                    }

                    onClicked: {
                        if (parent.parent.currentBidIndex + 3 < parent.parent.allBids.length) {
                            parent.parent.currentBidIndex = parent.parent.currentBidIndex + 3
                        }
                    }
                }
            }
        }

        // ========= Bouton "Passer" =========
        Button {
            text: "Passer"
            font.pixelSize: h * 0.09
            font.bold: true
            Layout.preferredWidth: w * 0.25
            Layout.preferredHeight: h * 0.25
            Layout.alignment: Qt.AlignHCenter

            background: Rectangle {
                color: parent.down ? "#cc0000" :
                       (parent.hovered ? "#ff3333" : "#ff0000")
                radius: 8
            }

            contentItem: Text {
                text: parent.text
                font: parent.font
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                bidTimer.stop()
                gameModel.passBid()
            }
        }
    }

    // ========= POPUP SELECTION COULEUR =========
    Popup {
        id: suitSelector
        anchors.centerIn: parent
        width: parent.width * 0.95
        height: parent.height * 0.65
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        property int selectedBidValue: 0

        background: Rectangle {
            color: "#2a2a2a"
            radius: 10
            border.color: "#FFD700"
            border.width: 2
        }

        // Utiliser un Item pour avoir une reference stable pour les dimensions
        contentItem: Item {
            anchors.fill: parent

            Row {
                anchors.centerIn: parent
                spacing: parent.width * 0.025

                SuitButton {
                    text: "♥"
                    suitColor: "#E60000"
                    suitValue: 3
                    popupWidth: suitSelector.width
                    popupHeight: suitSelector.height
                    onClicked: { bidTimer.stop(); gameModel.makeBid(suitSelector.selectedBidValue, 3); suitSelector.close() }
                }
                SuitButton {
                    text: "♣"
                    suitColor: "#000000"
                    suitValue: 4
                    popupWidth: suitSelector.width
                    popupHeight: suitSelector.height
                    onClicked: { bidTimer.stop(); gameModel.makeBid(suitSelector.selectedBidValue, 4); suitSelector.close() }
                }
                SuitButton {
                    text: "♦"
                    suitColor: "#E60000"
                    suitValue: 5
                    popupWidth: suitSelector.width
                    popupHeight: suitSelector.height
                    onClicked: { bidTimer.stop(); gameModel.makeBid(suitSelector.selectedBidValue, 5); suitSelector.close() }
                }
                SuitButton {
                    text: "♠"
                    suitColor: "#000000"
                    suitValue: 6
                    popupWidth: suitSelector.width
                    popupHeight: suitSelector.height
                    onClicked: { bidTimer.stop(); gameModel.makeBid(suitSelector.selectedBidValue, 6); suitSelector.close() }
                }
                // Bouton Tout Atout
                Button {
                    width: suitSelector.height * 0.6
                    height: suitSelector.height * 0.6

                    background: Rectangle {
                        color: parent.down ? "#444444" :
                               (parent.hovered ? "#555555" : "#333333")
                        radius: 10
                        border.color: "#FFD700"
                        border.width: 3
                    }

                    contentItem: Text {
                        text: "TA"
                        font.pixelSize: suitSelector.height * 0.21
                        font.bold: true
                        color: "#FFD700"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        bidTimer.stop();
                        gameModel.makeBid(suitSelector.selectedBidValue, 7);  // 7 = Tout Atout
                        suitSelector.close()
                    }
                }

                // Bouton Sans Atout
                Button {
                    width: suitSelector.height * 0.6
                    height: suitSelector.height * 0.6

                    background: Rectangle {
                        color: parent.down ? "#444444" :
                               (parent.hovered ? "#555555" : "#333333")
                        radius: 10
                        border.color: "#FFD700"
                        border.width: 3
                    }

                    contentItem: Text {
                        text: "SA"
                        font.pixelSize: suitSelector.height * 0.21
                        font.bold: true
                        color: "#FFD700"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        bidTimer.stop();
                        gameModel.makeBid(suitSelector.selectedBidValue, 8);  // 8 = Sans Atout
                        suitSelector.close()
                    }
                }
            }
        }
    }

    // ========= Fonctions =========
    function showSuitSelector(bidValue) {
        suitSelector.selectedBidValue = bidValue
        suitSelector.open()
    }

    // ========= Composants =========
    component BidButton: Button {
        property int bidValue

        background: Rectangle {
            color: parent.enabled
                   ? (parent.down ? "#0066cc" :
                      (parent.hovered ? "#0080ff" : "#0099ff"))
                   : "#333333"
            radius: 6
            border.color: parent.enabled ? "#FFD700" : "#555555"
            border.width: 1
        }

        contentItem: Text {
            text: parent.text
            font.pixelSize: h * 0.08
            font.bold: true
            color: parent.enabled ? "white" : "#666666"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    component SuitButton: Button {
        property color suitColor
        property int suitValue
        property real popupWidth: 100
        property real popupHeight: 100

        width: popupHeight * 0.6
        height: popupHeight * 0.6

        background: Rectangle {
            color: parent.down ? "#444444" :
                   (parent.hovered ? "#555555" : "#333333")
            radius: 10
            border.color: suitColor
            border.width: 3
        }

        contentItem: Text {
            text: parent.text
            font.pixelSize: popupHeight * 0.3
            color: suitColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
