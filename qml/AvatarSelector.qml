import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Composant de sélection d'avatar
Rectangle {
    id: root
    color: "transparent"

    // Liste des avatars disponibles
    readonly property var avatarList: [
        "avataaars1.svg",
        "avataaars2.svg",
        "avataaars3.svg",
        "avataaars4.svg",
        "avataaars5.svg",
        "avataaars6.svg",
        "avataaars7.svg",
        "avataaars8.svg",
        "avataaars9.svg",
        "avataaars10.svg",
        "avataaars11.svg",
        "avataaars12.svg",
        "avataaars13.svg",
        "avataaars14.svg",
        "avataaars15.svg",
        "avataaars16.svg",
        "avataaars17.svg",
        "avataaars18.svg",
        "avataaars19.svg",
        "avataaars20.svg",
        "avataaars21.svg",
        "avataaars22.svg",
        "avataaars23.svg",
        "avataaars24.svg"
    ]

    // Avatar actuellement sélectionné
    property string selectedAvatar: "avataaars1.svg"

    // Dimensions responsive
    property real minRatio: Math.min(width / 800, height / 600)

    signal avatarSelected(string avatarName)

    ColumnLayout {
        anchors.fill: parent
        spacing: 15 * minRatio

        // Titre
        Text {
            text: "Choisissez votre avatar :"
            font.pixelSize: 20 * minRatio
            font.bold: true
            color: "#FFD700"
            Layout.alignment: Qt.AlignHCenter
        }

        // Grille d'avatars
        GridLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            columns: 4
            rowSpacing: 15 * minRatio
            columnSpacing: 15 * minRatio

            Repeater {
                model: root.avatarList

                delegate: Rectangle {
                    Layout.preferredWidth: 80 * minRatio
                    Layout.preferredHeight: 80 * minRatio
                    color: root.selectedAvatar === modelData ? "#FFD700" : "#444444"
                    radius: 10 * minRatio
                    border.color: root.selectedAvatar === modelData ? "#FFFFFF" : "#666666"
                    border.width: root.selectedAvatar === modelData ? 3 * minRatio : 1 * minRatio

                    // Image de l'avatar
                    Image {
                        anchors.fill: parent
                        anchors.margins: 5 * minRatio
                        source: "qrc:/resources/avatar/" + modelData
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    // Effet de survol
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            root.selectedAvatar = modelData
                            root.avatarSelected(modelData)
                        }

                        onEntered: {
                            parent.scale = 1.1
                        }

                        onExited: {
                            parent.scale = 1.0
                        }
                    }

                    Behavior on scale {
                        NumberAnimation { duration: 150 }
                    }

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
