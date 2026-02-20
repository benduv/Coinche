import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    anchors.fill: parent
    color: "#1a472a"

    Rectangle { // Solution du pauvre pour mettre en noire la bar du haut de l'ecran (selfie)
        anchors.left: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width / 5
        height: parent.height
        color: "black"
    }

    Rectangle {
        anchors.right: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width / 5
        height: parent.height
        color: "black"
    }

    property string lobbyCode: ""
    property bool isHost: false

    // Ratios pour le responsive
    property real widthRatio: width / 1920
    property real heightRatio: height / 1080
    property real minRatio: Math.min(widthRatio, heightRatio)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 40 * root.minRatio
        spacing: 30 * root.minRatio

        // En-tête avec code du lobby
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 180 * root.heightRatio
            color: "#2a2a2a"
            radius: 20 * root.minRatio
            border.color: "#FFD700"
            border.width: 3 * root.minRatio

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 8 * root.minRatio

                Text {
                    text: "Code du lobby"
                    font.pixelSize: 28 * root.minRatio
                    color: "#aaaaaa"
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: root.lobbyCode
                    font.pixelSize: 64 * root.minRatio
                    font.bold: true
                    color: "#FFD700"
                    font.family: "Courier"
                    font.letterSpacing: 12
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: root.isHost ? "👑 Vous êtes l'hôte" : "Partagez ce code !"
                    font.pixelSize: 32 * root.minRatio
                    color: root.isHost ? "#FFD700" : "#888888"
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        // Zone des joueurs
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 530 * root.heightRatio
            color: "#2a2a2a"
            radius: 20 * root.minRatio
            border.color: "#FFD700"
            border.width: 3 * root.minRatio

            ColumnLayout {
                id: lobbyLayout
                anchors.fill: parent
                anchors.margins: 25 * root.minRatio
                spacing: 20 * root.minRatio

                Text {
                    text: "Joueurs (" + networkManager.lobbyPlayers.length + "/4)"
                    font.pixelSize: 32 * root.minRatio
                    font.bold: true
                    color: "#FFD700"
                }

                // Liste verticale des joueurs
                Column {
                    id: playerColumn
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 12 * root.minRatio

                    Repeater {
                        model: networkManager.lobbyPlayers

                        Rectangle {
                            width: lobbyLayout.width
                            height: 90 * root.minRatio
                            color: "#3a4a3a"
                            radius: 15 * root.minRatio
                            border.color: "#4CAF50"
                            border.width: 3 * root.minRatio

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 15 * root.minRatio
                                spacing: 15 * root.minRatio

                                // Avatar avec cercle
                                Rectangle {
                                    Layout.preferredWidth: 60 * root.minRatio
                                    Layout.preferredHeight: 60 * root.minRatio
                                    radius: 30 * root.minRatio
                                    color: "#444444"
                                    border.color: "#FFD700"
                                    border.width: 2 * root.minRatio

                                    Image {
                                        anchors.fill: parent
                                        anchors.margins: 8 * root.minRatio
                                        source: "qrc:/resources/avatar/" + modelData.avatar
                                        fillMode: Image.PreserveAspectFit
                                    }
                                }

                                // Nom du joueur
                                Text {
                                    text: modelData.name
                                    font.pixelSize: 32 * root.minRatio
                                    font.bold: true
                                    color: "white"
                                    Layout.fillWidth: true
                                }

                                // Badge hôte
                                Text {
                                    text: "👑"
                                    font.pixelSize: 32 * root.minRatio
                                    visible: modelData.isHost
                                    Layout.preferredWidth: 35 * root.minRatio
                                }

                                // Indicateur ready
                                Rectangle {
                                    Layout.preferredWidth: 16 * root.minRatio
                                    Layout.preferredHeight: 16 * root.minRatio
                                    radius: 8 * root.minRatio
                                    color: modelData.ready ? "#4CAF50" : "#FF5252"

                                    SequentialAnimation on scale {
                                        running: modelData.ready
                                        loops: Animation.Infinite
                                        NumberAnimation { to: 1.3; duration: 600 }
                                        NumberAnimation { to: 1.0; duration: 600 }
                                    }
                                }

                                // Texte statut
                                Text {
                                    text: modelData.ready ? "Prêt ✓" : "En attente..."
                                    font.pixelSize: 32 * root.minRatio
                                    font.bold: modelData.ready
                                    color: modelData.ready ? "#4CAF50" : "#FFA726"
                                    Layout.preferredWidth: 200 * root.minRatio
                                }
                            }
                        }
                    }
                }
            }
        }

        // Message d'information
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 55 * root.heightRatio
            color: {
                var count = networkManager.lobbyPlayers.length
                if (count === 1 || count === 3) return "#5D4037"
                else if (count === 2) return "#1565C0"
                else if (count === 4) return "#2E7D32"
                return "transparent"
            }
            radius: 10 * root.minRatio
            visible: networkManager.lobbyPlayers.length > 0

            Text {
                anchors.centerIn: parent
                text: {
                    var count = networkManager.lobbyPlayers.length
                    if (count === 1 || count === 3) {
                        return "⚠️  Il faut 2 ou 4 joueurs pour lancer une partie"
                    } else if (count === 2) {
                        return "ℹ️  A 2 joueurs, vous serez partenaires contre d'autres joueurs"
                    } else if (count === 4) {
                        return "✓  Lobby complet ! Tous prêts ?"
                    }
                    return ""
                }
                font.pixelSize: 32 * root.minRatio
                font.bold: true
                color: "white"
            }
        }

        // Boutons
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 120 * root.heightRatio
            spacing: 40 * root.minRatio

            // Bouton Prêt / Annuler
            Button {
                id: readyButton
                Layout.fillWidth: true
                Layout.fillHeight: true

                property bool isReady: false

                background: Rectangle {
                    color: readyButton.isReady ?
                           (parent.down ? "#E64A19" : (parent.hovered ? "#FF6F00" : "#FF5722")) :
                           (parent.down ? "#2E7D32" : (parent.hovered ? "#43A047" : "#4CAF50"))
                    radius: 12 * root.minRatio
                    border.color: readyButton.isReady ? "#BF360C" : "#1B5E20"
                    border.width: 2 * root.minRatio

                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                contentItem: Row {
                    anchors.centerIn: parent
                    spacing: 10 * root.minRatio

                    Text {
                        text: readyButton.isReady ? "✗" : "✓"
                        font.pixelSize: 36 * root.minRatio
                        font.bold: true
                        color: "white"
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: readyButton.isReady ? "Annuler" : "Prêt"
                        font.pixelSize: 32 * root.minRatio
                        font.bold: true
                        color: "white"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                onClicked: {
                    readyButton.isReady = !readyButton.isReady
                    networkManager.toggleLobbyReady(readyButton.isReady)
                }
            }

            // Bouton Jouer (visible seulement pour l'hôte)
            Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: root.isHost

                enabled: networkManager.lobbyPlayers.length === 2 || networkManager.lobbyPlayers.length === 4

                background: Rectangle {
                    color: parent.enabled ?
                           (parent.down ? "#1565C0" : (parent.hovered ? "#1E88E5" : "#2196F3")) :
                           "#424242"
                    radius: 12 * root.minRatio
                    border.color: parent.enabled ? "#0D47A1" : "#212121"
                    border.width: 2 * root.minRatio

                    Behavior on color { ColorAnimation { duration: 200 } }

                    // Animation de pulsation quand actif
                    SequentialAnimation on scale {
                        running: parent.parent.enabled
                        loops: Animation.Infinite
                        NumberAnimation { to: 1.05; duration: 800; easing.type: Easing.InOutQuad }
                        NumberAnimation { to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
                    }
                }

                contentItem: Row {
                    anchors.centerIn: parent
                    spacing: 12 * root.minRatio

                    Text {
                        text: "▶"
                        font.pixelSize: 36 * root.minRatio
                        font.bold: true
                        color: parent.parent.parent.enabled ? "white" : "#666666"
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "JOUER"
                        font.pixelSize: 36 * root.minRatio
                        font.bold: true
                        color: parent.parent.parent.enabled ? "white" : "#666666"
                        anchors.verticalCenter: parent.verticalCenter
                        font.letterSpacing: 2
                    }
                }

                onClicked: {
                    networkManager.startLobbyGame()
                }
            }

            // Bouton Quitter
            Button {
                Layout.preferredWidth: 260 * root.widthRatio
                Layout.fillHeight: true

                background: Rectangle {
                    color: parent.down ? "#B71C1C" : (parent.hovered ? "#D32F2F" : "#C62828")
                    radius: 12 * root.minRatio
                    border.color: "#7F0000"
                    border.width: 2 * root.minRatio

                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                contentItem: Text {
                    text: "✗ Quitter"
                    font.pixelSize: 36 * root.minRatio
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    networkManager.leaveLobby()
                    stackView.pop()
                    stackView.pop()
                }
            }
        }
    }

    // Surveiller les changements de lobbyPlayers
    Connections {
        target: networkManager
        function onLobbyPlayersChanged() {
        }

        function onLobbyError(errorMessage) {
            errorPopup.errorText = errorMessage
            errorPopup.open()
        }

        function onLobbyGameStarting() {
            var playerCount = networkManager.lobbyPlayers.length

            if (playerCount === 2) {
                // Lobby à 2 joueurs: pousser MatchMakingView pour attendre 2 autres joueurs
                stackView.push("qrc:/qml/MatchMakingView.qml", { autoJoin: false })
            } else if (playerCount === 4) {
                // Lobby à 4 joueurs: ne rien faire, gameFound arrivera immédiatement
                // et sera géré par MainMenu qui chargera directement CoincheView
            }
        }
    }

    // Popup d'erreur
    Popup {
        id: errorPopup
        anchors.centerIn: parent
        width: 600 * root.widthRatio
        height: 400 * root.heightRatio
        modal: true
        focus: true

        property string errorText: ""

        background: Rectangle {
            color: "#2a2a2a"
            radius: 15 * root.minRatio
            border.color: "#ff0000"
            border.width: 3 * root.minRatio
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20 * root.minRatio
            spacing: 30 * root.minRatio

            Text {
                text: "Erreur"
                font.pixelSize: 36 * root.minRatio
                font.bold: true
                color: "#ff0000"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: errorPopup.errorText
                font.pixelSize: 36 * root.minRatio
                color: "white"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                width: 180 * root.widthRatio
                height: 120 * root.heightRatio

                background: Rectangle {
                    color: parent.down ? "#cc0000" : (parent.hovered ? "#ff3333" : "#ff0000")
                    radius: 8 * root.minRatio
                }

                contentItem: Text {
                    text: "OK"
                    font.pixelSize: 36 * root.minRatio
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: errorPopup.close()
            }
        }
    }

    Component.onCompleted: {
    }
}
