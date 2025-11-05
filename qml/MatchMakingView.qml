import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    anchors.fill: parent
    color: "#1a1a1a"

    signal cancelClicked()

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 40

        // Animation de recherche
        Item {
            Layout.preferredWidth: 200
            Layout.preferredHeight: 200
            Layout.alignment: Qt.AlignHCenter

            // Cercles animés
            Repeater {
                model: 3
                Rectangle {
                    width: 180 - (index * 40)
                    height: width
                    radius: width / 2
                    color: "transparent"
                    border.color: "#00cc00"
                    border.width: 3
                    anchors.centerIn: parent
                    opacity: 0

                    SequentialAnimation on opacity {
                        running: true
                        loops: Animation.Infinite
                        PauseAnimation { duration: index * 400 }
                        NumberAnimation { to: 1.0; duration: 600 }
                        NumberAnimation { to: 0.0; duration: 600 }
                    }

                    SequentialAnimation on scale {
                        running: true
                        loops: Animation.Infinite
                        PauseAnimation { duration: index * 400 }
                        NumberAnimation { to: 1.2; duration: 1200 }
                        PropertyAction { value: 1.0 }
                    }
                }
            }

            // Icône centrale
            Text {
                text: "🎴"
                font.pixelSize: 80
                anchors.centerIn: parent
                
                RotationAnimation on rotation {
                    from: 0
                    to: 360
                    duration: 3000
                    loops: Animation.Infinite
                }
            }
        }

        // Texte de statut
        Text {
            text: "Recherche de joueurs..."
            font.pixelSize: 32
            font.bold: true
            color: "#FFD700"
            Layout.alignment: Qt.AlignHCenter
        }

        // Nombre de joueurs
        Rectangle {
            Layout.preferredWidth: 400
            Layout.preferredHeight: 100
            Layout.alignment: Qt.AlignHCenter
            color: "#2a2a2a"
            radius: 10
            border.color: "#00cc00"
            border.width: 2

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10

                Text {
                    text: networkManager.playersInQueue + " / 4 joueurs"
                    font.pixelSize: 36
                    font.bold: true
                    color: "#00ff00"
                    Layout.alignment: Qt.AlignHCenter
                }

                // Barre de progression
                Rectangle {
                    Layout.preferredWidth: 300
                    Layout.preferredHeight: 10
                    Layout.alignment: Qt.AlignHCenter
                    color: "#444444"
                    radius: 5

                    Rectangle {
                        width: parent.width * (networkManager.playersInQueue / 4.0)
                        height: parent.height
                        color: "#00ff00"
                        radius: parent.radius

                        Behavior on width {
                            NumberAnimation { duration: 300 }
                        }
                    }
                }
            }
        }

        // Indicateurs de joueurs
        Row {
            Layout.alignment: Qt.AlignHCenter
            spacing: 20

            Repeater {
                model: 4
                Rectangle {
                    width: 60
                    height: 60
                    radius: 30
                    color: index < networkManager.playersInQueue ? "#00cc00" : "#333333"
                    border.color: "#FFD700"
                    border.width: 2

                    Behavior on color {
                        ColorAnimation { duration: 300 }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: index < networkManager.playersInQueue ? "✓" : "?"
                        font.pixelSize: 30
                        color: "white"
                    }
                }
            }
        }

        Item { height: 20 }

        // Bouton Annuler
        Button {
            Layout.preferredWidth: 200
            Layout.preferredHeight: 50
            Layout.alignment: Qt.AlignHCenter

            background: Rectangle {
                color: parent.down ? "#cc0000" : (parent.hovered ? "#ff3333" : "#ff0000")
                radius: 5
                Behavior on color { ColorAnimation { duration: 150 } }
            }

            contentItem: Text {
                text: "Annuler"
                font.pixelSize: 18
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                networkManager.leaveMatchmaking()
                root.cancelClicked()
            }
        }
    }

    // Connexions aux signaux du NetworkManager
    Connections {
        target: networkManager
        
        function onGameFound(playerPosition, opponents) {
            console.log("Partie trouvée! Position:", playerPosition)
            // Passer à l'écran de jeu avec les infos
            stackView.push("qrc:/qml/CoincheView.qml", {
                "playerPosition": playerPosition,
                "opponents": opponents,
                "isOnline": true
            })
        }
    }

    Component.onCompleted: {
        if (networkManager.connected) {
            // S'enregistrer d'abord
            //networkManager.registerPlayer("Joueur" + Math.floor(Math.random() * 1000))
            //networkManager.registerPlayer(defaultPlayerName)
            // Puis rejoindre le matchmaking
            networkManager.joinMatchmaking()
        }
    }
}
