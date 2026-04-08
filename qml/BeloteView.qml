import QtQuick
import QtQuick.Controls
import QtMultimedia

Item {
    id: beloteRoot
    anchors.fill: parent

    // Couche de base : CoincheView complète (avec les guards qui masquent les éléments Coinche)
    Loader {
        id: coincheLoader
        anchors.fill: parent
        source: "qrc:/qml/CoincheView.qml"
    }

    // Overlay enchères Belote : retournée + boutons côte à côte
    Item {
        id: beloteBidOverlay
        visible: gameModel.isBeloteMode &&
                 gameModel.biddingPhase &&
                 gameModel.distributionPhase === 0 &&
                 gameModel.retourneeSuit >= 0 &&
                 gameModel.retourneeValue >= 0 &&
                 !gameModel.showGoodGameAnimation

        anchors.centerIn: parent

        // Largeur : carte + espacement + boutons
        width: cardItem.width + 24 * scaleF + buttonsColumn.width
        height: Math.max(cardItem.height + retourneeLabel.height + 8, buttonsColumn.height)

        property real scaleF: beloteRoot.height / 768

        // --- Carte retournée ---
        Item {
            id: cardItem
            width: beloteRoot.width * 0.10
            height: width * 1.5
            anchors.verticalCenter: parent.verticalCenter

            Card {
                anchors.fill: parent
                value: gameModel.retourneeValue >= 0 ? gameModel.retourneeValue : 7
                suit:  gameModel.retourneeSuit  >= 0 ? gameModel.retourneeSuit  : 3
                faceUp: true
                cardRatio: 1.0
            }

            // Étiquette "Retournée"
            Rectangle {
                id: retourneeLabel
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.bottom
                anchors.topMargin: 4
                color: "#CC000000"
                radius: 4
                width: labelText.width + 10
                height: labelText.height + 5

                Text {
                    id: labelText
                    anchors.centerIn: parent
                    text: "Retournée"
                    font.pixelSize: beloteRoot.height * 0.018
                    color: "#FFD700"
                    font.bold: true
                }
            }
        }

        // --- Boutons Prendre / Passer ---
        Column {
            id: buttonsColumn
            anchors.left: cardItem.right
            anchors.leftMargin: 24 * beloteBidOverlay.scaleF
            anchors.verticalCenter: parent.verticalCenter
            spacing: 10 * beloteBidOverlay.scaleF

            property bool isMyTurn: gameModel.biddingPlayer === gameModel.myPosition

            // Bouton Prendre (tour 1) ou couleur choisie (tour 2 géré dans BeloteAnnoncesPanel)
            AppButton {
                id: btnPrendre
                width: 140 * beloteBidOverlay.scaleF
                height: 84 * beloteBidOverlay.scaleF
                enabled: buttonsColumn.isMyTurn && gameModel.beloteBidRound === 1

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
                    font.pixelSize: beloteBidOverlay.scaleF * 20
                    font.bold: true
                    color: parent.enabled ? "white" : "#777777"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: gameModel.prendreBid(gameModel.retourneeSuit)
            }

            // Bouton Passer
            AppButton {
                id: btnPasser
                width: 140 * beloteBidOverlay.scaleF
                height: 84 * beloteBidOverlay.scaleF
                enabled: buttonsColumn.isMyTurn

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
                    font.pixelSize: beloteBidOverlay.scaleF * 20
                    font.bold: true
                    color: parent.enabled ? "#cccccc" : "#555555"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: gameModel.passBeloteBid()
            }
        }
    }

    // Overlay tour 2 : choisir une autre couleur
    BeloteAnnoncesPanel {
        anchors.centerIn: parent
        visible: gameModel.isBeloteMode &&
                 gameModel.biddingPhase &&
                 gameModel.distributionPhase === 0 &&
                 gameModel.beloteBidRound === 2
    }
}
