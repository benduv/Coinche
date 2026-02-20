import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtMultimedia

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1024
    height: 768
    title: "Jeu de Coinche"

    // Son de démarrage
    MediaPlayer {
        id: startupSound
        source: "qrc:/resources/sons/watr-double-overhead-11517.mp3"
        audioOutput: AudioOutput {
            id: audioOutput
        }

        onErrorOccurred: function(error, errorString) {
            console.error("MediaPlayer ERREUR:", error, "-", errorString)
        }
    }

    // Fonction pour jouer la musique
    function playMusic() {
        if (AudioSettings.musicEnabled) {
            startupSound.play()
        }
    }

    // Surveiller les changements de paramètres audio
    Connections {
        target: AudioSettings
        function onMusicEnabledChanged() {
            if (AudioSettings.musicEnabled && Qt.application.state === Qt.ApplicationActive) {
                startupSound.play()
            } else {
                startupSound.stop()
            }
        }
    }

    // Surveiller l'état de l'application (premier plan / arrière-plan)
    Connections {
        target: Qt.application
        function onStateChanged() {
            if (Qt.application.state === Qt.ApplicationActive) {
                // L'application revient au premier plan
                // Ne pas relancer la musique du menu si on est en partie (CoincheView)
                var isInGame = mainWindow.shouldLoadCoincheView ||
                               (stackView.currentItem && stackView.currentItem.toString().indexOf("coincheViewLoader") >= 0)
                if (AudioSettings.musicEnabled && !isInGame) {
                    startupSound.play()
                }
            } else if (Qt.application.state === Qt.ApplicationSuspended ||
                       Qt.application.state === Qt.ApplicationHidden ||
                       Qt.application.state === Qt.ApplicationInactive) {
                // L'application passe en arrière-plan ou écran verrouillé
                startupSound.pause()
            }
        }
    }

    // Timer pour retarder le démarrage de la musique (laisse le temps à AudioSettings de charger)
    Timer {
        id: musicStartTimer
        interval: 500  // Attendre 500ms après le chargement
        repeat: false
        onTriggered: {
            playMusic()
        }
    }

    Component.onCompleted: {
        // Ne pas forcer l'orientation ici - le splashScreen gère allOrientations,
        // puis chaque composant (LoginView, mainMenuComponent) gère sa propre orientation
        if (Qt.platform.os !== "android") {
            // Positionner automatiquement la fenêtre au démarrage (desktop uniquement)
            windowPositioner.positionWindow(mainWindow)

            // Empêcher le redimensionnement de la fenêtre après positionnement
            mainWindow.minimumWidth = mainWindow.width
            mainWindow.maximumWidth = mainWindow.width
            mainWindow.minimumHeight = mainWindow.height
            mainWindow.maximumHeight = mainWindow.height
        }
    }

    // Variable pour stocker le nom du joueur connecté
    property string loggedInPlayerName: ""
    property string accountType: ""
    property bool shouldLoadCoincheView: false

    // Fonction helper pour obtenir le nom du joueur actuel
    // Vérifie d'abord networkManager.playerPseudo, puis fallback sur loggedInPlayerName
    function getPlayerName() {
        if (networkManager.playerPseudo !== "") {
            return networkManager.playerPseudo
        }
        return loggedInPlayerName
    }

    // Fonction pour retourner au menu principal depuis n'importe où
    function returnToMainMenu() {
        // Réinitialiser le flag de chargement
        shouldLoadCoincheView = false

        // Nettoyer le gameModel
        networkManager.clearGameModel()

        // Toujours remplacer par le menu principal
        // Cela fonctionne que ce soit depuis CoincheView, MatchMakingView, ou autre
        stackView.replace(mainMenuComponent)

        // Relancer la musique du menu
        if (AudioSettings.musicEnabled && Qt.application.state === Qt.ApplicationActive) {
            startupSound.play()
        }
    }

    // Ratio responsive pour adapter la taille des composants
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Écouter le signal gameFound pour créer le GameModel lors de la reconnexion
    Connections {
        target: networkManager

        function onGameFound(playerPosition, opponents, isReconnection) {
            networkManager.createGameModel(
                networkManager.myPosition,
                networkManager.myCards,
                networkManager.opponents,
                isReconnection
            )
        }

        function onGameModelReady() {
            // MainMenu gère toujours le chargement de CoincheView
            // Vérifier si on n'est pas déjà sur l'écran de chargement
            var currentItem = stackView.currentItem
            var currentItemStr = currentItem ? currentItem.toString() : ""
            var isAlreadyLoading = currentItemStr.indexOf("coincheViewLoader") >= 0

            if (!isAlreadyLoading) {
                mainWindow.shouldLoadCoincheView = false
                stackView.replace(coincheViewLoaderComponent)
                loadDelayTimer.start()
            }
        }

        function onReturnToMainMenu() {
            // Retourner au menu principal (nettoie la pile)
            if (stackView.depth > 1) {
                stackView.pop(null)  // Pop toutes les vues jusqu'à la première (menu principal)
            }
        }

        function onLobbyCreated(lobbyCode) {
            stackView.push(lobbyRoomViewComponent, { "lobbyCode": lobbyCode, "isHost": true })
        }

        function onLobbyJoined(lobbyCode) {
            stackView.push(lobbyRoomViewComponent, { "lobbyCode": lobbyCode, "isHost": false })
        }
    }

    // Timer pour retarder légèrement l'activation du Loader
    // Doit attendre la fin de l'animation de distribution (3250ms)
    Timer {
        id: loadDelayTimer
        interval: 500
        repeat: false
        onTriggered: {
            mainWindow.shouldLoadCoincheView = true
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: splashScreenComponent

        Component {
            id: splashScreenComponent

            Loader {
                anchors.fill: parent
                source: "qrc:/qml/SplashScreen.qml"
                Component.onCompleted: {
                    if (Qt.platform.os === "android") {
                        orientationHelper.setAllOrientations()
                    }
                }
                onLoaded: {
                    // Auto-login réussi pendant le splash -> aller directement au menu principal
                    item.autoLoginSuccess.connect(function(playerName) {
                        mainWindow.loggedInPlayerName = playerName
                        mainWindow.accountType = "account"

                        // Ne pas remplacer si on est déjà dans CoincheView (reconnexion à une partie)
                        var currentItem = stackView.currentItem
                        var currentItemStr = currentItem ? currentItem.toString() : ""
                        var isInGame = currentItemStr.indexOf("CoincheView") >= 0 || currentItemStr.indexOf("coincheViewLoader") >= 0

                        if (!isInGame) {
                            stackView.replace(mainMenuComponent)
                        }
                    })

                    // Pas d'auto-login ou échec -> aller vers LoginView
                    item.loadingComplete.connect(function() {
                        stackView.replace(loginViewComponent)
                    })
                }
            }
        }

        Component {
            id: loginViewComponent

            LoginView {
                onLoginSuccess: function(playerName, accType) {
                    mainWindow.loggedInPlayerName = playerName
                    mainWindow.accountType = accType
                    stackView.replace(mainMenuComponent)
                }

                Config {
                    id: config
                }

                // Timer pour la reconnexion automatique
                Timer {
                    id: reconnectTimer
                    interval: 3000  // Tente de se reconnecter toutes les 3 secondes
                    repeat: true
                    running: false

                    onTriggered: {
                        if (!networkManager.connected) {
                            var serverUrl = config.getServerUrl()
                            networkManager.connectToServer(serverUrl)
                        } else {
                            // Si reconnecté avec succès, arrêter le timer
                            stop()
                        }
                    }
                }

                // Surveiller les changements de connexion
                Connections {
                    target: networkManager

                    function onConnectedChanged() {
                        if (!networkManager.connected) {
                            reconnectTimer.start()
                        } else {
                            reconnectTimer.stop()
                        }
                    }
                }

                // Note: La connexion au serveur est maintenant initiée par SplashScreen
                // Le timer de reconnexion reste actif pour gérer les déconnexions
                Component.onCompleted: {
                    // Autoriser toutes les orientations pour la saisie clavier
                    if (Qt.platform.os === "android") {
                        orientationHelper.setPortrait()
                    }
                    // Démarrer le timer de reconnexion seulement si déconnecté
                    if (!networkManager.connected) {
                        reconnectTimer.start()
                    }
                    // Lancer la musique quand on arrive sur le LoginView
                    musicStartTimer.start()
                }

                Component.onDestruction: {
                    // Restaurer le mode paysage via JNI natif
                    if (Qt.platform.os === "android") {
                        orientationHelper.setLandscape()
                    }
                }
            }
        }

        Component {
            id: mainMenuComponent
            Rectangle {
                anchors.fill: parent
                color: "#0a0a2e"

                // Lancer la musique quand on arrive sur le menu principal
                Component.onCompleted: {
                    // Forcer le mode paysage sur Android
                    if (Qt.platform.os === "android") {
                        orientationHelper.setLandscape()
                    }
                    musicStartTimer.start()
                }

                // Étoiles scintillantes en arrière-plan
                StarryBackground {
                    minRatio: mainWindow.minRatio
                }

                // Animations de fond - Colonnes de symboles de cartes
                // Colonne 1 (gauche) - Coeurs montant
                /*Column {
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
                            color: "#AD1111"
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
                            color: "#422E2E"
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
                    x: parent.width * 0.75
                    y: -parent.height
                    spacing: 80 * mainWindow.minRatio
                    z: 0
                    opacity: 0.15

                    Repeater {
                        model: 15
                        Text {
                            text: "♦"
                            font.pixelSize: 60 * mainWindow.minRatio
                            color: "#AD1111"
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
                    x: parent.width * 0.90
                    y: -parent.height
                    spacing: 80 * mainWindow.minRatio
                    z: 0
                    opacity: 0.15

                    Repeater {
                        model: 15
                        Text {
                            text: "♠"
                            font.pixelSize: 60 * mainWindow.minRatio
                            color: "#422E2E"
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
                }*/

                // Animation de fond - Symboles de cartes tombant comme des flocons
                Item {
                    anchors.fill: parent
                    z: 100
                    clip: false

                    Repeater {
                        model: 20

                        delegate: Text {
                            property int symbolIndex: index
                            property real randomOffset: (symbolIndex * 37) % 30
                            property real oscillationOffset: 0  // Déplacement pour l'oscillation

                            text: {
                                var symbols = ["♥", "♣", "♦", "♠"]
                                return symbols[symbolIndex % 4]
                            }
                            color: {
                                var colors = ["#AD1111", "#422E2E", "#AD1111", "#422E2E"]
                                return colors[symbolIndex % 4]
                            }
                            font.pixelSize: (40 + (symbolIndex % 5) * 12) * mainWindow.minRatio
                            opacity: 0.2 + (symbolIndex % 3) * 0.05

                            // Position horizontale avec binding + oscillation
                            x: {
                                var quarterWidth = mainWindow.width * 0.23
                                var baseX = 0
                                if (symbolIndex < 10) {
                                    // Quart gauche (0-23%)
                                    baseX = (quarterWidth / 10) * symbolIndex + randomOffset
                                } else {
                                    // Quart droit (66-89%) (89% car on a le notch caché)
                                    var rightIndex = symbolIndex - 10
                                    baseX = mainWindow.width * 0.66 + (quarterWidth / 10) * rightIndex + randomOffset
                                }
                                return baseX + oscillationOffset
                            }

                            // Position verticale initiale
                            y: -150 - (symbolIndex % 5) * 50

                            // Animation de chute continue
                            SequentialAnimation on y {
                                running: true
                                loops: Animation.Infinite

                                PauseAnimation {
                                    duration: (symbolIndex * 300) % 2000
                                }

                                NumberAnimation {
                                    to: mainWindow.height + 150
                                    duration: 13000 + (symbolIndex % 8) * 3000
                                    easing.type: Easing.Linear
                                }

                                PropertyAction {
                                    value: -150 - (symbolIndex % 5) * 50
                                }
                            }

                            // Rotation pendant la chute
                            SequentialAnimation on rotation {
                                running: true
                                loops: Animation.Infinite

                                PauseAnimation {
                                    duration: (symbolIndex * 300) % 2000
                                }

                                NumberAnimation {
                                    from: (symbolIndex * 37) % 360
                                    to: (symbolIndex * 37) % 360 + 360
                                    duration: 20000 + (symbolIndex % 5) * 3000
                                    easing.type: Easing.InOutQuad
                                }
                            }

                            // Oscillation horizontale (balancement)
                            SequentialAnimation on oscillationOffset {
                                running: true
                                loops: Animation.Infinite

                                PauseAnimation {
                                    duration: (symbolIndex * 300) % 2000
                                }

                                NumberAnimation {
                                    from: 0
                                    to: 40 * mainWindow.minRatio
                                    duration: 2500 + (symbolIndex % 4) * 500
                                    easing.type: Easing.InOutSine
                                }

                                NumberAnimation {
                                    from: 40 * mainWindow.minRatio
                                    to: 0
                                    duration: 2500 + (symbolIndex % 4) * 500
                                    easing.type: Easing.InOutSine
                                }
                            }
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

                        // Avatar du joueur (cliquable pour changer)
                        Rectangle {
                            width: 120 * mainWindow.minRatio
                            height: width
                            radius: width / 2
                            color: avatarMouseArea.containsMouse ? "#555555" : "#444444"
                            border.color: "#FFD700"
                            border.width: avatarMouseArea.containsMouse ? 3 * mainWindow.minRatio : 2 * mainWindow.minRatio
                            anchors.verticalCenter: parent.verticalCenter

                            Behavior on color { ColorAnimation { duration: 200 } }
                            Behavior on border.width { NumberAnimation { duration: 200 } }

                            Image {
                                anchors.fill: parent
                                anchors.margins: parent.radius / 4
                                source: "qrc:/resources/avatar/" + networkManager.playerAvatar
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                            }

                            // Icône de modification (toujours visible)
                            Rectangle {
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                width: 30 * mainWindow.minRatio
                                height: 30 * mainWindow.minRatio
                                radius: 15 * mainWindow.minRatio
                                color: "#FFD700"

                                Image {
                                    anchors.fill: parent
                                    anchors.margins: 3 * mainWindow.minRatio
                                    source: "qrc:/resources/pencil-svgrepo-com.svg"
                                    fillMode: Image.PreserveAspectFit
                                    smooth: true
                                }
                            }

                            MouseArea {
                                id: avatarMouseArea
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                hoverEnabled: true
                                onClicked: {
                                    avatarSelectorPopup.open()
                                }
                            }
                        }

                        Text {
                            text: mainWindow.getPlayerName() +
                                  (mainWindow.accountType === "guest" ? " (Invité)" : "")
                            font.pixelSize: 42 * mainWindow.minRatio
                            font.bold: true
                            color: "#FFD700"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    // Bouton Jouer
                    Button {
                        Layout.preferredWidth: 300 * mainWindow.widthRatio
                        Layout.preferredHeight: 140 * mainWindow.heightRatio
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
                            // Utiliser le pseudo sauvegardé dans networkManager s'il existe, sinon loggedInPlayerName
                            var pseudoToUse = networkManager.playerPseudo !== "" ? networkManager.playerPseudo : mainWindow.loggedInPlayerName
                            networkManager.registerPlayer(pseudoToUse, networkManager.playerAvatar)
                            stackView.push("qrc:/qml/MatchMakingView.qml")
                        }
                    }

                    // Bouton Jouer avec des amis
                    Button {
                        Layout.preferredWidth: 300 * mainWindow.widthRatio
                        Layout.preferredHeight: 150 * mainWindow.heightRatio
                        Layout.alignment: Qt.AlignHCenter
                        enabled: networkManager.connected

                        background: Rectangle {
                            color: parent.enabled ?
                                   (parent.down ? "#6a4c93" : (parent.hovered ? "#8a6cb3" : "#7a5ca3")) :
                                   "#555555"
                            radius: 10 * mainWindow.minRatio
                            border.color: parent.enabled ? "#FFD700" : "#888888"
                            border.width: 3 * mainWindow.minRatio
                        }

                        contentItem: Column {
                            anchors.centerIn: parent
                            spacing: 5 * mainWindow.minRatio

                            Row {
                                anchors.horizontalCenter: parent.horizontalCenter
                                spacing: 10 * mainWindow.minRatio

                                Text {
                                    text: "👥"
                                    font.pixelSize: 40 * mainWindow.minRatio
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Text {
                                    text: "Jouer"
                                    font.pixelSize: 40 * mainWindow.minRatio
                                    font.bold: true
                                    color: parent.parent.parent.parent.enabled ? "white" : "#aaaaaa"
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            Text {
                                text: "avec des amis"
                                font.pixelSize: 28 * mainWindow.minRatio
                                color: parent.parent.enabled ? "white" : "#aaaaaa"
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }

                        onClicked: {
                            networkManager.registerPlayer(mainWindow.getPlayerName(), networkManager.playerAvatar)
                            stackView.push("qrc:/qml/PrivateLobbyView.qml")
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

                // Bouton Contact en haut à gauche
                Button {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.leftMargin: 50 * mainWindow.minRatio
                    anchors.topMargin: 50 * mainWindow.minRatio
                    width: 120 * mainWindow.minRatio
                    height: 120 * mainWindow.minRatio
                    z: 101

                    background: Rectangle {
                        color: parent.down ? "#0088cc" : (parent.hovered ? "#0099dd" : "#0077bb")
                        radius: 10 * mainWindow.minRatio
                        border.color: "#FFD700"
                        border.width: 3 * mainWindow.minRatio
                    }

                    contentItem: Image {
                        source: "qrc:/resources/message-svgrepo-com.svg"
                        fillMode: Image.PreserveAspectFit
                        anchors.fill: parent
                        anchors.margins: 20 * mainWindow.minRatio
                        smooth: true
                    }

                    onClicked: {
                        stackView.push(contactViewComponent)
                    }
                }

                // Bouton Statistiques en bas à gauche
                Item {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: 50 * mainWindow.minRatio
                    anchors.bottomMargin: 50 * mainWindow.minRatio
                    width: 120 * mainWindow.minRatio
                    height: 120 * mainWindow.minRatio
                    z: 101

                    Button {
                        id: statsButton
                        anchors.fill: parent
                        enabled: mainWindow.accountType !== "guest" && networkManager.connected
                        opacity: (mainWindow.accountType !== "guest" && networkManager.connected) ? 1.0 : 0.4

                        background: Rectangle {
                            color: parent.enabled ?
                                   (parent.down ? "#0088cc" : (parent.hovered ? "#0099dd" : "#0077bb")) :
                                   "#555555"
                            radius: 10 * mainWindow.minRatio
                            border.color: (mainWindow.accountType !== "guest" && networkManager.connected) ? "#FFD700" : "#888888"
                            border.width: 3 * mainWindow.minRatio
                        }

                        contentItem: Image {
                            source: "qrc:/resources/stats-svgrepo-com.svg"
                            fillMode: Image.PreserveAspectFit
                            anchors.fill: parent
                            anchors.margins: 20 * mainWindow.minRatio
                            smooth: true
                        }

                        onClicked: {
                            stackView.push(statsViewComponent)
                        }
                    }

                    // MouseArea pour capturer les clics quand le bouton est désactivé
                    MouseArea {
                        anchors.fill: parent
                        enabled: mainWindow.accountType === "guest"
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            guestMessageRect.visible = true
                            guestMessageTimer.start()
                        }
                    }
                }

                // Message pour les invités
                Rectangle {
                    id: guestMessageRect
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 300 * mainWindow.minRatio
                    width: mainWindow.width * 0.8
                    height: 100 * mainWindow.minRatio
                    radius: 10 * mainWindow.minRatio
                    color: "#2a2a2a"
                    border.color: "#FFD700"
                    border.width: 2 * mainWindow.minRatio
                    visible: false
                    z: 200

                    Text {
                        anchors.centerIn: parent
                        text: "Vous devez avoir un compte pour voir vos statistiques"
                        font.pixelSize: 36 * mainWindow.minRatio
                        color: "#FFD700"
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        width: parent.width - 20 * mainWindow.minRatio
                    }

                    // Animation d'apparition
                    NumberAnimation on opacity {
                        running: guestMessageRect.visible
                        from: 0
                        to: 1
                        duration: 300
                    }

                    Timer {
                        id: guestMessageTimer
                        interval: 5000
                        repeat: false
                        onTriggered: {
                            guestMessageRect.visible = false
                        }
                    }
                }

                // Bouton Règles en haut à droite
                Button {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.rightMargin: 50 * mainWindow.minRatio
                    anchors.topMargin: 50 * mainWindow.minRatio
                    width: 120 * mainWindow.minRatio
                    height: 120 * mainWindow.minRatio
                    z: 101

                    background: Rectangle {
                        color: parent.down ? "#0088cc" : (parent.hovered ? "#0099dd" : "#0077bb")
                        radius: 10 * mainWindow.minRatio
                        border.color: "#FFD700"
                        border.width: 3 * mainWindow.minRatio
                    }

                    contentItem: Image {
                        source: "qrc:/resources/question-svgrepo-com.svg"
                        fillMode: Image.PreserveAspectFit
                        anchors.fill: parent
                        anchors.margins: 17 * mainWindow.minRatio
                        smooth: true
                    }

                    onClicked: {
                        stackView.push(rulesViewComponent)
                    }
                }

                // Bouton Réglages en bas à droite
                Button {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.rightMargin: 50 * mainWindow.minRatio
                    anchors.bottomMargin: 50 * mainWindow.minRatio
                    width: 120 * mainWindow.minRatio
                    height: 120 * mainWindow.minRatio
                    z: 101

                    background: Rectangle {
                        color: parent.down ? "#0088cc" : (parent.hovered ? "#0099dd" : "#0077bb")
                        radius: 10 * mainWindow.minRatio
                        border.color: "#FFD700"
                        border.width: 3 * mainWindow.minRatio
                    }

                    contentItem: Image {
                        source: "qrc:/resources/setting-svgrepo-com.svg"
                        fillMode: Image.PreserveAspectFit
                        anchors.fill: parent
                        anchors.margins: 20 * mainWindow.minRatio
                        smooth: true
                    }

                    onClicked: {
                        stackView.push(configViewComponent)
                    }
                }

                // Popup de sélection d'avatar
                Popup {
                    id: avatarSelectorPopup
                    anchors.centerIn: parent
                    width: Math.min(parent.width, 500 * mainWindow.widthRatio)
                    height: Math.min(parent.height, 600 * mainWindow.heightRatio)
                    modal: true
                    focus: true
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                    parent: Overlay.overlay

                    background: Rectangle {
                        color: "#1a1a1a"
                        radius: 15 * mainWindow.minRatio
                        border.color: "#FFD700"
                        border.width: 3 * mainWindow.minRatio
                    }

                    AvatarSelector {
                        anchors.fill: parent
                        selectedAvatar: networkManager.playerAvatar

                        onAvatarSelected: function(avatar) {
                            networkManager.updateAvatar(avatar)
                            avatarSelectorPopup.close()
                        }
                    }
                }
            }
        }

        Component {
            id: statsViewComponent

            StatsView {
                playerName: mainWindow.getPlayerName()

                onBackToMenu: {
                    stackView.pop()
                }
            }
        }

        Component {
            id: configViewComponent

            Settings {
                playerName: mainWindow.getPlayerName()
                playerEmail: networkManager.playerEmail
                accountType: mainWindow.accountType

                onBackToMenu: {
                    stackView.pop()
                }

                onAccountDeleted: {
                    // Retourner à l'écran de login après suppression du compte
                    mainWindow.loggedInPlayerName = ""
                    mainWindow.accountType = ""
                    networkManager.clearCredentials()
                    while (stackView.depth > 1) {
                        stackView.pop()
                    }
                    stackView.replace(loginViewComponent)
                }
            }
        }

        Component {
            id: rulesViewComponent

            Rules {
                onBackToMenu: {
                    stackView.pop()
                }
            }
        }

        Component {
            id: contactViewComponent

            Contact {
                onBackToMenu: {
                    stackView.pop()
                }
            }
        }

        Component {
            id: lobbyRoomViewComponent

            LobbyRoomView {
                // Les propriétés lobbyCode et isHost seront passées lors du push
            }
        }

        Component {
            id: coincheViewLoaderComponent

            Rectangle {
                id: loaderBackground
                anchors.fill: parent
                color: "#0a0a1a"  // Fond spatial sombre

                // Étoiles scintillantes en arrière-plan
                StarryBackground {
                    minRatio: mainWindow.minRatio
                }

                // Planète 1 - Grande planète rouge/orange (style Mars)
                Rectangle {
                    id: planet1
                    x: loaderBackground.width * 0.15
                    y: loaderBackground.height * 0.25
                    width: 120 * mainWindow.minRatio
                    height: width
                    radius: width / 2
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#ff6b4a" }
                        GradientStop { position: 0.5; color: "#d44a2a" }
                        GradientStop { position: 1.0; color: "#8b2a1a" }
                    }

                    // Ombre sur la planète
                    Rectangle {
                        anchors.fill: parent
                        radius: parent.radius
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: "transparent" }
                            GradientStop { position: 0.6; color: "transparent" }
                            GradientStop { position: 1.0; color: "#40000000" }
                        }
                    }

                    // Animation de flottement
                    SequentialAnimation on y {
                        running: true
                        loops: Animation.Infinite
                        NumberAnimation { to: loaderBackground.height * 0.25 + 15; duration: 3000; easing.type: Easing.InOutSine }
                        NumberAnimation { to: loaderBackground.height * 0.25; duration: 3000; easing.type: Easing.InOutSine }
                    }
                }

                // Planète 2 - Planète bleue avec anneaux (style Saturne)
                Item {
                    id: planet2Container
                    x: loaderBackground.width * 0.75
                    y: loaderBackground.height * 0.6
                    width: 90 * mainWindow.minRatio
                    height: width

                    // Anneaux arrière (derrière la planète) - avec gradients pour plus de réalisme
                    Item {
                        anchors.centerIn: parent
                        width: parent.width * 2.2
                        height: parent.height * 0.5
                        rotation: -20
                        clip: true

                        // Anneau externe principal avec gradient
                        Rectangle {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            radius: width / 2
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "transparent" }
                                GradientStop { position: 0.2; color: "#40667799" }
                                GradientStop { position: 0.5; color: "#608899bb" }
                                GradientStop { position: 0.8; color: "#40667799" }
                                GradientStop { position: 1.0; color: "transparent" }
                            }
                        }

                        // Anneau intermédiaire (bande sombre)
                        Rectangle {
                            anchors.centerIn: parent
                            width: parent.width * 0.85
                            height: parent.height * 0.7
                            radius: width / 2
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "transparent" }
                                GradientStop { position: 0.3; color: "#30445566" }
                                GradientStop { position: 0.5; color: "#50556677" }
                                GradientStop { position: 0.7; color: "#30445566" }
                                GradientStop { position: 1.0; color: "transparent" }
                            }
                        }

                        // Anneau interne clair
                        Rectangle {
                            anchors.centerIn: parent
                            width: parent.width * 0.7
                            height: parent.height * 0.5
                            radius: width / 2
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "transparent" }
                                GradientStop { position: 0.3; color: "#509aabcc" }
                                GradientStop { position: 0.5; color: "#70aabbdd" }
                                GradientStop { position: 0.7; color: "#509aabcc" }
                                GradientStop { position: 1.0; color: "transparent" }
                            }
                        }

                        // Cache pour masquer la partie centrale (sera cachée par la planète)
                        Rectangle {
                            anchors.centerIn: parent
                            width: parent.parent.width * 1.05
                            height: width
                            radius: width / 2
                            color: loaderBackground.color
                        }
                    }

                    // Planète
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width
                        height: width
                        radius: width / 2
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#4a7ab8" }
                            GradientStop { position: 0.5; color: "#2a5a98" }
                            GradientStop { position: 1.0; color: "#1a3a68" }
                        }

                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "transparent" }
                                GradientStop { position: 0.7; color: "transparent" }
                                GradientStop { position: 1.0; color: "#50000000" }
                            }
                        }
                    }

                    // Animation de flottement
                    SequentialAnimation on y {
                        running: true
                        loops: Animation.Infinite
                        NumberAnimation { to: loaderBackground.height * 0.6 - 10; duration: 4000; easing.type: Easing.InOutSine }
                        NumberAnimation { to: loaderBackground.height * 0.6; duration: 4000; easing.type: Easing.InOutSine }
                    }
                }

                // Planète 3 - Petite planète verte/cyan
                Rectangle {
                    id: planet3
                    x: loaderBackground.width * 0.85
                    y: loaderBackground.height * 0.2
                    width: 45 * mainWindow.minRatio
                    height: width
                    radius: width / 2
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#4ade80" }
                        GradientStop { position: 0.5; color: "#22c55e" }
                        GradientStop { position: 1.0; color: "#15803d" }
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: parent.radius
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: "transparent" }
                            GradientStop { position: 0.6; color: "transparent" }
                            GradientStop { position: 1.0; color: "#40000000" }
                        }
                    }

                    SequentialAnimation on y {
                        running: true
                        loops: Animation.Infinite
                        NumberAnimation { to: loaderBackground.height * 0.2 + 8; duration: 2500; easing.type: Easing.InOutSine }
                        NumberAnimation { to: loaderBackground.height * 0.2; duration: 2500; easing.type: Easing.InOutSine }
                    }
                }

                // Grande étoile brillante (soleil lointain)
                Rectangle {
                    id: distantStar
                    x: loaderBackground.width * 0.1
                    y: loaderBackground.height * 0.7
                    width: 8 * mainWindow.minRatio
                    height: width
                    radius: width / 2
                    color: "#ffffaa"

                    // Halo autour de l'étoile
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width * 4
                        height: width
                        radius: width / 2
                        color: "transparent"
                        border.color: "#40ffffaa"
                        border.width: 2 * mainWindow.minRatio

                        SequentialAnimation on scale {
                            running: true
                            loops: Animation.Infinite
                            NumberAnimation { to: 1.5; duration: 1500; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 1.0; duration: 1500; easing.type: Easing.InOutSine }
                        }

                        SequentialAnimation on opacity {
                            running: true
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.3; duration: 1500; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 1.0; duration: 1500; easing.type: Easing.InOutSine }
                        }
                    }

                    SequentialAnimation on opacity {
                        running: true
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.7; duration: 1000 }
                        NumberAnimation { to: 1.0; duration: 1000 }
                    }
                }

                // Indicateur de chargement
                Column {
                    anchors.centerIn: parent
                    spacing: 30 * mainWindow.minRatio
                    visible: !coincheLoader.active
                    z: 10

                    // Icône de joueurs trouvés (4 cercles représentant les joueurs)
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 15 * mainWindow.minRatio

                        Repeater {
                            model: 4
                            Rectangle {
                                width: 50 * mainWindow.minRatio
                                height: 50 * mainWindow.minRatio
                                radius: 25 * mainWindow.minRatio
                                color: "#1a1a2e"
                                border.color: "#FFD700"
                                border.width: 3 * mainWindow.minRatio

                                Text {
                                    anchors.centerIn: parent
                                    text: "✓"
                                    font.pixelSize: 30 * mainWindow.minRatio
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
                        font.pixelSize: 36 * mainWindow.minRatio
                        font.bold: true
                        color: "#FFD700"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Lancement de la partie..."
                        font.pixelSize: 28 * mainWindow.minRatio
                        color: "#cccccc"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    // Points de suspension animés
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 8 * mainWindow.minRatio

                        Repeater {
                            model: 3
                            Rectangle {
                                width: 10 * mainWindow.minRatio
                                height: 10 * mainWindow.minRatio
                                radius: 5 * mainWindow.minRatio
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
                    active: mainWindow.shouldLoadCoincheView

                    sourceComponent: Component {
                        CoincheView {}
                    }

                    onLoaded: {
                        // Arrêter le son du menu principal
                        startupSound.stop()
                    }
                }
            }
        }
    }
}
