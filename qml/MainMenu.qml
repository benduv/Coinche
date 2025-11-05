import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 1024
    height: 768
    title: "Jeu de Coinche"
    color: "#2d5016"

    // Définir un StackView comme élément racine
    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: mainMenuComponent

        Component {
            id: mainMenuComponent
            Rectangle {
                id: root
                anchors.fill: parent
                color: "#1a1a1a"
                signal settingsClicked()

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 30
                    // Logo / Titre
                    Text {
                        text: "COINCHE"
                        font.pixelSize: 72
                        font.bold: true
                        color: "#FFD700"
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: "Jeu de cartes multijoueur"
                        font.pixelSize: 20
                        color: "#aaaaaa"
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Item { height: 40 }
                    // Bouton Jouer
                    Button {
                        Layout.preferredWidth: 300
                        Layout.preferredHeight: 80
                        Layout.alignment: Qt.AlignHCenter
                        background: Rectangle {
                            color: parent.down ? "#00aa00" : (parent.hovered ? "#00dd00" : "#00cc00")
                            radius: 10
                            border.color: "#FFD700"
                            border.width: 3
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                        contentItem: Text {
                            text: "JOUER"
                            font.pixelSize: 32
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            // Enregistrer le nom et rejoindre
                            if (networkManager.connected) {
                                //networkManager.registerPlayer(playerNameField.text)
                                networkManager.registerPlayer(defaultPlayerName)
                            }
                            stackView.push("qrc:/qml/MatchMakingView.qml")
                        }
                    }
                    // Bouton Mode Solo (test)
                    Button {
                        Layout.preferredWidth: 300
                        Layout.preferredHeight: 60
                        Layout.alignment: Qt.AlignHCenter
                        background: Rectangle {
                            color: parent.down ? "#666666" : (parent.hovered ? "#888888" : "#777777")
                            radius: 10
                            border.color: "#aaaaaa"
                            border.width: 2
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                        contentItem: Text {
                            text: "Mode Solo (Test)"
                            font.pixelSize: 20
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            stackView.push("qrc:/qml/CoincheView.qml")
                        }
                    }
                    // Bouton Paramètres
                    Button {
                        Layout.preferredWidth: 300
                        Layout.preferredHeight: 60
                        Layout.alignment: Qt.AlignHCenter
                        background: Rectangle {
                            color: parent.down ? "#333333" : (parent.hovered ? "#555555" : "#444444")
                            radius: 10
                            border.color: "#888888"
                            border.width: 2
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                        contentItem: Text {
                            text: "Paramètres"
                            font.pixelSize: 20
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: root.settingsClicked()
                    }
                    Item { height: 20 }
                    // Statut de connexion
                    Row {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 10
                        Rectangle {
                            width: 12
                            height: 12
                            radius: 6
                            color: networkManager.connected ? "#00ff00" : "#ff0000"
                            anchors.verticalCenter: parent.verticalCenter
                            SequentialAnimation on opacity {
                                running: !networkManager.connected
                                loops: Animation.Infinite
                                NumberAnimation { to: 0.3; duration: 500 }
                                NumberAnimation { to: 1.0; duration: 500 }
                            }
                        }
                        Text {
                            text: networkManager.connected ? "Connecte au serveur" : "Deconnecte"
                            font.pixelSize: 14
                            color: networkManager.connected ? "#00ff00" : "#ff6666"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
                // Connexion automatique au serveur au démarrage
                Component.onCompleted: {
                    console.log("MainMenu completed")
                    networkManager.connectToServer("ws://localhost:1234")
                }
            }
        }
    }
}
