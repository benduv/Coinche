import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    anchors.fill: parent
    color: "#0a0a1a"  // Fond plus sombre pour mieux voir les étoiles

    signal cancelClicked()

    // Si true, appelle joinMatchmaking automatiquement (mode normal)
    // Si false, le matchmaking a déjà été lancé (mode lobby)
    property bool autoJoin: true

    // Ratios pour le responsive
    property real widthRatio: width / 1920
    property real heightRatio: height / 1080
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Étoiles fixes scintillantes en arrière-plan
    Repeater {
        model: 50
        Rectangle {
            x: Math.random() * root.width
            y: Math.random() * root.height
            width: (Math.random() * 3 + 2) * root.minRatio
            height: width
            radius: width / 2
            color: "white"
            opacity: 0.5
            z: 0

            SequentialAnimation on opacity {
                running: true
                loops: Animation.Infinite
                PauseAnimation { duration: Math.random() * 2000 }
                NumberAnimation { to: 0.9; duration: 1000 + Math.random() * 1000 }
                NumberAnimation { to: 0.3; duration: 1000 + Math.random() * 1000 }
            }
        }
    }

    // Étoiles filantes en arrière-plan
    Item {
        id: shootingStarsLayer
        anchors.fill: parent
        z: 1

        Timer {
            id: shootingStarTimer
            running: true
            repeat: true
            interval: 2000 + Math.random() * 3000  // Intervalle initial
            onTriggered: {
                // Créer une nouvelle étoile filante
                var component = Qt.createComponent("qrc:/qml/ShootingStar.qml")
                if (component.status === Component.Ready) {
                    var star = component.createObject(shootingStarsLayer, {
                        "startX": Math.random() * root.width,
                        "startY": 0,
                        "minRatio": root.minRatio,
                        "starWidth": (200 + Math.random() * 200) * root.minRatio,  // Entre 200 et 400
                        "starHeight": (4 + Math.random() * 4) * root.minRatio,     // Entre 4 et 8
                        "starRotation": 20 + Math.random() * 140  // Entre 20 et 160 degrés
                    })
                    if (star) {
                        star.start()
                    }
                }

                // Recalculer un nouvel intervalle aléatoire pour le prochain déclenchement
                shootingStarTimer.interval = 2000 + Math.random() * 3000
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 40 * root.minRatio
        z: 10  // Au-dessus des étoiles

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

            // Icône centrale - Saturne
            Image {
                width: 150 * root.minRatio
                height: 150 * root.minRatio
                anchors.centerIn: parent
                source: "qrc:/resources/saturn-svgrepo-com.svg"
                fillMode: Image.PreserveAspectFit
                smooth: true

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
            font.pixelSize: 46 * root.minRatio
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
                console.log("4 joueurs trouvés! En attente du GameModel...")
                // MainMenu gère la création du GameModel et le chargement de CoincheView
            }
        }

        // Note: onGameFound est géré par MainMenu qui appelle createGameModel
        // Ne pas dupliquer l'appel ici pour éviter de créer 2 instances de CoincheView
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
