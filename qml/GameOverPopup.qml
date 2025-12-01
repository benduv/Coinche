import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: gameOverRoot
    anchors.fill: parent
    color: "#000000CC"  // Fond semi-transparent

    property int winnerTeam: 1
    property int scoreTeam1: 0
    property int scoreTeam2: 0

    signal returnToMenu()

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
        width: Math.min(parent.width * 0.6, 600)
        height: Math.min(parent.height * 0.7, 500)
        color: "#1a1a1a"
        radius: 15
        border.color: winnerTeam === 1 ? "#FFD700" : "#FFD700"
        border.width: 4

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 30
            spacing: 25

            // Titre "Partie terminée"
            Text {
                text: "PARTIE TERMINEE"
                font.pixelSize: 48
                font.bold: true
                color: "#FFD700"
                Layout.alignment: Qt.AlignHCenter
            }

            Item { height: 10 }

            // Équipe gagnante
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                color: "#2a2a2a"
                radius: 10
                border.color: winnerTeam === 1 ? "#00dd00" : "#0099dd"
                border.width: 3

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 10

                    Text {
                        text: "Equipe " + winnerTeam + " remporte la partie !"
                        font.pixelSize: 36
                        font.bold: true
                        color: winnerTeam === 1 ? "#00ff00" : "#00ccff"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: winnerTeam === 1 ? "Joueurs Sud et Nord" : "Joueurs Est et Ouest"
                        font.pixelSize: 20
                        color: "#aaaaaa"
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            Item { height: 10 }

            // Scores finaux
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 150
                color: "#2a2a2a"
                radius: 10
                border.color: "#555555"
                border.width: 2

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    Text {
                        text: "Scores finaux"
                        font.pixelSize: 28
                        font.bold: true
                        color: "#FFD700"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 20

                        // Score Équipe 1
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 5

                            Text {
                                text: "Equipe 1"
                                font.pixelSize: 22
                                color: "#aaaaaa"
                                Layout.alignment: Qt.AlignHCenter
                            }

                            Text {
                                text: scoreTeam1.toString()
                                font.pixelSize: 42
                                font.bold: true
                                color: winnerTeam === 1 ? "#00ff00" : "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                            }
                        }

                        // Séparateur
                        Rectangle {
                            width: 2
                            Layout.fillHeight: true
                            color: "#555555"
                        }

                        // Score Équipe 2
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 5

                            Text {
                                text: "Equipe 2"
                                font.pixelSize: 22
                                color: "#aaaaaa"
                                Layout.alignment: Qt.AlignHCenter
                            }

                            Text {
                                text: scoreTeam2.toString()
                                font.pixelSize: 42
                                font.bold: true
                                color: winnerTeam === 2 ? "#00ff00" : "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                            }
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }

            // Bouton Retour au menu
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                background: Rectangle {
                    color: parent.down ? "#0088cc" : (parent.hovered ? "#0099dd" : "#0077bb")
                    radius: 10
                    border.color: "#FFD700"
                    border.width: 3
                }

                contentItem: Text {
                    text: "Retour au menu"
                    font.pixelSize: 32
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    returnToMenu()
                }
            }
        }
    }
}
