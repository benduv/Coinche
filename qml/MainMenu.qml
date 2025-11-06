import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 1024
    height: 768
    title: "Jeu de Coinche"

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: mainMenuComponent

        Component {
            id: mainMenuComponent
            Rectangle {
                anchors.fill: parent
                color: "#1a1a1a"

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 30

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
                        enabled: networkManager.connected

                        background: Rectangle {
                            color: parent.enabled ?
                                   (parent.down ? "#00aa00" : (parent.hovered ? "#00dd00" : "#00cc00")) :
                                   "#555555"
                            radius: 10
                            border.color: parent.enabled ? "#FFD700" : "#888888"
                            border.width: 3
                        }

                        contentItem: Text {
                            text: "JOUER"
                            font.pixelSize: 32
                            font.bold: true
                            color: parent.enabled ? "white" : "#aaaaaa"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            networkManager.registerPlayer(defaultPlayerName)
                            stackView.push("qrc:/qml/MatchMakingView.qml")
                        }
                    }

                    Item { height: 20 }

                    // Statut connexion
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
                            text: networkManager.connected ? "Connecté" : "Déconnecté"
                            font.pixelSize: 14
                            color: networkManager.connected ? "#00ff00" : "#ff6666"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                Component.onCompleted: {
                    networkManager.connectToServer("ws://localhost:1234")
                }
            }
        }
    }
}
