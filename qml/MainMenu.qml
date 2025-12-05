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

                // Animations de fond - Colonnes de symboles de cartes
                // Colonne 1 (gauche) - Coeurs montant
                Column {
                    id: heartsColumn
                    x: parent.width * 0.08
                    y: -parent.height
                    spacing: 80 * mainWindow.minRatio
                    z: 0
                    opacity: 0.15

                    Repeater {
                        model: 15
                        Text {
                            text: "♥"
                            font.pixelSize: 60 * mainWindow.minRatio
                            color: "#FF0000"
                        }
                    }

                    SequentialAnimation on y {
                        running: true
                        loops: Animation.Infinite
                        NumberAnimation {
                            from: parent.height
                            to: -heartsColumn.height
                            duration: 15000
                        }
                    }
                }

                // Colonne 2 (gauche-centre) - Trèfles descendant
                Column {
                    id: clubsColumn
                    x: parent.width * 0.23
                    y: -parent.height
                    spacing: 80 * mainWindow.minRatio
                    z: 0
                    opacity: 0.15

                    Repeater {
                        model: 15
                        Text {
                            text: "♣"
                            font.pixelSize: 60 * mainWindow.minRatio
                            color: "#000000"
                        }
                    }

                    SequentialAnimation on y {
                        running: true
                        loops: Animation.Infinite
                        NumberAnimation {
                            from: -clubsColumn.height
                            to: parent.height
                            duration: 18000
                        }
                    }
                }

                // Colonne 3 (droite-centre) - Carreaux descendant
                Column {
                    id: diamondsColumn
                    x: parent.width * 0.77
                    y: -parent.height
                    spacing: 80 * mainWindow.minRatio
                    z: 0
                    opacity: 0.15

                    Repeater {
                        model: 15
                        Text {
                            text: "♦"
                            font.pixelSize: 60 * mainWindow.minRatio
                            color: "#FF0000"
                        }
                    }

                    SequentialAnimation on y {
                        running: true
                        loops: Animation.Infinite
                        NumberAnimation {
                            from: -diamondsColumn.height
                            to: parent.height
                            duration: 16000
                        }
                    }
                }

                // Colonne 4 (droite) - Piques montant
                Column {
                    id: spadesColumn
                    x: parent.width * 0.92
                    y: -parent.height
                    spacing: 80 * mainWindow.minRatio
                    z: 0
                    opacity: 0.15

                    Repeater {
                        model: 15
                        Text {
                            text: "♠"
                            font.pixelSize: 60 * mainWindow.minRatio
                            color: "#000000"
                        }
                    }

                    SequentialAnimation on y {
                        running: true
                        loops: Animation.Infinite
                        NumberAnimation {
                            from: parent.height
                            to: -spadesColumn.height
                            duration: 17000
                        }
                    }
                }

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 30 * mainWindow.minRatio
                    z: 1

                    Text {
                        text: "COINCHE"
                        font.pixelSize: 72 * mainWindow.minRatio
                        font.bold: true
                        color: "#FFD700"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: "Bienvenue !"
                        font.pixelSize: 36 * mainWindow.minRatio
                        color: "#aaaaaa"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    // Afficher l'avatar et le nom du joueur
                    Row {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 15 * mainWindow.minRatio

                        // Avatar du joueur
                        Rectangle {
                            width: 120 * mainWindow.minRatio
                            height: 120 * mainWindow.minRatio
                            radius: 60 * mainWindow.minRatio
                            color: "#444444"
                            border.color: "#FFD700"
                            border.width: 2 * mainWindow.minRatio
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                anchors.fill: parent
                                anchors.margins: 15 * mainWindow.minRatio
                                source: "qrc:/resources/avatar/" + networkManager.playerAvatar
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                            }
                        }

                        Text {
                            text: mainWindow.loggedInPlayerName +
                                  (mainWindow.accountType === "guest" ? " (Invité)" : "")
                            font.pixelSize: 42 * mainWindow.minRatio
                            font.bold: true
                            color: "#FFD700"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    //Item { height: 10 * mainWindow.minRatio }

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
                            font.pixelSize: 20 * mainWindow.minRatio
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
