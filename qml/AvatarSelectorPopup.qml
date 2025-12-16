import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: avatarPopup
    anchors.centerIn: parent
    width: Math.min(parent.width * 0.8, 900)
    height: Math.min(parent.height * 0.7, 650)
    modal: true
    focus: true

    property string selectedAvatar: ""
    property real minRatio: Math.min(width / 900, height / 650)

    signal avatarSelected(string avatar)

    background: Rectangle {
        color: "#2a2a2a"
        radius: 15
        border.color: "#FFD700"
        border.width: 3
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Text {
            text: "Choisissez votre avatar"
            font.pixelSize: 36 * avatarPopup.minRatio
            font.bold: true
            color: "#FFD700"
            Layout.alignment: Qt.AlignHCenter
        }

        // Grille d'avatars
        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 4
            rowSpacing: 15
            columnSpacing: 15

            Repeater {
                model: 24

                Rectangle {
                    Layout.preferredWidth: 120 * avatarPopup.minRatio
                    Layout.preferredHeight: 120 * avatarPopup.minRatio
                    Layout.alignment: Qt.AlignHCenter
                    color: avatarPopup.selectedAvatar === "avataaars" + (index + 1) + ".svg" ? "#FFD700" : "#444444"
                    radius: 60 * avatarPopup.minRatio
                    border.color: "#FFD700"
                    border.width: avatarPopup.selectedAvatar === "avataaars" + (index + 1) + ".svg" ? 4 : 2

                    Image {
                        anchors.fill: parent
                        anchors.margins: 10
                        source: "qrc:/resources/avatar/avataaars" + (index + 1) + ".svg"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            avatarPopup.selectedAvatar = "avataaars" + (index + 1) + ".svg"
                        }
                    }

                    // Animation de pulsation pour l'avatar sélectionné
                    SequentialAnimation on scale {
                        running: avatarPopup.selectedAvatar === "avataaars" + (index + 1) + ".svg"
                        loops: Animation.Infinite
                        NumberAnimation { to: 1.1; duration: 600 }
                        NumberAnimation { to: 1.0; duration: 600 }
                    }
                }
            }
        }

        // Boutons
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 60 * avatarPopup.minRatio

                background: Rectangle {
                    color: parent.down ? "#cc0000" : (parent.hovered ? "#ff3333" : "#ff0000")
                    radius: 8
                }

                contentItem: Text {
                    text: "Annuler"
                    font.pixelSize: 28 * avatarPopup.minRatio
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: avatarPopup.close()
            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 60 * avatarPopup.minRatio
                enabled: avatarPopup.selectedAvatar !== ""

                background: Rectangle {
                    color: parent.enabled ?
                           (parent.down ? "#00aa00" : (parent.hovered ? "#00dd00" : "#00cc00")) :
                           "#555555"
                    radius: 8
                }

                contentItem: Text {
                    text: "Valider"
                    font.pixelSize: 28 * avatarPopup.minRatio
                    color: parent.parent.enabled ? "white" : "#aaaaaa"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    avatarPopup.avatarSelected(avatarPopup.selectedAvatar)
                    avatarPopup.close()
                }
            }
        }
    }
}
