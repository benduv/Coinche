import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: rootArea
    anchors.fill: parent
    color: "#2d5016"

    // Fonction pour calculer la position visuelle d'un joueur
    // Retourne l'index visuel (0=sud, 1=ouest, 2=nord, 3=est)
    function getVisualPosition(actualPlayerIndex) {
        var myPos = gameModel.myPosition
        var relativePos = (actualPlayerIndex - myPos + 4) % 4
        return relativePos
    }

    // Fonction inverse: obtenir l'index reel d'un joueur a partir de sa position visuelle
    function getActualPlayerIndex(visualPosition) {
        var myPos = gameModel.myPosition
        return (visualPosition + myPos) % 4
    }

    // Obtenir le nom d'un joueur par son index reel
    function getPlayerName(actualPlayerIndex) {
        // TODO: A remplacer par les vrais noms des joueurs quand disponibles
        var names = ["Joueur 1", "Joueur 2", "Joueur 3", "Joueur 4"]
        return names[actualPlayerIndex]
    }

        // =====================
        // ZONE CENTRALE DE JEU
        // =====================
        Rectangle {
            id: playArea
            //anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: playerNorthRow.bottom
            anchors.topMargin: parent.height * 0.005
            anchors.bottom: playerSouthRow.top
            anchors.bottomMargin: parent.height * 0.005
            anchors.right: playerEastColumn.left
            anchors.rightMargin: parent.width * 0.015
            anchors.left: playerWestColumn.right
            anchors.leftMargin: parent.width * 0.015
            color: "#1a3d0f"
            radius: 10
            border.color: "#8b6914"
            border.width: 3

            // ---- Panneau d'annonces ----
            AnnoncesPanel {
                anchors.fill: parent
                anchors.margins: parent.width * 0.05
                visible: gameModel.biddingPhase && gameModel.biddingPlayer === gameModel.myPosition
            }

            // ---- Zone du pli (cartes jouées) ----
            Item {
                id: pliArea
                anchors.fill: parent

                // Compteur de cartes pour detecter les nouvelles
                property int lastCardCount: 0

                Repeater {
                    id: pliRepeater
                    model: gameModel.currentPli

                    onCountChanged: {
                        console.log("Pli count changed: " + count + " (was: " + pliArea.lastCardCount + ")")
                        if (count === 0) {
                            pliArea.lastCardCount = 0
                        }
                    }

                    Card {
                        id: cardInPli
                        width: {
                            var desiredHeight = playArea.height * 0.4
                            return desiredHeight * cardRatio
                        }
                        height: playArea.height * 0.4
                        value: modelData.value
                        suit: modelData.suit
                        faceUp: true

                        // Flag pour savoir si cette carte doit animer
                        property bool willAnimate: false

                        // Position finale calculee
                        property real finalX: {
                            var visualPos = rootArea.getVisualPosition(modelData.playerId)
                            switch (visualPos) {
                                case 0: return pliArea.width / 2 - width / 2;            // Sud
                                case 1: return pliArea.width / 2 - width / 2 - width * 0.4; // Ouest
                                case 2: return pliArea.width / 2 - width / 2;            // Nord
                                case 3: return pliArea.width / 2 - width / 2 + width * 0.4; // Est
                            }
                        }
                        property real finalY: {
                            var visualPos = rootArea.getVisualPosition(modelData.playerId)
                            switch (visualPos) {
                                case 0: return pliArea.height / 2 - height * 0.15; // Sud
                                case 1: return pliArea.height / 2 - height * 0.5;               // Ouest
                                case 2: return pliArea.height / 2 - height * 0.85; // Nord
                                case 3: return pliArea.height / 2 - height * 0.5;               // Est
                            }
                        }
                        property real finalRotation: {
                            var visualPos = rootArea.getVisualPosition(modelData.playerId)
                            switch (visualPos) {
                                case 0: return 10;            // Sud
                                case 1: return 100; // Ouest
                                case 2: return -10;            // Nord
                                case 3: return -100; // Est
                            }
                        }

                        // Position initiale - LA SEULE PLACE OU ON DECIDE
                        Component.onCompleted: {
                            // Capturer IMMEDIATEMENT les valeurs dans des variables locales
                            var currentLastCount = pliArea.lastCardCount
                            var currentCount = pliRepeater.count
                            var currentIndex = index

                            // Decider MAINTENANT si c'est une nouvelle carte
                            var isNewCard = currentIndex === currentCount - 1 && currentCount > currentLastCount

                            console.log("Card " + currentIndex + ": isNewCard=" + isNewCard + " (index=" + currentIndex + ", count=" + currentCount + ", lastCount=" + currentLastCount + ")")

                            // Mettre a jour IMMEDIATEMENT pour les prochaines cartes
                            if (isNewCard) {
                                pliArea.lastCardCount = currentCount
                                console.log("  -> Updated lastCardCount to " + currentCount)
                            }

                            if (isNewCard) {
                                console.log("  -> Will ANIMATE (playerId=" + modelData.playerId + ")")
                                // IMPORTANT: Garder willAnimate = false pour le positionnement initial
                                willAnimate = false
                                opacity = 0

                                // Lancer l'animation (qui positionnera la carte au bon endroit)
                                animationTimer.start()
                            } else {
                                console.log("  -> Will position DIRECTLY")
                                willAnimate = false

                                // Carte existante: positionner directement SANS animation
                                x = finalX
                                y = finalY
                                rotation = finalRotation
                                opacity = 1
                            }
                        }

                        Timer {
                            id: animationTimer
                            interval: 10
                            running: false
                            repeat: false
                            onTriggered: {
                                // Calculer la position de depart MAINTENANT (dimensions disponibles)
                                var visualPos = rootArea.getVisualPosition(modelData.playerId)
                                var cardW = cardInPli.width
                                var cardH = cardInPli.height
                                var areaW = pliArea.width
                                var areaH = pliArea.height
                                console.log("Timer: visualPos=" + visualPos + ", pliArea=" + areaW + "x" + areaH + ", card=" + cardW + "x" + cardH)

                                // Positionner hors ecran selon la position du joueur
                                switch (visualPos) {
                                    case 0: // Sud - vient du bas
                                        cardInPli.x = areaW / 2 - cardW / 2
                                        cardInPli.y = areaH + cardH
                                        cardInPli.rotation = 180
                                        console.log("  Starting from BOTTOM: x=" + cardInPli.x + ", y=" + cardInPli.y)
                                        break
                                    case 1: // Ouest - vient de la gauche
                                        cardInPli.x = -cardW * 2
                                        cardInPli.y = areaH / 2 - cardH / 2
                                        cardInPli.rotation = 270
                                        console.log("  Starting from LEFT: x=" + cardInPli.x + ", y=" + cardInPli.y)
                                        break
                                    case 2: // Nord - vient du haut
                                        cardInPli.x = areaW / 2 - cardW / 2
                                        cardInPli.y = -cardH * 2
                                        cardInPli.rotation = 180
                                        console.log("  Starting from TOP: x=" + cardInPli.x + ", y=" + cardInPli.y)
                                        break
                                    case 3: // Est - vient de la droite
                                        cardInPli.x = areaW + cardW * 2
                                        cardInPli.y = areaH / 2 - cardH / 2
                                        cardInPli.rotation = -270
                                        console.log("  Starting from RIGHT: x=" + cardInPli.x + ", y=" + cardInPli.y)
                                        break
                                }

                                // Declencher l'animation vers la position finale
                                animateToFinalTimer.start()
                            }
                        }

                        Timer {
                            id: animateToFinalTimer
                            interval: 10
                            running: false
                            repeat: false
                            onTriggered: {
                                // ACTIVER les Behaviors pour l'animation
                                willAnimate = true

                                // Maintenant animer vers la position finale
                                cardInPli.x = cardInPli.finalX
                                cardInPli.y = cardInPli.finalY
                                cardInPli.rotation = cardInPli.finalRotation
                                cardInPli.opacity = 1
                                console.log("Animating to FINAL: x=" + cardInPli.finalX + ", y=" + cardInPli.finalY + ", rot=" + cardInPli.finalRotation)
                            }
                        }

                        Behavior on x {
                            enabled: willAnimate
                            NumberAnimation { duration: 600; easing.type: Easing.OutCubic }
                        }
                        Behavior on y {
                            enabled: willAnimate
                            NumberAnimation { duration: 600; easing.type: Easing.OutCubic }
                        }
                        Behavior on rotation {
                            enabled: willAnimate
                            NumberAnimation { duration: 600; easing.type: Easing.OutCubic }
                        }
                        Behavior on opacity {
                            enabled: willAnimate
                            NumberAnimation { duration: 400; easing.type: Easing.InOutQuad }
                        }
                    }

                    // Reinitialiser quand le pli est vide
                    Component.onCompleted: {
                        if (count === 0) {
                            pliArea.lastCardCount = 0
                        }
                    }
                }

                // Reinitialiser quand currentPli devient vide
                Connections {
                    target: gameModel
                    function onCurrentPliChanged() {
                        if (gameModel.currentPli.length === 0) {
                            console.log("Pli cleared, resetting lastCardCount")
                            pliArea.lastCardCount = 0
                        }
                    }
                }
            }
        }

        // =====================
        // JOUEUR SUD (vous)
        // =====================
        Row {
            id: playerSouthRow
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: - parent.height * 0.09
            spacing: parent.width * 0.01

            property int actualPlayerIndex: rootArea.getActualPlayerIndex(0)

            Text {
                text: rootArea.getPlayerName(playerSouthRow.actualPlayerIndex)
                color: gameModel.currentPlayer === playerSouthRow.actualPlayerIndex ? "#ffff66" : "white"
                font.pixelSize: rootArea.height * 0.025
                font.bold: gameModel.currentPlayer === playerSouthRow.actualPlayerIndex
                anchors.verticalCenter: parent.verticalCenter
            }

            Row {
                id: playerSouth
                spacing: - rootArea.height * 0.03
                Repeater {
                    model: {
                        switch (playerSouthRow.actualPlayerIndex) {
                            case 0: return gameModel.player0Hand
                            case 1: return gameModel.player1Hand
                            case 2: return gameModel.player2Hand
                            case 3: return gameModel.player3Hand
                        }
                    }
                    Card {
                        width: {
                            var desiredHeight = rootArea.height * 0.25
                            return desiredHeight * cardRatio
                        }
                        height: rootArea.height * 0.25
                        value: model.value
                        suit: model.suit
                        faceUp: model.faceUp
                        isPlayable: model.isPlayable
                        enabled: gameModel.currentPlayer === playerSouthRow.actualPlayerIndex

                        MouseArea {
                            anchors.fill: parent
                            enabled: gameModel.currentPlayer === playerSouthRow.actualPlayerIndex && model.isPlayable
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                            onClicked: gameModel.playCard(index)
                        }
                    }
                }
            }
        }

        // =====================
        // JOUEUR NORD (partenaire)
        // =====================
        Row {
            id: playerNorthRow
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: - parent.height * 0.06
            spacing: parent.width * 0.01

            property int actualPlayerIndex: rootArea.getActualPlayerIndex(2)

            Text {
                text: rootArea.getPlayerName(playerNorthRow.actualPlayerIndex)
                color: gameModel.currentPlayer === playerNorthRow.actualPlayerIndex ? "#ffff66" : "white"
                font.pixelSize: rootArea.height * 0.025
                font.bold: gameModel.currentPlayer === playerNorthRow.actualPlayerIndex
                anchors.verticalCenter: parent.verticalCenter
            }

            Row {
                id: playerNorth
                spacing: - rootArea.height * 0.025
                Repeater {
                    model: {
                        switch (playerNorthRow.actualPlayerIndex) {
                            case 0: return gameModel.player0Hand
                            case 1: return gameModel.player1Hand
                            case 2: return gameModel.player2Hand
                            case 3: return gameModel.player3Hand
                        }
                    }
                    Card {
                        width: {
                            var desiredHeight = rootArea.height * 0.125
                            return desiredHeight * cardRatio
                        }
                        height: rootArea.height * 0.125
                        value: model.value
                        suit: model.suit
                        faceUp: model.faceUp
                        isPlayable: model.isPlayable
                        enabled: gameModel.currentPlayer === playerNorthRow.actualPlayerIndex

                        MouseArea {
                            anchors.fill: parent
                            enabled: gameModel.currentPlayer === playerNorthRow.actualPlayerIndex && model.isPlayable
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                            onClicked: gameModel.playCard(index)
                        }
                    }
                }
            }
        }

        // =====================
        // JOUEUR OUEST (adversaire)
        // =====================
        Column {
            id: playerWestColumn
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: parent.width * 0.01
            spacing: parent.height * 0.008
            rotation: 0

            property int actualPlayerIndex: rootArea.getActualPlayerIndex(1)

            Text {
                text: rootArea.getPlayerName(playerWestColumn.actualPlayerIndex)
                color: gameModel.currentPlayer === playerWestColumn.actualPlayerIndex ? "#ffff66" : "white"
                font.pixelSize: parent.height * 0.03
                font.bold: gameModel.currentPlayer === playerWestColumn.actualPlayerIndex
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Column {
                id: playerWest
                spacing: - rootArea.height * 0.05
                Repeater {
                    model: {
                        switch (playerWestColumn.actualPlayerIndex) {
                            case 0: return gameModel.player0Hand
                            case 1: return gameModel.player1Hand
                            case 2: return gameModel.player2Hand
                            case 3: return gameModel.player3Hand
                        }
                    }
                    Card {
                        width: {
                            var desiredHeight = rootArea.height * 0.125
                            return desiredHeight * cardRatio
                        }
                        height: rootArea.height * 0.125
                        rotation: 90
                        faceUp: model.faceUp
                        value: model.value
                        suit: model.suit
                        isPlayable: model.isPlayable
                        enabled: gameModel.currentPlayer === playerWestColumn.actualPlayerIndex

                        MouseArea {
                            anchors.fill: parent
                            enabled: gameModel.currentPlayer === playerWestColumn.actualPlayerIndex && model.isPlayable
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                            onClicked: gameModel.playCard(index)
                        }
                    }
                }
            }
        }

        // =====================
        // JOUEUR EST (adversaire)
        // =====================
        Column {
            id: playerEastColumn
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: parent.width * 0.01
            spacing: parent.height * 0.008

            property int actualPlayerIndex: rootArea.getActualPlayerIndex(3)

            Text {
                text: rootArea.getPlayerName(playerEastColumn.actualPlayerIndex)
                color: gameModel.currentPlayer === playerEastColumn.actualPlayerIndex ? "#ffff66" : "white"
                font.pixelSize: parent.height * 0.03
                font.bold: gameModel.currentPlayer === playerEastColumn.actualPlayerIndex
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Column {
                id: playerEast
                spacing: - rootArea.height * 0.05
                Repeater {
                    model: {
                        switch (playerEastColumn.actualPlayerIndex) {
                            case 0: return gameModel.player0Hand
                            case 1: return gameModel.player1Hand
                            case 2: return gameModel.player2Hand
                            case 3: return gameModel.player3Hand
                        }
                    }
                    Card {
                        width: {
                            var desiredHeight = rootArea.height * 0.125
                            return desiredHeight * cardRatio
                        }
                        height: rootArea.height * 0.125
                        rotation: -90
                        faceUp: model.faceUp
                        value: model.value
                        suit: model.suit
                        isPlayable: model.isPlayable
                        enabled: gameModel.currentPlayer === playerEastColumn.actualPlayerIndex

                        MouseArea {
                            anchors.fill: parent
                            enabled: gameModel.currentPlayer === playerEastColumn.actualPlayerIndex && model.isPlayable
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                            onClicked: gameModel.playCard(index)
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
                Text { text: "Manche"; color: "white"; font.pixelSize: parent.height * 0.1; font.bold: true }
                Text { text: "Total"; color: "white"; font.pixelSize: parent.height * 0.1; font.bold: true }

                Text { text: "Équipe 1:"; color: "white"; font.pixelSize: parent.height * 0.1 }
                Text { text: gameModel.scoreTeam1; color: "white"; font.pixelSize: parent.height * 0.1 }
                Text { text: gameModel.scoreTotalTeam1; color: "white"; font.pixelSize: parent.height * 0.1 }

                Text { text: "Équipe 2:"; color: "white"; font.pixelSize: parent.height * 0.1 }
                Text { text: gameModel.scoreTeam2; color: "white"; font.pixelSize: parent.height * 0.1 }
                Text { text: gameModel.scoreTotalTeam2; color: "white"; font.pixelSize: parent.height * 0.1 }
            }
        }
}

