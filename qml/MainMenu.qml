import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
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
            console.log("MediaPlayer ERREUR:", error, "-", errorString)
        }

        onPlaybackStateChanged: {
            console.log("MediaPlayer playbackState:", playbackState)
        }

        onMediaStatusChanged: {
            console.log("MediaPlayer mediaStatus:", mediaStatus)
        }
    }

    // Fonction pour jouer la musique avec logs
    function playMusic() {
        console.log("playMusic() appelé - musicEnabled:", AudioSettings.musicEnabled)
        console.log("startupSound.source:", startupSound.source)
        console.log("startupSound.playbackState:", startupSound.playbackState)
        console.log("startupSound.mediaStatus:", startupSound.mediaStatus)

        if (AudioSettings.musicEnabled) {
            console.log("Tentative de lecture de la musique...")
            startupSound.play()
        }
    }

    // Surveiller les changements de paramètres audio
    Connections {
        target: AudioSettings
        function onMusicEnabledChanged() {
            console.log("Signal musicEnabledChanged reçu - musicEnabled:", AudioSettings.musicEnabled)
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
            console.log("Application state changed:", Qt.application.state)
            if (Qt.application.state === Qt.ApplicationActive) {
                // L'application revient au premier plan
                // Ne pas relancer la musique du menu si on est en partie (CoincheView)
                var isInGame = mainWindow.shouldLoadCoincheView ||
                               (stackView.currentItem && stackView.currentItem.toString().indexOf("coincheViewLoader") >= 0)
                console.log("Application active - isInGame:", isInGame)
                if (AudioSettings.musicEnabled && !isInGame) {
                    console.log("Reprise de la musique du menu")
                    startupSound.play()
                }
            } else if (Qt.application.state === Qt.ApplicationSuspended ||
                       Qt.application.state === Qt.ApplicationHidden ||
                       Qt.application.state === Qt.ApplicationInactive) {
                // L'application passe en arrière-plan ou écran verrouillé
                console.log("Application en arrière-plan - arrêt de la musique du menu")
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
            console.log("musicStartTimer déclenché")
            playMusic()
        }
    }

    Component.onCompleted: {
        console.log("MainMenu.onCompleted - Démarrage")
        // Positionner automatiquement la fenêtre au démarrage
        windowPositioner.positionWindow(mainWindow)

        console.log("MainMenu - AudioSettings.musicEnabled:", AudioSettings.musicEnabled)
        console.log("MainMenu - Application state:", Qt.application.state)

        // Utiliser un timer pour laisser le temps à AudioSettings de charger
        musicStartTimer.start()
    }

    // Variable pour stocker le nom du joueur connecté
    property string loggedInPlayerName: ""
    property string accountType: ""
    property bool shouldLoadCoincheView: false

    // Fonction pour retourner au menu principal depuis n'importe où
    function returnToMainMenu() {
        console.log("MainMenu.returnToMainMenu - Debut, stackView depth:", stackView.depth)

        // Réinitialiser le flag de chargement
        shouldLoadCoincheView = false

        // Nettoyer le gameModel
        networkManager.clearGameModel()

        // Pop toutes les pages jusqu'au MainMenu (depth 2)
        while (stackView.depth > 2) {
            console.log("MainMenu.returnToMainMenu - Pop, depth:", stackView.depth)
            stackView.pop()
        }
        console.log("MainMenu.returnToMainMenu - Termine, depth final:", stackView.depth)
    }

    // Ratio responsive pour adapter la taille des composants
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Écouter le signal gameFound pour créer le GameModel lors de la reconnexion
    Connections {
        target: networkManager

        function onGameFound(playerPosition, opponents) {
            console.log("MainMenu - gameFound recu! Position:", playerPosition)
            networkManager.createGameModel(
                networkManager.myPosition,
                networkManager.myCards,
                networkManager.opponents
            )
        }

        function onGameModelReady() {
            console.log("MainMenu - gameModelReady reçu")

            // MainMenu gère toujours le chargement de CoincheView
            // Vérifier si on n'est pas déjà sur l'écran de chargement
            var currentItem = stackView.currentItem
            var currentItemStr = currentItem ? currentItem.toString() : ""
            var isAlreadyLoading = currentItemStr.indexOf("coincheViewLoader") >= 0

            console.log("Current item:", currentItem)
            console.log("Is already loading:", isAlreadyLoading)
            console.log("StackView depth:", stackView.depth)

            if (!isAlreadyLoading) {
                console.log("MainMenu - PUSH coincheViewLoaderComponent")
                mainWindow.shouldLoadCoincheView = false
                stackView.push(coincheViewLoaderComponent)
                loadDelayTimer.start()
            } else {
                console.log("MainMenu - PAS de push, deja en chargement")
            }
        }

        function onLobbyCreated(lobbyCode) {
            console.log("MainMenu - lobbyCreated reçu, code:", lobbyCode)
            stackView.push(lobbyRoomViewComponent, { "lobbyCode": lobbyCode, "isHost": true })
        }

        function onLobbyJoined(lobbyCode) {
            console.log("MainMenu - lobbyJoined reçu, code:", lobbyCode)
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
            console.log("Activation du chargement de CoincheView")
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
                onLoaded: {
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
                    stackView.push(mainMenuComponent)
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
                            console.log("Tentative de reconnexion au serveur...")
                            var serverUrl = config.getServerUrl()
                            networkManager.connectToServer(serverUrl)
                        } else {
                            // Si reconnecté avec succès, arrêter le timer
                            console.log("Reconnexion réussie!")
                            stop()
                        }
                    }
                }

                // Surveiller les changements de connexion
                Connections {
                    target: networkManager

                    function onConnectedChanged() {
                        if (!networkManager.connected) {
                            console.log("Connexion perdue, démarrage de la reconnexion automatique...")
                            reconnectTimer.start()
                        } else {
                            console.log("Connexion établie")
                            reconnectTimer.stop()
                        }
                    }
                }

                Component.onCompleted: {
                    var serverUrl = config.getServerUrl()
                    console.log("Connexion au serveur:", serverUrl)
                    networkManager.connectToServer(serverUrl)
                }
            }
        }

        Component {
            id: mainMenuComponent
            Rectangle {
                anchors.fill: parent
                //color: "#1a1a1a"
                color: "#0a0a2e"
                // Étoiles scintillantes en arrière-plan
                Repeater {
                    model: 80
                    Rectangle {
                        x: Math.random() * mainWindow.width
                        y: Math.random() * mainWindow.height
                        width: (Math.random() * 2 + 1) * mainWindow.minRatio
                        height: width
                        radius: width / 2
                        color: "white"
                        opacity: 0.3

                        SequentialAnimation on opacity {
                            running: true
                            loops: Animation.Infinite
                            PauseAnimation { duration: Math.random() * 2000 }
                            NumberAnimation { to: 0.8; duration: 1000 + Math.random() * 1000 }
                            NumberAnimation { to: 0.3; duration: 1000 + Math.random() * 1000 }
                        }
                    }
                }

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
                            height: 120 * mainWindow.minRatio
                            radius: 60 * mainWindow.minRatio
                            color: avatarMouseArea.containsMouse ? "#555555" : "#444444"
                            border.color: "#FFD700"
                            border.width: avatarMouseArea.containsMouse ? 3 * mainWindow.minRatio : 2 * mainWindow.minRatio
                            anchors.verticalCenter: parent.verticalCenter

                            Behavior on color { ColorAnimation { duration: 200 } }
                            Behavior on border.width { NumberAnimation { duration: 200 } }

                            Image {
                                anchors.fill: parent
                                anchors.margins: 15 * mainWindow.minRatio
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

                                Text {
                                    anchors.centerIn: parent
                                    text: "✎"
                                    font.pixelSize: 18 * mainWindow.minRatio
                                    color: "#000000"
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
                            networkManager.registerPlayer(mainWindow.loggedInPlayerName, networkManager.playerAvatar)
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
                            networkManager.registerPlayer(mainWindow.loggedInPlayerName, networkManager.playerAvatar)
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
                    z: 2

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

                // Bouton Statistiques en bas à gauche (uniquement pour les comptes enregistrés)
                Button {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: 50 * mainWindow.minRatio
                    anchors.bottomMargin: 50 * mainWindow.minRatio
                    width: 120 * mainWindow.minRatio
                    height: 120 * mainWindow.minRatio
                    visible: mainWindow.accountType !== "guest"
                    enabled: networkManager.connected
                    opacity: enabled ? 1.0 : 0.4
                    z: 2

                    background: Rectangle {
                        color: parent.down ? "#0088cc" : (parent.hovered ? "#0099dd" : "#0077bb")
                        radius: 10 * mainWindow.minRatio
                        border.color: "#FFD700"
                        border.width: 3 * mainWindow.minRatio
                    }

                    contentItem: Image {
                        source: "qrc:/resources/stats-svgrepo-com.svg"
                        fillMode: Image.PreserveAspectFit
                        anchors.fill: parent
                        anchors.margins: 15 * mainWindow.minRatio
                        smooth: true
                    }

                    onClicked: {
                        stackView.push(statsViewComponent)
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
                    z: 2

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
                        anchors.margins: 15 * mainWindow.minRatio
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
                    z: 2

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
                        anchors.margins: 15 * mainWindow.minRatio
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
                            console.log("Avatar sélectionné:", avatar)
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
                playerName: mainWindow.loggedInPlayerName

                onBackToMenu: {
                    stackView.pop()
                }
            }
        }

        Component {
            id: configViewComponent

            Settings {
                playerName: mainWindow.loggedInPlayerName
                accountType: mainWindow.accountType

                onBackToMenu: {
                    stackView.pop()
                }

                onAccountDeleted: {
                    // Retourner à l'écran de login après suppression du compte
                    console.log("Compte supprimé - Retour à l'écran de login")
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
                Repeater {
                    model: 80
                    Rectangle {
                        property real starX: Math.random()
                        property real starY: Math.random()
                        property real starSize: 1 + Math.random() * 3
                        property real starDelay: Math.random() * 2000

                        x: starX * loaderBackground.width
                        y: starY * loaderBackground.height
                        width: starSize * mainWindow.minRatio
                        height: width
                        radius: width / 2
                        color: Qt.rgba(1, 1, 1, 0.6 + Math.random() * 0.4)

                        SequentialAnimation on opacity {
                            running: true
                            loops: Animation.Infinite
                            PauseAnimation { duration: starDelay }
                            NumberAnimation { to: 0.2; duration: 800 + Math.random() * 400 }
                            NumberAnimation { to: 1.0; duration: 800 + Math.random() * 400 }
                        }
                    }
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

                    // Anneaux (derrière la planète)
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width * 1.8
                        height: parent.height * 0.3
                        radius: height / 2
                        color: "transparent"
                        border.color: "#8899bb"
                        border.width: 4 * mainWindow.minRatio
                        opacity: 0.6
                        rotation: -20
                    }

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width * 1.5
                        height: parent.height * 0.2
                        radius: height / 2
                        color: "transparent"
                        border.color: "#aabbdd"
                        border.width: 3 * mainWindow.minRatio
                        opacity: 0.5
                        rotation: -20
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
                        console.log("CoincheView chargé avec succès!")
                        // Arrêter le son du menu principal
                        startupSound.stop()
                    }
                }
            }
        }
    }
}
