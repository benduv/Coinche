import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: statsRoot
    anchors.fill: parent
    color: "#1a1a1a"

    // Ratio responsive pour adapter la taille des composants
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Propriétés pour les statistiques
    property int gamesPlayed: 0
    property int gamesWon: 0
    property real winRatio: 0.0
    property int coincheAttempts: 0
    property int coincheSuccess: 0
    property string playerName: ""

    signal backToMenu()

    Component.onCompleted: {
        // Demander les statistiques au serveur
        if (playerName !== "") {
            requestStats()
        }
    }

    function requestStats() {
        var message = {
            "type": "getStats",
            "pseudo": playerName
        }
        networkManager.sendMessage(JSON.stringify(message))
    }

    Connections {
        target: networkManager

        function onMessageReceived(message) {
            var data = JSON.parse(message)

            if (data.type === "statsData") {
                gamesPlayed = data.gamesPlayed
                gamesWon = data.gamesWon
                winRatio = data.winRatio
                coincheAttempts = data.coincheAttempts
                coincheSuccess = data.coincheSuccess
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 30 * statsRoot.minRatio
        width: 600 * statsRoot.widthRatio

        // Titre
        Text {
            text: "STATISTIQUES DE JEU"
            font.pixelSize: 48 * statsRoot.minRatio
            font.bold: true
            color: "#FFD700"
            Layout.alignment: Qt.AlignHCenter
        }

        // Nom du joueur
        Text {
            text: playerName
            font.pixelSize: 28 * statsRoot.minRatio
            color: "#aaaaaa"
            Layout.alignment: Qt.AlignHCenter
        }

        Item { height: 20 * statsRoot.minRatio }

        // Section: Parties
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 200 * statsRoot.heightRatio
            color: "#2a2a2a"
            radius: 10 * statsRoot.minRatio
            border.color: "#FFD700"
            border.width: 2 * statsRoot.minRatio

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20 * statsRoot.minRatio
                spacing: 15 * statsRoot.minRatio

                Text {
                    text: "Parties jouées"
                    font.pixelSize: 24 * statsRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    Layout.alignment: Qt.AlignHCenter
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 40 * statsRoot.widthRatio

                    // Parties jouées
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5 * statsRoot.minRatio

                        Text {
                            text: "Parties jouées"
                            font.pixelSize: 18 * statsRoot.minRatio
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: gamesPlayed.toString()
                            font.pixelSize: 36 * statsRoot.minRatio
                            font.bold: true
                            color: "#00cc00"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }

                    // Parties gagnées
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5 * statsRoot.minRatio

                        Text {
                            text: "Parties gagnées"
                            font.pixelSize: 18 * statsRoot.minRatio
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: gamesWon.toString()
                            font.pixelSize: 36 * statsRoot.minRatio
                            font.bold: true
                            color: "#00dd00"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }

                    // Ratio de victoire
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5 * statsRoot.minRatio

                        Text {
                            text: "Taux de victoire"
                            font.pixelSize: 18 * statsRoot.minRatio
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: (winRatio * 100).toFixed(1) + "%"
                            font.pixelSize: 36 * statsRoot.minRatio
                            font.bold: true
                            color: "#FFD700"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }
        }

        // Section: Coinches
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 180 * statsRoot.heightRatio
            color: "#2a2a2a"
            radius: 10 * statsRoot.minRatio
            border.color: "#cc0000"
            border.width: 2 * statsRoot.minRatio

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20 * statsRoot.minRatio
                spacing: 15 * statsRoot.minRatio

                Text {
                    text: "Coinches"
                    font.pixelSize: 24 * statsRoot.minRatio
                    font.bold: true
                    color: "#cc0000"
                    Layout.alignment: Qt.AlignHCenter
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 40 * statsRoot.widthRatio

                    // Coinches tentées
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5 * statsRoot.minRatio

                        Text {
                            text: "Tentatives"
                            font.pixelSize: 18 * statsRoot.minRatio
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: coincheAttempts.toString()
                            font.pixelSize: 36 * statsRoot.minRatio
                            font.bold: true
                            color: "#ff6666"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }

                    // Coinches réussies
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5 * statsRoot.minRatio

                        Text {
                            text: "Réussies"
                            font.pixelSize: 18 * statsRoot.minRatio
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: coincheSuccess.toString()
                            font.pixelSize: 36 * statsRoot.minRatio
                            font.bold: true
                            color: "#00dd00"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }

                    // Taux de réussite
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5 * statsRoot.minRatio

                        Text {
                            text: "Taux de réussite"
                            font.pixelSize: 18 * statsRoot.minRatio
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: coincheAttempts > 0 ? ((coincheSuccess / coincheAttempts) * 100).toFixed(1) + "%" : "0%"
                            font.pixelSize: 36 * statsRoot.minRatio
                            font.bold: true
                            color: "#FFD700"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }
        }

        Item { height: 5 * statsRoot.minRatio }

        // Bouton Retour
        Button {
            Layout.preferredWidth: 250 * statsRoot.widthRatio
            Layout.preferredHeight: 120 * statsRoot.heightRatio
            Layout.alignment: Qt.AlignHCenter

            background: Rectangle {
                color: parent.down ? "#0088cc" : (parent.hovered ? "#0099dd" : "#0077bb")
                radius: 10 * statsRoot.minRatio
                border.color: "#FFD700"
                border.width: 2 * statsRoot.minRatio
            }

            contentItem: Text {
                text: "Retour au menu"
                font.pixelSize: 48 * statsRoot.minRatio
                font.bold: true
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                backToMenu()
            }
        }
    }
}
