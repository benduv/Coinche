import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: w * 0.04
        spacing: h * 0.04

        // ========= Titre =========
        Text {
            text: "Phase d'annonces"
            font.pixelSize: h * 0.08
            font.bold: true
            color: "#FFD700"
            Layout.alignment: Qt.AlignHCenter
        }

        // ========= Dernière annonce =========
        /*Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: h * 0.20
            color: "#1a1a1a"
            radius: 8
            border.color: "#FFD700"
            border.width: 2

            ColumnLayout {
                anchors.centerIn: parent
                spacing: h * 0.01

                Text {
                    text: "Dernière annonce"
                    font.pixelSize: h * 0.03
                    color: "#aaaaaa"
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: gameModel.lastBid
                    font.pixelSize: h * 0.1
                    font.bold: true
                    color: "#FFD700"
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: gameModel.lastBidSuit
                    font.pixelSize: h * 0.05
                    color: "#ffffff"
                    Layout.alignment: Qt.AlignHCenter
                    visible: gameModel.lastBidValue > 0
                }
            }
        }*/

        // ========= Tour du joueur =========
        /*Text {
            text: gameModel.biddingPlayer === 0 ?
                  "À vous d'annoncer !" :
                  "Joueur " + (gameModel.biddingPlayer + 1) + " annonce..."
            font.pixelSize: h * 0.035
            color: gameModel.biddingPlayer === 0 ? "#00ff00" : "#ffffff"
            Layout.alignment: Qt.AlignHCenter
        }*/

        //Item { Layout.fillHeight: true }

        // ========= Choix des annonces =========
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: h * 0.02

            /*Text {
                text: "Choisissez votre annonce :"
                font.pixelSize: h * 0.05
                color: "#ffffff"
                Layout.alignment: Qt.AlignHCenter
            }*/

            // ---- Navigation avec fleches ----
            property int currentBidIndex: 0
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
                               ? (parent.down ? "#555555" : (parent.hovered ? "#666666" : "#444444"))
                               : "#222222"
                        radius: 6
                        border.color: parent.enabled ? "#FFD700" : "#333333"
                        border.width: 2
                    }

                    contentItem: Text {
                        text: "◀  "
                        font.pixelSize: h * 0.1
                        color: parent.enabled ? "#FFD700" : "#555555"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
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
                               ? (parent.down ? "#555555" : (parent.hovered ? "#666666" : "#444444"))
                               : "#222222"
                        radius: 6
                        border.color: parent.enabled ? "#FFD700" : "#333333"
                        border.width: 2
                    }

                    contentItem: Text {
                        text: "  ▶"
                        font.pixelSize: h * 0.1
                        color: parent.enabled ? "#FFD700" : "#555555"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        if (parent.parent.currentBidIndex + 3 < parent.parent.allBids.length) {
                            parent.parent.currentBidIndex = parent.parent.currentBidIndex + 3
                        }
                    }
                }
            }

            // Indicateur de position
            /*Row {
                Layout.alignment: Qt.AlignHCenter
                spacing: w * 0.01

                Repeater {
                    model: Math.ceil(parent.parent.allBids.length / 3)
                    Rectangle {
                        width: w * 0.015
                        height: w * 0.015
                        radius: width / 2
                        color: Math.floor(parent.parent.parent.currentBidIndex / 3) === index ? "#FFD700" : "#555555"
                    }
                }
            }*/
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

            onClicked: gameModel.passBid()
        }

        //Item { Layout.fillHeight: true }
    }

    // ========= POPUP SELECTION COULEUR =========
    Popup {
        id: suitSelector
        anchors.centerIn: parent
        width: parent.width * 0.9
        height: parent.height * 0.9
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape
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
                spacing: parent.width * 0.05

                SuitButton {
                    text: "♥"
                    suitColor: "#E60000"
                    suitValue: 3
                    popupWidth: suitSelector.width
                    popupHeight: suitSelector.height
                    onClicked: { gameModel.makeBid(suitSelector.selectedBidValue, 3); suitSelector.close() }
                }
                SuitButton {
                    text: "♣"
                    suitColor: "#000000"
                    suitValue: 4
                    popupWidth: suitSelector.width
                    popupHeight: suitSelector.height
                    onClicked: { gameModel.makeBid(suitSelector.selectedBidValue, 4); suitSelector.close() }
                }
                SuitButton {
                    text: "♦"
                    suitColor: "#E60000"
                    suitValue: 5
                    popupWidth: suitSelector.width
                    popupHeight: suitSelector.height
                    onClicked: { gameModel.makeBid(suitSelector.selectedBidValue, 5); suitSelector.close() }
                }
                SuitButton {
                    text: "♠"
                    suitColor: "#000000"
                    suitValue: 6
                    popupWidth: suitSelector.width
                    popupHeight: suitSelector.height
                    onClicked: { gameModel.makeBid(suitSelector.selectedBidValue, 6); suitSelector.close() }
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

        width: popupWidth * 0.18
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
            font.pixelSize: popupHeight * 0.35
            color: suitColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
