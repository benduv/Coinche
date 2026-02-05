import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: botReplacementRoot
    anchors.fill: parent
    color: "#000000CC"  // Fond semi-transparent

    property string message: "Un bot a pris le relai car vous semblez inactif."

    // Ratio responsive
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    signal okClicked()

    // Animation d'apparition
    opacity: 0
    NumberAnimation on opacity {
        from: 0
        to: 1
        duration: 300
        easing.type: Easing.OutCubic
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            // Empêche les clics de passer à travers
        }
    }

    // Boîte de dialogue centrale
    Rectangle {
        id: dialogBox
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.75, 500 * widthRatio)
        height: Math.min(parent.height * 0.5, 350 * heightRatio)
        color: "#1a1a1a"
        radius: 20 * minRatio
        border.color: "#FF8C00"
        border.width: 4 * minRatio

        // Animation d'entrée (scale)
        scale: 0.7
        NumberAnimation on scale {
            from: 0.7
            to: 1.0
            duration: 400
            easing.type: Easing.OutBack
        }

        // Effet de brillance sur la bordure
        Rectangle {
            anchors.fill: parent
            anchors.margins: -2 * minRatio
            color: "transparent"
            radius: parent.radius
            border.color: "#FF8C00"
            border.width: 2 * minRatio
            opacity: 0.3

            SequentialAnimation on opacity {
                loops: Animation.Infinite
                NumberAnimation { from: 0.3; to: 0.8; duration: 1000; easing.type: Easing.InOutQuad }
                NumberAnimation { from: 0.8; to: 0.3; duration: 1000; easing.type: Easing.InOutQuad }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 25 * minRatio
            spacing: 20 * minRatio

            // Icône Robot
            Text {
                text: "\u{1F916}"
                font.pixelSize: 60 * minRatio
                Layout.alignment: Qt.AlignHCenter

                SequentialAnimation on scale {
                    loops: Animation.Infinite
                    NumberAnimation { from: 1.0; to: 1.1; duration: 600; easing.type: Easing.InOutQuad }
                    NumberAnimation { from: 1.1; to: 1.0; duration: 600; easing.type: Easing.InOutQuad }
                }
            }

            // Titre
            Text {
                text: "BOT ACTIVÉ"
                font.pixelSize: 32 * minRatio
                font.bold: true
                color: "#FF8C00"
                Layout.alignment: Qt.AlignHCenter

                style: Text.Outline
                styleColor: "#000000"
            }

            // Message
            Text {
                text: message
                font.pixelSize: 26 * minRatio
                color: "#cccccc"
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            // Bouton OK
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 100 * heightRatio

                background: Rectangle {
                    color: parent.down ? "#006699" : (parent.hovered ? "#0088cc" : "#0077bb")
                    radius: 12 * minRatio
                    border.color: "#FFD700"
                    border.width: 2 * minRatio

                    // Effet de brillance au survol
                    Rectangle {
                        anchors.fill: parent
                        radius: parent.radius
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#FFFFFF" }
                            GradientStop { position: 0.5; color: "transparent" }
                        }
                        opacity: parent.parent.hovered ? 0.2 : 0.1
                    }
                }

                contentItem: Text {
                    text: "OK - Reprendre le contrôle"
                    font.pixelSize: 26 * minRatio
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    style: Text.Outline
                    styleColor: "#000000"
                }

                onClicked: {
                    okClicked()
                }

                // Animation au clic
                scale: 1.0
                Behavior on scale {
                    NumberAnimation { duration: 100 }
                }
                onPressed: scale = 0.95
                onReleased: scale = 1.0
            }
        }
    }
}
