import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: loginRoot
    anchors.fill: parent
    color: "#0a0a2e"

    // Signal émis quand l'utilisateur est connecté
    signal loginSuccess(string playerName, string accountType)

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
                font.pixelSize: (40 + (symbolIndex % 5) * 12) * loginRoot.minRatio
                opacity: 0.2 + (symbolIndex % 3) * 0.05

                // Position horizontale avec binding + oscillation
                x: {
                    var quarterWidth = loginRoot.width * 0.23
                    var baseX = 0
                    if (symbolIndex < 10) {
                        // Quart gauche (0-23%)
                        baseX = (quarterWidth / 10) * symbolIndex + randomOffset
                    } else {
                        // Quart droit (71-100%)
                        var rightIndex = symbolIndex - 10
                        baseX = loginRoot.width * 0.71 + (quarterWidth / 10) * rightIndex + randomOffset
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
                        to: loginRoot.height + 150
                        duration: 10000 + (symbolIndex % 8) * 2000
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
                        to: 40 * loginRoot.minRatio
                        duration: 2500 + (symbolIndex % 4) * 500
                        easing.type: Easing.InOutSine
                    }

                    NumberAnimation {
                        from: 40 * loginRoot.minRatio
                        to: 0
                        duration: 2500 + (symbolIndex % 4) * 500
                        easing.type: Easing.InOutSine
                    }
                }
            }
        }
    }

    // Ratio responsive pour adapter la taille des composants
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Note: L'auto-login est maintenant géré par SplashScreen.qml
    // LoginView ne gère que les connexions manuelles

    StackView {
        id: loginStack
        anchors.fill: parent
        initialItem: welcomeScreen
        z: 1  // Au-dessus de l'animation de fond

        // Écran de bienvenue
        Component {
            id: welcomeScreen

            Rectangle {
                anchors.centerIn: parent
                color: "#0a0a2e"

                // Étoiles scintillantes en arrière-plan
                StarryBackground {
                    minRatio: loginRoot.minRatio
                }

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 15 * loginRoot.minRatio
                    width: Math.min(parent.width * 0.4, 500 * loginRoot.widthRatio)

                    Text {
                        text: "COINCHE"
                        font.pixelSize: 72 * loginRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: "DE L'ESPACE"
                        font.pixelSize: 36 * loginRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Item { height: 35 * loginRoot.minRatio }

                    // Bouton Créer un compte
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 120 * loginRoot.heightRatio
                        enabled: networkManager.connected
                        opacity: enabled ? 1.0 : 0.4

                        background: Rectangle {
                            color: parent.down ? "#0088cc" : (parent.hovered ? "#00aaee" : "#0099dd")
                            radius: 10 * loginRoot.minRatio
                            border.color: "#FFD700"
                            border.width: 2 * loginRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Créer un compte"
                            font.pixelSize: 48 * loginRoot.minRatio
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            loginStack.push(registerScreen)
                        }
                    }

                    Item { height: 10 * loginRoot.minRatio }

                    // Bouton Se connecter
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 120 * loginRoot.heightRatio
                        enabled: networkManager.connected
                        opacity: enabled ? 1.0 : 0.4

                        background: Rectangle {
                            color: parent.down ? "#00aa00" : (parent.hovered ? "#00dd00" : "#00cc00")
                            radius: 10 * loginRoot.minRatio
                            border.color: "#FFD700"
                            border.width: 2 * loginRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Se connecter"
                            font.pixelSize: 48 * loginRoot.minRatio
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            loginStack.push(loginScreen)
                        }
                    }

                    Item { height: 10 * loginRoot.minRatio }

                    // Bouton Jouer en tant qu'invité
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 120 * loginRoot.heightRatio
                        enabled: networkManager.connected
                        opacity: enabled ? 1.0 : 0.4

                        background: Rectangle {
                            color: parent.down ? "#666666" : (parent.hovered ? "#888888" : "#777777")
                            radius: 10 * loginRoot.minRatio
                            border.color: "#aaaaaa"
                            border.width: 2 * loginRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Jouer en tant qu'invité"
                            font.pixelSize: 48 * loginRoot.minRatio
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            loginStack.push(guestScreen)
                        }
                    }

                    Item { height: 30 * loginRoot.minRatio }

                    // Statut connexion
                    Row {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 10 * loginRoot.minRatio

                        Rectangle {
                            width: 12 * loginRoot.minRatio
                            height: 12 * loginRoot.minRatio
                            radius: 6 * loginRoot.minRatio
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
                            text: networkManager.connected ? "Connecté au serveur" : "Connexion au serveur..."
                            font.pixelSize: 28 * loginRoot.minRatio
                            color: networkManager.connected ? "#00ff00" : "#ff6666"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }
        }

        // Écran d'inscription
        Component {
            id: registerScreen

            Rectangle {
                id: registerScreenRec
                color: "#0a0a2e"

                // Étoiles scintillantes en arrière-plan
                StarryBackground {
                    minRatio: loginRoot.minRatio
                }

                // Variable pour stocker l'avatar sélectionné
                property string selectedAvatar: "avataaars1.svg"

                // Bouton retour en haut à gauche
                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 40 * loginRoot.minRatio
                    width: 100 * loginRoot.minRatio
                    height: 100 * loginRoot.minRatio
                    color: "transparent"
                    z: 100

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width * 0.6
                        height: parent.height * 0.6
                        color: "lightgrey"
                    }

                    Image {
                        anchors.fill: parent
                        source: "qrc:/resources/back-square-svgrepo-com.svg"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            loginStack.pop()
                        }
                        onEntered: {
                            parent.scale = 1.1
                        }
                        onExited: {
                            parent.scale = 1.0
                        }
                    }

                    Behavior on scale {
                        NumberAnimation { duration: 150 }
                    }
                }

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 10 * loginRoot.minRatio
                    width: Math.min(parent.width * 0.4, 500 * loginRoot.widthRatio)

                    Text {
                        text: "Créer un compte"
                        font.pixelSize: 42 * loginRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Item { height: 5 * loginRoot.minRatio }

                    // Pseudonyme
                    Column {
                        Layout.fillWidth: true
                        spacing: 8 * loginRoot.minRatio

                        Text {
                            text: "Pseudonyme"
                            font.pixelSize: 32 * loginRoot.minRatio
                            color: "#aaaaaa"
                        }

                        TextField {
                            id: registerPseudo
                            width: parent.width
                            height: 80 * loginRoot.heightRatio
                            placeholderText: ""
                            font.pixelSize: 36 * loginRoot.minRatio

                            background: Rectangle {
                                color: "#2a2a2a"
                                border.color: registerPseudo.activeFocus ? "#FFD700" : "#555555"
                                border.width: 2 * loginRoot.minRatio
                                radius: 5 * loginRoot.minRatio
                            }

                            color: "white"

                            Text {
                                text: registerPseudo.text.length === 0 ? "   Votre pseudonyme" : ""
                                font.pixelSize: 36 * loginRoot.minRatio
                                color: "#888888"
                                anchors.fill: parent
                                anchors.leftMargin: 10 * loginRoot.minRatio
                                verticalAlignment: Text.AlignVCenter
                                visible: registerPseudo.text.length === 0
                            }
                        }
                    }

                    // Email
                    Column {
                        Layout.fillWidth: true
                        spacing: 8 * loginRoot.minRatio

                        Text {
                            text: "Adresse email"
                            font.pixelSize: 32 * loginRoot.minRatio
                            color: "#aaaaaa"
                        }

                        TextField {
                            id: registerEmail
                            width: parent.width
                            height: 80 * loginRoot.heightRatio
                            placeholderText: ""
                            font.pixelSize: 36 * loginRoot.minRatio

                            background: Rectangle {
                                color: "#2a2a2a"
                                border.color: registerEmail.activeFocus ? "#FFD700" : "#555555"
                                border.width: 2 * loginRoot.minRatio
                                radius: 5 * loginRoot.minRatio
                            }

                            color: "white"

                            Text {
                                text: registerEmail.text.length === 0 ? "   votre@email.com" : ""
                                font.pixelSize: 36 * loginRoot.minRatio
                                color: "#888888"
                                anchors.fill: parent
                                anchors.leftMargin: 10 * loginRoot.minRatio
                                verticalAlignment: Text.AlignVCenter
                                visible: registerEmail.text.length === 0
                            }
                        }
                    }

                    // Mot de passe
                    Column {
                        Layout.fillWidth: true
                        spacing: 8 * loginRoot.minRatio

                        Text {
                            text: "Mot de passe"
                            font.pixelSize: 32 * loginRoot.minRatio
                            color: "#aaaaaa"
                        }

                        TextField {
                            id: registerPassword
                            width: parent.width
                            height: 80 * loginRoot.heightRatio
                            placeholderText: ""
                            echoMode: TextInput.Password
                            font.pixelSize: 36 * loginRoot.minRatio

                            background: Rectangle {
                                color: "#2a2a2a"
                                border.color: registerPassword.activeFocus ? "#FFD700" : "#555555"
                                border.width: 2 * loginRoot.minRatio
                                radius: 5 * loginRoot.minRatio
                            }

                            color: "white"

                            Text {
                                id: regPwdTxtLogin
                                text: registerPassword.text.length === 0 ? "   Votre mot de passe" : ""
                                font.pixelSize: 36 * loginRoot.minRatio
                                color: "#888888"
                                anchors.fill: parent
                                anchors.leftMargin: 10 * loginRoot.minRatio
                                verticalAlignment: Text.AlignVCenter
                                visible: registerPassword.text.length === 0
                            }
                        }
                    }

                    // Bouton pour ouvrir la sélection d'avatar
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100 * loginRoot.heightRatio

                        background: Rectangle {
                            color: parent.down ? "#555555" : (parent.hovered ? "#666666" : "#444444")
                            radius: 8 * loginRoot.minRatio
                            border.color: "#FFD700"
                            border.width: 2 * loginRoot.minRatio
                        }

                        contentItem: Row {
                            spacing: 15 * loginRoot.minRatio
                            anchors.centerIn: parent

                            // Aperçu de l'avatar sélectionné
                            Rectangle {
                                width: 50 * loginRoot.minRatio
                                height: 50 * loginRoot.minRatio
                                radius: 25 * loginRoot.minRatio
                                color: "#2a2a2a"
                                border.color: "#FFD700"
                                border.width: 2 * loginRoot.minRatio
                                anchors.verticalCenter: parent.verticalCenter

                                Image {
                                    anchors.fill: parent
                                    anchors.margins: 3 * loginRoot.minRatio
                                    source: "qrc:/resources/avatar/" + registerScreenRec.selectedAvatar
                                    fillMode: Image.PreserveAspectFit
                                    smooth: true
                                }
                            }

                            Text {
                                text: "Choisir un avatar"
                                font.pixelSize: 32 * loginRoot.minRatio
                                font.bold: true
                                color: "white"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        onClicked: {
                            registerAvatarPopup.open()
                        }
                    }

                    // Popup de sélection d'avatar
                    Popup {
                        id: registerAvatarPopup
                        anchors.centerIn: parent
                        width: Math.min(parent.width, 700 * loginRoot.widthRatio)
                        height: Math.min(parent.height, 600 * loginRoot.heightRatio)
                        modal: true
                        focus: true
                        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

                        background: Rectangle {
                            color: "#1a1a1a"
                            radius: 15 * loginRoot.minRatio
                            border.color: "#FFD700"
                            border.width: 3 * loginRoot.minRatio
                        }

                        AvatarSelector {
                            anchors.fill: parent
                            selectedAvatar: registerScreenRec.selectedAvatar

                            onAvatarSelected: function(avatar) {
                                registerScreenRec.selectedAvatar = avatar
                                registerAvatarPopup.close()
                            }
                        }
                    }

                    // Message d'erreur
                    Text {
                        id: registerError
                        text: ""
                        font.pixelSize: 28 * loginRoot.minRatio
                        color: "#ff6666"
                        Layout.alignment: Qt.AlignHCenter
                        visible: true
                        opacity: text === "" ? 0 : 1
                    }

                    // Bouton créer
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100 * loginRoot.heightRatio

                        background: Rectangle {
                            color: parent.down ? "#0088cc" : (parent.hovered ? "#00aaee" : "#0099dd")
                            radius: 8 * loginRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Créer mon compte"
                            font.pixelSize: 40 * loginRoot.minRatio
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (registerPseudo.text === "" || registerEmail.text === "" || registerPassword.text === "") {
                                registerError.text = "Tous les champs sont obligatoires"
                                return
                            }

                            // Stocker temporairement pour sauvegarder après succès
                            registerScreenRec.pendingEmail = registerEmail.text
                            registerScreenRec.pendingPassword = registerPassword.text

                            // Envoyer la requête au serveur avec l'avatar sélectionné
                            networkManager.registerAccount(registerPseudo.text, registerEmail.text, registerPassword.text, registerScreenRec.selectedAvatar)
                        }
                    }
                }

                // Propriétés pour stocker les credentials en attente de confirmation
                property string pendingEmail: ""
                property string pendingPassword: ""

                Connections {
                    target: networkManager
                    function onRegisterSuccess(playerName, avatar) {
                        // Sauvegarder les credentials pour l'auto-login
                        if (registerScreenRec.pendingEmail !== "" && registerScreenRec.pendingPassword !== "") {
                            networkManager.saveCredentials(registerScreenRec.pendingEmail, registerScreenRec.pendingPassword)
                            registerScreenRec.pendingEmail = ""
                            registerScreenRec.pendingPassword = ""
                        }
                        loginRoot.loginSuccess(playerName, "account")
                    }
                    function onRegisterFailed(error) {
                        registerError.text = error
                        registerScreenRec.pendingEmail = ""
                        registerScreenRec.pendingPassword = ""
                    }
                }
            }
        }

        // Écran de connexion
        Component {
            id: loginScreen

            Rectangle {
                id: loginScreenRect
                //color: "#1a1a1a"
                color: "#0a0a2e"

                // Étoiles scintillantes en arrière-plan
                StarryBackground {
                    minRatio: loginRoot.minRatio
                }

                // Bouton retour en haut à gauche
                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 40 * loginRoot.minRatio
                    width: 100 * loginRoot.minRatio
                    height: 100 * loginRoot.minRatio
                    color: "transparent"
                    z: 100

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width * 0.6
                        height: parent.height * 0.6
                        color: "lightgrey"
                    }

                    Image {
                        anchors.fill: parent
                        source: "qrc:/resources/back-square-svgrepo-com.svg"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            loginStack.pop()
                        }
                        onEntered: {
                            parent.scale = 1.1
                        }
                        onExited: {
                            parent.scale = 1.0
                        }
                    }

                    Behavior on scale {
                        NumberAnimation { duration: 150 }
                    }
                }

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20 * loginRoot.minRatio
                    width: Math.min(parent.width * 0.4, 500 * loginRoot.widthRatio)

                    Text {
                        text: "Se connecter"
                        font.pixelSize: 72 * loginRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Item { height: 10 * loginRoot.minRatio }

                    // Email
                    Column {
                        Layout.fillWidth: true
                        spacing: 8 * loginRoot.minRatio

                        Text {
                            text: "Adresse email"
                            font.pixelSize: 32 * loginRoot.minRatio
                            color: "#aaaaaa"
                        }

                        TextField {
                            id: loginEmail
                            width: parent.width
                            height: 80 * loginRoot.heightRatio
                            placeholderText: ""
                            font.pixelSize: 36 * loginRoot.minRatio

                            background: Rectangle {
                                color: "#2a2a2a"
                                border.color: loginEmail.activeFocus ? "#FFD700" : "#555555"
                                border.width: 2 * loginRoot.minRatio
                                radius: 5 * loginRoot.minRatio
                            }

                            color: "white"

                            Text {
                                text: loginEmail.text.length === 0 ? "   votre@email.com" : ""
                                font.pixelSize: 36 * loginRoot.minRatio
                                color: "#888888"
                                anchors.fill: parent
                                anchors.leftMargin: 10 * loginRoot.minRatio
                                verticalAlignment: Text.AlignVCenter
                                visible: loginEmail.text.length === 0
                            }
                        }
                    }

                    // Mot de passe
                    Column {
                        Layout.fillWidth: true
                        spacing: 8 * loginRoot.minRatio

                        Text {
                            text: "Mot de passe"
                            font.pixelSize: 32 * loginRoot.minRatio
                            color: "#aaaaaa"
                        }

                        TextField {
                            id: loginPassword
                            width: parent.width
                            height: 80 * loginRoot.heightRatio
                            placeholderText: ""
                            echoMode: TextInput.Password
                            font.pixelSize: 36 * loginRoot.minRatio

                            background: Rectangle {
                                color: "#2a2a2a"
                                border.color: loginPassword.activeFocus ? "#FFD700" : "#555555"
                                border.width: 2 * loginRoot.minRatio
                                radius: 5 * loginRoot.minRatio
                            }

                            color: "white"

                            Text {
                                text: loginPassword.text.length === 0 ? "   Votre mot de passe" : ""
                                font.pixelSize: 36 * loginRoot.minRatio
                                color: "#888888"
                                anchors.fill: parent
                                anchors.leftMargin: 10 * loginRoot.minRatio
                                verticalAlignment: Text.AlignVCenter
                                visible: loginPassword.text.length === 0
                            }
                        }
                    }

                    // Message d'erreur
                    Text {
                        id: loginError
                        text: ""
                        font.pixelSize: 28 * loginRoot.minRatio
                        color: "#ff6666"
                        Layout.alignment: Qt.AlignHCenter
                        visible: true
                        opacity: text === "" ? 0 : 1
                    }

                    //Item { height: 10 * loginRoot.minRatio }

                    // Bouton connexion
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100 * loginRoot.heightRatio

                        background: Rectangle {
                            color: parent.down ? "#00aa00" : (parent.hovered ? "#00dd00" : "#00cc00")
                            radius: 8 * loginRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Se connecter"
                            font.pixelSize: 40 * loginRoot.minRatio
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (loginEmail.text === "" || loginPassword.text === "") {
                                loginError.text = "Tous les champs sont obligatoires"
                                return
                            }

                            // Stocker temporairement pour sauvegarder après succès
                            loginScreenRect.pendingEmail = loginEmail.text
                            loginScreenRect.pendingPassword = loginPassword.text
                            networkManager.loginAccount(loginEmail.text, loginPassword.text)
                        }
                    }
                }

                // Propriétés pour stocker les credentials en attente de confirmation
                property string pendingEmail: ""
                property string pendingPassword: ""

                Connections {
                    target: networkManager
                    function onLoginSuccess(playerName, avatar) {
                        // Sauvegarder les credentials pour l'auto-login
                        if (loginScreenRect.pendingEmail !== "" && loginScreenRect.pendingPassword !== "") {
                            networkManager.saveCredentials(loginScreenRect.pendingEmail, loginScreenRect.pendingPassword)
                            loginScreenRect.pendingEmail = ""
                            loginScreenRect.pendingPassword = ""
                        }
                        loginRoot.loginSuccess(playerName, "account")
                    }
                    function onLoginFailed(error) {
                        loginError.text = error
                        loginScreenRect.pendingEmail = ""
                        loginScreenRect.pendingPassword = ""
                    }
                }
            }
        }

        // Écran invité
        Component {
            id: guestScreen

            Rectangle {
                id: guestScreenRect
                //color: "#1a1a1a"
                color: "#0a0a2e"

                // Étoiles scintillantes en arrière-plan
                StarryBackground {
                    minRatio: loginRoot.minRatio
                }

                // Variable pour stocker l'avatar sélectionné
                property string selectedAvatar: "avataaars1.svg"

                // Bouton retour en haut à gauche
                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 40 * loginRoot.minRatio
                    width: 100 * loginRoot.minRatio
                    height: 100 * loginRoot.minRatio
                    color: "transparent"
                    z: 100

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width * 0.6
                        height: parent.height * 0.6
                        color: "lightgrey"
                    }

                    Image {
                        anchors.fill: parent
                        source: "qrc:/resources/back-square-svgrepo-com.svg"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            loginStack.pop()
                        }
                        onEntered: {
                            parent.scale = 1.1
                        }
                        onExited: {
                            parent.scale = 1.0
                        }
                    }

                    Behavior on scale {
                        NumberAnimation { duration: 150 }
                    }
                }

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 25 * loginRoot.minRatio
                    width: Math.min(parent.width * 0.4, 500 * loginRoot.widthRatio)

                    Text {
                        text: "Jouer en tant qu'invité"
                        font.pixelSize: 48 * loginRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Item { height: 20 * loginRoot.minRatio }

                    // Pseudonyme
                    Column {
                        Layout.fillWidth: true
                        spacing: 8 * loginRoot.minRatio

                        Text {
                            text: "Choisissez un pseudonyme"
                            font.pixelSize: 32 * loginRoot.minRatio
                            color: "#aaaaaa"
                        }

                        TextField {
                            id: guestPseudo
                            width: parent.width
                            height: 80 * loginRoot.heightRatio
                            placeholderText: ""
                            text: defaultPlayerName
                            font.pixelSize: 36 * loginRoot.minRatio

                            background: Rectangle {
                                color: "#2a2a2a"
                                border.color: guestPseudo.activeFocus ? "#FFD700" : "#555555"
                                border.width: 2 * loginRoot.minRatio
                                radius: 5 * loginRoot.minRatio
                            }

                            color: "white"

                            Text {
                                text: guestPseudo.text.length === 0 ? "   Invité123" : ""
                                font.pixelSize: 36 * loginRoot.minRatio
                                color: "#888888"
                                anchors.fill: parent
                                anchors.leftMargin: 10 * loginRoot.minRatio
                                verticalAlignment: Text.AlignVCenter
                                visible: guestPseudo.text.length === 0
                            }
                        }
                    }

                    // Bouton pour ouvrir la sélection d'avatar
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100 * loginRoot.heightRatio

                        background: Rectangle {
                            color: parent.down ? "#555555" : (parent.hovered ? "#666666" : "#444444")
                            radius: 8 * loginRoot.minRatio
                            border.color: "#FFD700"
                            border.width: 2 * loginRoot.minRatio
                        }

                        contentItem: Row {
                            spacing: 15 * loginRoot.minRatio
                            anchors.centerIn: parent

                            // Aperçu de l'avatar sélectionné
                            Rectangle {
                                width: 50 * loginRoot.minRatio
                                height: 50 * loginRoot.minRatio
                                radius: 25 * loginRoot.minRatio
                                color: "#2a2a2a"
                                border.color: "#FFD700"
                                border.width: 2 * loginRoot.minRatio
                                anchors.verticalCenter: parent.verticalCenter

                                Image {
                                    anchors.fill: parent
                                    anchors.margins: 3 * loginRoot.minRatio
                                    source: "qrc:/resources/avatar/" + guestScreenRect.selectedAvatar
                                    fillMode: Image.PreserveAspectFit
                                    smooth: true
                                }
                            }

                            Text {
                                text: "Choisir un avatar"
                                font.pixelSize: 32 * loginRoot.minRatio
                                font.bold: true
                                color: "white"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        onClicked: {
                            guestAvatarPopup.open()
                        }
                    }

                    // Popup de sélection d'avatar
                    Popup {
                        id: guestAvatarPopup
                        anchors.centerIn: parent
                        width: Math.min(parent.width, 700 * loginRoot.widthRatio)
                        height: Math.min(parent.height, 600 * loginRoot.heightRatio)
                        modal: true
                        focus: true
                        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

                        background: Rectangle {
                            color: "#1a1a1a"
                            radius: 15 * loginRoot.minRatio
                            border.color: "#FFD700"
                            border.width: 3 * loginRoot.minRatio
                        }

                        AvatarSelector {
                            anchors.fill: parent
                            selectedAvatar: guestScreenRect.selectedAvatar

                            onAvatarSelected: function(avatar) {
                                guestScreenRect.selectedAvatar = avatar
                                guestAvatarPopup.close()
                            }
                        }
                    }

                    Text {
                        text: "En tant qu'invité, votre progression ne sera pas sauvegardée"
                        font.pixelSize: 24 * loginRoot.minRatio
                        color: "#888888"
                        Layout.alignment: Qt.AlignHCenter
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Item { height: 10 * loginRoot.minRatio }

                    // Bouton continuer
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100 * loginRoot.heightRatio

                        background: Rectangle {
                            color: parent.down ? "#666666" : (parent.hovered ? "#888888" : "#777777")
                            radius: 8 * loginRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Continuer"
                            font.pixelSize: 40 * loginRoot.minRatio
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (guestPseudo.text === "") {
                                return
                            }

                            // Récupérer l'avatar sélectionné et enregistrer l'invité avec son avatar
                            //var guestRect = registerScreenRec
                            networkManager.registerPlayer(guestPseudo.text, guestScreenRect.selectedAvatar)
                            loginRoot.loginSuccess(guestPseudo.text, "guest")
                        }
                    }
                }
            }
        }
    }
}
