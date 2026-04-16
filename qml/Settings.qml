import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

Rectangle {
    id: settingsRoot

    SoundEffect {
        id: backSound
        source: "qrc:/resources/sons/742832__sadiquecat__woosh-metal-tea-strainer-1.wav"
    }
    anchors.fill: parent
    color: "transparent"

    // Détection d'orientation
    property bool isPortrait: height > width
    property bool isLandscape: width > height

    // Ratio responsive adapté à l'orientation
    property real widthRatio: isPortrait ? width / 600 : width / 1024
    property real heightRatio: isPortrait ? height / 1024 : height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Largeur du formulaire adaptée à l'orientation
    property real formWidthRatio: 0.8

    Component.onCompleted: {
        // Synchroniser avec les paramètres globaux
        musicEnabled = AudioSettings.musicEnabled
        effectsEnabled = AudioSettings.effectsEnabled
        initialized = true
    }

    // Fond étoilé
    StarryBackground {
        anchors.fill: parent
        minRatio: settingsRoot.minRatio
        z: -1
    }

    // Propriétés passées par le parent
    property string playerName: ""
    property string playerEmail: ""
    property string accountType: ""

    // État du panneau de vérification email
    property bool showEmailVerificationStep: false
    property string pendingEmailToVerify: ""
    property int emailResendCooldown: 0

    signal backToMenu()
    signal accountDeleted()
    signal openContact()

    // Flag pour éviter de sauvegarder pendant l'initialisation
    property bool initialized: false

    // Écouter les signaux de suppression de compte
    Connections {
        target: networkManager
        function onDeleteAccountSuccess() {
            deleteConfirmPopup.close()
            settingsRoot.accountDeleted()
        }
        function onDeleteAccountFailed(error) {
            deleteErrorText.text = error
            deleteErrorText.visible = true
        }
        function onChangePseudoSuccess(newPseudo) {
            pseudoErrorText.visible = false
            pseudoSuccessText.visible = true
            pseudoInput.text = newPseudo
            settingsRoot.playerName = newPseudo
        }
        function onChangePseudoFailed(error) {
            pseudoSuccessText.visible = false
            pseudoErrorText.text = error
            pseudoErrorText.visible = true
        }
        function onChangeEmailSuccess(newEmail) {
            settingsRoot.showEmailVerificationStep = false
            emailVerifCodeField.text = ""
            emailVerifError.text = ""
            emailErrorText.visible = false
            emailSuccessText.visible = true
            emailInput.text = newEmail
            settingsRoot.playerEmail = newEmail
        }
        function onChangeEmailFailed(error) {
            emailSuccessText.visible = false
            emailErrorText.text = error
            emailErrorText.visible = true
        }
        function onEmailChangeCodeSent(newEmail) {
            settingsRoot.pendingEmailToVerify = newEmail
            settingsRoot.showEmailVerificationStep = true
            emailVerifCodeField.text = ""
            emailVerifError.text = ""
            settingsRoot.emailResendCooldown = 60
            emailResendCooldownTimer.restart()
        }
        function onEmailChangeCodeFailed(error) {
            emailErrorText.text = error
            emailErrorText.visible = true
        }
        function onVerifyEmailChangeFailed(error) {
            emailVerifError.text = error
        }
        function onSetAnonymousFailed(error) {
            anonymousErrorText.text = error
            anonymousErrorText.visible = true
        }
    }

    // Propriétés pour l'état des sons
    property bool musicEnabled: AudioSettings.musicEnabled
    property bool effectsEnabled: AudioSettings.effectsEnabled

    // Propriété pour le tri des cartes
    property bool strongCardsLeft: DisplaySettings.strongCardsLeft

    // Propriété pour le sens de jeu
    property bool antiClockwisePlay: DisplaySettings.antiClockwisePlay

    // Watcher pour mettre à jour les paramètres globaux (seulement après initialisation)
    onMusicEnabledChanged: {
        if (initialized) {
            AudioSettings.saveMusicEnabled(musicEnabled)
        }
    }

    onEffectsEnabledChanged: {
        if (initialized) {
            AudioSettings.saveEffectsEnabled(effectsEnabled)
        }
    }

    onStrongCardsLeftChanged: {
        if (initialized) {
            DisplaySettings.saveStrongCardsLeft(strongCardsLeft)
        }
    }

    onAntiClockwisePlayChanged: {
        if (initialized) {
            DisplaySettings.saveAntiClockwisePlay(antiClockwisePlay)
        }
    }

    // Bouton retour
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 40 * minRatio
        width: 100 * minRatio
        height: 100 * minRatio
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
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (AudioSettings.effectsEnabled) backSound.play()
                settingsRoot.backToMenu()
            }
        }
    }

    // Contenu des réglages
    Flickable {
        id: settingsScrollView
        anchors.top: parent.top
        anchors.topMargin: isPortrait ? 100 * minRatio : 50 * minRatio
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: isPortrait ? 20 * minRatio : parent.width * 0.1
        anchors.rightMargin: isPortrait ? 20 * minRatio : parent.width * 0.1
        anchors.bottomMargin: isPortrait ? 20 * minRatio : 40 * minRatio
        clip: true
        contentHeight: settingsColumn.height
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: settingsColumn
            width: parent.width
            spacing: 30 * settingsRoot.minRatio

            // Titre
            Text {
                id: settingsTitle
                anchors.horizontalCenter: parent.horizontalCenter
                text: "RÉGLAGES"
                font.pixelSize: isPortrait ? 50 * minRatio : 60 * minRatio
                font.bold: true
                color: "#FFD700"
            }

            // Section Audio
            Rectangle {
                width: parent.width
                height: audioColumn.height + 40 * settingsRoot.minRatio
                color: "#992a2a2a"  // 0.6 opacity via alpha channel (99 hex = 153/255 ≈ 0.6)
                radius: 10 * settingsRoot.minRatio
                border.color: "#FFD700"
                border.width: 2 * settingsRoot.minRatio

                Column {
                    id: audioColumn
                    anchors.centerIn: parent
                    width: parent.width - 40 * settingsRoot.minRatio
                    spacing: 20 * settingsRoot.minRatio

                    Row {
                        spacing: 10 * settingsRoot.minRatio

                        Image {
                            source: "qrc:/resources/speaker-high-volume-svgrepo-com.svg"
                            width: 40 * settingsRoot.minRatio
                            height: 40 * settingsRoot.minRatio
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "AUDIO"
                            font.pixelSize: 40 * settingsRoot.minRatio
                            font.bold: true
                            color: "#FFD700"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    // Musique ON/OFF
                    Row {

                        spacing: 60 * settingsRoot.minRatio

                        Item {
                            height: 40 * settingsRoot.minRatio
                            width: parent.width * 0.001
                        }

                        Text {
                            text: "Musique :"
                            font.pixelSize: 30 * settingsRoot.minRatio
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width * 0.5
                        }

                        Image {
                            width: 100 * settingsRoot.minRatio
                            height: 80 * settingsRoot.minRatio
                            anchors.verticalCenter: parent.verticalCenter
                            source: settingsRoot.musicEnabled ? "qrc:/resources/switchon-svgrepo-com.svg" : "qrc:/resources/switchoff-svgrepo-com.svg"
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: settingsRoot.musicEnabled = !settingsRoot.musicEnabled
                            }
                        }
                    }

                    // Effets sonores ON/OFF
                    Row {
                        spacing: 60 * settingsRoot.minRatio

                        Item {
                            height: 40 * settingsRoot.minRatio
                            width: parent.width * 0.001
                        }

                        Text {
                            text: "Effets sonores :"
                            font.pixelSize: 30 * settingsRoot.minRatio
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width * 0.5
                        }

                        Image {
                            width: 100 * settingsRoot.minRatio
                            height: 80 * settingsRoot.minRatio
                            anchors.verticalCenter: parent.verticalCenter
                            source: settingsRoot.effectsEnabled ? "qrc:/resources/switchon-svgrepo-com.svg" : "qrc:/resources/switchoff-svgrepo-com.svg"
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: settingsRoot.effectsEnabled = !settingsRoot.effectsEnabled
                            }
                        }
                    }
                }
            }

            // Section Affichage
            Rectangle {
                width: parent.width
                height: displayColumn.height + 40 * settingsRoot.minRatio
                color: "#992a2a2a"  // 0.6 opacity via alpha channel
                radius: 10 * settingsRoot.minRatio
                border.color: "#FFD700"
                border.width: 2 * settingsRoot.minRatio

                Column {
                    id: displayColumn
                    anchors.centerIn: parent
                    width: parent.width - 40 * settingsRoot.minRatio
                    spacing: 20 * settingsRoot.minRatio

                    Row {
                        spacing: 10 * settingsRoot.minRatio

                        Image {
                            source: "qrc:/resources/artist-palette-svgrepo-com.svg"
                            width: 40 * settingsRoot.minRatio
                            height: 40 * settingsRoot.minRatio
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "AFFICHAGE"
                            font.pixelSize: 40 * settingsRoot.minRatio
                            font.bold: true
                            color: "#FFD700"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    // Tri des cartes
                    Row {
                        spacing: 60 * settingsRoot.minRatio

                        Item {
                            height: 80 * settingsRoot.minRatio
                            width: parent.parent.width * 0.001
                        }

                        Text {
                            text: "Tri des cartes :"
                            font.pixelSize: 30 * settingsRoot.minRatio
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width * 0.28
                        }

                        // Segmented control
                        Row {
                            spacing: 0
                            anchors.verticalCenter: parent.verticalCenter

                            // Bouton "Fortes à gauche"
                            Rectangle {
                                width: 240 * settingsRoot.minRatio
                                height: 60 * settingsRoot.minRatio
                                radius: 10 * settingsRoot.minRatio
                                color: settingsRoot.strongCardsLeft ? "#FFD700" : "#3a3a3a"
                                border.color: settingsRoot.strongCardsLeft ? "#FFD700" : "#3a3a3a"
                                border.width: 2 * settingsRoot.minRatio

                                Rectangle {
                                    anchors.right: parent.right
                                    width: parent.radius
                                    height: parent.height
                                    color: parent.color
                                }

                                Text {
                                    anchors.centerIn: parent
                                    text: "Fortes à gauche"
                                    font.pixelSize: 24 * settingsRoot.minRatio
                                    font.bold: settingsRoot.strongCardsLeft
                                    color: settingsRoot.strongCardsLeft ? "#2a2a2a" : "#aaaaaa"
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: settingsRoot.strongCardsLeft = true
                                }
                            }

                            // Bouton "Fortes à droite"
                            Rectangle {
                                width: 240 * settingsRoot.minRatio
                                height: 60 * settingsRoot.minRatio
                                radius: 10 * settingsRoot.minRatio
                                color: !settingsRoot.strongCardsLeft ? "#FFD700" : "#3a3a3a"
                                border.color: !settingsRoot.strongCardsLeft ? "#FFD700" : "#3a3a3a"
                                border.width: 2 * settingsRoot.minRatio

                                Rectangle {
                                    anchors.left: parent.left
                                    width: parent.radius
                                    height: parent.height
                                    color: parent.color
                                    border.width: 0
                                }

                                Text {
                                    anchors.centerIn: parent
                                    text: "Fortes à droite"
                                    font.pixelSize: 24 * settingsRoot.minRatio
                                    font.bold: !settingsRoot.strongCardsLeft
                                    color: !settingsRoot.strongCardsLeft ? "#2a2a2a" : "#aaaaaa"
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: settingsRoot.strongCardsLeft = false
                                }
                            }
                        }
                    }

                    // Sens de jeu
                    Row {
                        spacing: 60 * settingsRoot.minRatio

                        Item {
                            height: 80 * settingsRoot.minRatio
                            width: parent.parent.width * 0.001
                        }

                        Text {
                            text: "Sens de jeu :"
                            font.pixelSize: 30 * settingsRoot.minRatio
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width * 0.28
                        }

                        // Segmented control
                        Row {
                            spacing: 0
                            anchors.verticalCenter: parent.verticalCenter

                            // Bouton "Horaire"
                            Rectangle {
                                width: 240 * settingsRoot.minRatio
                                height: 60 * settingsRoot.minRatio
                                radius: 10 * settingsRoot.minRatio
                                color: !settingsRoot.antiClockwisePlay ? "#FFD700" : "#3a3a3a"
                                border.color: !settingsRoot.antiClockwisePlay ? "#FFD700" : "#3a3a3a"
                                border.width: 2 * settingsRoot.minRatio

                                Rectangle {
                                    anchors.right: parent.right
                                    width: parent.radius
                                    height: parent.height
                                    color: parent.color
                                }

                                Text {
                                    anchors.centerIn: parent
                                    text: "Horaire"
                                    font.pixelSize: 24 * settingsRoot.minRatio
                                    font.bold: !settingsRoot.antiClockwisePlay
                                    color: !settingsRoot.antiClockwisePlay ? "#2a2a2a" : "#aaaaaa"
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: settingsRoot.antiClockwisePlay = false
                                }
                            }

                            // Bouton "Antihoraire"
                            Rectangle {
                                width: 240 * settingsRoot.minRatio
                                height: 60 * settingsRoot.minRatio
                                radius: 10 * settingsRoot.minRatio
                                color: settingsRoot.antiClockwisePlay ? "#FFD700" : "#3a3a3a"
                                border.color: settingsRoot.antiClockwisePlay ? "#FFD700" : "#3a3a3a"
                                border.width: 2 * settingsRoot.minRatio

                                Rectangle {
                                    anchors.left: parent.left
                                    width: parent.radius
                                    height: parent.height
                                    color: parent.color
                                    border.width: 0
                                }

                                Text {
                                    anchors.centerIn: parent
                                    text: "Antihoraire"
                                    font.pixelSize: 24 * settingsRoot.minRatio
                                    font.bold: settingsRoot.antiClockwisePlay
                                    color: settingsRoot.antiClockwisePlay ? "#2a2a2a" : "#aaaaaa"
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: settingsRoot.antiClockwisePlay = true
                                }
                            }
                        }
                    }
                }
            }

            // Section Compte (visible uniquement pour les comptes enregistrés)
            Rectangle {
                width: parent.width
                height: accountColumn.height + 40 * settingsRoot.minRatio
                color: "#992a2a2a"  // 0.6 opacity via alpha channel
                radius: 10 * settingsRoot.minRatio
                border.color: "#FFD700"
                border.width: 2 * settingsRoot.minRatio
                visible: settingsRoot.accountType !== "guest"

                Column {
                    id: accountColumn
                    anchors.centerIn: parent
                    width: parent.width - 40 * settingsRoot.minRatio
                    spacing: 20 * settingsRoot.minRatio

                    Row {
                        spacing: 10 * settingsRoot.minRatio

                        Image {
                            source: "qrc:/resources/bust-in-silhouette-svgrepo-com.svg"
                            width: 40 * settingsRoot.minRatio
                            height: 40 * settingsRoot.minRatio
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "COMPTE"
                            font.pixelSize: 40 * settingsRoot.minRatio
                            font.bold: true
                            color: "#FFD700"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Text {
                        text: "   Connecté en tant que : " + settingsRoot.playerName
                        font.pixelSize: 30 * settingsRoot.minRatio
                        color: "white"
                    }

                    // Séparateur avant suppression
                    Rectangle {
                        width: parent.width
                        height: 1 * settingsRoot.minRatio
                        color: "#555555"
                    }

                    // Modifier le pseudo
                    Text {
                        text: "Pseudo"
                        leftPadding: 30 * settingsRoot.minRatio
                        font.pixelSize: 30 * settingsRoot.minRatio
                        color: "#cccccc"
                        topPadding: 10 * settingsRoot.minRatio
                    }

                    Row {
                        width: parent.width
                        spacing: 20 * settingsRoot.minRatio

                        Item {
                            height: 40 * settingsRoot.minRatio
                            width: parent.width * 0.001
                        }

                        Rectangle {
                            width: parent.width - pseudoButton.width - 100 * settingsRoot.minRatio
                            height: 80 * settingsRoot.minRatio
                            color: "#3a3a3a"
                            radius: 8 * settingsRoot.minRatio
                            border.color: pseudoInput.activeFocus ? "#FFD700" : "#666666"
                            border.width: 2 * settingsRoot.minRatio

                            TextInput {
                                id: pseudoInput
                                font.family: systemFontFamily
                                anchors.fill: parent
                                anchors.margins: 10 * settingsRoot.minRatio
                                text: settingsRoot.playerName
                                font.pixelSize: 30 * settingsRoot.minRatio
                                color: "white"
                                verticalAlignment: TextInput.AlignVCenter
                                clip: true
                                maximumLength: 12
                                onTextChanged: if (text.indexOf(' ') >= 0) text = text.replace(/ /g, '')
                            }
                        }

                        Item {
                            height: 40 * settingsRoot.minRatio
                            width: parent.width * 0.001
                        }

                        AppButton {
                            id: pseudoButton
                            width: 160 * settingsRoot.minRatio
                            height: 80 * settingsRoot.minRatio

                            background: Rectangle {
                                color: parent.down ? "#ccaa00" : (parent.hovered ? "#e6c200" : "#FFD700")
                                radius: 5 * settingsRoot.minRatio
                            }

                            contentItem: Text {
                                text: "Modifier "
                                font.pixelSize: 28 * settingsRoot.minRatio
                                font.bold: true
                                color: "#2a2a2a"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                pseudoErrorText.visible = false
                                pseudoSuccessText.visible = false
                                if (pseudoInput.text.trim() === "") {
                                    pseudoErrorText.text = "Le pseudo ne peut pas être vide"
                                    pseudoErrorText.visible = true
                                    return
                                }
                                if (pseudoInput.text.trim() === settingsRoot.playerName) {
                                    pseudoErrorText.text = "Le nouveau pseudo est identique à l'actuel"
                                    pseudoErrorText.visible = true
                                    return
                                }
                                networkManager.changePseudo(settingsRoot.playerName, pseudoInput.text.trim())
                            }
                        }

                        Item {
                            height: 40 * settingsRoot.minRatio
                            width: parent.width * 0.15
                        }
                    }

                    Text {
                        id: pseudoErrorText
                        width: parent.width
                        leftPadding: 30 * settingsRoot.minRatio
                        text: ""
                        font.pixelSize: 24 * settingsRoot.minRatio
                        color: "#ff6666"
                        wrapMode: Text.WordWrap
                        visible: false
                    }

                    Text {
                        id: pseudoSuccessText
                        width: parent.width
                        leftPadding: 30 * settingsRoot.minRatio
                        text: "Pseudo modifié avec succès"
                        font.pixelSize: 24 * settingsRoot.minRatio
                        color: "#66ff66"
                        visible: false
                    }

                    // Modifier l'email
                    Text {
                        text: "Email"
                        leftPadding: 30 * settingsRoot.minRatio
                        font.pixelSize: 30 * settingsRoot.minRatio
                        color: "#cccccc"
                        topPadding: 10 * settingsRoot.minRatio
                    }

                    Row {
                        width: parent.width
                        spacing: 20 * settingsRoot.minRatio

                        Item {
                            height: 40 * settingsRoot.minRatio
                            width: parent.width * 0.001
                        }

                        Rectangle {
                            width: parent.width - emailButton.width - 100 * settingsRoot.minRatio
                            height: 80 * settingsRoot.minRatio
                            color: "#3a3a3a"
                            radius: 8 * settingsRoot.minRatio
                            border.color: emailInput.activeFocus ? "#FFD700" : "#666666"
                            border.width: 2 * settingsRoot.minRatio

                            TextInput {
                                id: emailInput
                                font.family: systemFontFamily
                                anchors.fill: parent
                                anchors.margins: 10 * settingsRoot.minRatio
                                text: settingsRoot.playerEmail
                                font.pixelSize: 30 * settingsRoot.minRatio
                                color: "white"
                                verticalAlignment: TextInput.AlignVCenter
                                clip: true
                                inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                                onTextChanged: if (text.indexOf(' ') >= 0) text = text.replace(/ /g, '')
                            }
                        }

                        Item {
                            height: 40 * settingsRoot.minRatio
                            width: parent.width * 0.001
                        }

                        AppButton {
                            id: emailButton
                            width: 160 * settingsRoot.minRatio
                            height: 80 * settingsRoot.minRatio

                            background: Rectangle {
                                color: parent.down ? "#ccaa00" : (parent.hovered ? "#e6c200" : "#FFD700")
                                radius: 5 * settingsRoot.minRatio
                            }

                            contentItem: Text {
                                text: "Modifier "
                                font.pixelSize: 28 * settingsRoot.minRatio
                                font.bold: true
                                color: "#2a2a2a"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                emailErrorText.visible = false
                                emailSuccessText.visible = false
                                if (emailInput.text.trim() === "") {
                                    emailErrorText.text = "L'email ne peut pas être vide"
                                    emailErrorText.visible = true
                                    return
                                }
                                var email = emailInput.text.trim()
                                if (email.indexOf("@") === -1 || email.indexOf(".") === -1) {
                                    emailErrorText.text = "Adresse email invalide"
                                    emailErrorText.visible = true
                                    return
                                }
                                // Vérification plus stricte : au moins un caractère avant @,
                                // un domaine avec au moins un point après @
                                var parts = email.split("@")
                                if (parts.length !== 2 || parts[0].length === 0 || parts[1].indexOf(".") === -1 || parts[1].split(".").pop().length < 2) {
                                    emailErrorText.text = "Adresse email invalide"
                                    emailErrorText.visible = true
                                    return
                                }
                                if (email === settingsRoot.playerEmail) {
                                    emailErrorText.text = "Le nouvel email est identique à l'actuel"
                                    emailErrorText.visible = true
                                    return
                                }
                                networkManager.requestEmailChangeCode(settingsRoot.playerName, email)
                            }
                        }

                        Item {
                            height: 40 * settingsRoot.minRatio
                            width: parent.width * 0.15
                        }
                    }

                    Text {
                        id: emailErrorText
                        leftPadding: 30 * settingsRoot.minRatio
                        width: parent.width
                        text: ""
                        font.pixelSize: 24 * settingsRoot.minRatio
                        color: "#ff6666"
                        wrapMode: Text.WordWrap
                        visible: false
                    }

                    Text {
                        id: emailSuccessText
                        width: parent.width
                        leftPadding: 30 * settingsRoot.minRatio
                        text: "Email modifié avec succès"
                        font.pixelSize: 24 * settingsRoot.minRatio
                        color: "#66ff66"
                        visible: false
                    }

                    // Séparateur avant suppression
                    Rectangle {
                        width: parent.width
                        height: 1 * settingsRoot.minRatio
                        color: "#555555"
                    }

                    // Anonymiser mon profil
                    Row {
                        spacing: 30 * settingsRoot.minRatio

                        Item {
                            height: 40 * settingsRoot.minRatio
                            width: parent.width * 0.001
                        }

                        Text {
                            text: "Anonymiser mon profil :"
                            font.pixelSize: 30 * settingsRoot.minRatio
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width * 0.6
                        }

                        Item {
                            height: 40 * settingsRoot.minRatio
                            width: parent.width * 0.06
                        }

                        Image {
                            width: 100 * settingsRoot.minRatio
                            height: 80 * settingsRoot.minRatio
                            anchors.verticalCenter: parent.verticalCenter
                            source: networkManager.isAnonymous ? "qrc:/resources/switchon-svgrepo-com.svg" : "qrc:/resources/switchoff-svgrepo-com.svg"
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    anonymousErrorText.visible = false
                                    networkManager.setAnonymous(!networkManager.isAnonymous)
                                }
                            }
                        }
                    }

                    Text {
                        width: parent.width
                        leftPadding: 30 * settingsRoot.minRatio
                        text: "Les autres joueurs verront \"Anonyme\" comme pseudo et ne verront plus vos statistiques en cliquant sur votre avatar"
                        font.pixelSize: 20 * settingsRoot.minRatio
                        color: "#888888"
                        font.italic: true
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        id: anonymousErrorText
                        width: parent.width
                        text: ""
                        font.pixelSize: 20 * settingsRoot.minRatio
                        color: "#ff6666"
                        wrapMode: Text.WordWrap
                        visible: false
                    }

                    // Séparateur avant suppression
                    Rectangle {
                        width: parent.width
                        height: 1 * settingsRoot.minRatio
                        color: "#555555"
                    }

                    // Bouton Supprimer mon compte
                    AppButton {
                        width: parent.width
                        height: 100 * settingsRoot.minRatio

                        background: Rectangle {
                            color: parent.down ? "#cc0000" : (parent.hovered ? "#bb0000" : "#aa0000")
                            radius: 10 * settingsRoot.minRatio
                            border.color: "#ff6666"
                            border.width: 2 * settingsRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Supprimer mon compte"
                            font.pixelSize: 34 * settingsRoot.minRatio
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            deleteErrorText.visible = false
                            deleteConfirmPopup.open()
                        }
                    }
                }
            }

            // Section Contacter Nebuludik
            Rectangle {
                width: parent.width
                height: contactColumn.height + 40 * settingsRoot.minRatio
                color: "#992a2a2a"
                radius: 10 * settingsRoot.minRatio
                border.color: "#FFD700"
                border.width: 2 * settingsRoot.minRatio

                Column {
                    id: contactColumn
                    anchors.centerIn: parent
                    width: parent.width - 40 * settingsRoot.minRatio
                    spacing: 20 * settingsRoot.minRatio

                    Row {
                        spacing: 20 * settingsRoot.minRatio

                        Rectangle {
                            width: 60 * settingsRoot.minRatio
                            height: 60 * settingsRoot.minRatio
                            radius: 10 * settingsRoot.minRatio
                            color: "#FFD700"
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                anchors.fill: parent
                                anchors.margins: 8 * settingsRoot.minRatio
                                source: "qrc:/resources/support-svgrepo-com.svg"
                                fillMode: Image.PreserveAspectFit
                            }
                        }

                        Text {
                            text: "CONTACTER NEBULUDIK"
                            font.pixelSize: 40 * settingsRoot.minRatio
                            font.bold: true
                            color: "#FFD700"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Text {
                        text: "Une suggestion, un bug ou un avis à partager ? Cliquez ci-dessous !"
                        leftPadding: 14 * settingsRoot.minRatio
                        font.pixelSize: 26 * settingsRoot.minRatio
                        color: "#cccccc"
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }

                    AppButton {
                        width: parent.width * 0.6
                        height: 80 * settingsRoot.minRatio
                        anchors.horizontalCenter: parent.horizontalCenter

                        background: Rectangle {
                            color: parent.down ? "#444444" : (parent.hovered ? "#555555" : "#3a3a3a")
                            radius: 10 * settingsRoot.minRatio
                            border.color: "#FFD700"
                            border.width: 2 * settingsRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Contacter Nebuludik"
                            font.pixelSize: 30 * settingsRoot.minRatio
                            font.bold: true
                            color: "#FFD700"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: settingsRoot.openContact()
                    }
                }
            }

            // ===================== À PROPOS =====================
            Rectangle {
                width: parent.width
                height: aboutColumn.height + 40 * settingsRoot.minRatio
                color: "#992a2a2a"
                radius: 10 * settingsRoot.minRatio
                border.color: "#FFD700"
                border.width: 2 * settingsRoot.minRatio

                Column {
                    id: aboutColumn
                    anchors.centerIn: parent
                    width: parent.width - 40 * settingsRoot.minRatio
                    spacing: 20 * settingsRoot.minRatio

                    Row {
                        spacing: 20 * settingsRoot.minRatio

                        Rectangle {
                            width: 60 * settingsRoot.minRatio
                            height: 60 * settingsRoot.minRatio
                            radius: 10 * settingsRoot.minRatio
                            color: "#FFD700"
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                anchors.centerIn: parent
                                width: parent.width * 0.7
                                height: parent.height * 0.7
                                source: "qrc:/resources/question-svgrepo-com.svg"
                                fillMode: Image.PreserveAspectFit
                            }
                        }

                        Text {
                            text: "À PROPOS"
                            font.pixelSize: 40 * settingsRoot.minRatio
                            font.bold: true
                            color: "#FFD700"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Text {
                        text: "Coinche de l'Espace est une application indépendante, créée par Nebuludik, sans publicité, par passion de la Coinche. Merci pour votre soutien !"
                        leftPadding: 14 * settingsRoot.minRatio
                        font.pixelSize: 26 * settingsRoot.minRatio
                        color: "#cccccc"
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }

                    AppButton {
                        width: parent.width * 0.6
                        height: 80 * settingsRoot.minRatio
                        anchors.horizontalCenter: parent.horizontalCenter

                        background: Rectangle {
                            color: parent.down ? "#444444" : (parent.hovered ? "#555555" : "#3a3a3a")
                            radius: 10 * settingsRoot.minRatio
                            border.color: "#FFD700"
                            border.width: 2 * settingsRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Visiter le site web"
                            font.pixelSize: 30 * settingsRoot.minRatio
                            font.bold: true
                            color: "#FFD700"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: Qt.openUrlExternally("https://nebuludik.fr/")
                    }
                }
            }

            // ===================== REMERCIEMENTS =====================
            Rectangle {
                width: parent.width
                height: creditsSection.height + 40 * settingsRoot.minRatio
                color: "#992a2a2a"
                radius: 10 * settingsRoot.minRatio
                border.color: "#FFD700"
                border.width: 2 * settingsRoot.minRatio

                Column {
                    id: creditsSection
                    anchors.centerIn: parent
                    width: parent.width - 40 * settingsRoot.minRatio
                    spacing: 20 * settingsRoot.minRatio

                    Row {
                        spacing: 20 * settingsRoot.minRatio

                        Rectangle {
                            width: 60 * settingsRoot.minRatio
                            height: 60 * settingsRoot.minRatio
                            radius: 10 * settingsRoot.minRatio
                            color: "#FFD700"
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                anchors.centerIn: parent
                                width: parent.width * 0.65
                                height: parent.height * 0.65
                                source: "qrc:/resources/heart-svgrepo-com.svg"
                                fillMode: Image.PreserveAspectFit
                            }
                        }

                        Text {
                            text: "REMERCIEMENTS"
                            font.pixelSize: 40 * settingsRoot.minRatio
                            font.bold: true
                            color: "#FFD700"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    AppButton {
                        width: parent.width * 0.6
                        height: 80 * settingsRoot.minRatio
                        anchors.horizontalCenter: parent.horizontalCenter

                        background: Rectangle {
                            color: parent.down ? "#444444" : (parent.hovered ? "#555555" : "#3a3a3a")
                            radius: 10 * settingsRoot.minRatio
                            border.color: "#FFD700"
                            border.width: 2 * settingsRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Crédits"
                            font.pixelSize: 30 * settingsRoot.minRatio
                            font.bold: true
                            color: "#FFD700"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: creditsPopup.open()
                    }
                }
            }

            // Espace supplémentaire pour scroller les champs au-dessus du clavier
            Item { height: settingsRoot.height * 0.2; width: 1 }
        }
    }

    // Popup Crédits
    Popup {
        id: creditsPopup
        anchors.centerIn: parent
        width: parent.width * 0.7
        height: parent.height * 0.8
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#2a2a2a"
            radius: 15 * settingsRoot.minRatio
            border.color: "#FFD700"
            border.width: 2 * settingsRoot.minRatio
        }

        Item {
            anchors.fill: parent

            ScrollView {
                id: creditsScrollView
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: creditsCloseButton.top
                anchors.bottomMargin: 10 * settingsRoot.minRatio
                clip: true
                contentWidth: availableWidth
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                Column {
                    id: creditsColumn
                    width: creditsScrollView.width - 20 * settingsRoot.minRatio
                    topPadding: 30 * settingsRoot.minRatio
                    spacing: 30 * settingsRoot.minRatio

            Text {
                text: "Crédits"
                font.pixelSize: 42 * settingsRoot.minRatio
                font.bold: true
                color: "#FFD700"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // Qt
            Column {
                width: parent.width
                spacing: 6 * settingsRoot.minRatio

                Text {
                    text: "Qt 6"
                    font.pixelSize: 30 * settingsRoot.minRatio
                    font.bold: true
                    color: "white"
                }
                Text {
                    text: "Licence : GNU Lesser General Public License v3 (LGPL v3)"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#cccccc"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Text {
                    text: "www.qt.io/licensing"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#00aaee"
                    font.underline: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://www.qt.io/licensing")
                    }
                }
            }

            // OpenSSL
            Column {
                width: parent.width
                spacing: 6 * settingsRoot.minRatio

                Text {
                    text: "OpenSSL 3"
                    font.pixelSize: 30 * settingsRoot.minRatio
                    font.bold: true
                    color: "white"
                }
                Text {
                    text: "Licence : Apache License 2.0"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#cccccc"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Text {
                    text: "www.openssl.org/source/license.html"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#00aaee"
                    font.underline: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://www.openssl.org/source/license.html")
                    }
                }
            }

            // SVG-cards
            Column {
                width: parent.width
                spacing: 6 * settingsRoot.minRatio

                Text {
                    text: "SVG-cards 1.1"
                    font.pixelSize: 30 * settingsRoot.minRatio
                    font.bold: true
                    color: "white"
                }
                Text {
                    text: "Copyright (c) 2004 David Bellot"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#cccccc"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Text {
                    text: "Licence : GNU Lesser General Public License v2.1 (LGPL v2.1)"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#cccccc"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Text {
                    text: "sourceforge.net/projects/svg-cards"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#00aaee"
                    font.underline: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://sourceforge.net/projects/svg-cards/files/")
                    }
                }
            }

            // Avataaars
            Column {
                width: parent.width
                spacing: 6 * settingsRoot.minRatio

                Text {
                    text: "Avataaars"
                    font.pixelSize: 30 * settingsRoot.minRatio
                    font.bold: true
                    color: "white"
                }
                Text {
                    text: "Copyright (c) 2017 Fang-Pen Lin"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#cccccc"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Text {
                    text: "Licence : MIT License"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#cccccc"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Text {
                    text: "github.com/fangpenlin/avataaars-generator"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#00aaee"
                    font.underline: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://github.com/fangpenlin/avataaars-generator")
                    }
                }
            }

            // Sons Freesound
            Column {
                width: parent.width
                spacing: 6 * settingsRoot.minRatio

                Text {
                    text: "Sons (Freesound)"
                    font.pixelSize: 30 * settingsRoot.minRatio
                    font.bold: true
                    color: "white"
                }
                Text {
                    text: "Licence : Creative Commons Attribution 4.0 (CC BY 4.0)"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#cccccc"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Text {
                    text: "rocket.wav by Sergenious"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#00aaee"
                    font.underline: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://freesound.org/s/55847/")
                    }
                }
                Text {
                    text: "Explosion 1 by Parasonya"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#00aaee"
                    font.underline: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://freesound.org/s/405571/")
                    }
                }
                Text {
                    text: "card3 by elliottliu"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#00aaee"
                    font.underline: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://freesound.org/s/827603/")
                    }
                }
            }

            // Icônes SVG
            Column {
                width: parent.width
                spacing: 6 * settingsRoot.minRatio

                Text {
                    text: "Icônes SVG (SVG Repo)"
                    font.pixelSize: 30 * settingsRoot.minRatio
                    font.bold: true
                    color: "white"
                }
                Text {
                    text: "Back square button by Iconsax — MIT License"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#cccccc"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Text {
                    text: "github.com/lusaxweb/iconsax"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#00aaee"
                    font.underline: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://github.com/lusaxweb/iconsax")
                    }
                }
                Text {
                    text: "Settings 377 by Carbon Design — Apache License"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#cccccc"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Text {
                    text: "github.com/carbon-design-system/carbon"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#00aaee"
                    font.underline: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://github.com/carbon-design-system/carbon")
                    }
                }
                Text {
                    text: "Question 61 by Iconscout — Apache License"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#cccccc"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Text {
                    text: "github.com/Iconscout/unicons"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#00aaee"
                    font.underline: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://github.com/Iconscout/unicons")
                    }
                }
                Text {
                    text: "Bot 6 by Boxicons — CC Attribution License (CC BY 4.0)"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#cccccc"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Text {
                    text: "github.com/box-icons/boxicons"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#00aaee"
                    font.underline: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://github.com/box-icons/boxicons")
                    }
                }
                Text {
                    text: "www.svgrepo.com"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#00aaee"
                    font.underline: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://www.svgrepo.com")
                    }
                }
            }

            // Google Fonts Material Symbols
            Column {
                width: parent.width
                spacing: 6 * settingsRoot.minRatio

                Text {
                    text: "Google Fonts Material Symbols"
                    font.pixelSize: 30 * settingsRoot.minRatio
                    font.bold: true
                    color: "white"
                }
                Text {
                    text: "Licence : Apache License 2.0"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#cccccc"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Text {
                    text: "fonts.google.com/icons"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#00aaee"
                    font.underline: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://fonts.google.com/icons")
                    }
                }
            }

            // Orbitron
            Column {
                width: parent.width
                spacing: 6 * settingsRoot.minRatio

                Text {
                    text: "Police Orbitron"
                    font.pixelSize: 30 * settingsRoot.minRatio
                    font.bold: true
                    color: "white"
                }
                Text {
                    text: "Licence : SIL Open Font License 1.1 (OFL)"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#cccccc"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Text {
                    text: "scripts.sil.org/OFL"
                    font.pixelSize: 24 * settingsRoot.minRatio
                    color: "#00aaee"
                    font.underline: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://scripts.sil.org/OFL")
                    }
                }
            }

                } // fin Column creditsColumn
            } // fin ScrollView

            AppButton {
                id: creditsCloseButton
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 15 * settingsRoot.minRatio
                width: 160 * settingsRoot.minRatio
                height: 70 * settingsRoot.minRatio

                background: Rectangle {
                    color: parent.down ? "#cc0000" : (parent.hovered ? "#ff3333" : "#ff0000")
                    radius: 8 * settingsRoot.minRatio
                }

                contentItem: Text {
                    text: "Fermer"
                    font.pixelSize: 26 * settingsRoot.minRatio
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: creditsPopup.close()
            }
        } // fin Item
    }

    // Popup de confirmation de suppression
    Popup {
        id: deleteConfirmPopup
        anchors.centerIn: parent
        width: parent.width * 0.4
        height: deletePopupColumn.height + 60 * settingsRoot.minRatio
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#2a2a2a"
            radius: 15 * settingsRoot.minRatio
            border.color: "#ff6666"
            border.width: 3 * settingsRoot.minRatio
        }

        Column {
            id: deletePopupColumn
            anchors.centerIn: parent
            width: parent.width - 40 * settingsRoot.minRatio
            spacing: 20 * settingsRoot.minRatio

            Text {
                text: "Supprimer le compte ?"
                font.pixelSize: 36 * settingsRoot.minRatio
                font.bold: true
                color: "#ff6666"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                width: parent.width
                text: "Cette action est irréversible. Toutes vos données seront supprimées :"
                font.pixelSize: 26 * settingsRoot.minRatio
                color: "white"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                width: parent.width
                text: "• Toutes vos statistiques de jeu\n• Votre adresse email\n• Votre mot de passe"
                font.pixelSize: 24 * settingsRoot.minRatio
                color: "#cccccc"
                wrapMode: Text.WordWrap
                lineHeight: 1.3
            }

            // Message d'erreur
            Text {
                id: deleteErrorText
                width: parent.width
                text: ""
                font.pixelSize: 22 * settingsRoot.minRatio
                color: "#ff6666"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                visible: false
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 20 * settingsRoot.minRatio

                // Bouton Annuler
                AppButton {
                    width: 170 * settingsRoot.minRatio
                    height: 100 * settingsRoot.minRatio

                    background: Rectangle {
                        color: parent.down ? "#444444" : (parent.hovered ? "#555555" : "#333333")
                        radius: 10 * settingsRoot.minRatio
                        border.color: "#888888"
                        border.width: 2 * settingsRoot.minRatio
                    }

                    contentItem: Text {
                        text: "Annuler "
                        font.pixelSize: 24 * settingsRoot.minRatio
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        deleteConfirmPopup.close()
                    }
                }

                // Bouton Confirmer
                AppButton {
                    width: 170 * settingsRoot.minRatio
                    height: 100 * settingsRoot.minRatio

                    background: Rectangle {
                        color: parent.down ? "#cc0000" : (parent.hovered ? "#bb0000" : "#aa0000")
                        radius: 10 * settingsRoot.minRatio
                        border.color: "#ff6666"
                        border.width: 2 * settingsRoot.minRatio
                    }

                    contentItem: Text {
                        text: "Supprimer "
                        font.pixelSize: 24 * settingsRoot.minRatio
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        console.log("Suppression du compte:" + settingsRoot.playerName)
                        networkManager.deleteAccount(settingsRoot.playerName)
                    }
                }
            }

        }
    }

    // Timer pour le cooldown de renvoi du code email
    Timer {
        id: emailResendCooldownTimer
        interval: 1000
        repeat: true
        onTriggered: {
            if (settingsRoot.emailResendCooldown > 0)
                settingsRoot.emailResendCooldown--
            else
                stop()
        }
    }

    // Panneau de vérification du code email (overlay plein écran)
    Rectangle {
        anchors.fill: parent
        color: "#1a1a2e"
        z: 10
        visible: settingsRoot.showEmailVerificationStep

        StarryBackground {
            anchors.fill: parent
            minRatio: settingsRoot.minRatio
            z: -1
        }

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 15 * settingsRoot.minRatio
            width: Math.min(parent.width * settingsRoot.formWidthRatio, 500 * settingsRoot.widthRatio)

            Text {
                text: "Vérification de\nvotre email"
                font.pixelSize: 48 * settingsRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                text: "Un code de vérification a été envoyé à\n" + settingsRoot.pendingEmailToVerify
                font.pixelSize: 26 * settingsRoot.minRatio
                color: "white"
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            // Champ code à 6 chiffres
            TextField {
                id: emailVerifCodeField
                Layout.fillWidth: true
                Layout.preferredHeight: 80 * settingsRoot.heightRatio
                placeholderText: ""
                font.pixelSize: 48 * settingsRoot.minRatio
                horizontalAlignment: Text.AlignHCenter
                maximumLength: 6
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: 0; top: 999999 }
                background: Rectangle {
                    color: "#2a2a2a"
                    border.color: emailVerifCodeField.activeFocus ? "#FFD700" : "#555555"
                    border.width: 2 * settingsRoot.minRatio
                    radius: 5 * settingsRoot.minRatio
                }
                color: "white"

                Text {
                    text: "000000"
                    font.pixelSize: 48 * settingsRoot.minRatio
                    color: "#888888"
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    visible: emailVerifCodeField.displayText.length === 0
                }
            }

            Text {
                id: emailVerifError
                text: ""
                font.pixelSize: 24 * settingsRoot.minRatio
                color: "#ff6666"
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                opacity: text === "" ? 0 : 1
            }

            // Bouton Valider
            AppButton {
                Layout.fillWidth: true
                Layout.preferredHeight: settingsRoot.isPortrait ? 100 * settingsRoot.heightRatio : 90 * settingsRoot.heightRatio

                background: Rectangle {
                    color: parent.down ? "#0088cc" : (parent.hovered ? "#00aaee" : "#0099dd")
                    radius: 8 * settingsRoot.minRatio
                }

                contentItem: Text {
                    text: "Valider"
                    font.pixelSize: 40 * settingsRoot.minRatio
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    if (emailVerifCodeField.text.length !== 6) {
                        emailVerifError.text = "Veuillez entrer le code à 6 chiffres"
                        return
                    }
                    emailVerifError.text = ""
                    networkManager.verifyCodeAndChangeEmail(
                        settingsRoot.playerName,
                        settingsRoot.pendingEmailToVerify,
                        emailVerifCodeField.text
                    )
                }
            }

            // Lien "Renvoyer le code"
            Text {
                text: settingsRoot.emailResendCooldown > 0
                      ? "Renvoyer le code (" + settingsRoot.emailResendCooldown + "s)"
                      : "Renvoyer le code"
                font.pixelSize: 26 * settingsRoot.minRatio
                color: settingsRoot.emailResendCooldown > 0 ? "#666666" : "#00aaee"
                font.underline: settingsRoot.emailResendCooldown === 0
                Layout.alignment: Qt.AlignHCenter

                MouseArea {
                    anchors.fill: parent
                    cursorShape: settingsRoot.emailResendCooldown > 0 ? Qt.ArrowCursor : Qt.PointingHandCursor
                    onClicked: {
                        if (settingsRoot.emailResendCooldown > 0) return
                        emailVerifError.text = ""
                        networkManager.requestEmailChangeCode(
                            settingsRoot.playerName,
                            settingsRoot.pendingEmailToVerify
                        )
                    }
                }
            }

            // Lien "Modifier l'email"
            Text {
                text: "Modifier l'email"
                font.pixelSize: 26 * settingsRoot.minRatio
                color: "#aaaaaa"
                font.underline: true
                Layout.alignment: Qt.AlignHCenter

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        settingsRoot.showEmailVerificationStep = false
                        emailVerifCodeField.text = ""
                        emailVerifError.text = ""
                    }
                }
            }
        }
    }
}
