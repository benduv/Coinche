import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1024
    height: 768
    title: "Jeu de Coinche"

    Component.onCompleted: {
        // Positionner automatiquement la fenêtre au démarrage
        windowPositioner.positionWindow(mainWindow)
    }

    // Variable pour stocker le nom du joueur connecté
    property string loggedInPlayerName: ""
    property string accountType: ""

    // Ratio responsive pour adapter la taille des composants
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: loginViewComponent

        Component {
            id: loginViewComponent

            LoginView {
                onLoginSuccess: function(playerName, accType) {
                    mainWindow.loggedInPlayerName = playerName
                    mainWindow.accountType = accType
                    stackView.push(mainMenuComponent)
                }

                Component.onCompleted: {
                    networkManager.connectToServer("ws://localhost:1234")
                }
            }
        }

        Component {
            id: mainMenuComponent
            Rectangle {
                anchors.fill: parent
                color: "#1a1a1a"

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 30 * mainWindow.minRatio

                    Text {
                        text: "COINCHE"
                        font.pixelSize: 72 * mainWindow.minRatio
                        font.bold: true
                        color: "#FFD700"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: "Jeu de cartes multijoueur"
                        font.pixelSize: 20 * mainWindow.minRatio
                        color: "#aaaaaa"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Item { height: 40 * mainWindow.minRatio }

                    // Bouton Jouer
                    Button {
                        Layout.preferredWidth: 300 * mainWindow.widthRatio
                        Layout.preferredHeight: 160 * mainWindow.heightRatio
                        Layout.alignment: Qt.AlignHCenter
                        enabled: networkManager.connected

                        background: Rectangle {
                            color: parent.enabled ?
                                   (parent.down ? "#00aa00" : (parent.hovered ? "#00dd00" : "#00cc00")) :
                                   "#555555"
                            radius: 10 * mainWindow.minRatio
                            border.color: parent.enabled ? "#FFD700" : "#888888"
                            border.width: 3 * mainWindow.minRatio
                        }

                        contentItem: Text {
                            text: "JOUER"
                            font.pixelSize: 64 * mainWindow.minRatio
                            font.bold: true
                            color: parent.enabled ? "white" : "#aaaaaa"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            networkManager.registerPlayer(mainWindow.loggedInPlayerName, networkManager.playerAvatar)
                            stackView.push("qrc:/qml/MatchMakingView.qml")
                        }
                    }

                    // Bouton Statistiques (uniquement pour les comptes enregistrés)
                    Button {
                        Layout.preferredWidth: 300 * mainWindow.widthRatio
                        Layout.preferredHeight: 120 * mainWindow.heightRatio
                        Layout.alignment: Qt.AlignHCenter
                        visible: mainWindow.accountType !== "guest"

                        background: Rectangle {
                            color: parent.down ? "#0088cc" : (parent.hovered ? "#0099dd" : "#0077bb")
                            radius: 10 * mainWindow.minRatio
                            border.color: "#FFD700"
                            border.width: 2 * mainWindow.minRatio
                        }

                        contentItem: Text {
                            text: "Statistiques"
                            font.pixelSize: 48 * mainWindow.minRatio
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            stackView.push(statsViewComponent)
                        }
                    }

                    // Afficher l'avatar et le nom du joueur
                    Row {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 15 * mainWindow.minRatio

                        // Avatar du joueur
                        Rectangle {
                            width: 60 * mainWindow.minRatio
                            height: 60 * mainWindow.minRatio
                            radius: 30 * mainWindow.minRatio
                            color: "#444444"
                            border.color: "#FFD700"
                            border.width: 2 * mainWindow.minRatio
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                anchors.fill: parent
                                anchors.margins: 5 * mainWindow.minRatio
                                source: "qrc:/resources/avatar/" + networkManager.playerAvatar
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                            }
                        }

                        Text {
                            text: mainWindow.loggedInPlayerName +
                                  (mainWindow.accountType === "guest" ? " (Invité)" : "")
                            font.pixelSize: 20 * mainWindow.minRatio
                            font.bold: true
                            color: "#FFD700"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Item { height: 20 * mainWindow.minRatio }

                    // Statut connexion
                    Row {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 10 * mainWindow.minRatio

                        Rectangle {
                            width: 12 * mainWindow.minRatio
                            height: 12 * mainWindow.minRatio
                            radius: 6 * mainWindow.minRatio
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
                            font.pixelSize: 14 * mainWindow.minRatio
                            color: networkManager.connected ? "#00ff00" : "#ff6666"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }
        }

        Component {
            id: statsViewComponent

            StatsView {
                playerName: mainWindow.loggedInPlayerName

                onBackToMenu: {
                    stackView.pop()
                }
            }
        }
    }
}
