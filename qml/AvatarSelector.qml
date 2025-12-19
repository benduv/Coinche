import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Composant de sélection d'avatar
Rectangle {
    id: root
    color: "transparent"

    // Avatar actuellement sélectionné
    property string selectedAvatar: "avataaars1.svg"

    // Dimensions responsive
    property real minRatio: Math.min(width / 800, height / 600)

    signal avatarSelected(string avatarName)

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 15 * minRatio
        anchors.bottomMargin: 1 * minRatio
        anchors.leftMargin: 15 * minRatio
        anchors.rightMargin: 15 * minRatio
        spacing: 20 * minRatio

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth

            GridLayout {
                id: avatarGrid
                width: parent.parent.width - 20 * root.minRatio
                columns: 3
                rowSpacing: 20 * root.minRatio
                columnSpacing: 20 * root.minRatio

                // Calcul de la taille optimale pour que 3 colonnes remplissent exactement la largeur
                property real avatarSize: (width - columnSpacing * (columns - 1)) / columns

                Repeater {
                    model: [
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

                    delegate: Rectangle {
                        Layout.preferredWidth: avatarGrid.avatarSize
                        Layout.preferredHeight: avatarGrid.avatarSize
                        color: root.selectedAvatar === modelData ? "#FFD700" : "#2a2a2a"
                        radius: 15 * root.minRatio
                        border.color: root.selectedAvatar === modelData ? "#FFFFFF" : "#666666"
                        border.width: root.selectedAvatar === modelData ? 4 * root.minRatio : 2 * root.minRatio

                        Image {
                            anchors.fill: parent
                            anchors.margins: 10 * root.minRatio
                            source: "qrc:/resources/avatar/" + modelData
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }

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
        }
    }
}
