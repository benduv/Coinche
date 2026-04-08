import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

// Panneau tour 2 uniquement : choisir une couleur (hors retournée) ou Passer
Rectangle {
    id: root
    color: "#E8000000"
    radius: 12
    border.color: "#FFD700"
    border.width: 2

    property real scaleF: height / 200

    // Dimensions adaptées au contenu
    width: contentRow.width + 32 * scaleF
    height: 160 * scaleF

    // Suit data : COEUR=3, TREFLE=4, CARREAU=5, PIQUE=6
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
        return result
    }

    property bool isMyTurn: gameModel.biddingPlayer === gameModel.myPosition

    ColumnLayout {
        id: contentRow
        anchors.centerIn: parent
        spacing: 10 * scaleF

        Text {
            text: "Choisissez votre couleur"
            font.pixelSize: 16 * scaleF
            font.bold: true
            color: "#FFD700"
            Layout.alignment: Qt.AlignHCenter
        }

        RowLayout {
            spacing: 10 * scaleF
            Layout.alignment: Qt.AlignHCenter

            Repeater {
                model: root.availableSuits

                AppButton {
                    Layout.preferredWidth:  60 * scaleF
                    Layout.preferredHeight: 60 * scaleF
                    enabled: root.isMyTurn

                    background: Rectangle {
                        radius: 8
                        color: parent.enabled
                               ? (parent.down ? "#333333" : (parent.hovered ? "#555555" : "#222222"))
                               : "#111111"
                        border.color: parent.enabled ? modelData.color : "#444444"
                        border.width: 2
                        opacity: parent.enabled ? 1.0 : 0.4
                    }

                    contentItem: Image {
                        source: modelData.icon
                        fillMode: Image.PreserveAspectFit
                        anchors.fill: parent
                        anchors.margins: 8 * scaleF
                    }

                    onClicked: gameModel.prendreBid(modelData.value)
                }
            }

            // Bouton Passer
            AppButton {
                Layout.preferredWidth:  70 * scaleF
                Layout.preferredHeight: 60 * scaleF
                enabled: root.isMyTurn

                background: Rectangle {
                    radius: 8
                    color: parent.enabled
                           ? (parent.down ? "#444444" : (parent.hovered ? "#666666" : "#333333"))
                           : "#1a1a1a"
                    border.color: parent.enabled ? "#888888" : "#444444"
                    border.width: 2
                    opacity: parent.enabled ? 1.0 : 0.4
                }

                contentItem: Text {
                    text: "Passer"
                    font.pixelSize: 14 * scaleF
                    font.bold: true
                    color: parent.enabled ? "#cccccc" : "#555555"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: gameModel.passBeloteBid()
            }
        }
    }
}
