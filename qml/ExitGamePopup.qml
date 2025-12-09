import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    anchors.fill: parent
    color: "#80000000"  // Fond semi-transparent noir
    visible: false
    z: 1000  // Au-dessus de tout

    signal confirmExit()
    signal cancelExit()

    MouseArea {
        anchors.fill: parent
        onClicked: {
            // Empêcher les clics de passer à travers
        }
    }

    // Popup centrale
    Rectangle {
        id: popup
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.8, 500)
        height: Math.min(parent.height * 0.4, 300)
        color: "#1a1a1a"
        radius: 15
        border.color: "#FFD700"
        border.width: 3

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: parent.width * 0.05
            spacing: parent.height * 0.05

            // Icône et titre
            Row {
                Layout.alignment: Qt.AlignHCenter
                spacing: 15

                Image {
                    source: "qrc:/resources/exit-svgrepo-com.svg.svg"
                    width: 40
                    height: 40
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    text: "Quitter la partie"
                    font.pixelSize: popup.height * 0.12
                    font.bold: true
                    color: "#FFD700"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // Message
            Text {
                text: "Êtes-vous sûr de vouloir quitter ?\n\nCela comptera comme une défaite."
                font.pixelSize: popup.height * 0.08
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
            }

            Item { Layout.fillHeight: true }

            // Boutons
            Row {
                Layout.alignment: Qt.AlignHCenter
                spacing: popup.width * 0.1

                // Bouton Non
                Button {
                    width: popup.width * 0.3
                    height: popup.height * 0.15

                    background: Rectangle {
                        color: parent.down ? "#555555" : (parent.hovered ? "#666666" : "#777777")
                        radius: 10
                        border.color: "#aaaaaa"
                        border.width: 2
                    }

                    contentItem: Text {
                        text: "Non, continuer"
                        font.pixelSize: popup.height * 0.08
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        root.cancelExit()
                        root.visible = false
                    }
                }

                // Bouton Oui
                Button {
                    width: popup.width * 0.3
                    height: popup.height * 0.15

                    background: Rectangle {
                        color: parent.down ? "#aa0000" : (parent.hovered ? "#cc0000" : "#ff3333")
                        radius: 10
                        border.color: "#ff6666"
                        border.width: 2
                    }

                    contentItem: Text {
                        text: "Oui, quitter"
                        font.pixelSize: popup.height * 0.08
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        root.confirmExit()
                        root.visible = false
                    }
                }
            }
        }
    }
}
