import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: settingsRoot
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

                    Text {
                        text: "🔊 AUDIO"
                        font.pixelSize: 40 * settingsRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
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

                    Text {
                        text: "🎨 AFFICHAGE"
                        font.pixelSize: 40 * settingsRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
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

                    Text {
                        text: "👤 COMPTE"
                        font.pixelSize: 40 * settingsRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
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

                        Button {
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

                        Button {
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
                                if (email === settingsRoot.playerEmail) {
                                    emailErrorText.text = "Le nouvel email est identique à l'actuel"
                                    emailErrorText.visible = true
                                    return
                                }
                                networkManager.changeEmail(settingsRoot.playerName, email)
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
                    Button {
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

                    Button {
                        width: parent.width * 0.6
                        height: 80 * settingsRoot.minRatio
                        anchors.horizontalCenter: parent.horizontalCenter

                        background: Rectangle {
                            color: parent.down ? "#ccaa00" : (parent.hovered ? "#e6c200" : "#FFD700")
                            radius: 10 * settingsRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Contacter Nebuludik"
                            font.pixelSize: 30 * settingsRoot.minRatio
                            font.bold: true
                            color: "#2a2a2a"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: settingsRoot.openContact()
                    }
                }
            }

            // ===================== REMERCIEMENTS =====================
            Column {
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

                        Text {
                            anchors.centerIn: parent
                            text: "♥"
                            font.pixelSize: 36 * settingsRoot.minRatio
                            color: "#2a2a2a"
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

                Button {
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

            // Espace supplémentaire pour scroller les champs au-dessus du clavier
            Item { height: settingsRoot.height * 0.2; width: 1 }
        }
    }

    // Popup Crédits
    Popup {
        id: creditsPopup
        anchors.centerIn: parent
        width: parent.width * 0.7
        height: creditsColumn.height + 80 * settingsRoot.minRatio
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#2a2a2a"
            radius: 15 * settingsRoot.minRatio
            border.color: "#FFD700"
            border.width: 2 * settingsRoot.minRatio
        }

        Column {
            id: creditsColumn
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 30 * settingsRoot.minRatio
            width: parent.width - 60 * settingsRoot.minRatio
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

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
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
        }
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
                Button {
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
                Button {
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
}
