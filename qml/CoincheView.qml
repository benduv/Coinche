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

    // Obtenir l'avatar d'un joueur par son index reel
    function getPlayerAvatar(actualPlayerIndex) {
        var avatars = [
            "qrc:/resources/avatar/animal-1293181.svg",
            "qrc:/resources/avatar/bee-24633.svg",
            "qrc:/resources/avatar/cartoon-1294036.svg",
            "qrc:/resources/avatar/warrior-149098.svg"
        ]
        return avatars[actualPlayerIndex]
    }

    // Obtenir la valeur de l'annonce d'un joueur (80, 90, Passe, etc.)
    function getPlayerBidValue(actualPlayerIndex) {
        if (actualPlayerIndex >= 0 && actualPlayerIndex < gameModel.playerBids.length) {
            return gameModel.playerBids[actualPlayerIndex].bidValue || ""
        }
        return ""
    }

    // Obtenir le symbole de la couleur de l'annonce
    function getPlayerBidSymbol(actualPlayerIndex) {
        if (actualPlayerIndex >= 0 && actualPlayerIndex < gameModel.playerBids.length) {
            return gameModel.playerBids[actualPlayerIndex].suitSymbol || ""
        }
        return ""
    }

    // Obtenir la couleur du symbole (rouge pour coeur/carreau, blanc sinon)
    function getPlayerBidSymbolColor(actualPlayerIndex) {
        if (actualPlayerIndex >= 0 && actualPlayerIndex < gameModel.playerBids.length) {
            var isRed = gameModel.playerBids[actualPlayerIndex].isRed || false
            return isRed ? "#ff6666" : "black"
        }
        return "white"
    }

    // =====================
    // ZONE CENTRALE DE JEU
    // =====================
    Rectangle {
        id: playArea
        anchors.top: playerNorthColumn.bottom
        anchors.topMargin: - parent.height * 0.15
        anchors.bottom: playerSouthRow.top
        anchors.bottomMargin: - parent.height * 0.15
        anchors.right: playerEastRow.left
        anchors.rightMargin: parent.width * 0.015 - parent.width * 0.085
        anchors.left: playerWestRow.right
        anchors.leftMargin: parent.width * 0.015 - parent.width * 0.085
        color: "transparent"
        radius: 10

        // Image de fond du tapis avec coins arrondis
        Rectangle {
            id: tapisBackground
            anchors.fill: parent
            radius: playArea.radius
            color: "#1a3d0f"
            clip: true

            Image {
                anchors.fill: parent
                source: "qrc:/resources/Tapis poker3.jpg"
                fillMode: Image.PreserveAspectCrop
                smooth: true
                antialiasing: true
            }
        }

        // Bordure par-dessus l'image
        Rectangle {
            anchors.fill: parent
            anchors.margins: -parent.height * 0.01
            radius: playArea.radius
            color: "transparent"
            border.color: "#8b6914"
            border.width: parent.height * 0.06
        }

        // Bordure par-dessus l'image
        Rectangle {
            anchors.fill: parent
            anchors.margins: -parent.height * 0.01
            radius: playArea.radius
            color: "transparent"
            border.color: "black"
            border.width: parent.height * 0.005
        }

        // Bordure par-dessus l'image
        Rectangle {
            anchors.fill: parent
            anchors.margins: parent.height * 0.04
            radius: playArea.radius
            color: "transparent"
            border.color: "black"
            border.width: parent.height * 0.01
        }

        // ---- Panneau d'annonces ----
        AnnoncesPanel {
            anchors.fill: parent
            anchors.margins: parent.width * 0.09
            visible: gameModel.biddingPhase &&
                     gameModel.distributionPhase === 0 &&    // Attendre fin de distribution
                     gameModel.biddingPlayer === gameModel.myPosition &&
                     !gameModel.showCoincheAnimation &&      // Masquer si animation Coinche
                     !gameModel.showSurcoincheAnimation &&   // Masquer si animation Surcoinche
                     !gameModel.surcoincheAvailable          // Masquer si bouton Surcoinche visible
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
                    if (count === 0) {
                        pliArea.lastCardCount = 0
                    }
                }

                Card {
                    id: cardInPli
                    width: {
                        var desiredHeight = playArea.height * 0.38
                        return desiredHeight * cardRatio
                    }
                    height: playArea.height * 0.38
                    value: modelData.value
                    suit: modelData.suit
                    isAtout: modelData.isAtout
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

                        // Mettre a jour IMMEDIATEMENT pour les prochaines cartes
                        if (isNewCard) {
                            pliArea.lastCardCount = currentCount
                        }

                        if (isNewCard) {
                            // IMPORTANT: Garder willAnimate = false pour le positionnement initial
                            willAnimate = false
                            opacity = 0

                            // Lancer l'animation (qui positionnera la carte au bon endroit)
                            animationTimer.start()
                        } else {
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

                            // Positionner hors ecran selon la position du joueur
                            switch (visualPos) {
                                case 0: // Sud - vient du bas
                                    cardInPli.x = areaW / 2 - cardW / 2
                                    cardInPli.y = areaH + cardH
                                    cardInPli.rotation = 180
                                    break
                                case 1: // Ouest - vient de la gauche
                                    cardInPli.x = -cardW * 2
                                    cardInPli.y = areaH / 2 - cardH / 2
                                    cardInPli.rotation = 270
                                    break
                                case 2: // Nord - vient du haut
                                    cardInPli.x = areaW / 2 - cardW / 2
                                    cardInPli.y = -cardH * 2
                                    cardInPli.rotation = 180
                                    break
                                case 3: // Est - vient de la droite
                                    cardInPli.x = areaW + cardW * 2
                                    cardInPli.y = areaH / 2 - cardH / 2
                                    cardInPli.rotation = -270
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
            anchors.bottomMargin: - parent.height * 0.07
            spacing: parent.width * 0.02
            height: rootArea.height * 0.3  // Hauteur fixe pour eviter que playArea change de taille lorsque plus de carte dans la main

            property int actualPlayerIndex: rootArea.getActualPlayerIndex(0)

            // Avatar, nom et annonce a gauche du jeu
            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: rootArea.width * 0.01

                Column {
                    spacing: rootArea.height * 0.005

                    Rectangle {
                        width: rootArea.width * 0.075
                        height: rootArea.width * 0.075
                        radius: width / 2
                        color: "#f0f0f0"
                        border.color: gameModel.currentPlayer === playerSouthRow.actualPlayerIndex ? "#ffff66" : "#888888"
                        border.width: 3

                        Image {
                            anchors.fill: parent
                            anchors.margins: parent.width * 0.1
                            source: rootArea.getPlayerAvatar(playerSouthRow.actualPlayerIndex)
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }
                    }

                    Text {
                        text: rootArea.getPlayerName(playerSouthRow.actualPlayerIndex)
                        color: gameModel.currentPlayer === playerSouthRow.actualPlayerIndex ? "#ffff66" : "white"
                        font.pixelSize: rootArea.height * 0.02
                        font.bold: gameModel.currentPlayer === playerSouthRow.actualPlayerIndex
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                // Indicateur d'annonce (a droite)
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: bidRowSouth.width + rootArea.width * 0.02
                    height: rootArea.height * 0.04
                    radius: height / 4
                    color: "#2a5a2a"
                    border.color: "#4a8a4a"
                    border.width: 1
                    opacity: 0.4
                    visible: rootArea.getPlayerBidValue(playerSouthRow.actualPlayerIndex) !== ""

                    Row {
                        id: bidRowSouth
                        anchors.centerIn: parent
                        Text {
                            text: rootArea.getPlayerBidValue(playerSouthRow.actualPlayerIndex)
                            color: "white"
                            font.pixelSize: rootArea.height * 0.025
                            font.bold: true
                        }
                        Text {
                            text: rootArea.getPlayerBidSymbol(playerSouthRow.actualPlayerIndex)
                            color: rootArea.getPlayerBidSymbolColor(playerSouthRow.actualPlayerIndex)
                            font.pixelSize: rootArea.height * 0.025
                            font.bold: true
                        }
                    }
                }
            }

            Row {
                id: playerSouth
                spacing: - rootArea.height * 0.12
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
                            var desiredHeight = rootArea.height * 0.35
                            return desiredHeight * cardRatio
                        }
                        height: rootArea.height * 0.35
                        value: model.value
                        suit: model.suit
                        faceUp: model.faceUp
                        isAtout: model.isAtout
                        isPlayable: gameModel.biddingPhase || gameModel.currentPlayer !== playerSouthRow.actualPlayerIndex || model.isPlayable
                        enabled: !gameModel.biddingPhase && gameModel.currentPlayer === playerSouthRow.actualPlayerIndex

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
        Item {
            id: playerNorthColumn
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: - parent.height * 0.01
            width: parent.width * 0.5
            height: rootArea.height * 0.22

            property int actualPlayerIndex: rootArea.getActualPlayerIndex(2)

            // Avatar, nom et annonce en bas
            Row {
                id: northAvatar
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: rootArea.width * 0.01

                Column {
                    spacing: rootArea.height * 0.005

                    Rectangle {
                        width: rootArea.width * 0.075
                        height: rootArea.width * 0.075
                        radius: width / 2
                        color: "#f0f0f0"
                        border.color: gameModel.currentPlayer === playerNorthColumn.actualPlayerIndex ? "#ffff66" : "#888888"
                        border.width: 2
                        anchors.horizontalCenter: parent.horizontalCenter

                        Image {
                            anchors.fill: parent
                            anchors.margins: parent.width * 0.1
                            source: rootArea.getPlayerAvatar(playerNorthColumn.actualPlayerIndex)
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }
                    }

                    Text {
                        text: rootArea.getPlayerName(playerNorthColumn.actualPlayerIndex)
                        color: gameModel.currentPlayer === playerNorthColumn.actualPlayerIndex ? "#ffff66" : "white"
                        font.pixelSize: rootArea.height * 0.018
                        font.bold: gameModel.currentPlayer === playerNorthColumn.actualPlayerIndex
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                // Indicateur d'annonce (a droite)
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: bidRowNorth.width + rootArea.width * 0.02
                    height: rootArea.height * 0.035
                    radius: height / 4
                    color: "#2a5a2a"
                    border.color: "#4a8a4a"
                    border.width: 1
                    opacity: 0.4
                    visible: rootArea.getPlayerBidValue(playerNorthColumn.actualPlayerIndex) !== ""

                    Row {
                        id: bidRowNorth
                        anchors.centerIn: parent
                        Text {
                            text: rootArea.getPlayerBidValue(playerNorthColumn.actualPlayerIndex)
                            color: "white"
                            font.pixelSize: rootArea.height * 0.02
                            font.bold: true
                        }
                        Text {
                            text: rootArea.getPlayerBidSymbol(playerNorthColumn.actualPlayerIndex)
                            color: rootArea.getPlayerBidSymbolColor(playerNorthColumn.actualPlayerIndex)
                            font.pixelSize: rootArea.height * 0.02
                            font.bold: true
                        }
                    }
                }
            }

            // Cartes au-dessus de l'avatar
            Row {
                id: playerNorth
                anchors.bottom: northAvatar.top
                anchors.bottomMargin: rootArea.height * 0.005
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: - rootArea.height * 0.05
                Repeater {
                    model: {
                        switch (playerNorthColumn.actualPlayerIndex) {
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
                        isAtout: model.isAtout
                        isPlayable: model.isPlayable
                        enabled: gameModel.currentPlayer === playerNorthColumn.actualPlayerIndex

                        MouseArea {
                            anchors.fill: parent
                            enabled: gameModel.currentPlayer === playerNorthColumn.actualPlayerIndex && model.isPlayable
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
        Row {
            id: playerWestRow
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: parent.width * 0.04
            spacing: parent.height * 0.008
            rotation: 0
            width: rootArea.width * 0.0625 // Largeur fixe pour eviter que playArea change de taille lorsque plus de carte dans la main
            layoutDirection: Qt.RightToLeft  // Inverse l'ordre visuel pour que l'avatar reste fixe

            property int actualPlayerIndex: rootArea.getActualPlayerIndex(1)

            // Avatar, nom et annonce a droite des cartes
            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: rootArea.height * 0.005

                Rectangle {
                    width: rootArea.width * 0.075
                    height: rootArea.width * 0.075
                    radius: width / 2
                    color: "#f0f0f0"
                    border.color: gameModel.currentPlayer === playerWestRow.actualPlayerIndex ? "#ffff66" : "#888888"
                    border.width: 2
                    anchors.horizontalCenter: parent.horizontalCenter

                    Image {
                        anchors.fill: parent
                        anchors.margins: parent.width * 0.1
                        source: rootArea.getPlayerAvatar(playerWestRow.actualPlayerIndex)
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }
                }

                Text {
                    text: rootArea.getPlayerName(playerWestRow.actualPlayerIndex)
                    color: gameModel.currentPlayer === playerWestRow.actualPlayerIndex ? "#ffff66" : "white"
                    font.pixelSize: rootArea.height * 0.018
                    font.bold: gameModel.currentPlayer === playerWestRow.actualPlayerIndex
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // Indicateur d'annonce (en dessous)
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: bidRowWest.width + rootArea.width * 0.015
                    height: rootArea.height * 0.03
                    radius: height / 4
                    color: "#2a5a2a"
                    border.color: "#4a8a4a"
                    border.width: 1
                    opacity: 0.4
                    visible: rootArea.getPlayerBidValue(playerWestRow.actualPlayerIndex) !== ""

                    Row {
                        id: bidRowWest
                        anchors.centerIn: parent
                        Text {
                            text: rootArea.getPlayerBidValue(playerWestRow.actualPlayerIndex)
                            color: "white"
                            font.pixelSize: rootArea.height * 0.018
                            font.bold: true
                        }
                        Text {
                            text: rootArea.getPlayerBidSymbol(playerWestRow.actualPlayerIndex)
                            color: rootArea.getPlayerBidSymbolColor(playerWestRow.actualPlayerIndex)
                            font.pixelSize: rootArea.height * 0.018
                            font.bold: true
                        }
                    }
                }
            }

            Column {
                id: playerWest
                spacing: - rootArea.height * 0.07
                Repeater {
                    model: {
                        switch (playerWestRow.actualPlayerIndex) {
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
                        isAtout: model.isAtout
                        isPlayable: model.isPlayable
                        enabled: gameModel.currentPlayer === playerWestRow.actualPlayerIndex

                        MouseArea {
                            anchors.fill: parent
                            enabled: gameModel.currentPlayer === playerWestRow.actualPlayerIndex && model.isPlayable
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
        Row {
            id: playerEastRow
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: parent.width * 0.04
            spacing: parent.height * 0.008
            width: rootArea.width * 0.0625  // Largeur fixe pour eviter que playArea change de taille lorsque plus de carte dans la main

            property int actualPlayerIndex: rootArea.getActualPlayerIndex(3)

            // Avatar et nom a coté du jeu
            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: rootArea.height * 0.005

                Rectangle {
                    width: rootArea.width * 0.075
                    height: rootArea.width * 0.075
                    radius: width / 2
                    color: "#f0f0f0"
                    border.color: gameModel.currentPlayer === playerEastRow.actualPlayerIndex ? "#ffff66" : "#888888"
                    border.width: 2

                    Image {
                        anchors.fill: parent
                        anchors.margins: parent.width * 0.1
                        source: rootArea.getPlayerAvatar(playerEastRow.actualPlayerIndex)
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }
                }

                Text {
                    text: rootArea.getPlayerName(playerEastRow.actualPlayerIndex)
                    color: gameModel.currentPlayer === playerEastRow.actualPlayerIndex ? "#ffff66" : "white"
                    font.pixelSize: rootArea.height * 0.018
                    font.bold: gameModel.currentPlayer === playerEastRow.actualPlayerIndex
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // Indicateur d'annonce (en dessous)
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: bidRowEast.width + rootArea.width * 0.02
                    height: rootArea.height * 0.04
                    radius: height / 4
                    color: "#2a5a2a"
                    border.color: "#4a8a4a"
                    border.width: 1
                    opacity: 0.4
                    visible: rootArea.getPlayerBidValue(playerEastRow.actualPlayerIndex) !== ""

                    Row {
                        id: bidRowEast
                        anchors.centerIn: parent
                        Text {
                            text: rootArea.getPlayerBidValue(playerEastRow.actualPlayerIndex)
                            color: "white"
                            font.pixelSize: rootArea.height * 0.025
                            font.bold: true
                        }
                        Text {
                            text: rootArea.getPlayerBidSymbol(playerEastRow.actualPlayerIndex)
                            color: rootArea.getPlayerBidSymbolColor(playerEastRow.actualPlayerIndex)
                            font.pixelSize: rootArea.height * 0.025
                            font.bold: true
                        }
                    }
                }
            }

            Column {
                id: playerEast
                spacing: - rootArea.height * 0.07
                Repeater {
                    model: {
                        switch (playerEastRow.actualPlayerIndex) {
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
                        isAtout: model.isAtout
                        isPlayable: model.isPlayable
                        enabled: gameModel.currentPlayer === playerEastRow.actualPlayerIndex

                        MouseArea {
                            anchors.fill: parent
                            enabled: gameModel.currentPlayer === playerEastRow.actualPlayerIndex && model.isPlayable
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
            width: parent.width * 0.13
            height: parent.height * 0.13
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: parent.width * 0.007
            color: "#1a1a1a"
            opacity: 0.85
            radius: 8
            border.color: "#aaaaaa"

            GridLayout {
                anchors.fill: parent
                anchors.margins: parent.width * 0.013
                rows: 3
                columns: 3
                rowSpacing: parent.height * 0.03
                columnSpacing: parent.width * 0.02

                Text { text: "Score"; color: "white"; font.pixelSize: parent.height * 0.13; font.bold: true }
                Text { text: "Manche"; color: "white"; font.pixelSize: parent.height * 0.13; font.bold: true }
                Text { text: "Total"; color: "white"; font.pixelSize: parent.height * 0.13; font.bold: true }

                Text { text: "Équipe 1:"; color: "white"; font.pixelSize: parent.height * 0.13 }
                Text { text: gameModel.scoreTeam1; color: "white"; font.pixelSize: parent.height * 0.13 }
                Text { text: gameModel.scoreTotalTeam1; color: "white"; font.pixelSize: parent.height * 0.13 }

                Text { text: "Équipe 2:"; color: "white"; font.pixelSize: parent.height * 0.13 }
                Text { text: gameModel.scoreTeam2; color: "white"; font.pixelSize: parent.height * 0.13 }
                Text { text: gameModel.scoreTotalTeam2; color: "white"; font.pixelSize: parent.height * 0.13 }
            }
        }

        // =====================
        // BOUTON COINCHE
        // =====================
        Button {
            id: coincheButton
            text: "Coinche"

            // Dimensions et position
            width: rootArea.width * 0.08
            height: rootArea.height * 0.15
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: parent.width * 0.015

            // Visible uniquement pendant la phase d'enchères
            visible: gameModel.biddingPhase &&
                     gameModel.lastBidValue > 0 &&
                     gameModel.lastBidValue !== 14 &&  // Pas PASSE (14)
                     gameModel.lastBidValue !== 12 &&  // Pas déjà COINCHE (12)
                     gameModel.lastBidValue !== 13 &&  // Pas déjà SURCOINCHE (13)
                     (gameModel.lastBidderIndex % 2) !== (gameModel.playerIndex % 2)

            // Enabled seulement si:
            // 1. Il y a eu une annonce (lastBidValue > 0)
            // 2. Le joueur local est dans l'équipe adverse
            // 3. Pas déjà COINCHE ou SURCOINCHE
            enabled: gameModel.lastBidValue > 0 &&
                     gameModel.lastBidValue !== 14 &&  // Pas PASSE (14)
                     gameModel.lastBidValue !== 12 &&  // Pas déjà COINCHE (12)
                     gameModel.lastBidValue !== 13 &&  // Pas déjà SURCOINCHE (13)
                     (gameModel.lastBidderIndex % 2) !== (gameModel.playerIndex % 2)

            background: Rectangle {
                color: parent.enabled
                       ? (parent.down ? "#cc6600" :
                          (parent.hovered ? "#ff9933" : "#ff8800"))
                       : "#333333"
                radius: 8
                border.color: parent.enabled ? "#FFD700" : "#555555"
                border.width: 3

                // Animation de pulsation quand le bouton est enabled
                SequentialAnimation on opacity {
                    running: parent.enabled
                    loops: Animation.Infinite
                    NumberAnimation { from: 1.0; to: 0.7; duration: 800; easing.type: Easing.InOutQuad }
                    NumberAnimation { from: 0.7; to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
                }
            }

            contentItem: Text {
                text: parent.text
                font.pixelSize: rootArea.height * 0.035
                font.bold: true
                color: parent.enabled ? "white" : "#666666"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                console.log("Bouton Coinche clique!")
                gameModel.coincheBid()
            }
        }

        // =====================
        // ANIMATION "COINCHE !"
        // =====================
        Item {
            anchors.centerIn: playArea
            width: playArea.width * 0.5
            height: playArea.height * 0.3
            visible: gameModel.showCoincheAnimation
            z: 100

            // Animation d'apparition avec zoom et fade
            scale: gameModel.showCoincheAnimation ? 1 : 0
            opacity: gameModel.showCoincheAnimation ? 1 : 0

            Behavior on scale {
                NumberAnimation {
                    duration: 600
                    easing.type: Easing.OutElastic
                    easing.amplitude: 1.5
                    easing.period: 0.5
                }
            }

            Behavior on opacity {
                NumberAnimation { duration: 400 }
            }

            // Couches d'ombre
            Rectangle {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 12
                width: parent.width + 8
                height: parent.height + 8
                color: "#40000000"
                radius: 20
            }

            Rectangle {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 8
                width: parent.width + 4
                height: parent.height + 4
                color: "#50000000"
                radius: 18
            }

            // Rectangle principal
            Rectangle {
                anchors.fill: parent
                color: "#1a1a1a"
                radius: 15
                border.color: "#FF8800"
                border.width: 6

                // Animation de pulsation de la bordure
                SequentialAnimation on border.width {
                    running: gameModel.showCoincheAnimation
                    loops: Animation.Infinite
                    NumberAnimation { from: 6; to: 10; duration: 500; easing.type: Easing.InOutQuad }
                    NumberAnimation { from: 10; to: 6; duration: 500; easing.type: Easing.InOutQuad }
                }

                Text {
                    anchors.centerIn: parent
                    text: "COINCHE !"
                    font.pixelSize: parent.height * 0.35
                    font.bold: true
                    color: "#FF8800"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    // Animation de rotation subtile
                    SequentialAnimation on rotation {
                        running: gameModel.showCoincheAnimation
                        loops: Animation.Infinite
                        NumberAnimation { from: -5; to: 5; duration: 400; easing.type: Easing.InOutSine }
                        NumberAnimation { from: 5; to: -5; duration: 400; easing.type: Easing.InOutSine }
                    }
                }
            }
        }

        // =====================
        // ANIMATION "SURCOINCHE !"
        // =====================
        Item {
            anchors.centerIn: playArea
            width: playArea.width * 0.5
            height: playArea.height * 0.3
            visible: gameModel.showSurcoincheAnimation
            z: 100

            // Animation d'apparition avec zoom et fade
            scale: gameModel.showSurcoincheAnimation ? 1 : 0
            opacity: gameModel.showSurcoincheAnimation ? 1 : 0

            Behavior on scale {
                NumberAnimation {
                    duration: 600
                    easing.type: Easing.OutElastic
                    easing.amplitude: 1.5
                    easing.period: 0.5
                }
            }

            Behavior on opacity {
                NumberAnimation { duration: 400 }
            }

            // Couches d'ombre
            Rectangle {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 12
                width: parent.width + 8
                height: parent.height + 8
                color: "#40000000"
                radius: 20
            }

            Rectangle {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 8
                width: parent.width + 4
                height: parent.height + 4
                color: "#50000000"
                radius: 18
            }

            // Rectangle principal
            Rectangle {
                anchors.fill: parent
                color: "#1a1a1a"
                radius: 15
                border.color: "#FF0088"
                border.width: 6

                // Animation de pulsation de la bordure
                SequentialAnimation on border.width {
                    running: gameModel.showSurcoincheAnimation
                    loops: Animation.Infinite
                    NumberAnimation { from: 6; to: 10; duration: 500; easing.type: Easing.InOutQuad }
                    NumberAnimation { from: 10; to: 6; duration: 500; easing.type: Easing.InOutQuad }
                }

                Text {
                    anchors.centerIn: parent
                    text: "SURCOINCHE !"
                    font.pixelSize: parent.height * 0.3
                    font.bold: true
                    color: "#FF0088"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    // Animation de rotation subtile
                    SequentialAnimation on rotation {
                        running: gameModel.showSurcoincheAnimation
                        loops: Animation.Infinite
                        NumberAnimation { from: -5; to: 5; duration: 400; easing.type: Easing.InOutSine }
                        NumberAnimation { from: 5; to: -5; duration: 400; easing.type: Easing.InOutSine }
                    }
                }
            }
        }

        // =====================
        // ANIMATION "BELOTE"
        // =====================
        Item {
            anchors.centerIn: playArea
            width: playArea.width * 0.5
            height: playArea.height * 0.3
            visible: gameModel.showBeloteAnimation
            z: 100

            Text {
                anchors.centerIn: parent
                text: "Belote"
                font.pixelSize: parent.height * 0.5
                font.family: "Serif"
                font.italic: true
                font.weight: Font.DemiBold
                color: "#D4AF37"  // Or antique
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                // Ombre portée
                style: Text.Outline
                styleColor: "#40000000"

                // Animation de fade in/out
                opacity: gameModel.showBeloteAnimation ? 1.0 : 0.0
                Behavior on opacity {
                    NumberAnimation {
                        duration: 800
                        easing.type: Easing.InOutQuad
                    }
                }

                // Légère mise à l'échelle à l'apparition
                scale: gameModel.showBeloteAnimation ? 1.0 : 0.8
                Behavior on scale {
                    NumberAnimation {
                        duration: 600
                        easing.type: Easing.OutCubic
                    }
                }
            }
        }

        // =====================
        // ANIMATION "REBELOTE"
        // =====================
        Item {
            anchors.centerIn: playArea
            width: playArea.width * 0.5
            height: playArea.height * 0.3
            visible: gameModel.showRebeloteAnimation
            z: 100

            Text {
                anchors.centerIn: parent
                text: "Rebelote"
                font.pixelSize: parent.height * 0.5
                font.family: "Serif"
                font.italic: true
                font.weight: Font.DemiBold
                color: "#D4AF37"  // Or antique
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                // Ombre portée
                style: Text.Outline
                styleColor: "#40000000"

                // Animation de fade in/out
                opacity: gameModel.showRebeloteAnimation ? 1.0 : 0.0
                Behavior on opacity {
                    NumberAnimation {
                        duration: 800
                        easing.type: Easing.InOutQuad
                    }
                }

                // Légère mise à l'échelle à l'apparition
                scale: gameModel.showRebeloteAnimation ? 1.0 : 0.8
                Behavior on scale {
                    NumberAnimation {
                        duration: 600
                        easing.type: Easing.OutCubic
                    }
                }
            }
        }

        // =====================
        // DERNIER PLI (en haut à gauche)
        // =====================
        Row {
            id: lastPliDisplay
            anchors.top: parent.top
            anchors.left: parent.left
            //anchors.margins: parent.width * 0.01
            anchors.topMargin: parent.height * 0.02
            anchors.leftMargin: parent.width * 0.075
            spacing: parent.width * 0.005
            visible: gameModel.lastPliCards.length > 0
            z: 50

            // Fonction pour obtenir le symbole de la couleur
            function getSuitSymbol(suitValue) {
                switch (suitValue) {
                    case 3: return "♥"  // COEUR
                    case 4: return "♣"  // TREFLE
                    case 5: return "♦"  // CARREAU
                    case 6: return "♠"  // PIQUE
                    default: return ""
                }
            }

            // Fonction pour obtenir la couleur du texte selon la couleur de carte
            function getTextColor(suitValue) {
                // Rouge pour coeur (3) et carreau (5)
                // Noir pour trèfle (4) et pique (6)
                return (suitValue === 3 || suitValue === 5) ? "#FF0000" : "#000000"
            }

            // Fonction pour obtenir le texte de la valeur
            function getValueText(cardValue) {
                switch (cardValue) {
                    case 7: return "7"
                    case 8: return "8"
                    case 9: return "9"
                    case 10: return "10"
                    case 11: return "V"   // Valet
                    case 12: return "D"   // Dame
                    case 13: return "R"   // Roi
                    case 14: return "A"   // As
                    default: return "?"
                }
            }

            Repeater {
                model: gameModel.lastPliCards

                Rectangle {
                    width: rootArea.width * 0.04
                    height: rootArea.height * 0.06
                    color: "white"
                    radius: 3
                    border.color: "#aaaaaa"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: lastPliDisplay.getValueText(modelData.value) + lastPliDisplay.getSuitSymbol(modelData.suit)
                        font.pixelSize: parent.height * 0.5
                        font.bold: true
                        color: lastPliDisplay.getTextColor(modelData.suit)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }

        // =====================
        // BOUTON SURCOINCHE (au centre de l'écran)
        // =====================
        Item {
            anchors.centerIn: playArea
            width: playArea.width * 0.3
            height: playArea.height * 0.3
            visible: gameModel.surcoincheAvailable

            // Couches d'ombre (du plus flou au plus net)
            Rectangle {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 12
                width: parent.width + 8
                height: parent.height + 8
                color: "#20000000"
                radius: 18
            }
            Rectangle {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 10
                width: parent.width + 6
                height: parent.height + 6
                color: "#30000000"
                radius: 17
            }
            Rectangle {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 8
                width: parent.width + 4
                height: parent.height + 4
                color: "#40000000"
                radius: 16
            }

            // Rectangle principal
            Rectangle {
                id: surcoincheContainer
                anchors.fill: parent
                color: "#1a1a1a"
                radius: 15
                border.color: "#FFD700"
                border.width: 4

                ColumnLayout {
                anchors.fill: parent
                anchors.margins: parent.height * 0.1
                spacing: parent.height * 0.08

                /*Text {
                    text: "Surcoinche"
                    font.pixelSize: parent.height * 0.12
                    font.bold: true
                    color: "#FFD700"
                    Layout.alignment: Qt.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                }*/

                Button {
                    id: surcoincheButton
                    text: "SURCOINCHE"
                    Layout.preferredWidth: parent.width * 0.8
                    Layout.preferredHeight: parent.height * 0.8
                    Layout.alignment: Qt.AlignHCenter

                    background: Rectangle {
                        color: parent.down ? "#cc0066" :
                               (parent.hovered ? "#ff3399" : "#ff0088")
                        radius: 10
                        border.color: "#FFD700"
                        border.width: 3

                        // Animation de pulsation
                        SequentialAnimation on opacity {
                            running: true
                            loops: Animation.Infinite
                            NumberAnimation { from: 1.0; to: 0.6; duration: 600; easing.type: Easing.InOutQuad }
                            NumberAnimation { from: 0.6; to: 1.0; duration: 600; easing.type: Easing.InOutQuad }
                        }
                    }

                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: surcoincheContainer.height * 0.1
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        console.log("Bouton Surcoinche clique!")
                        gameModel.surcoincheBid()
                    }
                }

                // Compte à rebours
                Text {
                    text: gameModel.surcoincheTimeLeft + "s restantes"
                    font.pixelSize: parent.height * 0.1
                    color: gameModel.surcoincheTimeLeft <= 3 ? "#ff0000" : "white"
                    Layout.alignment: Qt.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
        }

    // =====================
    // ANIMATION DE DISTRIBUTION
    // =====================
    Repeater {
        id: distributionAnimation
        model: {
            if (gameModel.distributionPhase === 0) return 0
            // Nombre total de cartes = (cartes par joueur) * 4 joueurs
            if (gameModel.distributionPhase === 1) return 12  // 3 cartes * 4 joueurs
            if (gameModel.distributionPhase === 2) return 8   // 2 cartes * 4 joueurs
            if (gameModel.distributionPhase === 3) return 12  // 3 cartes * 4 joueurs
            return 0
        }

        // Une carte animée par itération
        Item {
            id: flyingCard
            width: rootArea.height * 0.2 * 0.7 // Taille de carte
            height: rootArea.height * 0.2
            x: rootArea.width  // Démarre hors écran à droite
            y: rootArea.height / 2
            z: 1000  // Au-dessus de tout

            // Calculer à quel joueur cette carte est destinée (round-robin)
            property int targetPlayer: index % 4
            // Calculer quelle carte c'est pour ce joueur (0, 1, ou 2 pour phase 1/3, ou 0, 1 pour phase 2)
            property int cardNumber: Math.floor(index / 4)
            // Décalage dans le temps pour cette carte
            property int cardDelay: cardNumber * 250  // 150ms entre chaque "tour" de distribution

            Card {
                anchors.fill: parent
                faceUp: false  // Toujours face cachée pendant l'animation
                value: 0
                suit: 3
                isPlayable: false
            }

            // Animation vers chaque joueur (on anime les 4 joueurs en parallèle)
            ParallelAnimation {
                id: toNorthAnimation
                running: false
                NumberAnimation {
                    target: flyingCard
                    property: "x"
                    to: rootArea.width / 2
                    duration: 500
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: flyingCard
                    property: "y"
                    to: rootArea.height * 0.05
                    duration: 500
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: flyingCard
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 500
                    easing.type: Easing.InQuad
                }
            }

            ParallelAnimation {
                id: toSouthAnimation
                running: false
                NumberAnimation {
                    target: flyingCard
                    property: "x"
                    to: rootArea.width / 2
                    duration: 500
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: flyingCard
                    property: "y"
                    to: rootArea.height * 0.85
                    duration: 500
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: flyingCard
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 500
                    easing.type: Easing.InQuad
                }
            }

            ParallelAnimation {
                id: toWestAnimation
                running: false
                NumberAnimation {
                    target: flyingCard
                    property: "x"
                    to: rootArea.width * 0.05
                    duration: 500
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: flyingCard
                    property: "y"
                    to: rootArea.height / 2
                    duration: 500
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: flyingCard
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 500
                    easing.type: Easing.InQuad
                }
            }

            ParallelAnimation {
                id: toEastAnimation
                running: false
                NumberAnimation {
                    target: flyingCard
                    property: "x"
                    to: rootArea.width * 0.92
                    duration: 500
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: flyingCard
                    property: "y"
                    to: rootArea.height / 2
                    duration: 500
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: flyingCard
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 500
                    easing.type: Easing.InQuad
                }
            }

            // Démarrer l'animation vers le bon joueur après le délai
            Timer {
                interval: cardDelay
                running: gameModel.distributionPhase > 0
                repeat: false
                onTriggered: {
                    // Distribuer selon targetPlayer : 0=Sud, 1=Ouest, 2=Nord, 3=Est
                    if (targetPlayer === 0) {
                        toSouthAnimation.start()
                    } else if (targetPlayer === 1) {
                        toWestAnimation.start()
                    } else if (targetPlayer === 2) {
                        toNorthAnimation.start()
                    } else {
                        toEastAnimation.start()
                    }
                }
            }

            Component.onCompleted: {
                opacity = 0  // Invisible au départ
            }

            onVisibleChanged: {
                if (visible) {
                    opacity = 1
                    x = rootArea.width
                    y = rootArea.height / 2
                }
            }
        }
    }
}

