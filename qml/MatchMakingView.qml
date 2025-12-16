import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    anchors.fill: parent
    color: "#1a1a1a"

    signal cancelClicked()

    // Si true, appelle joinMatchmaking automatiquement (mode normal)
    // Si false, le matchmaking a déjà été lancé (mode lobby)
    property bool autoJoin: true

    // Ratios pour le responsive
    property real widthRatio: width / 1920
    property real heightRatio: height / 1080
    property real minRatio: Math.min(widthRatio, heightRatio)

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 40 * root.minRatio

        // Animation de recherche
        Item {
            Layout.preferredWidth: 300 * root.minRatio
            Layout.preferredHeight: 300 * root.minRatio
            Layout.alignment: Qt.AlignHCenter

            // Cercles animés
            Repeater {
                model: 3
                Rectangle {
                    width: (270 - (index * 60)) * root.minRatio
                    height: width
                    radius: width / 2
                    color: "transparent"
                    border.color: "#00cc00"
                    border.width: 5 * root.minRatio
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
                font.pixelSize: 120 * root.minRatio
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
            font.pixelSize: 32 * root.minRatio
            font.bold: true
            color: "#FFD700"
            Layout.alignment: Qt.AlignHCenter
        }

        // Nombre de joueurs
        Rectangle {
            Layout.preferredWidth: 800 * root.widthRatio
            Layout.preferredHeight: 200 * root.heightRatio
            Layout.alignment: Qt.AlignHCenter
            color: "#2a2a2a"
            radius: 10 * root.minRatio
            border.color: "#00cc00"
            border.width: 3 * root.minRatio

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 20 * root.minRatio

                Text {
                    text: networkManager.playersInQueue + " / 4 joueurs"
                    font.pixelSize: 48 * root.minRatio
                    font.bold: true
                    color: "#00ff00"
                    Layout.alignment: Qt.AlignHCenter
                }

                // Barre de progression
                Rectangle {
                    Layout.preferredWidth: 600 * root.widthRatio
                    Layout.preferredHeight: 20 * root.heightRatio
                    Layout.alignment: Qt.AlignHCenter
                    color: "#444444"
                    radius: 5 * root.minRatio

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
            spacing: 20 * root.minRatio

            Repeater {
                model: 4
                Rectangle {
                    width: 120 * root.minRatio
                    height: 120 * root.minRatio
                    radius: 60 * root.minRatio
                    color: index < networkManager.playersInQueue ? "#00cc00" : "#333333"
                    border.color: "#FFD700"
                    border.width: 2 * root.minRatio

                    Behavior on color {
                        ColorAnimation { duration: 300 }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: index < networkManager.playersInQueue ? "✓" : "?"
                        font.pixelSize: 60 * root.minRatio
                        color: "white"
                    }
                }
            }
        }

        // Bouton Annuler
        Button {
            Layout.preferredWidth: 250 * root.widthRatio
            Layout.preferredHeight: 120 * root.heightRatio
            Layout.alignment: Qt.AlignHCenter

            background: Rectangle {
                color: parent.down ? "#cc0000" : (parent.hovered ? "#ff3333" : "#ff0000")
                radius: 5 * root.minRatio
                Behavior on color { ColorAnimation { duration: 150 } }
            }

            contentItem: Text {
                text: "Annuler"
                font.pixelSize: 36 * root.minRatio
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                networkManager.leaveMatchmaking()
                stackView.pop()  // Retour au MainMenu
            }
        }
    }

    Connections {
        target: networkManager

        function onPlayersInQueueChanged() {
            if (networkManager.playersInQueue === 4) {
                console.log("4 joueurs trouvés! Affichage de l'écran de chargement")
                // Pusher l'écran de chargement immédiatement
                var mainWindow = root.Window.window
                if (mainWindow) {
                    mainWindow.shouldLoadCoincheView = false
                    stackView.push(coincheViewLoaderComponent)
                }
            }
        }

        function onGameFound(playerPosition, opponents) {
            console.log("Partie trouvee! Position:", playerPosition)
            networkManager.createGameModel(
                networkManager.myPosition,
                networkManager.myCards,
                networkManager.opponents
            )
        }

        function onGameModelReady() {
            console.log("QML: GameModel prêt, activation du chargement de CoincheView")
            // Activer le Loader après un délai
            loadDelayTimer.start()
        }
    }

    // Timer pour retarder l'activation du Loader
    Timer {
        id: loadDelayTimer
        interval: 200
        repeat: false
        onTriggered: {
            console.log("Activation du chargement de CoincheView depuis MatchMakingView")
            var mainWindow = root.Window.window
            if (mainWindow) {
                mainWindow.shouldLoadCoincheView = true
            }
        }
    }

    // Component pour l'écran de chargement avec Loader
    Component {
        id: coincheViewLoaderComponent

        Rectangle {
            anchors.fill: parent
            color: "#1a472a"

            // Indicateur de chargement
            Column {
                anchors.centerIn: parent
                spacing: 30
                visible: !coincheLoader.active

                // Icône de joueurs trouvés (4 cercles représentant les joueurs)
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 15

                    Repeater {
                        model: 4
                        Rectangle {
                            width: 50
                            height: 50
                            radius: 25
                            color: "#444444"
                            border.color: "#FFD700"
                            border.width: 3

                            Text {
                                anchors.centerIn: parent
                                text: "✓"
                                font.pixelSize: 30
                                color: "#00ff00"
                                font.bold: true
                            }

                            // Animation de pulsation
                            SequentialAnimation on scale {
                                running: true
                                loops: Animation.Infinite
                                NumberAnimation { to: 1.1; duration: 500; easing.type: Easing.InOutQuad }
                                NumberAnimation { to: 1.0; duration: 500; easing.type: Easing.InOutQuad }
                                PauseAnimation { duration: index * 200 }
                            }
                        }
                    }
                }

                Text {
                    text: "Joueurs trouvés !"
                    font.pixelSize: 36
                    font.bold: true
                    color: "#FFD700"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    text: "Lancement de la partie..."
                    font.pixelSize: 28
                    color: "#aaaaaa"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // Points de suspension animés
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 8

                    Repeater {
                        model: 3
                        Rectangle {
                            width: 10
                            height: 10
                            radius: 5
                            color: "#FFD700"

                            SequentialAnimation on opacity {
                                running: true
                                loops: Animation.Infinite
                                PauseAnimation { duration: index * 200 }
                                NumberAnimation { to: 0.3; duration: 400 }
                                NumberAnimation { to: 1.0; duration: 400 }
                                PauseAnimation { duration: (2 - index) * 200 }
                            }
                        }
                    }
                }
            }

            // Loader qui ne charge CoincheView que lorsque shouldLoadCoincheView est true
            Loader {
                id: coincheLoader
                anchors.fill: parent
                active: {
                    var mainWindow = root.Window.window
                    return mainWindow ? mainWindow.shouldLoadCoincheView : false
                }

                sourceComponent: Component {
                    CoincheView {}
                }

                onLoaded: {
                    console.log("CoincheView chargé avec succès depuis MatchMakingView!")
                }
            }
        }
    }

    Component.onCompleted: {
        if (networkManager.connected && autoJoin) {
            console.log("MatchmakingView - Rejoindre le matchmaking (mode normal)")
            networkManager.joinMatchmaking()
        } else if (!autoJoin) {
            console.log("MatchmakingView - Matchmaking déjà lancé (mode lobby)")
        }
    }
}
