import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: loginRoot
    anchors.fill: parent
    color: "#1a1a1a"

    // Signal émis quand l'utilisateur est connecté
    signal loginSuccess(string playerName, string accountType)

    // Ratio responsive pour adapter la taille des composants
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Auto-login si les credentials sont fournis
    property bool autoLoginPending: false

    Component.onCompleted: {
        if (autoLoginEmail !== "" && autoLoginPassword !== "") {
            console.log("Auto-login préparé pour:", autoLoginEmail)
            autoLoginPending = true
        }
    }

    function performAutoLogin() {
        if (autoLoginPending && networkManager.connected) {
            console.log("Exécution auto-login avec:", autoLoginEmail)
            networkManager.loginAccount(autoLoginEmail, autoLoginPassword)
            autoLoginPending = false
        }
    }

    Connections {
        target: networkManager

        function onConnectedChanged() {
            if (networkManager.connected && autoLoginPending) {
                performAutoLogin()
            }
        }

        function onLoginSuccess(playerName) {
            loginRoot.loginSuccess(playerName, "account")
        }
        function onLoginFailed(error) {
            console.error("Erreur auto-login:", error)
        }
    }

    StackView {
        id: loginStack
        anchors.fill: parent
        initialItem: welcomeScreen

        // Écran de bienvenue
        Component {
            id: welcomeScreen

            Rectangle {
                color: "#1a1a1a"

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 25 * loginRoot.minRatio
                    width: Math.min(parent.width * 0.4, 500 * loginRoot.widthRatio)

                    Text {
                        text: "COINCHE"
                        font.pixelSize: 72 * loginRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: "Jeu de cartes multijoueur"
                        font.pixelSize: 20 * loginRoot.minRatio
                        color: "#aaaaaa"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Item { height: 40 * loginRoot.minRatio }

                    // Bouton Créer un compte
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 120 * loginRoot.heightRatio

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

                    // Bouton Se connecter
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 120 * loginRoot.heightRatio

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

                    // Bouton Jouer en tant qu'invité
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 120 * loginRoot.heightRatio

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

                    Item { height: 20 * loginRoot.minRatio }

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
                color: "#1a1a1a"

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

                    //Item { height: 10 * loginRoot.minRatio }

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

                            // TODO: Envoyer la requête au serveur
                            networkManager.registerAccount(registerPseudo.text, registerEmail.text, registerPassword.text)
                        }
                    }

                    // Bouton retour
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100 * loginRoot.heightRatio

                        background: Rectangle {
                            color: "transparent"
                            border.color: "#555555"
                            border.width: 2 * loginRoot.minRatio
                            radius: 8 * loginRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Retour"
                            font.pixelSize: 32 * loginRoot.minRatio
                            color: "#aaaaaa"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            loginStack.pop()
                        }
                    }
                }

                Connections {
                    target: networkManager
                    function onRegisterSuccess(playerName) {
                        loginRoot.loginSuccess(playerName, "account")
                    }
                    function onRegisterFailed(error) {
                        registerError.text = error
                    }
                }
            }
        }

        // Écran de connexion
        Component {
            id: loginScreen

            Rectangle {
                color: "#1a1a1a"

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

                            // TODO: Envoyer la requête au serveur
                            networkManager.loginAccount(loginEmail.text, loginPassword.text)
                        }
                    }

                    // Bouton retour
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100 * loginRoot.heightRatio

                        background: Rectangle {
                            color: "transparent"
                            border.color: "#555555"
                            border.width: 2 * loginRoot.minRatio
                            radius: 8 * loginRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Retour"
                            font.pixelSize: 32 * loginRoot.minRatio
                            color: "#aaaaaa"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            loginStack.pop()
                        }
                    }
                }

                Connections {
                    target: networkManager
                    function onLoginSuccess(playerName) {
                        loginRoot.loginSuccess(playerName, "account")
                    }
                    function onLoginFailed(error) {
                        loginError.text = error
                    }
                }
            }
        }

        // Écran invité
        Component {
            id: guestScreen

            Rectangle {
                color: "#1a1a1a"

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 25 * loginRoot.minRatio
                    width: Math.min(parent.width * 0.4, 500 * loginRoot.widthRatio)

                    Text {
                        text: "Jouer en tant qu'invité"
                        font.pixelSize: 72 * loginRoot.minRatio
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

                            loginRoot.loginSuccess(guestPseudo.text, "guest")
                        }
                    }

                    // Bouton retour
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100 * loginRoot.heightRatio

                        background: Rectangle {
                            color: "transparent"
                            border.color: "#555555"
                            border.width: 2 * loginRoot.minRatio
                            radius: 8 * loginRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Retour"
                            font.pixelSize: 32 * loginRoot.minRatio
                            color: "#aaaaaa"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            loginStack.pop()
                        }
                    }
                }
            }
        }
    }
}
