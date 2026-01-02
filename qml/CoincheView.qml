import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

Rectangle {
    id: rootArea
    anchors.fill: parent

    // Musique de fond pour la partie
    MediaPlayer {
        id: gameMusic
        source: "qrc:/resources/sons/lofi-boy-night-waves-lofi-relax-instrumental-278248.mp3"
        loops: MediaPlayer.Infinite
        audioOutput: AudioOutput {}
    }

    // Son pour jouer une carte
    MediaPlayer {
        id: cardSound
        source: "qrc:/resources/sons/card-sounds-35956.mp3"
        audioOutput: AudioOutput {}
    }

    // Son quand un pli est remporté
    MediaPlayer {
        id: pliWonSound
        source: "qrc:/resources/sons/701848__perduuus__card-step-2.wav"
        audioOutput: AudioOutput {}
    }

    // Son distribution
    MediaPlayer {
        id: dealingSound
        source: "qrc:/resources/sons/827603__elliottliu__card3.wav"
        audioOutput: AudioOutput {}
    }

    // Surveiller les changements de paramètres audio
    Connections {
        target: AudioSettings
        function onMusicEnabledChanged() {
            if (AudioSettings.musicEnabled) {
                gameMusic.play()
            } else {
                gameMusic.pause()
            }
        }
    }

    Component.onDestruction: {
        // Arrêter la musique quand on quitte la vue
        gameMusic.stop()
    }

    // Ratios pour le responsive (résolution de référence: 1920x1080)
    property real widthRatio: width / 1920
    property real heightRatio: height / 1080
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Propriétés pour la popup de fin de partie
    property bool showGameOverPopup: false
    property int gameOverWinner: 1
    property int gameOverScoreTeam1: 0
    property int gameOverScoreTeam2: 0

    Image {
        anchors.fill: parent
        source: "qrc:/resources/cielEtoile.jpg"
        fillMode: Image.PreserveAspectCrop
    }

    // Fonction pour retourner au menu principal
    function returnToMainMenu() {
        console.log("CoincheView.returnToMainMenu - Appel de la fonction centralisée")

        // Appeler la fonction centralisée dans mainWindow
        var mainWindow = rootArea.Window.window
        if (mainWindow && mainWindow.returnToMainMenu) {
            mainWindow.returnToMainMenu()
        } else {
            console.log("CoincheView.returnToMainMenu - ERREUR: mainWindow ou sa fonction returnToMainMenu non trouvée!")
        }
    }

    // Gérer l'abandon d'un joueur
    Connections {
        target: networkManager
        function onPlayerForfeited(playerIndex, playerName) {
            console.log("CoincheView - Joueur", playerName, "a abandonné")
            // Afficher une notification que le joueur a été remplacé par un bot
            // Note: Si c'est le joueur local qui a abandonné, returnToMainMenu() est déjà
            // appelé depuis ExitGamePopup, donc on ne le fait pas ici pour éviter de le faire deux fois
            console.log("Le joueur", playerName, "a été remplacé par un bot")
        }
    }

    // Fonction pour calculer la position visuelle d'un joueur
    // Retourne l'index visuel (0=sud, 1=ouest, 2=nord, 3=est)
    function getVisualPosition(actualPlayerIndex) {
        if (!gameModel) return 0
        var myPos = gameModel.myPosition
        var relativePos = (actualPlayerIndex - myPos + 4) % 4
        return relativePos
    }

    // Fonction inverse: obtenir l'index reel d'un joueur a partir de sa position visuelle
    function getActualPlayerIndex(visualPosition) {
        if (!gameModel) return 0
        var myPos = gameModel.myPosition
        return (visualPosition + myPos) % 4
    }

    // Obtenir le nom d'un joueur par son index reel
    function getPlayerName(actualPlayerIndex) {
        if (!gameModel) return ""
        // Récupérer le vrai nom du joueur depuis GameModel
        return gameModel.getPlayerName(actualPlayerIndex)
    }

    // Obtenir l'avatar d'un joueur par son index reel
    function getPlayerAvatar(actualPlayerIndex) {
        if (!gameModel) return "qrc:/resources/avatar/avataaars1.svg"
        var avatarName = gameModel.getPlayerAvatar(actualPlayerIndex)
        return "qrc:/resources/avatar/" + avatarName
    }

    // Obtenir la valeur de l'annonce d'un joueur (80, 90, Passe, etc.)
    function getPlayerBidValue(actualPlayerIndex) {
        if (!gameModel || !gameModel.playerBids) return ""
        if (actualPlayerIndex >= 0 && actualPlayerIndex < gameModel.playerBids.length) {
            return gameModel.playerBids[actualPlayerIndex].bidValue || ""
        }
        return ""
    }

    // Obtenir le symbole de la couleur de l'annonce
    function getPlayerBidSymbol(actualPlayerIndex) {
        if (!gameModel || !gameModel.playerBids) return ""
        if (actualPlayerIndex >= 0 && actualPlayerIndex < gameModel.playerBids.length) {
            return gameModel.playerBids[actualPlayerIndex].suitSymbol || ""
        }
        return ""
    }

    // Obtenir la couleur du symbole (rouge pour coeur/carreau, blanc sinon)
    function getPlayerBidSymbolColor(actualPlayerIndex) {
        if (!gameModel || !gameModel.playerBids) return "white"
        if (actualPlayerIndex >= 0 && actualPlayerIndex < gameModel.playerBids.length) {
            var isRed = gameModel.playerBids[actualPlayerIndex].isRed || false
            return isRed ? "red" : "black"
        }
        return "white"
    }

    // =====================
    // ZONE CENTRALE DE JEU
    // =====================
    Rectangle {
        id: playArea
        anchors.top: playerNorthColumn.bottom
        anchors.topMargin: - parent.height * 0.17
        anchors.bottom: playerSouthRow.top
        anchors.bottomMargin: - parent.height * 0.13
        anchors.right: playerEastRow.left
        anchors.rightMargin: - parent.width * 0.07
        anchors.left: playerWestRow.right
        anchors.leftMargin: - parent.width * 0.07
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

        // ---- Indicateur "XXX annonce ..." ----
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.5
            height: 120 * rootArea.minRatio
            color: "#2a2a2a"
            radius: 15 * rootArea.minRatio
            border.color: "#FFD700"
            border.width: 4 * rootArea.minRatio
            opacity: 0.8
            visible: gameModel.biddingPhase &&
                     gameModel.distributionPhase === 0 &&
                     gameModel.biddingPlayer !== gameModel.myPosition &&
                     gameModel.biddingPlayer >= 0 &&         // Masquer si biddingPlayer invalide
                     !gameModel.showCoincheAnimation &&      // Masquer si animation Coinche
                     !gameModel.showSurcoincheAnimation &&   // Masquer si animation Surcoinche
                     !gameModel.surcoincheAvailable          // Masquer si bouton Surcoinche visible
            z: 5

            Row {
                anchors.centerIn: parent
                spacing: 0

                Text {
                    id: biddingIndicatorText
                    text: rootArea.getPlayerName(gameModel.biddingPlayer) + " annonce "
                    font.pixelSize: 60 * rootArea.minRatio
                    font.bold: true
                    color: "#FFD700"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                Text {
                    id: dotsText
                    text: dotsAnimation.dots
                    font.pixelSize: 60 * rootArea.minRatio
                    font.bold: true
                    color: "#FFD700"
                    horizontalAlignment: Text.AlignLeft
                    width: 80 * rootArea.minRatio  // Largeur fixe pour 3 points
                }
            }

            // Animation des points "..."
            QtObject {
                id: dotsAnimation
                property string dots: ""
            }

            Timer {
                running: parent.visible
                repeat: true
                interval: 500
                onTriggered: {
                    if (dotsAnimation.dots === "") {
                        dotsAnimation.dots = "."
                    } else if (dotsAnimation.dots === ".") {
                        dotsAnimation.dots = ".."
                    } else if (dotsAnimation.dots === "..") {
                        dotsAnimation.dots = "..."
                    } else {
                        dotsAnimation.dots = ""
                    }
                }
            }

            // Animation de pulsation
            SequentialAnimation on scale {
                running: parent.visible
                loops: Animation.Infinite
                NumberAnimation { to: 1.05; duration: 800; easing.type: Easing.InOutQuad }
                NumberAnimation { to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
            }
        }

        // ---- Panneau d'annonces ----
        AnnoncesPanel {
            anchors.fill: parent
            anchors.leftMargin: parent.width * 0.09
            anchors.rightMargin: parent.width * 0.09
            anchors.topMargin: parent.height * 0.2
            anchors.bottomMargin: parent.height * 0.2
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

                Card {
                    id: cardInPli
                    width: {
                        var desiredHeight = playArea.height * 0.42
                        return desiredHeight * cardRatio
                    }
                    height: playArea.height * 0.42
                    value: modelData.value
                    suit: modelData.suit
                    isAtout: modelData.isAtout
                    faceUp: true

                    // Flag pour savoir si cette carte doit animer
                    property bool willAnimate: false
                    property bool exitAnimating: false  // Pour l'animation de sortie

                    // Position finale calculee
                    property real finalX: {
                        var visualPos = rootArea.getVisualPosition(modelData.playerId)
                        switch (visualPos) {
                            case 0: return pliArea.width / 2 - width / 2;            // Sud
                            case 1: return pliArea.width / 2 - width / 2 - width * 0.7; // Ouest
                            case 2: return pliArea.width / 2 - width / 2;            // Nord
                            case 3: return pliArea.width / 2 - width / 2 + width * 0.7; // Est
                        }
                    }
                    property real finalY: {
                        var visualPos = rootArea.getVisualPosition(modelData.playerId)
                        switch (visualPos) {
                            case 0: return pliArea.height / 2 - height * 0.26; // Sud
                            case 1: return pliArea.height / 2 - height * 0.5;               // Ouest
                            case 2: return pliArea.height / 2 - height * 0.77; // Nord
                            case 3: return pliArea.height / 2 - height * 0.5;               // Est
                        }
                    }
                    property real finalRotation: {
                        var visualPos = rootArea.getVisualPosition(modelData.playerId)
                        switch (visualPos) {
                            case 0: return 5;            // Sud
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
                            if (AudioSettings.effectsEnabled) {
                                console.log(">>> LECTURE DU SON DE CARTE <<<")
                                //cardSound.stop()
                                cardSound.play()
                            }

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
                        enabled: willAnimate || exitAnimating
                        NumberAnimation { duration: exitAnimating ? 800 : 600; easing.type: exitAnimating ? Easing.InCubic : Easing.OutCubic }
                    }
                    Behavior on y {
                        enabled: willAnimate || exitAnimating
                        NumberAnimation { duration: exitAnimating ? 800 : 600; easing.type: exitAnimating ? Easing.InCubic : Easing.OutCubic }
                    }
                    Behavior on rotation {
                        enabled: willAnimate
                        NumberAnimation { duration: 600; easing.type: Easing.OutCubic }
                    }
                    Behavior on opacity {
                        enabled: willAnimate || exitAnimating
                        NumberAnimation { duration: exitAnimating ? 800 : 400; easing.type: Easing.InOutQuad }
                    }

                    // Animation de sortie vers le gagnant
                    Connections {
                        target: gameModel
                        function onPliWinnerIdChanged() {
                            if (gameModel.pliWinnerId >= 0) {
                                if(AudioSettings.effectsEnabled) {
                                    pliWonSound.stop()
                                    pliWonSound.play()
                                }
                                // Démarrer l'animation de sortie vers le gagnant
                                var winnerVisualPos = rootArea.getVisualPosition(gameModel.pliWinnerId)
                                cardInPli.exitAnimating = true

                                // Calculer la position de sortie selon la position du gagnant
                                switch (winnerVisualPos) {
                                    case 0: // Sud - sortir par le bas
                                        cardInPli.x = pliArea.width / 2 - cardInPli.width / 2
                                        cardInPli.y = pliArea.height + cardInPli.height * 2
                                        break
                                    case 1: // Ouest - sortir par la gauche
                                        cardInPli.x = -cardInPli.width * 2
                                        cardInPli.y = pliArea.height / 2 - cardInPli.height / 2
                                        break
                                    case 2: // Nord - sortir par le haut
                                        cardInPli.x = pliArea.width / 2 - cardInPli.width / 2
                                        cardInPli.y = -cardInPli.height * 2
                                        break
                                    case 3: // Est - sortir par la droite
                                        cardInPli.x = pliArea.width + cardInPli.width * 2
                                        cardInPli.y = pliArea.height / 2 - cardInPli.height / 2
                                        break
                                }
                                cardInPli.opacity = 0
                            }
                        }
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

    // Connexion pour afficher la popup de fin de partie
    Connections {
        target: gameModel
        function onGameOver(winner, scoreTeam1, scoreTeam2) {
            console.log("Game Over! Winner: Team", winner, "Scores:", scoreTeam1, scoreTeam2)
            rootArea.gameOverWinner = winner
            rootArea.gameOverScoreTeam1 = scoreTeam1
            rootArea.gameOverScoreTeam2 = scoreTeam2
            rootArea.showGameOverPopup = true
        }
    }

    // Connexion pour jouer le son quand un pli est remporté
    /*Connections {
        target: gameModel
        function onPliWinnerIdChanged() {
            if (gameModel.pliWinnerId >= 0 && AudioSettings.effectsEnabled) {
                console.log("Pli remporte par joueur", gameModel.pliWinnerId)
                pliWonSound.stop()
                pliWonSound.play()
            }
        }
    }*/

        // =====================
        // JOUEUR SUD (vous)
        // =====================
        Item {
            id: playerSouthRow
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: -rootArea.height * 0.08
            width: parent.width
            height: rootArea.height * 0.3  // Hauteur fixe pour eviter que playArea change de taille lorsque plus de carte dans la main

            property int actualPlayerIndex: rootArea.getActualPlayerIndex(0)

            // Avatar, nom et annonce a gauche (position absolue)
            Column {
                anchors.left: parent.left
                anchors.leftMargin: parent.width * 0.055
                anchors.top : parent.top
                anchors.topMargin: - parent.height * 0.13
                spacing: rootArea.height * 0.003

                // Row pour le jeton de dealer, avatar+annonce+nom
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: rootArea.width * 0.01

                    // Jeton de dealer à gauche
                    Rectangle {
                        width: rootArea.width * 0.03
                        height: rootArea.width * 0.03
                        radius: width / 2
                        color: "#FFD700"
                        border.color: "#8B7500"
                        border.width: 2
                        anchors.verticalCenter: parent.verticalCenter
                        opacity: gameModel.dealerPosition === playerSouthRow.actualPlayerIndex ? 1 : 0

                        Text {
                            anchors.centerIn: parent
                            text: "D"
                            font.pixelSize: parent.width * 0.6
                            font.bold: true
                            color: "#8B7500"
                        }
                    }

                    // Item wrapper pour annonce + avatar + nom (layout indépendant pour le nom)
                    Item {
                        width: rootArea.width * 0.08
                        height: rootArea.height * 0.045 + rootArea.height * 0.005 + rootArea.width * 0.075 + rootArea.height * 0.005 + rootArea.height * 0.04

                        // Indicateur d'annonce (espace toujours reserve)
                        Rectangle {
                            id: bidRectSouth
                            anchors.top: parent.top
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: Math.max(bidRowSouth.width + rootArea.width * 0.02, rootArea.width * 0.08)
                            height: rootArea.height * 0.045
                            radius: height / 4
                            color: "grey"
                            border.color: "#4a8a4a"
                            border.width: 1
                            opacity: {
                                if (rootArea.getPlayerBidValue(playerSouthRow.actualPlayerIndex) === "") return 0
                                if (!gameModel.biddingPhase && rootArea.getPlayerBidValue(playerSouthRow.actualPlayerIndex) === "Passe") return 0
                                return 0.85
                            }
                            visible: true  // Toujours visible pour reserver l'espace

                            Behavior on opacity {
                                NumberAnimation { duration: 200 }
                            }

                            Row {
                                id: bidRowSouth
                                anchors.centerIn: parent
                                visible: rootArea.getPlayerBidValue(playerSouthRow.actualPlayerIndex) !== ""
                                Text {
                                    text: rootArea.getPlayerBidValue(playerSouthRow.actualPlayerIndex)
                                    color: "white"
                                    font.pixelSize: rootArea.height * 0.04
                                    font.bold: true
                                }
                                Text {
                                    text: rootArea.getPlayerBidSymbol(playerSouthRow.actualPlayerIndex)
                                    color: rootArea.getPlayerBidSymbolColor(playerSouthRow.actualPlayerIndex)
                                    font.pixelSize: rootArea.height * 0.045
                                    font.bold: true
                                }
                            }
                        }

                        Rectangle {
                            id: avatarSouth
                            anchors.top: bidRectSouth.bottom
                            anchors.topMargin: rootArea.height * 0.005
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: rootArea.width * 0.075
                            height: rootArea.width * 0.075
                            radius: 5
                            color: "#80808080"  // Gris avec 50% de transparence
                            border.color: gameModel.currentPlayer === playerSouthRow.actualPlayerIndex ? "#ffff66" : "#888888"
                            border.width: 3

                            Image {
                                anchors.fill: parent
                                anchors.margins: parent.width * 0.05
                                source: rootArea.getPlayerAvatar(playerSouthRow.actualPlayerIndex)
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                            }

                            // Jauge de timer en overlay
                            Rectangle {
                                id: timerOverlaySouth
                                anchors.fill: parent
                                radius: 2
                                anchors.margins: 3
                                visible: !gameModel.biddingPhase && gameModel.currentPlayer === playerSouthRow.actualPlayerIndex
                                clip: true
                                color: "transparent"
                                z: 100

                                Rectangle {
                                    property real fillRatio: gameModel.playTimeRemaining / gameModel.maxPlayTime

                                    width: parent.width
                                    height: parent.height * fillRatio
                                    anchors.bottom: parent.bottom
                                    radius: parent.radius
                                    color: {
                                        if (gameModel.playTimeRemaining <= 3) return "#B0FF3333"
                                        if (gameModel.playTimeRemaining <= 7) return "#B0FFAA00"
                                        return "#B000CC00"
                                    }

                                    Behavior on height {
                                        NumberAnimation { duration: 300 }
                                    }

                                    Behavior on color {
                                        ColorAnimation { duration: 300 }
                                    }
                                }
                            }

                            // MouseArea pour afficher les stats
                            MouseArea {
                                anchors.fill: parent
                                property bool isGuest: {
                                    var name = rootArea.getPlayerName(playerSouthRow.actualPlayerIndex)
                                    return !name || name.startsWith("Invité") || name.startsWith("Joueur")
                                }
                                cursorShape: isGuest ? Qt.ArrowCursor : Qt.PointingHandCursor
                                onClicked: {
                                    if (!isGuest) {
                                        playerStatsPopup.loadPlayerStats(rootArea.getPlayerName(playerSouthRow.actualPlayerIndex))
                                        playerStatsPopup.visible = true
                                    }
                                }
                            }
                        }

                        Text {
                            text: rootArea.getPlayerName(playerSouthRow.actualPlayerIndex)
                            color: gameModel.currentPlayer === playerSouthRow.actualPlayerIndex ? "#ffff66" : "white"
                            font.pixelSize: rootArea.height * 0.04
                            font.bold: gameModel.currentPlayer === playerSouthRow.actualPlayerIndex
                            anchors.top: avatarSouth.bottom
                            anchors.topMargin: rootArea.height * 0.005
                            anchors.horizontalCenter: avatarSouth.horizontalCenter
                            width: rootArea.width * 0.11
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                        }
                    }
                }
            }

            // Cartes centrees
            Row {
                id: playerSouth
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                spacing: - rootArea.height * 0.102
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
            anchors.topMargin: parent.height * 0.005
            width: parent.width * 0.5
            height: rootArea.height * 0.23

            property int actualPlayerIndex: rootArea.getActualPlayerIndex(2)

            // Avatar, nom et annonce en bas
            Row {
                id: northAvatar
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: rootArea.width * 0.01

                // Item wrapper pour avatar + nom (layout indépendant pour le nom)
                Item {
                    width: rootArea.width * 0.075
                    height: rootArea.width * 0.075 + rootArea.height * 0.005 + rootArea.height * 0.04

                    Rectangle {
                        id: northAvatarRect
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: rootArea.width * 0.075
                        height: rootArea.width * 0.075
                        radius: 5
                        color: "#80808080"  // Gris avec 50% de transparence
                        border.color: gameModel.currentPlayer === playerNorthColumn.actualPlayerIndex ? "#ffff66" : "#888888"
                        border.width: 3

                        Image {
                            anchors.fill: parent
                            anchors.margins: parent.width * 0.05
                            source: rootArea.getPlayerAvatar(playerNorthColumn.actualPlayerIndex)
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }

                        // Jauge de timer en overlay
                        Rectangle {
                            anchors.fill: parent
                            radius: 2
                            anchors.margins: 3
                            visible: !gameModel.biddingPhase && gameModel.currentPlayer === playerNorthColumn.actualPlayerIndex
                            clip: true
                            color: "transparent"
                            z: 100

                            Rectangle {
                                property real fillRatio: gameModel.playTimeRemaining / gameModel.maxPlayTime

                                width: parent.width
                                height: parent.height * fillRatio
                                anchors.bottom: parent.bottom
                                radius: parent.radius
                                color: {
                                    if (gameModel.playTimeRemaining <= 3) return "#B0FF3333"
                                    if (gameModel.playTimeRemaining <= 7) return "#B0FFAA00"
                                    return "#B000CC00"
                                }

                                Behavior on height {
                                    NumberAnimation { duration: 300 }
                                }

                                Behavior on color {
                                    ColorAnimation { duration: 300 }
                                }
                            }
                        }

                        // MouseArea pour afficher les stats
                        MouseArea {
                            anchors.fill: parent
                            property bool isGuest: {
                                var name = rootArea.getPlayerName(playerNorthColumn.actualPlayerIndex)
                                return !name || name.startsWith("Invité") || name.startsWith("Joueur")
                            }
                            cursorShape: isGuest ? Qt.ArrowCursor : Qt.PointingHandCursor
                            onClicked: {
                                if (!isGuest) {
                                    playerStatsPopup.loadPlayerStats(rootArea.getPlayerName(playerNorthColumn.actualPlayerIndex))
                                    playerStatsPopup.visible = true
                                }
                            }
                        }
                    }

                    Text {
                        text: rootArea.getPlayerName(playerNorthColumn.actualPlayerIndex)
                        color: gameModel.currentPlayer === playerNorthColumn.actualPlayerIndex ? "#ffff66" : "white"
                        font.pixelSize: rootArea.height * 0.04
                        font.bold: gameModel.currentPlayer === playerNorthColumn.actualPlayerIndex
                        anchors.top: northAvatarRect.bottom
                        anchors.topMargin: rootArea.height * 0.005
                        anchors.horizontalCenter: northAvatarRect.horizontalCenter
                        width: rootArea.width * 0.11
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                    }
                }

                // Column pour jeton dealer + indicateur d'annonce à droite de l'avatar
                Column {
                    spacing: rootArea.height * 0.005
                    anchors.verticalCenter: parent.verticalCenter

                    // Jeton de dealer en haut
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: rootArea.width * 0.03
                        height: rootArea.width * 0.03
                        radius: width / 2
                        color: "#FFD700"
                        border.color: "#8B7500"
                        border.width: 2
                        opacity: gameModel.dealerPosition === playerNorthColumn.actualPlayerIndex ? 1 : 0

                        Text {
                            anchors.centerIn: parent
                            text: "D"
                            font.pixelSize: parent.width * 0.6
                            font.bold: true
                            color: "#8B7500"
                        }
                    }

                    // Indicateur d'annonce en bas
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: Math.max(bidRowNorth.width + rootArea.width * 0.02, rootArea.width * 0.08)
                        height: rootArea.height * 0.045
                        radius: height / 4
                        color: "grey"
                        border.color: "#4a8a4a"
                        border.width: 1
                        opacity: {
                            if (rootArea.getPlayerBidValue(playerNorthColumn.actualPlayerIndex) === "") return 0
                            if (!gameModel.biddingPhase && rootArea.getPlayerBidValue(playerNorthColumn.actualPlayerIndex) === "Passe") return 0
                            return 0.85
                        }

                        Behavior on opacity {
                            NumberAnimation { duration: 200 }
                        }

                        Row {
                            id: bidRowNorth
                            anchors.centerIn: parent
                            visible: rootArea.getPlayerBidValue(playerNorthColumn.actualPlayerIndex) !== ""
                            Text {
                                text: rootArea.getPlayerBidValue(playerNorthColumn.actualPlayerIndex)
                                color: "white"
                                font.pixelSize: rootArea.height * 0.04
                                font.bold: true
                            }
                            Text {
                                text: rootArea.getPlayerBidSymbol(playerNorthColumn.actualPlayerIndex)
                                color: rootArea.getPlayerBidSymbolColor(playerNorthColumn.actualPlayerIndex)
                                font.pixelSize: rootArea.height * 0.045
                                font.bold: true
                            }
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

            // Column pour annonce + avatar + dealer, avec nom en overlay
            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: rootArea.height * 0.005

                // Indicateur d'annonce (espace toujours reserve)
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Math.max(bidRowWest.width + rootArea.width * 0.015, rootArea.width * 0.08)
                    height: rootArea.height * 0.045
                    radius: height / 4
                    color: "grey"
                    border.color: "#4a8a4a"
                    border.width: 1
                    opacity: {
                        if (rootArea.getPlayerBidValue(playerWestRow.actualPlayerIndex) === "") return 0
                        if (!gameModel.biddingPhase && rootArea.getPlayerBidValue(playerWestRow.actualPlayerIndex) === "Passe") return 0
                        return 0.85
                    }
                    visible: true  // Toujours visible pour reserver l'espace

                    Behavior on opacity {
                        NumberAnimation { duration: 200 }
                    }

                    Row {
                        id: bidRowWest
                        anchors.centerIn: parent
                        visible: rootArea.getPlayerBidValue(playerWestRow.actualPlayerIndex) !== ""
                        Text {
                            text: rootArea.getPlayerBidValue(playerWestRow.actualPlayerIndex)
                            color: "white"
                            font.pixelSize: rootArea.height * 0.04
                            font.bold: true
                        }
                        Text {
                            text: rootArea.getPlayerBidSymbol(playerWestRow.actualPlayerIndex)
                            color: rootArea.getPlayerBidSymbolColor(playerWestRow.actualPlayerIndex)
                            font.pixelSize: rootArea.height * 0.045
                            font.bold: true
                        }
                    }
                }

                // Item wrapper pour avatar + nom (le nom n'affecte pas la largeur)
                Item {
                    width: rootArea.width * 0.075
                    height: rootArea.width * 0.075 + rootArea.height * 0.005 + rootArea.height * 0.04

                    Rectangle {
                        id: westAvatarRect
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: rootArea.width * 0.075
                        height: rootArea.width * 0.075
                        radius: 5
                        color: "#80808080"  // Gris avec 50% de transparence
                        border.color: gameModel.currentPlayer === playerWestRow.actualPlayerIndex ? "#ffff66" : "#888888"
                        border.width: 3

                        Image {
                            anchors.fill: parent
                            anchors.margins: parent.width * 0.05
                            source: rootArea.getPlayerAvatar(playerWestRow.actualPlayerIndex)
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }

                        // Jauge de timer en overlay
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 3
                            radius: 2
                            visible: !gameModel.biddingPhase && gameModel.currentPlayer === playerWestRow.actualPlayerIndex
                            clip: true
                            color: "transparent"
                            z: 100

                            Rectangle {
                                property real fillRatio: gameModel.playTimeRemaining / gameModel.maxPlayTime

                                width: parent.width
                                height: parent.height * fillRatio
                                anchors.bottom: parent.bottom
                                radius: parent.radius
                                color: {
                                    if (gameModel.playTimeRemaining <= 3) return "#B0FF3333"
                                    if (gameModel.playTimeRemaining <= 7) return "#B0FFAA00"
                                    return "#B000CC00"
                                }

                                Behavior on height {
                                    NumberAnimation { duration: 300 }
                                }

                                Behavior on color {
                                    ColorAnimation { duration: 300 }
                                }
                            }
                        }

                        // MouseArea pour afficher les stats
                        MouseArea {
                            anchors.fill: parent
                            property bool isGuest: {
                                var name = rootArea.getPlayerName(playerWestRow.actualPlayerIndex)
                                return !name || name.startsWith("Invité") || name.startsWith("Joueur")
                            }
                            cursorShape: isGuest ? Qt.ArrowCursor : Qt.PointingHandCursor
                            onClicked: {
                                if (!isGuest) {
                                    playerStatsPopup.loadPlayerStats(rootArea.getPlayerName(playerWestRow.actualPlayerIndex))
                                    playerStatsPopup.visible = true
                                }
                            }
                        }
                    }

                    Text {
                        text: rootArea.getPlayerName(playerWestRow.actualPlayerIndex)
                        color: gameModel.currentPlayer === playerWestRow.actualPlayerIndex ? "#ffff66" : "white"
                        font.pixelSize: rootArea.height * 0.04
                        font.bold: gameModel.currentPlayer === playerWestRow.actualPlayerIndex
                        anchors.top: westAvatarRect.bottom
                        anchors.topMargin: rootArea.height * 0.005
                        anchors.horizontalCenter: westAvatarRect.horizontalCenter
                        width: rootArea.width * 0.11
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                    }
                }

                // Jeton de dealer
                Rectangle {
                    width: rootArea.width * 0.03
                    height: rootArea.width * 0.03
                    radius: width / 2
                    color: "#FFD700"
                    border.color: "#8B7500"
                    border.width: 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: gameModel.dealerPosition === playerWestRow.actualPlayerIndex

                    Text {
                        anchors.centerIn: parent
                        text: "D"
                        font.pixelSize: parent.width * 0.6
                        font.bold: true
                        color: "#8B7500"
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

            // Column pour annonce + avatar + dealer, avec nom en overlay
            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: rootArea.height * 0.005

                // Indicateur d'annonce (espace toujours reserve)
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Math.max(bidRowEast.width + rootArea.width * 0.02, rootArea.width * 0.08)
                    height: rootArea.height * 0.045
                    radius: height / 4
                    color: "grey"
                    border.color: "#4a8a4a"
                    border.width: 1
                    opacity: {
                        if (rootArea.getPlayerBidValue(playerEastRow.actualPlayerIndex) === "") return 0
                        if (!gameModel.biddingPhase && rootArea.getPlayerBidValue(playerEastRow.actualPlayerIndex) === "Passe") return 0
                        return 0.85
                    }
                    visible: true  // Toujours visible pour reserver l'espace

                    Behavior on opacity {
                        NumberAnimation { duration: 200 }
                    }

                    Row {
                        id: bidRowEast
                        anchors.centerIn: parent
                        visible: rootArea.getPlayerBidValue(playerEastRow.actualPlayerIndex) !== ""
                        Text {
                            text: rootArea.getPlayerBidValue(playerEastRow.actualPlayerIndex)
                            color: "white"
                            font.pixelSize: rootArea.height * 0.04
                            font.bold: true
                        }
                        Text {
                            text: rootArea.getPlayerBidSymbol(playerEastRow.actualPlayerIndex)
                            color: rootArea.getPlayerBidSymbolColor(playerEastRow.actualPlayerIndex)
                            font.pixelSize: rootArea.height * 0.045
                            font.bold: true
                        }
                    }
                }

                // Item wrapper pour avatar + nom (le nom n'affecte pas la largeur)
                Item {
                    width: rootArea.width * 0.075
                    height: rootArea.width * 0.075 + rootArea.height * 0.005 + rootArea.height * 0.04

                    Rectangle {
                        id: eastAvatarRect
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: rootArea.width * 0.075
                        height: rootArea.width * 0.075
                        radius: 5
                        color: "#80808080"  // Gris avec 50% de transparence
                        border.color: gameModel.currentPlayer === playerEastRow.actualPlayerIndex ? "#ffff66" : "#888888"
                        border.width: 3

                        Image {
                            anchors.fill: parent
                            anchors.margins: parent.width * 0.05
                            source: rootArea.getPlayerAvatar(playerEastRow.actualPlayerIndex)
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }

                        // Jauge de timer en overlay
                        Rectangle {
                            anchors.fill: parent
                            radius: 2
                            anchors.margins: 3
                            visible: !gameModel.biddingPhase && gameModel.currentPlayer === playerEastRow.actualPlayerIndex
                            clip: true
                            color: "transparent"
                            z: 100

                            Rectangle {
                                property real fillRatio: gameModel.playTimeRemaining / gameModel.maxPlayTime

                                width: parent.width
                                height: parent.height * fillRatio
                                anchors.bottom: parent.bottom
                                radius: parent.radius
                                color: {
                                    if (gameModel.playTimeRemaining <= 3) return "#B0FF3333"
                                    if (gameModel.playTimeRemaining <= 7) return "#B0FFAA00"
                                    return "#B000CC00"
                                }

                                Behavior on height {
                                    NumberAnimation { duration: 300 }
                                }

                                Behavior on color {
                                    ColorAnimation { duration: 300 }
                                }
                            }
                        }

                        // MouseArea pour afficher les stats
                        MouseArea {
                            anchors.fill: parent
                            property bool isGuest: {
                                var name = rootArea.getPlayerName(playerEastRow.actualPlayerIndex)
                                return !name || name.startsWith("Invité") || name.startsWith("Joueur")
                            }
                            cursorShape: isGuest ? Qt.ArrowCursor : Qt.PointingHandCursor
                            onClicked: {
                                if (!isGuest) {
                                    playerStatsPopup.loadPlayerStats(rootArea.getPlayerName(playerEastRow.actualPlayerIndex))
                                    playerStatsPopup.visible = true
                                }
                            }
                        }
                    }

                    Text {
                        text: rootArea.getPlayerName(playerEastRow.actualPlayerIndex)
                        color: gameModel.currentPlayer === playerEastRow.actualPlayerIndex ? "#ffff66" : "white"
                        font.pixelSize: rootArea.height * 0.04
                        font.bold: gameModel.currentPlayer === playerEastRow.actualPlayerIndex
                        anchors.top: eastAvatarRect.bottom
                        anchors.topMargin: rootArea.height * 0.005
                        anchors.horizontalCenter: eastAvatarRect.horizontalCenter
                        width: rootArea.width * 0.11
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                    }
                }

                // Jeton de dealer
                Rectangle {
                    width: rootArea.width * 0.03
                    height: rootArea.width * 0.03
                    radius: width / 2
                    color: "#FFD700"
                    border.color: "#8B7500"
                    border.width: 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: gameModel.dealerPosition === playerEastRow.actualPlayerIndex

                    Text {
                        anchors.centerIn: parent
                        text: "D"
                        font.pixelSize: parent.width * 0.6
                        font.bold: true
                        color: "#8B7500"
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
            width: parent.width * 0.2
            height: parent.height * 0.2
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

                Text {
                    text: ""
                    color: "white"
                    font.pixelSize: parent.height * 0.2
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }
                Text {
                    text: "Manche"
                    color: "white"
                    font.pixelSize: parent.height * 0.2
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }
                Text {
                    text: "Total"
                    color: "white"
                    font.pixelSize: parent.height * 0.2
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }

                Text {
                    text: (gameModel.myPosition % 2 === 0) ? "Nous" : "Eux"
                    color: "white"
                    font.family: "Serif"
                    font.italic: true
                    font.weight: Font.DemiBold
                    font.pixelSize: parent.height * 0.2
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }
                Text {
                    text: gameModel.scoreTeam1
                    color: "white"
                    font.pixelSize: parent.height * 0.2
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }
                Text {
                    text: gameModel.scoreTotalTeam1
                    color: "white"
                    font.pixelSize: parent.height * 0.2
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }

                Text {
                    text: (gameModel.myPosition % 2 === 0) ? "Eux" : "Nous"
                    color: "white"
                    font.family: "Serif"
                    font.italic: true
                    font.weight: Font.DemiBold
                    font.pixelSize: parent.height * 0.2
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }
                Text {
                    text: gameModel.scoreTeam2
                    color: "white"
                    font.pixelSize: parent.height * 0.2
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }
                Text {
                    text: gameModel.scoreTotalTeam2
                    color: "white"
                    font.pixelSize: parent.height * 0.2
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }
            }
        }

        // =====================
        // BOUTON COINCHE
        // =====================
        Button {
            id: coincheButton
            text: "COINCHE !"

            // Dimensions et position
            width: rootArea.width * 0.14
            height: rootArea.height * 0.2
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
                font.pixelSize: rootArea.height * 0.05
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
                    font.pixelSize: parent.height * 0.5
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

        // ---- Animation CAPOT ----
        Rectangle {
            anchors.centerIn: parent
            width: playArea.width * 0.6
            height: playArea.height * 0.35
            visible: gameModel.showCapotAnimation
            z: 100

            Text {
                anchors.centerIn: parent
                text: "CAPOT !"
                font.pixelSize: parent.height * 0.6
                font.family: "Serif"
                font.italic: true
                font.weight: Font.Black
                color: "#FFD700"  // Or
                style: Text.Outline
                styleColor: "#8B0000"  // Rouge foncé pour le contour

                // Animation de fade in/out
                opacity: gameModel.showCapotAnimation ? 1.0 : 0.0
                Behavior on opacity {
                    NumberAnimation {
                        duration: 800
                        easing.type: Easing.InOutQuad
                    }
                }

                // Animation d'échelle à l'apparition
                scale: gameModel.showCapotAnimation ? 1.0 : 0.5
                Behavior on scale {
                    NumberAnimation {
                        duration: 600
                        easing.type: Easing.OutBack
                    }
                }

                // Animation de rotation légère
                rotation: gameModel.showCapotAnimation ? 0 : -15
                Behavior on rotation {
                    NumberAnimation {
                        duration: 600
                        easing.type: Easing.OutBack
                    }
                }
            }

            // Animation de pulsation pour le fond
            color: "transparent"
            border.color: "#FFD700"
            border.width: 5 * rootArea.minRatio
            radius: 20 * rootArea.minRatio
            opacity: gameModel.showCapotAnimation ? 0.9 : 0.0
            Behavior on opacity {
                NumberAnimation {
                    duration: 800
                    easing.type: Easing.InOutQuad
                }
            }

            SequentialAnimation on scale {
                running: gameModel.showCapotAnimation
                loops: Animation.Infinite
                NumberAnimation { to: 1.05; duration: 1000; easing.type: Easing.InOutQuad }
                NumberAnimation { to: 1.0; duration: 1000; easing.type: Easing.InOutQuad }
            }
        }

        // =====================
        // DERNIER PLI (en haut à gauche)
        // =====================
        // ZONE EN HAUT À GAUCHE : Bouton Exit + Miniatures dernier pli
        // =====================
        Row {
            id: topLeftRow
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: parent.height * 0.02
            anchors.leftMargin: parent.width * 0.02
            spacing: parent.width * 0.02
            z: 500

            // Bouton Exit
            Button {
                id: exitButton
                width: rootArea.height * 0.10
                height: rootArea.height * 0.12

                background: Rectangle {
                    color: parent.down ? "#aa0000" : (parent.hovered ? "#cc0000" : "#ff3333")
                    radius: 5
                    border.color: "#ff6666"
                    border.width: 2

                    Image {
                        source: "qrc:/resources/exit-svgrepo-com.svg"
                        anchors.fill: parent
                        anchors.margins: parent.width * 0.2
                        fillMode: Image.PreserveAspectFit
                    }
                }

                onClicked: {
                    exitPopup.visible = true
                }
            }

            // Miniatures du dernier pli (en losange)
            Item {
                id: lastPliDisplay
                anchors.verticalCenter: parent.verticalCenter
                width: rootArea.width * 0.15
                height: rootArea.width * 0.15
                visible: gameModel.lastPliCards.length > 0

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

                // Fonction pour obtenir la position relative du joueur
                // par rapport à ma position (gameModel.myPosition)
                function getRelativePosition(playerId) {
                    var myPos = gameModel.myPosition
                    var offset = (playerId - myPos + 4) % 4
                    // offset 0 = sud (moi), 1 = ouest, 2 = nord, 3 = est
                    return offset
                }

                Repeater {
                    model: gameModel.lastPliCards

                    Rectangle {
                        id: miniCard
                        width: rootArea.width * 0.045
                        height: rootArea.width * 0.045
                        color: "white"
                        radius: 3
                        border.color: modelData.isWinner ? "#FFD700" : "#aaaaaa"
                        border.width: modelData.isWinner ? 3 : 1

                        // Position selon la position relative du joueur
                        property int relPos: lastPliDisplay.getRelativePosition(modelData.playerId)

                        // Centre du losange
                        property real centerX: parent.width / 2
                        property real centerY: parent.height / 2

                        // Positionnement en losange
                        x: {
                            switch (relPos) {
                                case 0: return centerX - width / 2           // Sud (centre bas)
                                case 1: return 0                              // Ouest (gauche)
                                case 2: return centerX - width / 2           // Nord (centre haut)
                                case 3: return parent.width - width          // Est (droite)
                                default: return centerX - width / 2
                            }
                        }

                        y: {
                            switch (relPos) {
                                case 0: return parent.height - height        // Sud (bas)
                                case 1: return centerY - height / 2          // Ouest (centre)
                                case 2: return 0                              // Nord (haut)
                                case 3: return centerY - height / 2          // Est (centre)
                                default: return centerY - height / 2
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: lastPliDisplay.getValueText(modelData.value) + lastPliDisplay.getSuitSymbol(modelData.suit)
                            font.pixelSize: parent.height * 0.5
                            font.bold: true
                            color: lastPliDisplay.getTextColor(modelData.suit)
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        // Overlay rouge pour les cartes d'atout
                        Rectangle {
                            anchors.fill: parent
                            color: "#FF0000"
                            opacity: 0.2
                            radius: parent.radius
                            visible: modelData.suit === gameModel.lastBidSuitValue
                        }
                    }
                }
            }
        }

        // =====================
        // BOUTON SURCOINCHE (au centre de l'écran)
        // =====================
        Item {
            anchors.centerIn: playArea
            width: playArea.width * 0.4
            height: playArea.height * 0.4
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

                Button {
                    id: surcoincheButton
                    text: "SURCOINCHE ?"
                    Layout.preferredWidth: parent.width * 0.9
                    Layout.preferredHeight: parent.height * 0.65
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
                        font.pixelSize: surcoincheContainer.height * 0.2
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

                // Barre de progression du temps restant
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.08
                    color: "#333333"
                    radius: height / 2
                    border.color: "#FFD700"
                    border.width: 2

                    Rectangle {
                        width: parent.width * (gameModel.surcoincheTimeLeft / 10)
                        height: parent.height
                        radius: parent.radius
                        color: {
                            if (gameModel.surcoincheTimeLeft <= 3) return "#ff3333"
                            if (gameModel.surcoincheTimeLeft <= 6) return "#ffaa00"
                            return "#00cc00"
                        }

                        Behavior on width {
                            NumberAnimation { duration: 300 }
                        }

                        Behavior on color {
                            ColorAnimation { duration: 300 }
                        }
                    }
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
            z: 1000  // Au-dessus de tout

            // Calculer à quel joueur cette carte est destinée (round-robin)
            // Commencer par le joueur après le dealer, finir par le dealer
            // index % 4 donne 0,1,2,3,0,1,2,3... pour distribuer en séquence
            property int targetPlayer: (gameModel.dealerPosition + 1 + (index % 4)) % 4

            // Décalage dans le temps pour cette carte
            property int cardDelay: index * 300  // 300ms entre chaque carte

            // Position de départ selon le dealer
            Component.onCompleted: {
                if(AudioSettings.effectsEnabled) {
                    //dealingSound.stop()
                    dealingSound.play()
                }
                opacity = 0
                // Positionner selon le dealer: 0=Sud(bas), 1=Ouest(gauche), 2=Nord(haut), 3=Est(droite)
                if (gameModel.dealerPosition === 0) {
                    x = rootArea.width / 2
                    y = rootArea.height + height
                } else if (gameModel.dealerPosition === 1) {
                    x = -width
                    y = rootArea.height / 2
                } else if (gameModel.dealerPosition === 2) {
                    x = rootArea.width / 2
                    y = -height
                } else {
                    x = rootArea.width + width
                    y = rootArea.height / 2
                }
            }

            Card {
                id: firstCard
                anchors.fill: parent
                faceUp: false  // Toujours face cachée pendant l'animation
                value: 0
                suit: 3
                isPlayable: false
            }

            Card {
                id: secondCard
                width: firstCard.width
                height: firstCard.height
                anchors.left: firstCard.right
                anchors.leftMargin: -rootArea.width * 0.02
                faceUp: false  // Toujours face cachée pendant l'animation
                value: 0
                suit: 3
                isPlayable: false
            }

            Card {
                width: secondCard.width
                height: secondCard.height
                anchors.left: secondCard.right
                anchors.leftMargin: -rootArea.width * 0.02
                faceUp: false  // Toujours face cachée pendant l'animation
                value: 0
                suit: 3
                isPlayable: false
                visible: gameModel.distributionPhase !== 2
            }

            // Animation vers chaque joueur
            property real startX: 0
            property real startY: 0
            property real targetX: 0
            property real targetY: 0

            ParallelAnimation {
                id: cardAnimation
                running: false
                NumberAnimation {
                    target: flyingCard
                    property: "x"
                    from: flyingCard.startX
                    to: flyingCard.targetX
                    duration: 500
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: flyingCard
                    property: "y"
                    from: flyingCard.startY
                    to: flyingCard.targetY
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
                    // Convertir la position absolue du dealer en position relative par rapport au joueur local
                    var dealerVisualPosition = rootArea.getVisualPosition(gameModel.dealerPosition)
                    console.log("Animation carte", index, "- Dealer absolu:", gameModel.dealerPosition, "- Dealer visuel:", dealerVisualPosition, "- Target player:", targetPlayer)

                    // Positionner physiquement la carte à la position VISUELLE du dealer : 0=Sud(bas), 1=Ouest(gauche), 2=Nord(haut), 3=Est(droite)
                    if (dealerVisualPosition === 0) {
                        flyingCard.x = rootArea.width / 2
                        flyingCard.y = rootArea.height + flyingCard.height
                        flyingCard.startX = rootArea.width / 2
                        flyingCard.startY = rootArea.height + flyingCard.height
                    } else if (dealerVisualPosition === 1) {
                        flyingCard.x = -flyingCard.width
                        flyingCard.y = rootArea.height / 2
                        flyingCard.startX = -flyingCard.width
                        flyingCard.startY = rootArea.height / 2
                    } else if (dealerVisualPosition === 2) {
                        flyingCard.x = rootArea.width / 2
                        flyingCard.y = -flyingCard.height
                        flyingCard.startX = rootArea.width / 2
                        flyingCard.startY = -flyingCard.height
                    } else {
                        flyingCard.x = rootArea.width + flyingCard.width
                        flyingCard.y = rootArea.height / 2
                        flyingCard.startX = rootArea.width + flyingCard.width
                        flyingCard.startY = rootArea.height / 2
                    }

                    // Rendre visible
                    flyingCard.opacity = 1

                    // Convertir la position absolue du targetPlayer en position relative
                    var targetVisualPosition = rootArea.getVisualPosition(targetPlayer)
                    console.log("Target player absolu:", targetPlayer, "- Target visuel:", targetVisualPosition)

                    // Définir la position cible selon targetVisualPosition : 0=Sud, 1=Ouest, 2=Nord, 3=Est
                    if (targetVisualPosition === 0) {
                        // Sud
                        flyingCard.targetX = rootArea.width / 2
                        flyingCard.targetY = rootArea.height * 0.85
                    } else if (targetVisualPosition === 1) {
                        // Ouest
                        flyingCard.targetX = rootArea.width * 0.05
                        flyingCard.targetY = rootArea.height / 2
                    } else if (targetVisualPosition === 2) {
                        // Nord
                        flyingCard.targetX = rootArea.width / 2
                        flyingCard.targetY = rootArea.height * 0.05
                    } else {
                        // Est
                        flyingCard.targetX = rootArea.width * 0.92
                        flyingCard.targetY = rootArea.height / 2
                    }

                    // Démarrer l'animation
                    cardAnimation.start()
                }
            }
        }
    }

    // Popup de fin de partie
    Loader {
        id: gameOverLoader
        anchors.fill: parent
        active: rootArea.showGameOverPopup
        z: 1000

        sourceComponent: GameOverPopup {
            winnerTeam: rootArea.gameOverWinner
            scoreTeam1: rootArea.gameOverScoreTeam1
            scoreTeam2: rootArea.gameOverScoreTeam2
            myPosition: gameModel.myPosition
            playerNames: [
                rootArea.getPlayerName(0),
                rootArea.getPlayerName(1),
                rootArea.getPlayerName(2),
                rootArea.getPlayerName(3)
            ]

            onReturnToMenu: {
                rootArea.returnToMainMenu()
            }
        }
    }

    // Popup de confirmation de sortie
    ExitGamePopup {
        id: exitPopup
        anchors.fill: parent

        onConfirmExit: {
            // Enregistrer la défaite et retourner au menu
            gameModel.forfeit()
            rootArea.returnToMainMenu()
        }

        onCancelExit: {
            // Ne rien faire, la popup se ferme automatiquement
        }
    }

    // ---- Popup de statistiques du joueur ----
    PlayerStatsPopup {
        id: playerStatsPopup
        anchors.fill: parent

        onClosePopup: {
            playerStatsPopup.visible = false
        }
    }

    Component.onCompleted: {
        console.log("=== CoincheView charge - Instance ID:", rootArea, "===")
        console.log("Stack trace pour identifier le parent:")
        console.log("Parent:", parent)
        console.log("Parent.parent:", parent ? parent.parent : "null")
        console.log("gameModel:", gameModel)
        console.log("myPosition:", gameModel.myPosition)
        console.log("player0Hand:", gameModel.player0Hand)
        console.log("player1Hand:", gameModel.player1Hand)
        console.log("player2Hand:", gameModel.player2Hand)
        console.log("player3Hand:", gameModel.player3Hand)
        if (gameModel.player0Hand) {
            console.log("player0Hand count:", gameModel.player0Hand.count)
            console.log("player0Hand rowCount:", gameModel.player0Hand.rowCount())
        } else {
            console.log("player0Hand est null!")
        }
        console.log("playerSouthRow.actualPlayerIndex:", playerSouthRow.actualPlayerIndex)

        // Démarrer la musique de fond si elle est activée
        if (AudioSettings.musicEnabled) {
            gameMusic.play()
        }
    }
}

