import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

Rectangle {
    id: root
    anchors.fill: parent
    color: "transparent"

    StarryBackground {
        anchors.fill: parent
        minRatio: root.minRatio
        z: -1
    }

    Rectangle { // Solution du pauvre pour mettre en noire la bar du haut de l'ecran (selfie)
        anchors.left: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width / 5
        height: parent.height
        color: "black"
    }

    Rectangle {
        anchors.right: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width / 5
        height: parent.height
        color: "black"
    }

    property string lobbyCode: ""
    property bool isHost: {
        for (var i = 0; i < networkManager.lobbyPlayers.length; i++) {
            var p = networkManager.lobbyPlayers[i]
            if (p.name === networkManager.playerPseudo && p.isHost)
                return true
        }
        return false
    }
    property string accountType: ""
    property int draggedIndex: -1

    // Ratios pour le responsive
    property real widthRatio: width / 1920
    property real heightRatio: height / 1080
    property real minRatio: Math.min(widthRatio, heightRatio)

    ListModel { id: localPlayersModel }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 40 * root.minRatio
        spacing: 30 * root.minRatio

        // En-tête avec code du lobby
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 180 * root.heightRatio
            color: "#802a2a2a"
            radius: 20 * root.minRatio
            border.color: "#FFD700"
            border.width: 3 * root.minRatio

            SoundEffect {
                id: lobbyModeSwitchSound
                source: "qrc:/resources/sons/742832__sadiquecat__woosh-metal-tea-strainer-1.wav"
            }

            // Sélecteur de mode (top-right)
            Item {
                anchors.right: parent.right
                anchors.rightMargin: 60 * root.minRatio
                anchors.verticalCenter: parent.verticalCenter
                width: modeSelectorRow.width
                height: modeSelectorRow.height

                Row {
                    id: modeSelectorRow
                    spacing: 8 * root.minRatio

                    Image {
                        id: leftModeArrow
                        source: "qrc:/resources/left-arrowMainMenu-svgrepo-com.svg"
                        width: 60 * root.minRatio
                        height: 60 * root.minRatio
                        anchors.verticalCenter: parent.verticalCenter
                        visible: root.isHost
                        opacity: networkManager.lobbyGameMode === "coinche" ? 0.3 : 1.0

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: networkManager.lobbyGameMode !== "coinche" ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onClicked: {
                                if (networkManager.lobbyGameMode !== "coinche") {
                                    lobbyModeSwitchSound.play()
                                    networkManager.sendSetLobbyGameMode("coinche")
                                }
                            }
                        }
                    }

                    Item {
                        width: 280 * root.minRatio
                        height: 60 * root.minRatio
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            anchors.centerIn: parent
                            text: networkManager.lobbyGameMode === "belote" ? "Belote" : "Coinche"
                            font.pixelSize: 50 * root.minRatio
                            font.bold: true
                            color: "#FFD700"
                        }
                    }

                    Image {
                        id: rightModeArrow
                        source: "qrc:/resources/right-arrowMainMenu-svgrepo-com.svg"
                        width: 60 * root.minRatio
                        height: 60 * root.minRatio
                        anchors.verticalCenter: parent.verticalCenter
                        visible: root.isHost
                        opacity: networkManager.lobbyGameMode === "belote" ? 0.3 : 1.0

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: networkManager.lobbyGameMode !== "belote" ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onClicked: {
                                if (networkManager.lobbyGameMode !== "belote") {
                                    lobbyModeSwitchSound.play()
                                    networkManager.sendSetLobbyGameMode("belote")
                                }
                            }
                        }
                    }
                }
            }

            // Bouton inviter des amis (hôte uniquement)
            Rectangle {
                anchors.left: parent.left
                //anchors.top: parent.top
                anchors.leftMargin: 15 * root.minRatio
                anchors.verticalCenter: parent.verticalCenter
                width: 120 * root.minRatio
                height: 120 * root.minRatio
                radius: 15 * root.minRatio
                color: root.accountType !== "guest" ? "#1976D2" : "#555555"
                visible: root.isHost
                opacity: root.accountType !== "guest" ? 1.0 : 0.4
                z: 10

                Image {
                    anchors.fill: parent
                    anchors.margins: 8 * root.minRatio
                    source: "qrc:/resources/friend-svgrepo-com.svg"
                    fillMode: Image.PreserveAspectFit
                    sourceSize: Qt.size(width, height)
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (root.accountType === "guest") {
                            lobbyGuestMessageRect.visible = true
                            lobbyGuestMessageTimer.start()
                        } else {
                            networkManager.getFriendsList()
                            inviteFriendsPopup.visible = true
                        }
                    }
                }
            }

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 12 * root.minRatio

                Item {
                    height: 1
                    width: 1
                }

                Text {
                    text: root.isHost ? "Invitez des amis et/ou partagez le code du lobby" : "Code du lobby"
                    font.pixelSize: 28 * root.minRatio
                    color: "#aaaaaa"
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: root.lobbyCode
                    font.pixelSize: 56 * root.minRatio
                    font.bold: true
                    color: "#FFD700"
                    font.family: "Orbitron"
                    font.letterSpacing: 12
                    Layout.alignment: Qt.AlignHCenter
                }

                Row {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 8 * root.minRatio

                    Image {
                        visible: root.isHost
                        source: "qrc:/resources/crown-svgrepo-com.svg"
                        width: 32 * root.minRatio
                        height: 32 * root.minRatio
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: root.isHost ? "Vous êtes l'hôte" : "Partagez ce code !"
                        font.pixelSize: 32 * root.minRatio
                        color: root.isHost ? "#FFD700" : "#888888"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Item {
                    height: 1
                    width: 1
                }
            }
        }

        // Zone des joueurs
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 530 * root.heightRatio
            color: "#802a2a2a"
            radius: 20 * root.minRatio
            border.color: "#FFD700"
            border.width: 3 * root.minRatio

            ColumnLayout {
                id: lobbyLayout
                anchors.fill: parent
                anchors.margins: 25 * root.minRatio
                spacing: 20 * root.minRatio

                Text {
                    text: "Joueurs (" + networkManager.lobbyPlayers.length + "/4)"
                    font.pixelSize: 32 * root.minRatio
                    font.bold: true
                    color: "#FFD700"
                }

                // Liste verticale des joueurs avec drag & drop
                Item {
                    id: playerListContainer
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    property real rowHeight: 90 * root.minRatio
                    property real rowSpacing: 12 * root.minRatio
                    property int playerCount: localPlayersModel.count

                    Repeater {
                        id: playerRepeater
                        model: localPlayersModel

                        delegate: Item {
                            id: delegateRoot
                            width: lobbyLayout.width
                            height: playerListContainer.rowHeight
                            z: root.draggedIndex === index ? 10 : 1

                            property real defaultY: index * (playerListContainer.rowHeight + playerListContainer.rowSpacing)
                            y: root.draggedIndex === index ? y : defaultY
                            Behavior on y {
                                enabled: root.draggedIndex !== index
                                NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
                            }

                            // Couleurs d'équipe : indices 0,1 = Équipe 1 (vert), 2,3 = Équipe 2 (rouge)
                            property bool isTeam1: index < 2
                            property bool showTeams: playerListContainer.playerCount >= 3

                            opacity: root.draggedIndex === index ? 0.7 : 1.0

                            Rectangle {
                                anchors.fill: parent
                                color: delegateRoot.showTeams ? (delegateRoot.isTeam1 ? "#3a4a3a" : "#4a3a3a") : "#3a4a3a"
                                radius: 15 * root.minRatio
                                border.color: delegateRoot.showTeams ? (delegateRoot.isTeam1 ? "#4CAF50" : "#F44336") : "#4CAF50"
                                border.width: 3 * root.minRatio

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 15 * root.minRatio
                                    spacing: 30 * root.minRatio

                                    Item {
                                        width: 1
                                        height: 1
                                    }

                                    // Avatar avec cercle
                                    Rectangle {
                                        Layout.preferredWidth: 60 * root.minRatio
                                        Layout.preferredHeight: 60 * root.minRatio
                                        radius: 30 * root.minRatio
                                        color: "#444444"
                                        border.color: "#FFD700"
                                        border.width: 2 * root.minRatio

                                        Image {
                                            anchors.fill: parent
                                            anchors.margins: 8 * root.minRatio
                                            source: "qrc:/resources/avatar/" + model.avatar
                                            fillMode: Image.PreserveAspectFit
                                        }
                                    }

                                    // Nom du joueur
                                    Text {
                                        text: model.name
                                        font.pixelSize: 32 * root.minRatio
                                        font.bold: true
                                        color: "white"
                                        //Layout.fillWidth: true
                                        Layout.preferredWidth: 200 * root.minRatio
                                    }

                                    // Label équipe
                                    Text {
                                        text: delegateRoot.isTeam1 ? "Équipe 1" : "Équipe 2"
                                        font.pixelSize: 28 * root.minRatio
                                        font.bold: true
                                        color: delegateRoot.isTeam1 ? "#4CAF50" : "#F44336"
                                        //visible: delegateRoot.showTeams
                                        opacity: delegateRoot.showTeams ? 1 : 0
                                        //Layout.preferredWidth: visible ? 630 * root.minRatio : 0
                                        Layout.fillWidth: true
                                    }

                                    // Badge hôte
                                    Text {
                                        text: "\uD83D\uDC51"
                                        font.pixelSize: 32 * root.minRatio
                                        visible: model.isHost
                                        Layout.preferredWidth: 35 * root.minRatio
                                    }

                                    // Indicateur ready
                                    Rectangle {
                                        Layout.preferredWidth: 16 * root.minRatio
                                        Layout.preferredHeight: 16 * root.minRatio
                                        radius: 8 * root.minRatio
                                        color: model.ready ? "#4CAF50" : "#FF5252"

                                        SequentialAnimation on scale {
                                            running: model.ready
                                            loops: Animation.Infinite
                                            NumberAnimation { to: 1.3; duration: 600 }
                                            NumberAnimation { to: 1.0; duration: 600 }
                                        }
                                    }

                                    // Texte statut
                                    Text {
                                        text: model.ready ? "Prêt \u2713" : "En attente..."
                                        font.pixelSize: 32 * root.minRatio
                                        font.bold: model.ready
                                        color: model.ready ? "#4CAF50" : "#FFA726"
                                        Layout.preferredWidth: 200 * root.minRatio
                                    }

                                    // Drag handle (hôte uniquement, 3+ joueurs)
                                    Image {
                                        source: "qrc:/resources/drag-handle-vertical-1-svgrepo-com.svg"
                                        Layout.preferredWidth: 50 * root.minRatio
                                        Layout.preferredHeight: 50 * root.minRatio
                                        sourceSize: Qt.size(width, height)
                                        //visible: root.isHost && playerListContainer.playerCount >= 3
                                        opacity: root.isHost && playerListContainer.playerCount >= 3 ? 1 : 0

                                        MouseArea {
                                            id: dragArea
                                            anchors.fill: parent
                                            anchors.margins: -10 * root.minRatio  // zone de touch plus large
                                            enabled: root.isHost && playerListContainer.playerCount >= 3
                                            cursorShape: enabled ? Qt.SizeVerCursor : Qt.ArrowCursor

                                            property real startMouseY: 0
                                            property real startDelegateY: 0

                                            onPressed: function(mouse) {
                                                var mapped = mapToItem(playerListContainer, 0, mouse.y)
                                                dragArea.startMouseY = mapped.y
                                                dragArea.startDelegateY = delegateRoot.y
                                                root.draggedIndex = index
                                            }

                                            onPositionChanged: function(mouse) {
                                                if (root.draggedIndex !== index) return
                                                var mapped = mapToItem(playerListContainer, 0, mouse.y)
                                                var deltaY = mapped.y - dragArea.startMouseY
                                                delegateRoot.y = dragArea.startDelegateY + deltaY

                                                // Déterminer le slot cible
                                                var centerY = delegateRoot.y + playerListContainer.rowHeight / 2
                                                var targetIdx = Math.round(centerY / (playerListContainer.rowHeight + playerListContainer.rowSpacing))
                                                targetIdx = Math.max(0, Math.min(targetIdx, localPlayersModel.count - 1))

                                                if (targetIdx !== index) {
                                                    localPlayersModel.move(index, targetIdx, 1)
                                                    root.draggedIndex = targetIdx
                                                    // Recalculer le startDelegateY pour que le drag continue fluide
                                                    dragArea.startDelegateY = targetIdx * (playerListContainer.rowHeight + playerListContainer.rowSpacing) + deltaY - (mapped.y - dragArea.startMouseY)
                                                }
                                            }

                                            onReleased: {
                                                // Snap à la position grille
                                                delegateRoot.y = delegateRoot.defaultY
                                                root.draggedIndex = -1

                                                // Envoyer le nouvel ordre au serveur
                                                var newOrder = []
                                                for (var i = 0; i < localPlayersModel.count; i++) {
                                                    newOrder.push(localPlayersModel.get(i).name)
                                                }
                                                networkManager.reorderLobbyPlayers(newOrder)
                                            }
                                        }
                                    }

                                    Item {
                                        width: 1
                                        height: 1
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Message d'information
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 55 * root.heightRatio
            color: {
                var count = networkManager.lobbyPlayers.length
                if (count === 1 || count === 3) return "#5D4037"
                else if (count === 2) return "#1565C0"
                else if (count === 4) return "#2E7D32"
                return "transparent"
            }
            radius: 10 * root.minRatio
            visible: networkManager.lobbyPlayers.length > 0

            Text {
                anchors.centerIn: parent
                text: {
                    var count = networkManager.lobbyPlayers.length
                    if (count === 1 || count === 3) {
                        return "⚠️  Il faut 2 ou 4 joueurs pour lancer une partie"
                    } else if (count === 2) {
                        return "ℹ️  A 2 joueurs, vous serez partenaires contre d'autres joueurs"
                    } else if (count === 4) {
                        return "✓  Lobby complet ! Tous prêts ?"
                    }
                    return ""
                }
                font.pixelSize: 32 * root.minRatio
                font.bold: true
                color: "white"
            }
        }

        // Boutons
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 120 * root.heightRatio
            spacing: 300 * root.minRatio

            // Bouton Prêt / Annuler
            AppButton {
                id: readyButton
                Layout.fillWidth: true
                Layout.fillHeight: true

                property bool isReady: false

                background: Rectangle {
                    color: readyButton.isReady ?
                           (parent.down ? "#E64A19" : (parent.hovered ? "#FF6F00" : "#FF5722")) :
                           (parent.down ? "#2E7D32" : (parent.hovered ? "#43A047" : "#4CAF50"))
                    radius: 12 * root.minRatio
                    border.color: readyButton.isReady ? "#BF360C" : "#1B5E20"
                    border.width: 2 * root.minRatio

                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                contentItem: Item {
                    Row {
                        anchors.centerIn: parent
                        spacing: 20 * root.minRatio

                        Image {
                            source: readyButton.isReady
                                ? "qrc:/resources/cross-small-svgrepo-com.svg"
                                : "qrc:/resources/check-svgrepo-com.svg"
                            width: 55 * root.minRatio
                            height: 55 * root.minRatio
                            sourceSize: Qt.size(width, height)
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: readyButton.isReady ? "Annuler " : "Prêt  "
                            font.pixelSize: 42 * root.minRatio
                            font.bold: true
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                onClicked: {
                    readyButton.isReady = !readyButton.isReady
                    networkManager.toggleLobbyReady(readyButton.isReady)
                }
            }

            // Bouton Jouer (visible seulement pour l'hôte)
            AppButton {
                id: playButton
                Layout.fillWidth: true
                Layout.fillHeight: true
                opacity: root.isHost ? 1 : 0

                enabled: root.isHost && (networkManager.lobbyPlayers.length === 2 || networkManager.lobbyPlayers.length === 4)

                background: Rectangle {
                    color: playButton.enabled ?
                           (playButton.down ? "#1565C0" : (playButton.hovered ? "#1E88E5" : "#2196F3")) :
                           "#424242"
                    radius: 12 * root.minRatio
                    border.color: playButton.enabled ? "#0D47A1" : "#212121"
                    border.width: 2 * root.minRatio

                    Behavior on color { ColorAnimation { duration: 200 } }

                    // Animation de pulsation quand actif
                    SequentialAnimation on scale {
                        running: playButton.enabled
                        loops: Animation.Infinite
                        NumberAnimation { to: 1.05; duration: 800; easing.type: Easing.InOutQuad }
                        NumberAnimation { to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
                    }
                }

                contentItem: Item {
                    Row {
                        anchors.centerIn: parent
                        spacing: 12 * root.minRatio

                        Image {
                            source: "qrc:/resources/play-alt-svgrepo-com.svg"
                            width: 50 * root.minRatio
                            height: 50 * root.minRatio
                            sourceSize: Qt.size(width, height)
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "JOUER "
                            font.pixelSize: 42 * root.minRatio
                            font.bold: true
                            color: parent.parent.parent.parent.enabled ? "white" : "#666666"
                            anchors.verticalCenter: parent.verticalCenter
                            font.letterSpacing: 2
                        }
                    }
                }

                onClicked: {
                    networkManager.startLobbyGame()
                }
            }

            // Bouton Quitter
            AppButton {
                Layout.preferredWidth: 360 * root.widthRatio
                Layout.fillHeight: true

                background: Rectangle {
                    color: parent.down ? "#B71C1C" : (parent.hovered ? "#D32F2F" : "#C62828")
                    radius: 12 * root.minRatio
                    border.color: "#7F0000"
                    border.width: 2 * root.minRatio

                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                contentItem: Item {
                    Row {
                        anchors.centerIn: parent
                        spacing: 10 * root.minRatio

                        Image {
                            source: "qrc:/resources/cross-small-svgrepo-com.svg"
                            width: 55 * root.minRatio
                            height: 55 * root.minRatio
                            sourceSize: Qt.size(width, height)
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "Quitter  "
                            font.pixelSize: 42 * root.minRatio
                            font.bold: true
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                onClicked: {
                    networkManager.leaveLobby()
                    stackView.pop()
                    stackView.pop()
                }
            }
        }
    }

    // Surveiller les changements de lobbyPlayers
    Connections {
        target: networkManager
        function onLobbyPlayersChanged() {
            if (root.draggedIndex !== -1) return  // Ne pas sync pendant un drag
            syncLocalModel()
        }

        function onLobbyError(errorMessage) {
            errorPopup.errorText = errorMessage
            errorPopup.open()
        }

        function onLobbyGameStarting() {
            var playerCount = networkManager.lobbyPlayers.length

            if (playerCount === 2) {
                // Lobby à 2 joueurs: pousser MatchMakingView pour attendre 2 autres joueurs
                stackView.push("qrc:/qml/MatchMakingView.qml", { autoJoin: false })
            } else if (playerCount === 4) {
                // Lobby à 4 joueurs: ne rien faire, gameFound arrivera immédiatement
                // et sera géré par MainMenu qui chargera directement CoincheView
            }
        }
    }

    // Popup d'erreur
    Popup {
        id: errorPopup
        anchors.centerIn: parent
        width: 600 * root.widthRatio
        height: 400 * root.heightRatio
        modal: true
        focus: true

        property string errorText: ""

        background: Rectangle {
            color: "#2a2a2a"
            radius: 15 * root.minRatio
            border.color: "#ff0000"
            border.width: 3 * root.minRatio
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20 * root.minRatio
            spacing: 30 * root.minRatio

            Text {
                text: "Erreur"
                font.pixelSize: 36 * root.minRatio
                font.bold: true
                color: "#ff0000"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: errorPopup.errorText
                font.pixelSize: 36 * root.minRatio
                color: "white"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            AppButton {
                Layout.alignment: Qt.AlignHCenter
                width: 180 * root.widthRatio
                height: 120 * root.heightRatio

                background: Rectangle {
                    color: parent.down ? "#cc0000" : (parent.hovered ? "#ff3333" : "#ff0000")
                    radius: 8 * root.minRatio
                }

                contentItem: Text {
                    text: "OK "
                    font.pixelSize: 36 * root.minRatio
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: errorPopup.close()
            }
        }
    }

    // Message invité (compte requis pour les amis)
    Rectangle {
        id: lobbyGuestMessageRect
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 80 * root.minRatio
        width: root.width * 0.8
        height: 100 * root.minRatio
        radius: 10 * root.minRatio
        color: "#2a2a2a"
        border.color: "#FFD700"
        border.width: 2 * root.minRatio
        visible: false
        z: 200

        Text {
            anchors.centerIn: parent
            text: "Vous devez avoir un compte pour inviter des amis"
            font.pixelSize: 36 * root.minRatio
            color: "#FFD700"
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            width: parent.width - 20 * root.minRatio
        }

        NumberAnimation on opacity {
            running: lobbyGuestMessageRect.visible
            from: 0
            to: 1
            duration: 300
        }

        Timer {
            id: lobbyGuestMessageTimer
            interval: 5000
            repeat: false
            onTriggered: {
                lobbyGuestMessageRect.visible = false
            }
        }
    }

    // Popup d'invitation d'amis
    Rectangle {
        id: inviteFriendsPopup
        anchors.fill: parent
        color: "#CC000000"
        visible: false
        z: 500

        property int selectedCount: 0

        ListModel { id: onlineFriendsModel }

        function getSelectedPseudos() {
            var list = []
            for (var i = 0; i < onlineFriendsModel.count; i++) {
                if (onlineFriendsModel.get(i).selected) {
                    list.push(onlineFriendsModel.get(i).pseudo)
                }
            }
            return list
        }

        MouseArea {
            anchors.fill: parent
            onClicked: inviteFriendsPopup.visible = false
        }

        Rectangle {
            id: popupRect
            anchors.centerIn: parent
            width: parent.width * 0.5
            height: parent.height * 0.75
            color: "#1a1a1a"
            radius: 20 * root.minRatio
            border.color: "#FFD700"
            border.width: 3 * root.minRatio

            MouseArea {
                anchors.fill: parent
                onClicked: {} // bloquer propagation
            }

            // Titre
            Text {
                id: popupTitle
                text: "Inviter des amis"
                font.pixelSize: 44 * root.minRatio
                font.bold: true
                color: "#FFD700"
                anchors.top: parent.top
                anchors.topMargin: 25 * root.minRatio
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // Compteur
            Text {
                id: popupCounter
                text: inviteFriendsPopup.selectedCount + "/3 sélectionnés"
                font.pixelSize: 26 * root.minRatio
                color: "#aaaaaa"
                anchors.top: popupTitle.bottom
                anchors.topMargin: 15 * root.minRatio
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // Message si aucun ami en ligne
            Text {
                visible: onlineFriendsModel.count === 0
                text: "Aucun ami en ligne"
                font.pixelSize: 30 * root.minRatio
                color: "#888888"
                anchors.centerIn: parent
            }

            // Liste des amis en ligne
            Flickable {
                anchors.top: popupCounter.bottom
                anchors.topMargin: 20 * root.minRatio
                anchors.bottom: popupButtons.top
                anchors.bottomMargin: 15 * root.minRatio
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 25 * root.minRatio
                anchors.rightMargin: 25 * root.minRatio
                contentHeight: friendsColumn.height
                clip: true
                flickableDirection: Flickable.VerticalFlick

                Column {
                    id: friendsColumn
                    width: parent.width
                    spacing: 10 * root.minRatio

                    Repeater {
                        model: onlineFriendsModel

                        Rectangle {
                            width: friendsColumn.width
                            height: 90 * root.minRatio
                            radius: 10 * root.minRatio
                            color: model.selected ? "#2a4a2a" : "#2a2a2a"
                            border.color: model.selected ? "#4CAF50" : "#3a3a3a"
                            border.width: 2 * root.minRatio

                            Row {
                                anchors.fill: parent
                                anchors.margins: 10 * root.minRatio
                                spacing: 15 * root.minRatio

                                // Checkbox visuelle
                                Rectangle {
                                    width: 40 * root.minRatio
                                    height: 40 * root.minRatio
                                    radius: 5 * root.minRatio
                                    color: model.selected ? "#4CAF50" : "#3a3a3a"
                                    border.color: model.selected ? "#4CAF50" : "#666666"
                                    border.width: 2 * root.minRatio
                                    anchors.verticalCenter: parent.verticalCenter

                                    Image {
                                        anchors.fill: parent
                                        anchors.margins: 4 * root.minRatio
                                        source: "qrc:/resources/check-svgrepo-com.svg"
                                        fillMode: Image.PreserveAspectFit
                                        visible: model.selected
                                    }
                                }

                                // Avatar
                                Rectangle {
                                    width: 70 * root.minRatio
                                    height: 70 * root.minRatio
                                    radius: width / 2
                                    color: "#3a3a3a"
                                    anchors.verticalCenter: parent.verticalCenter
                                    clip: true

                                    Image {
                                        anchors.fill: parent
                                        anchors.margins: 9 * root.minRatio
                                        source: model.avatar ? "qrc:/resources/avatar/" + model.avatar : ""
                                        fillMode: Image.PreserveAspectFit
                                        visible: model.avatar !== ""
                                    }
                                }

                                // Pseudo
                                Text {
                                    text: model.pseudo
                                    font.pixelSize: 30 * root.minRatio
                                    font.bold: true
                                    color: "white"
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (model.selected) {
                                        onlineFriendsModel.setProperty(index, "selected", false)
                                        inviteFriendsPopup.selectedCount--
                                    } else if (inviteFriendsPopup.selectedCount < 3) {
                                        onlineFriendsModel.setProperty(index, "selected", true)
                                        inviteFriendsPopup.selectedCount++
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Boutons ancrés en bas de la popup
            Row {
                id: popupButtons
                spacing: 50 * root.minRatio
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 25 * root.minRatio
                anchors.horizontalCenter: parent.horizontalCenter

                Rectangle {
                    width: 250 * root.minRatio
                    height: 80 * root.minRatio
                    radius: 10 * root.minRatio
                    color: inviteFriendsPopup.selectedCount > 0 ? "#006600" : "#333333"

                    Text {
                        anchors.centerIn: parent
                        text: "Inviter"
                        font.pixelSize: 30 * root.minRatio
                        font.bold: true
                        color: inviteFriendsPopup.selectedCount > 0 ? "white" : "#666666"
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: inviteFriendsPopup.selectedCount > 0 ? Qt.PointingHandCursor : Qt.ArrowCursor
                        onClicked: {
                            if (inviteFriendsPopup.selectedCount > 0) {
                                var pseudos = inviteFriendsPopup.getSelectedPseudos()
                                networkManager.inviteFriendsToLobby(pseudos)
                                inviteFriendsPopup.visible = false
                                inviteFriendsPopup.selectedCount = 0
                            }
                        }
                    }
                }

                Rectangle {
                    width: 250 * root.minRatio
                    height: 80 * root.minRatio
                    radius: 10 * root.minRatio
                    color: "#3a3a3a"
                    border.color: "#FFD700"
                    border.width: 2 * root.minRatio

                    Text {
                        anchors.centerIn: parent
                        text: "Annuler "
                        font.pixelSize: 30 * root.minRatio
                        font.bold: true
                        color: "#FFD700"
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            inviteFriendsPopup.visible = false
                            inviteFriendsPopup.selectedCount = 0
                        }
                    }
                }
            }
        }
    }

    // Connexion pour recevoir la liste d'amis (popup invitation)
    Connections {
        target: networkManager
        enabled: inviteFriendsPopup.visible

        function onFriendsListReceived(friends, pendingRequests) {
            onlineFriendsModel.clear()
            inviteFriendsPopup.selectedCount = 0
            for (var i = 0; i < friends.length; i++) {
                if (friends[i].online) {
                    onlineFriendsModel.append({
                        pseudo: friends[i].pseudo,
                        avatar: friends[i].avatar || "",
                        selected: false
                    })
                }
            }
        }
    }

    function syncLocalModel() {
        localPlayersModel.clear()
        for (var i = 0; i < networkManager.lobbyPlayers.length; i++) {
            var p = networkManager.lobbyPlayers[i]
            localPlayersModel.append({
                name: p.name,
                avatar: p.avatar,
                ready: p.ready,
                isHost: p.isHost
            })
        }
    }

    Component.onCompleted: {
        syncLocalModel()
    }
}
