import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Rectangle {
    id: loginRoot
    anchors.fill: parent
    color: "#0a0a2e"

    // Signal émis quand l'utilisateur est connecté
    signal loginSuccess(string playerName, string accountType)

    // Properties for password change flow
    property string pendingChangeEmail: ""
    property string pendingChangePlayerName: ""
    property string pendingChangePassword: ""

    // Détection d'orientation
    property bool isPortrait: height > width
    property bool isLandscape: width > height

    // Ratio responsive adapté à l'orientation
    property real widthRatio: isPortrait ? width / 600 : width / 1024
    property real heightRatio: isPortrait ? height / 1024 : height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Largeur des formulaires adaptée à l'orientation
    property real formWidthRatio: isPortrait ? 0.85 : 0.4

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
                    width: Math.min(parent.width * loginRoot.formWidthRatio, 500 * loginRoot.widthRatio)

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
                            font.pixelSize: 52 * loginRoot.minRatio
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
                            font.pixelSize: 46 * loginRoot.minRatio
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
                    width: Math.min(parent.width * loginRoot.formWidthRatio, 500 * loginRoot.widthRatio)

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
                            height: 70 * loginRoot.heightRatio
                            placeholderText: ""
                            font.pixelSize: 30 * loginRoot.minRatio
                            maximumLength: 12

                            background: Rectangle {
                                color: "#2a2a2a"
                                border.color: registerPseudo.activeFocus ? "#FFD700" : "#555555"
                                border.width: 2 * loginRoot.minRatio
                                radius: 5 * loginRoot.minRatio
                            }

                            color: "white"

                            Text {
                                text: registerPseudo.text.length === 0 ? "   Votre pseudonyme" : ""
                                font.pixelSize: 30 * loginRoot.minRatio
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
                            height: 70 * loginRoot.heightRatio
                            placeholderText: ""
                            font.pixelSize: 30 * loginRoot.minRatio
                            inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase

                            background: Rectangle {
                                color: "#2a2a2a"
                                border.color: registerEmail.activeFocus ? "#FFD700" : "#555555"
                                border.width: 2 * loginRoot.minRatio
                                radius: 5 * loginRoot.minRatio
                            }

                            color: "white"

                            Text {
                                text: registerEmail.text.length === 0 ? "   votre@email.com" : ""
                                font.pixelSize: 30 * loginRoot.minRatio
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

                        property bool showPassword: false

                        Text {
                            text: "Mot de passe"
                            font.pixelSize: 32 * loginRoot.minRatio
                            color: "#aaaaaa"
                        }

                        Item {
                            width: parent.width
                            height: 70 * loginRoot.heightRatio

                            TextField {
                                id: registerPassword
                                width: parent.width
                                height: parent.height
                                placeholderText: ""
                                echoMode: parent.parent.showPassword ? TextInput.Normal : TextInput.Password
                                font.pixelSize: 30 * loginRoot.minRatio

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
                                    font.pixelSize: 30 * loginRoot.minRatio
                                    color: "#888888"
                                    anchors.fill: parent
                                    anchors.leftMargin: 10 * loginRoot.minRatio
                                    verticalAlignment: Text.AlignVCenter
                                    visible: registerPassword.text.length === 0
                                }
                            }

                            // Icône œil pour afficher/masquer le mot de passe
                            Rectangle {
                                anchors.right: parent.right
                                anchors.rightMargin: 10 * loginRoot.minRatio
                                anchors.verticalCenter: parent.verticalCenter
                                width: 50 * loginRoot.minRatio
                                height: 50 * loginRoot.minRatio
                                color: "transparent"

                                Image {
                                    anchors.centerIn: parent
                                    width: 35 * loginRoot.minRatio
                                    height: 35 * loginRoot.minRatio
                                    source: parent.parent.parent.showPassword ? "qrc:/resources/eye-slash-svgrepo-com.svg" : "qrc:/resources/eye-svgrepo-com.svg"
                                    fillMode: Image.PreserveAspectFit
                                    smooth: true
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        parent.parent.parent.showPassword = !parent.parent.parent.showPassword
                                    }
                                }
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
                                width: 70 * loginRoot.minRatio
                                height: width
                                radius: width / 2
                                color: "#2a2a2a"
                                border.color: "#FFD700"
                                border.width: 2 * loginRoot.minRatio
                                anchors.verticalCenter: parent.verticalCenter

                                Image {
                                    anchors.fill: parent
                                    anchors.margins: parent.radius / 4
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

                    Item { height: 10 * loginRoot.minRatio }

                    // Case à cocher RGPD
                    Row {
                        Layout.fillWidth: true
                        spacing: 10 * loginRoot.minRatio

                        Rectangle {
                            width: 40 * loginRoot.minRatio
                            height: 40 * loginRoot.minRatio
                            color: "#2a2a2a"
                            border.color: gdprCheckbox.checked ? "#FFD700" : "#555555"
                            border.width: 2 * loginRoot.minRatio
                            radius: 5 * loginRoot.minRatio
                            anchors.verticalCenter: parent.verticalCenter

                            Rectangle {
                                visible: gdprCheckbox.checked
                                anchors.centerIn: parent
                                width: parent.width * 0.6
                                height: parent.height * 0.6
                                color: "#FFD700"
                                radius: 3 * loginRoot.minRatio
                            }

                            MouseArea {
                                id: gdprCheckbox
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                property bool checked: false
                                onClicked: {
                                    checked = !checked
                                }
                            }
                        }

                        Column {
                            spacing: 5 * loginRoot.minRatio
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width - 50 * loginRoot.minRatio

                            Text {
                                text: "J'ai lu et j'accepte la Politique de confidentialité"
                                font.pixelSize: 24 * loginRoot.minRatio
                                color: "white"
                                wrapMode: Text.WordWrap
                                width: parent.width
                            }

                            Text {
                                text: "Voir la Politique de confidentialité"
                                font.pixelSize: 22 * loginRoot.minRatio
                                color: "#00aaee"
                                font.underline: true

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        privacyPolicyPopup.open()
                                    }
                                }
                            }
                        }
                    }

                    // Popup pour afficher la politique de confidentialité
                    Popup {
                        id: privacyPolicyPopup
                        anchors.centerIn: parent
                        width: Math.min(parent.width * 0.95, 900 * loginRoot.widthRatio)
                        height: Math.min(parent.height * 0.9, 700 * loginRoot.heightRatio)
                        modal: true
                        focus: true
                        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                        padding: 0

                        background: Rectangle {
                            color: "#0a0a2e"
                            radius: 15 * loginRoot.minRatio
                            border.color: "#FFD700"
                            border.width: 3 * loginRoot.minRatio

                            // Étoiles scintillantes en arrière-plan
                            StarryBackground {
                                anchors.fill: parent
                                minRatio: loginRoot.minRatio
                            }
                        }

                        // Bouton fermer en haut à droite
                        Rectangle {
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.margins: 20 * loginRoot.minRatio
                            width: 80 * loginRoot.minRatio
                            height: 80 * loginRoot.minRatio
                            color: "#cc4444"
                            radius: 10 * loginRoot.minRatio
                            z: 100

                            Text {
                                anchors.centerIn: parent
                                text: "✕"
                                font.pixelSize: 48 * loginRoot.minRatio
                                font.bold: true
                                color: "white"
                            }

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    privacyPolicyPopup.close()
                                }
                                onEntered: {
                                    parent.color = "#dd5555"
                                }
                                onExited: {
                                    parent.color = "#cc4444"
                                }
                            }
                        }

                        // ScrollView pour le contenu
                        ScrollView {
                            anchors.fill: parent
                            anchors.margins: 40 * loginRoot.minRatio
                            anchors.topMargin: 100 * loginRoot.minRatio
                            clip: true

                            ScrollBar.vertical: ScrollBar {
                                policy: ScrollBar.AsNeeded
                                width: 12 * loginRoot.minRatio
                                contentItem: Rectangle {
                                    implicitWidth: 12 * loginRoot.minRatio
                                    radius: 6 * loginRoot.minRatio
                                    color: "#FFD700"
                                    opacity: parent.pressed ? 1.0 : (parent.hovered ? 0.8 : 0.6)
                                }
                            }

                            ColumnLayout {
                                width: Math.min(parent.parent.width - 80 * loginRoot.minRatio, 1000 * loginRoot.minRatio)
                                spacing: 20 * loginRoot.minRatio

                                // Titre principal
                                Text {
                                    text: "Politique de Confidentialité"
                                    font.pixelSize: 52 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.alignment: Qt.AlignHCenter
                                    Layout.bottomMargin: 10 * loginRoot.minRatio
                                }

                                Text {
                                    text: "Coinche de l'Espace"
                                    font.pixelSize: 36 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.alignment: Qt.AlignHCenter
                                }

                                Text {
                                    text: "Dernière mise à jour : 18 février 2026"
                                    font.pixelSize: 24 * loginRoot.minRatio
                                    color: "#aaaaaa"
                                    Layout.alignment: Qt.AlignHCenter
                                    Layout.bottomMargin: 20 * loginRoot.minRatio
                                }

                                // Section 1 - Responsable du traitement
                                Text {
                                    text: "1. Responsable du traitement"
                                    font.pixelSize: 32 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.fillWidth: true
                                }

                                Text {
                                    text: "Le responsable du traitement des données est :\n\nNebuludik\nEmail : contact@nebuludik.fr"
                                    font.pixelSize: 26 * loginRoot.minRatio
                                    color: "white"
                                    Layout.fillWidth: true
                                    wrapMode: Text.WordWrap
                                }

                                // Section 2 - Introduction
                                Text {
                                    text: "2. Introduction"
                                    font.pixelSize: 32 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.fillWidth: true
                                    Layout.topMargin: 10 * loginRoot.minRatio
                                }

                                Text {
                                    text: "La protection de votre vie privée est importante pour nous. Cette politique explique quelles données sont collectées lorsque vous utilisez Coinche de l'Espace, pourquoi elles sont collectées et comment elles sont protégées.\n\nEn créant un compte et en utilisant l'application, vous acceptez les pratiques décrites ci-dessous."
                                    font.pixelSize: 26 * loginRoot.minRatio
                                    color: "white"
                                    Layout.fillWidth: true
                                    wrapMode: Text.WordWrap
                                }

                                // Section 3 - Données collectées
                                Text {
                                    text: "3. Données collectées"
                                    font.pixelSize: 32 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.fillWidth: true
                                    Layout.topMargin: 10 * loginRoot.minRatio
                                }

                                Text {
                                    text: "Nous collectons uniquement les données strictement nécessaires au fonctionnement du jeu :\n\n• Adresse e-mail — création et gestion du compte\n• Pseudonyme — identification en jeu\n• Mot de passe chiffré — sécurité du compte\n• Statistiques de jeu — scores, parties, classements\n• Date de consentement RGPD — traçabilité de votre accord\n\nNous ne collectons aucune donnée sensible, ni localisation, ni contacts, ni fichiers personnels."
                                    font.pixelSize: 26 * loginRoot.minRatio
                                    color: "white"
                                    Layout.fillWidth: true
                                    wrapMode: Text.WordWrap
                                }

                                // Section 4 - Finalités du traitement
                                Text {
                                    text: "4. Finalités du traitement"
                                    font.pixelSize: 32 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.fillWidth: true
                                    Layout.topMargin: 10 * loginRoot.minRatio
                                }

                                Text {
                                    text: "Les données sont utilisées uniquement pour :\n\n• Créer et gérer votre compte utilisateur\n• Permettre le fonctionnement du jeu en ligne\n• Afficher les scores et classements\n• Répondre aux demandes d'assistance\n• Sécuriser les comptes\n\nAucune donnée n'est utilisée à des fins publicitaires ou commerciales."
                                    font.pixelSize: 26 * loginRoot.minRatio
                                    color: "white"
                                    Layout.fillWidth: true
                                    wrapMode: Text.WordWrap
                                }

                                // Section 5 - Base légale
                                Text {
                                    text: "5. Base légale"
                                    font.pixelSize: 32 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.fillWidth: true
                                    Layout.topMargin: 10 * loginRoot.minRatio
                                }

                                Text {
                                    text: "Le traitement repose sur :\n\n• L'exécution du service (fonctionnement du jeu)\n• Votre consentement explicite lors de la création du compte"
                                    font.pixelSize: 26 * loginRoot.minRatio
                                    color: "white"
                                    Layout.fillWidth: true
                                    wrapMode: Text.WordWrap
                                }

                                // Section 6 - Hébergement et sécurité
                                Text {
                                    text: "6. Hébergement et sécurité"
                                    font.pixelSize: 32 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.fillWidth: true
                                    Layout.topMargin: 10 * loginRoot.minRatio
                                }

                                Text {
                                    text: "• Les données sont hébergées chez OVHcloud sur des serveurs situés en France\n• Les communications sont chiffrées via protocole SSL/TLS\n• Les mots de passe sont stockés sous forme hachée et sécurisée (SHA-256 + salt)\n• Des mesures techniques sont mises en œuvre pour empêcher tout accès non autorisé"
                                    font.pixelSize: 26 * loginRoot.minRatio
                                    color: "white"
                                    Layout.fillWidth: true
                                    wrapMode: Text.WordWrap
                                }

                                // Section 7 - Partage des données
                                Text {
                                    text: "7. Partage des données"
                                    font.pixelSize: 32 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.fillWidth: true
                                    Layout.topMargin: 10 * loginRoot.minRatio
                                }

                                Text {
                                    text: "Nous ne vendons, louons ni partageons vos données personnelles avec des tiers.\n\nLes données sont utilisées exclusivement pour le fonctionnement de l'application."
                                    font.pixelSize: 26 * loginRoot.minRatio
                                    color: "white"
                                    Layout.fillWidth: true
                                    wrapMode: Text.WordWrap
                                }

                                // Section 8 - Durée de conservation
                                Text {
                                    text: "8. Durée de conservation"
                                    font.pixelSize: 32 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.fillWidth: true
                                    Layout.topMargin: 10 * loginRoot.minRatio
                                }

                                Text {
                                    text: "• Les données de compte sont conservées tant que votre compte est actif\n• En cas de suppression du compte, les données sont supprimées sous 30 jours maximum\n• Les logs de sécurité sont conservés 30 jours pour la détection de brute force"
                                    font.pixelSize: 26 * loginRoot.minRatio
                                    color: "white"
                                    Layout.fillWidth: true
                                    wrapMode: Text.WordWrap
                                }

                                // Section 9 - Droits des utilisateurs (RGPD)
                                Text {
                                    text: "9. Droits des utilisateurs (RGPD)"
                                    font.pixelSize: 32 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.fillWidth: true
                                    Layout.topMargin: 10 * loginRoot.minRatio
                                }

                                Text {
                                    text: "Conformément au Règlement Général sur la Protection des Données, vous disposez des droits suivants :\n\n• Droit d'accès à vos données\n• Droit de rectification (modifier votre email, pseudo, mot de passe)\n• Droit de suppression (supprimer votre compte)\n• Droit à la limitation du traitement\n• Droit d'opposition\n• Droit à la portabilité des données\n\nPour exercer ces droits, contactez-nous à :\n📧 contact@nebuludik.fr"
                                    font.pixelSize: 26 * loginRoot.minRatio
                                    color: "white"
                                    Layout.fillWidth: true
                                    wrapMode: Text.WordWrap
                                }

                                // Section 10 - Suppression de compte
                                Text {
                                    text: "10. Suppression de compte"
                                    font.pixelSize: 32 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.fillWidth: true
                                    Layout.topMargin: 10 * loginRoot.minRatio
                                }

                                Text {
                                    text: "Vous pouvez supprimer votre compte directement depuis l'application dans les paramètres.\n\nToutes vos données personnelles (email, pseudo, mot de passe, statistiques) seront supprimées définitivement sous 30 jours."
                                    font.pixelSize: 26 * loginRoot.minRatio
                                    color: "white"
                                    Layout.fillWidth: true
                                    wrapMode: Text.WordWrap
                                }

                                // Section 11 - Enfants
                                Text {
                                    text: "11. Enfants"
                                    font.pixelSize: 32 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.fillWidth: true
                                    Layout.topMargin: 10 * loginRoot.minRatio
                                }

                                Text {
                                    text: "L'application ne cible pas spécifiquement les enfants de moins de 13 ans et ne collecte pas sciemment de données les concernant."
                                    font.pixelSize: 26 * loginRoot.minRatio
                                    color: "white"
                                    Layout.fillWidth: true
                                    wrapMode: Text.WordWrap
                                }

                                // Section 12 - Modifications
                                Text {
                                    text: "12. Modifications"
                                    font.pixelSize: 32 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.fillWidth: true
                                    Layout.topMargin: 10 * loginRoot.minRatio
                                }

                                Text {
                                    text: "Cette politique peut être modifiée à tout moment. La version la plus récente est toujours disponible dans l'application."
                                    font.pixelSize: 26 * loginRoot.minRatio
                                    color: "white"
                                    Layout.fillWidth: true
                                    wrapMode: Text.WordWrap
                                }

                                // Section 13 - Contact
                                Text {
                                    text: "13. Contact"
                                    font.pixelSize: 32 * loginRoot.minRatio
                                    font.bold: true
                                    color: "#FFD700"
                                    Layout.fillWidth: true
                                    Layout.topMargin: 10 * loginRoot.minRatio
                                }

                                Text {
                                    text: "Pour toute question concernant la confidentialité :\n\n📧 contact@nebuludik.fr"
                                    font.pixelSize: 26 * loginRoot.minRatio
                                    color: "white"
                                    Layout.fillWidth: true
                                    wrapMode: Text.WordWrap
                                    Layout.bottomMargin: 40 * loginRoot.minRatio
                                }
                            }
                        }
                    }

                    // Message d'erreur
                    Text {
                        id: registerError
                        text: ""
                        font.pixelSize: 24 * loginRoot.minRatio
                        color: "#ff6666"
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
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

                            if (!gdprCheckbox.checked) {
                                registerError.text = "Vous devez accepter la Politique de confidentialité"
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
                    id: loginForm
                    width: Math.min(parent.width * loginRoot.formWidthRatio, 500 * loginRoot.widthRatio)
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: loginRoot.isPortrait ? 200 * loginRoot.minRatio : 100 * loginRoot.minRatio
                    spacing: loginRoot.isPortrait ? 15 * loginRoot.minRatio : 20 * loginRoot.minRatio

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
                            font.pixelSize: 30 * loginRoot.minRatio
                            color: "#aaaaaa"
                        }

                        TextField {
                            id: loginEmail
                            width: parent.width
                            height: 70 * loginRoot.heightRatio
                            placeholderText: ""
                            font.pixelSize: 36 * loginRoot.minRatio
                            inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase

                            background: Rectangle {
                                color: "#2a2a2a"
                                border.color: loginEmail.activeFocus ? "#FFD700" : "#555555"
                                border.width: 2 * loginRoot.minRatio
                                radius: 5 * loginRoot.minRatio
                            }

                            color: "white"

                            Text {
                                text: loginEmail.text.length === 0 ? "   votre@email.com" : ""
                                font.pixelSize: 30 * loginRoot.minRatio
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
                            font.pixelSize: 30 * loginRoot.minRatio
                            color: "#aaaaaa"
                        }

                        TextField {
                            id: loginPassword
                            width: parent.width
                            height: 70 * loginRoot.heightRatio
                            placeholderText: ""
                            echoMode: TextInput.Password
                            font.pixelSize: 30 * loginRoot.minRatio
                            inputMethodHints: Qt.ImhSensitiveData | Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase

                            background: Rectangle {
                                color: "#2a2a2a"
                                border.color: loginPassword.activeFocus ? "#FFD700" : "#555555"
                                border.width: 2 * loginRoot.minRatio
                                radius: 5 * loginRoot.minRatio
                            }

                            color: "white"

                            Text {
                                text: loginPassword.text.length === 0 ? "   Votre mot de passe" : ""
                                font.pixelSize: 30 * loginRoot.minRatio
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
                        font.pixelSize: 24 * loginRoot.minRatio
                        color: "#ff6666"
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        visible: true
                        opacity: text === "" ? 0 : 1
                    }

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
                            Qt.inputMethod.hide()

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

                    // Bouton "Mot de passe oublié"
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 60 * loginRoot.heightRatio

                        background: Rectangle {
                            color: "transparent"
                        }

                        contentItem: Text {
                            text: "Mot de passe oublié ?"
                            font.pixelSize: 26 * loginRoot.minRatio
                            color: parent.hovered ? "#FFD700" : "#aaaaaa"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.underline: parent.hovered
                        }

                        onClicked: {
                            Qt.inputMethod.hide()
                            loginStack.push(forgotPasswordScreen)
                        }
                    }

                    Item { height: 20 * loginRoot.minRatio }
                }

                // Propriétés pour stocker les credentials en attente de confirmation
                property string pendingEmail: ""
                property string pendingPassword: ""

                Connections {
                    target: networkManager
                    function onLoginSuccess(playerName, avatar, usingTempPassword) {
                        if (usingTempPassword) {
                            // Force password change - don't save credentials yet
                            console.log("Login with temp password - forcing password change")
                            // Store email and info for changePasswordScreen
                            loginRoot.pendingChangeEmail = loginScreenRect.pendingEmail
                            loginRoot.pendingChangePlayerName = playerName
                            loginRoot.pendingChangePassword = ""
                            loginStack.push(changePasswordScreenComponent)
                        } else {
                            // Normal login - save credentials and proceed
                            if (loginScreenRect.pendingEmail !== "" && loginScreenRect.pendingPassword !== "") {
                                networkManager.saveCredentials(loginScreenRect.pendingEmail, loginScreenRect.pendingPassword)
                                loginScreenRect.pendingEmail = ""
                                loginScreenRect.pendingPassword = ""
                            }
                            loginRoot.loginSuccess(playerName, "account")
                        }
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
                    spacing: loginRoot.isPortrait ? 20 * loginRoot.minRatio : 25 * loginRoot.minRatio
                    width: Math.min(parent.width * loginRoot.formWidthRatio, 500 * loginRoot.widthRatio)

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
                            height: 70 * loginRoot.heightRatio
                            placeholderText: ""
                            text: defaultPlayerName
                            font.pixelSize: 30 * loginRoot.minRatio
                            maximumLength: 12

                            background: Rectangle {
                                color: "#2a2a2a"
                                border.color: guestPseudo.activeFocus ? "#FFD700" : "#555555"
                                border.width: 2 * loginRoot.minRatio
                                radius: 5 * loginRoot.minRatio
                            }

                            color: "white"

                            Text {
                                text: guestPseudo.text.length === 0 ? "   Invité123" : ""
                                font.pixelSize: 30 * loginRoot.minRatio
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
                                width: 70 * loginRoot.minRatio
                                height: width
                                radius: width / 2
                                color: "#2a2a2a"
                                border.color: "#FFD700"
                                border.width: 2 * loginRoot.minRatio
                                anchors.verticalCenter: parent.verticalCenter

                                Image {
                                    anchors.fill: parent
                                    anchors.margins: parent.radius / 4
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
                        color: "orange"
                        Layout.alignment: Qt.AlignHCenter
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        text: "En jouant, vous acceptez notre Politique de confidentialité"
                        font.pixelSize: 20 * loginRoot.minRatio
                        color: "#aaaaaa"
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
                            color: parent.down ? "#00aa00" : (parent.hovered ? "#00dd00" : "#00cc00")
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

        // Écran "Mot de passe oublié"
        Component {
            id: forgotPasswordScreen

            Rectangle {
                id: forgotPasswordScreenRect
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
                    id: forgotPasswordForm
                    width: Math.min(parent.width * loginRoot.formWidthRatio, 500 * loginRoot.widthRatio)
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: loginRoot.isPortrait ? 220 * loginRoot.minRatio : 120 * loginRoot.minRatio
                    spacing: loginRoot.isPortrait ? 15 * loginRoot.minRatio : 20 * loginRoot.minRatio

                    Text {
                        text: "Mot de passe oublié"
                        font.pixelSize: 52 * loginRoot.minRatio
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
                            font.pixelSize: 30 * loginRoot.minRatio
                            color: "#aaaaaa"
                        }

                        TextField {
                            id: forgotEmail
                            width: parent.width
                            height: 70 * loginRoot.heightRatio
                            placeholderText: ""
                            font.pixelSize: 36 * loginRoot.minRatio
                            inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase

                            background: Rectangle {
                                color: "#2a2a2a"
                                border.color: forgotEmail.activeFocus ? "#FFD700" : "#555555"
                                border.width: 2 * loginRoot.minRatio
                                radius: 5 * loginRoot.minRatio
                            }

                            color: "white"

                            Text {
                                text: forgotEmail.text.length === 0 ? "   votre@email.com" : ""
                                font.pixelSize: 30 * loginRoot.minRatio
                                color: "#888888"
                                anchors.fill: parent
                                anchors.leftMargin: 10 * loginRoot.minRatio
                                verticalAlignment: Text.AlignVCenter
                                visible: forgotEmail.text.length === 0
                            }
                        }
                    }

                    // Message d'erreur/succès
                    Text {
                        id: forgotPasswordMessage
                        text: ""
                        font.pixelSize: 26 * loginRoot.minRatio
                        color: forgotPasswordMessage.text.includes("succès") ? "#00ff00" : "#ff6666"
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        visible: true
                        opacity: text === "" ? 0 : 1
                    }

                    // Bouton envoyer
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100 * loginRoot.heightRatio

                        background: Rectangle {
                            color: parent.down ? "#00aa00" : (parent.hovered ? "#00dd00" : "#00cc00")
                            radius: 8 * loginRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Envoyer un nouveau mot de passe"
                            font.pixelSize: 30 * loginRoot.minRatio
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            Qt.inputMethod.hide()

                            if (forgotEmail.text === "") {
                                forgotPasswordMessage.text = "Veuillez entrer votre adresse email"
                                return
                            }
                            forgotPasswordMessage.text = ""
                            networkManager.forgotPassword(forgotEmail.text)
                        }
                    }

                    Item { height: 20 * loginRoot.minRatio }
                }

                Connections {
                    target: networkManager
                    function onForgotPasswordSuccess() {
                        forgotPasswordMessage.text = "Un mot de passe temporaire a été envoyé à votre adresse email"
                        forgotEmail.text = ""
                        // Auto-retour après 3 secondes
                        forgotPasswordSuccessTimer.start()
                    }
                    function onForgotPasswordFailed(error) {
                        forgotPasswordMessage.text = error
                    }
                }

                Timer {
                    id: forgotPasswordSuccessTimer
                    interval: 3000
                    repeat: false
                    onTriggered: {
                        loginStack.pop()
                    }
                }
            }
        }

        // Écran "Changement de mot de passe obligatoire"
        Component {
            id: changePasswordScreenComponent

            Rectangle {
                id: changePasswordScreenRect
                anchors.fill: parent

                // Étoiles scintillantes en arrière-plan
                StarryBackground {
                    minRatio: loginRoot.minRatio
                }

                // Pas de bouton retour - changement obligatoire

                // Titre
                Text {
                    id: changePasswordTitle
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: loginRoot.isPortrait ? 300 * loginRoot.minRatio : 80 * loginRoot.minRatio
                    text: "Changer le mot de passe"
                    font.pixelSize: 60 * mainWindow.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                    width: parent.width * 0.9
                    horizontalAlignment: Text.AlignHCenter
                }

                // Texte d'explication
                Text {
                    id: changePasswordExplanation
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: changePasswordTitle.bottom
                    anchors.topMargin: 50 * mainWindow.minRatio
                    text: "Vous vous êtes connecté avec un mot de passe temporaire.\nVeuillez choisir un nouveau mot de passe permanent."
                    font.pixelSize: 32 * mainWindow.minRatio
                    color: "#FFFFFF"
                    wrapMode: Text.WordWrap
                    width: parent.width * 0.95
                    horizontalAlignment: Text.AlignHCenter
                }

                // Formulaire
                Column {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: changePasswordExplanation.bottom
                    anchors.topMargin: loginRoot.isPortrait ? 120 * loginRoot.minRatio : 40 * loginRoot.minRatio
                    spacing: loginRoot.isPortrait ? 40 * mainWindow.minRatio : 30 * loginRoot.minRatio
                    width: 800 * mainWindow.minRatio

                    // Nouveau mot de passe
                    Rectangle {
                        width: parent.width
                        height: loginRoot.isPortrait ? 130 * mainWindow.minRatio : 90 * mainWindow.minRatio
                        color: "#2a2a2a"
                        radius: 10 * mainWindow.minRatio
                        border.color: newPassword.activeFocus ? "#FFD700" : "#555555"
                        border.width: 2 * mainWindow.minRatio

                        TextField {
                            id: newPassword
                            anchors.fill: parent
                            anchors.margins: 10 * mainWindow.minRatio
                            font.pixelSize: 36 * mainWindow.minRatio
                            color: "#FFFFFF"
                            placeholderText: "Nouveau mot de passe"
                            placeholderTextColor: "#888888"
                            echoMode: TextInput.Password
                            background: Rectangle { color: "transparent" }
                            selectByMouse: true
                        }
                    }

                    // Confirmer mot de passe
                    Rectangle {
                        width: parent.width
                        height: loginRoot.isPortrait ? 130 * mainWindow.minRatio : 90 * mainWindow.minRatio
                        color: "#2a2a2a"
                        radius: 10 * mainWindow.minRatio
                        border.color: confirmPassword.activeFocus ? "#FFD700" : "#555555"
                        border.width: 2 * mainWindow.minRatio

                        TextField {
                            id: confirmPassword
                            anchors.fill: parent
                            anchors.margins: 10 * mainWindow.minRatio
                            font.pixelSize: 36 * mainWindow.minRatio
                            color: "#FFFFFF"
                            placeholderText: "Confirmer le mot de passe"
                            placeholderTextColor: "#888888"
                            echoMode: TextInput.Password
                            background: Rectangle { color: "transparent" }
                            selectByMouse: true
                        }
                    }

                    // Message d'erreur
                    Text {
                        id: changePasswordError
                        width: parent.width
                        text: ""
                        font.pixelSize: 26 * mainWindow.minRatio
                        color: "#FF6B6B"
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        opacity: text !== "" ? 1 : 0
                    }

                    // Bouton changer
                    Button {
                        width: parent.width
                        height: loginRoot.isPortrait ? 140 * mainWindow.minRatio : 100 * mainWindow.minRatio
                        anchors.horizontalCenter: parent.horizontalCenter

                        background: Rectangle {
                            color: parent.down ? "#005599" : (parent.hovered ? "#0066AA" : "#0077BB")
                            radius: 10 * mainWindow.minRatio
                            border.color: "#FFD700"
                            border.width: 3 * mainWindow.minRatio
                        }

                        contentItem: Text {
                            text: "Changer le mot de passe"
                            font.pixelSize: 40 * mainWindow.minRatio
                            font.bold: true
                            color: "#FFFFFF"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            // Validation
                            if (newPassword.text === "" || confirmPassword.text === "") {
                                changePasswordError.text = "Veuillez remplir tous les champs"
                                return
                            }
                            if (newPassword.text !== confirmPassword.text) {
                                changePasswordError.text = "Les mots de passe ne correspondent pas"
                                return
                            }
                            if (newPassword.text.length < 6) {
                                changePasswordError.text = "Le mot de passe doit contenir au moins 6 caractères"
                                return
                            }

                            // Envoyer la demande de changement
                            changePasswordError.text = ""
                            loginRoot.pendingChangePassword = newPassword.text
                            networkManager.changePassword(loginRoot.pendingChangeEmail, newPassword.text)
                        }
                    }
                }

                Connections {
                    target: networkManager
                    function onChangePasswordSuccess() {
                        console.log("Password changed successfully - saving credentials and logging in")
                        // Sauvegarder les nouveaux identifiants
                        networkManager.saveCredentials(loginRoot.pendingChangeEmail, loginRoot.pendingChangePassword)
                        // Émettre le signal de succès de connexion
                        loginRoot.loginSuccess(loginRoot.pendingChangePlayerName, "account")
                    }
                    function onChangePasswordFailed(error) {
                        changePasswordError.text = error
                    }
                }
            }
        }
    }
}
