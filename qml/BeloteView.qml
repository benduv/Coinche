import QtQuick
import QtQuick.Controls

Item {
    id: beloteRoot
    anchors.fill: parent

    // Couche de base : CoincheView complète (avec les guards qui masquent les éléments Coinche)
    Loader {
        id: coincheLoader
        anchors.fill: parent
        source: "qrc:/qml/CoincheView.qml"
    }

    // Overlay : carte retournée au centre pendant la phase d'enchères Belote
    Item {
        id: retourneeOverlay
        visible: gameModel.isBeloteMode &&
                 gameModel.biddingPhase &&
                 gameModel.distributionPhase === 0 &&
                 gameModel.retourneeSuit >= 0 &&
                 gameModel.retourneeValue >= 0

        anchors.centerIn: parent
        width: parent.width * 0.12
        height: width * 1.5

        Card {
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            value: gameModel.retourneeValue >= 0 ? gameModel.retourneeValue : 7
            suit: gameModel.retourneeSuit >= 0 ? gameModel.retourneeSuit : 3
            faceUp: true
            cardRatio: 1.0
        }

        // Étiquette "Retournée"
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.bottom
            anchors.topMargin: 4
            color: "#000000CC"
            radius: 4
            width: retourneeLabel.width + 12
            height: retourneeLabel.height + 6

            Text {
                id: retourneeLabel
                anchors.centerIn: parent
                text: "Retournée"
                font.pixelSize: beloteRoot.height * 0.018
                color: "#FFD700"
                font.bold: true
            }
        }
    }

    // Overlay : panneau d'enchères Belote
    BeloteAnnoncesPanel {
        anchors.fill: parent
        anchors.leftMargin: parent.width * 0.09
        anchors.rightMargin: parent.width * 0.09
        anchors.topMargin: parent.height * 0.21
        anchors.bottomMargin: parent.height * 0.2
    }
}
