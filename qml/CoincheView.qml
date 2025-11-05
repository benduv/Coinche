import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 1024
    height: 768
    title: "Jeu de Coinche"
    color: "#2d5016"

    Rectangle {
        id: rootArea
        anchors.fill: parent
        color: "#2d5016"

        // =====================
        // ZONE CENTRALE DE JEU
        // =====================
        Rectangle {
            id: playArea
            anchors.centerIn: parent
            width: parent.width * 0.6
            height: parent.height * 0.5
            color: "#1a3d0f"
            radius: 10
            border.color: "#8b6914"
            border.width: 3

            // ---- Panneau d'annonces ----
            AnnoncesPanel {
                anchors.fill: parent
                anchors.margins: parent.width * 0.05
                visible: gameModel.biddingPhase
            }

            // ---- Zone du pli (cartes jouées) ----
            Item {
                id: pliArea
                anchors.fill: parent

                Repeater {
                    id: pliRepeater
                    model: gameModel.currentPli

                    Card {
                        width: playArea.width * 0.15
                        height: playArea.height * 0.3
                        value: modelData.value
                        suit: modelData.suit
                        faceUp: true

                        // Position selon le joueur
                        x: {
                            switch (modelData.playerId) {
                                case 0: return pliArea.width / 2 - width / 2;            // Sud
                                case 1: return pliArea.width / 2 - width / 2 - width * 1.6; // Ouest
                                case 2: return pliArea.width / 2 - width / 2;            // Nord
                                case 3: return pliArea.width / 2 - width / 2 + width * 1.6; // Est
                            }
                        }
                        y: {
                            switch (modelData.playerId) {
                                case 0: return pliArea.height / 2 + height * 0.6; // Sud
                                case 1: return pliArea.height / 2;               // Ouest
                                case 2: return pliArea.height / 2 - height * 1.3; // Nord
                                case 3: return pliArea.height / 2;               // Est
                            }
                        }

                        Behavior on x { NumberAnimation { duration: 400; easing.type: Easing.InOutQuad } }
                        Behavior on y { NumberAnimation { duration: 400; easing.type: Easing.InOutQuad } }
                    }
                }
            }
        }

        // =====================
        // JOUEUR SUD (vous)
        // =====================
        Column {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: parent.height * 0.05
            spacing: parent.height * 0.01

            Row {
                id: playerSouth
                spacing: parent.width * 0.03
                Repeater {
                    model: gameModel.player0Hand
                    Card {
                        width: {
                            var desiredHeight = rootArea.height * 0.13
                            return desiredHeight * cardRatio
                        }
                        height: rootArea.height * 0.12
                        value: model.value
                        suit: model.suit
                        faceUp: model.faceUp
                        isPlayable: model.isPlayable
                        enabled: gameModel.currentPlayer === 0

                        MouseArea {
                            anchors.fill: parent
                            enabled: gameModel.currentPlayer === 0 && model.isPlayable
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                            onClicked: gameModel.playCard(0, index)
                        }
                    }
                }
            }

            Text {
                //text: gameModel.player0Hand.m_player ? "Sud (Vous)" : "Sud"
                text: defaultPlayerName
                color: gameModel.currentPlayer === 0 ? "#ffff66" : "white"
                font.pixelSize: parent.height * 0.1
                font.bold: gameModel.currentPlayer === 0
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        // =====================
        // JOUEUR NORD
        // =====================
        Column {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: parent.height * 0.05
            spacing: parent.height * 0.01

            Text {
                text: defaultPlayerName
                color: gameModel.currentPlayer === 2 ? "#ffff66" : "white"
                font.pixelSize: parent.height * 0.1
                font.bold: gameModel.currentPlayer === 2
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Row {
                id: playerNorth
                spacing: parent.width * 0.03
                Repeater {
                    model: gameModel.player2Hand
                    Card {
                        width: {
                            var desiredHeight = rootArea.height * 0.13
                            return desiredHeight * cardRatio
                        }
                        height: rootArea.height * 0.12
                        value: model.value
                        suit: model.suit
                        faceUp: true
                        isPlayable: model.isPlayable
                        enabled: gameModel.currentPlayer === 2

                        MouseArea {
                            anchors.fill: parent
                            enabled: gameModel.currentPlayer === 2 && model.isPlayable
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                            onClicked: gameModel.playCard(2, index)
                        }
                    }
                }
            }
        }

        // =====================
        // JOUEUR OUEST
        // =====================
        Column {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: parent.width * 0.03
            spacing: parent.height * 0.008
            rotation: 0

            Text {
                text: "Ouest"
                color: gameModel.currentPlayer === 1 ? "#ffff66" : "white"
                font.pixelSize: parent.height * 0.03
                font.bold: gameModel.currentPlayer === 1
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Column {
                id: playerWest
                spacing: - rootArea.height * 0.008 // écart plus large pour tout afficher
                Repeater {
                    model: gameModel.player1Hand
                    Card {
                        width: {
                            var desiredHeight = rootArea.height * 0.1
                            return desiredHeight * cardRatio
                        }
                        height: rootArea.height * 0.1
                        rotation: 90
                        faceUp: true
                        value: model.value
                        suit: model.suit
                        isPlayable: model.isPlayable
                        enabled: gameModel.currentPlayer === 1

                        MouseArea {
                            anchors.fill: parent
                            enabled: gameModel.currentPlayer === 1 && model.isPlayable
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                            onClicked: gameModel.playCard(1, index)
                        }
                    }
                }
            }
        }

        // =====================
        // JOUEUR EST
        // =====================
        Column {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: parent.width * 0.03
            spacing: parent.height * 0.008

            Text {
                text: "Est"
                color: gameModel.currentPlayer === 3 ? "#ffff66" : "white"
                font.pixelSize: parent.height * 0.03
                font.bold: gameModel.currentPlayer === 3
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Column {
                id: playerEast
                spacing: - rootArea.height * 0.008
                Repeater {
                    model: gameModel.player3Hand
                    Card {
                        width: {
                            var desiredHeight = rootArea.height * 0.1
                            return desiredHeight * cardRatio
                        }
                        height: rootArea.height * 0.1
                        rotation: -90
                        faceUp: true
                        value: model.value
                        suit: model.suit
                        isPlayable: model.isPlayable
                        enabled: gameModel.currentPlayer === 3

                        MouseArea {
                            anchors.fill: parent
                            enabled: gameModel.currentPlayer === 3 && model.isPlayable
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                            onClicked: gameModel.playCard(3, index)
                        }
                    }
                }
            }
        }

        // =====================
        // PANNEAU DE SCORE
        // =====================
        Rectangle {
            id: scorePanel
            width: parent.width * 0.15
            height: parent.height * 0.15
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: parent.width * 0.02
            color: "#1a1a1a"
            opacity: 0.85
            radius: 8
            border.color: "#aaaaaa"

            GridLayout {
                anchors.fill: parent
                anchors.margins: parent.width * 0.015
                rows: 3
                columns: 3
                rowSpacing: parent.height * 0.03
                columnSpacing: parent.width * 0.02

                Text { text: "Score"; color: "white"; font.pixelSize: parent.height * 0.12; font.bold: true }
                Text { text: "Manche"; color: "white"; font.pixelSize: parent.height * 0.08; font.bold: true }
                Text { text: "Total"; color: "white"; font.pixelSize: parent.height * 0.08; font.bold: true }

                Text { text: "Équipe 1:"; color: "white"; font.pixelSize: parent.height * 0.08 }
                Text { text: gameModel.scoreTeam1; color: "white"; font.pixelSize: parent.height * 0.08 }
                Text { text: gameModel.scoreTotalTeam1; color: "white"; font.pixelSize: parent.height * 0.08 }

                Text { text: "Équipe 2:"; color: "white"; font.pixelSize: parent.height * 0.08 }
                Text { text: gameModel.scoreTeam2; color: "white"; font.pixelSize: parent.height * 0.08 }
                Text { text: gameModel.scoreTotalTeam2; color: "white"; font.pixelSize: parent.height * 0.08 }
            }
        }
    }
}
