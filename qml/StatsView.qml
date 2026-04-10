import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

Rectangle {
    id: statsRoot

    SoundEffect {
        id: backSound
        source: "qrc:/resources/sons/742832__sadiquecat__woosh-metal-tea-strainer-1.wav"
    }
    anchors.fill: parent
    color: "transparent"

    // Ratio responsive pour adapter la taille des composants
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Fond étoilé
    StarryBackground {
        anchors.fill: parent
        minRatio: statsRoot.minRatio
        z: -1
    }

    // Propriétés pour les statistiques Coinche
    property int gamesPlayed: 0
    property int gamesWon: 0
    property real winRatio: 0.0
    property int coincheAttempts: 0
    property int coincheSuccess: 0
    property int capotRealises: 0
    property int capotAnnoncesRealises: 0
    property int capotAnnoncesTentes: 0
    property int generaleAttempts: 0
    property int generaleSuccess: 0
    property int annoncesCoinchees: 0
    property int annoncesCoincheesgagnees: 0
    property int surcoincheAttempts: 0
    property int surcoincheSuccess: 0
    property int annoncesSurcoinchees: 0
    property int annoncesSurcoincheesGagnees: 0
    property int maxWinStreak: 0
    property string playerName: ""

    // Propriétés pour les statistiques Belote
    property int beloteGamesPlayed: 0
    property int beloteGamesWon: 0
    property int beloteMaxWinStreak: 0
    property int beloteCapots: 0

    // Sélecteur de mode affiché
    property int selectedMode: 0  // 0 = Coinche, 1 = Belote

    signal backToMenu()

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
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (AudioSettings.effectsEnabled) backSound.play()
                backToMenu()
            }
            onEntered: {
                parent.scale = 1.1
            }
            onExited: {
                parent.scale = 1.0
            }
        }

        Behavior on scale {
            NumberAnimation { duration: 100 }
        }
    }

    Component.onCompleted: {
        // Demander les statistiques au serveur
        if (playerName !== "") {
            requestStats()
        }
    }

    // Redemander les statistiques quand la vue devient visible
    onVisibleChanged: {
        if (visible && playerName !== "") {
            console.log("StatsView visible, mise à jour des statistiques")
            requestStats()
        }
    }

    function requestStats() {
        networkManager.requestStats(playerName)
    }

    Connections {
        target: networkManager

        function onMessageReceived(message) {
            var data = JSON.parse(message)

            if (data.type === "statsData") {
                gamesPlayed = data.gamesPlayed
                gamesWon = data.gamesWon
                winRatio = data.winRatio
                coincheAttempts = data.coincheAttempts
                coincheSuccess = data.coincheSuccess
                capotRealises = data.capotRealises || 0
                capotAnnoncesRealises = data.capotAnnoncesRealises || 0
                capotAnnoncesTentes = data.capotAnnoncesTentes || 0
                generaleAttempts = data.generaleAttempts || 0
                generaleSuccess = data.generaleSuccess || 0
                annoncesCoinchees = data.annoncesCoinchees || 0
                annoncesCoincheesgagnees = data.annoncesCoincheesgagnees || 0
                surcoincheAttempts = data.surcoincheAttempts || 0
                surcoincheSuccess = data.surcoincheSuccess || 0
                annoncesSurcoinchees = data.annoncesSurcoinchees || 0
                annoncesSurcoincheesGagnees = data.annoncesSurcoincheesGagnees || 0
                maxWinStreak = data.maxWinStreak || 0

                // Stats Belote
                beloteGamesPlayed = data.beloteGamesPlayed || 0
                beloteGamesWon = data.beloteGamesWon || 0
                beloteMaxWinStreak = data.beloteMaxWinStreak || 0
                beloteCapots = data.beloteCapots || 0
            }
        }
    }

    // Scroll view pour contenu responsive
    ScrollView {
        anchors.fill: parent
        anchors.margins: 20 * minRatio
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: parent.width
            spacing: 20 * minRatio

            // Titre
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                spacing: 10 * minRatio

                Text {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                text: "STATISTIQUES DE JEU"
                font.pixelSize: 42 * minRatio
                font.bold: true
                color: "#FFD700"
                style: Text.Outline
                styleColor: "#000000"
                horizontalAlignment: Text.AlignHCenter
                }
            }

            // Nom du joueur
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                spacing: 10 * minRatio

                Text {
                    text: "👤"
                    font.pixelSize: 26 * minRatio
                }

                Text {
                    text: playerName
                    font.pixelSize: Math.min(28 * minRatio, 24)
                    font.bold: true
                    color: "#FFD700"
                }
            }

            Item { Layout.preferredHeight: 10 * minRatio }

            // Sélecteur Coinche / Belote
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 20 * minRatio

                SoundEffect {
                    id: modeSwitchSound
                    source: "qrc:/resources/sons/742832__sadiquecat__woosh-metal-tea-strainer-1.wav"
                }

                Image {
                    source: "qrc:/resources/left-arrowMainMenu-svgrepo-com.svg"
                    Layout.preferredWidth: 40 * minRatio
                    Layout.preferredHeight: 40 * minRatio
                    opacity: statsRoot.selectedMode > 0 ? 1.0 : 0.3
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (statsRoot.selectedMode > 0) {
                                statsRoot.selectedMode--
                                if (AudioSettings.effectsEnabled) modeSwitchSound.play()
                            }
                        }
                    }
                }

                Item {
                    Layout.preferredWidth: 180 * minRatio
                    Layout.preferredHeight: 40 * minRatio
                    Text {
                        anchors.centerIn: parent
                        text: statsRoot.selectedMode === 0 ? "Coinche" : "Belote"
                        font.pixelSize: 32 * minRatio
                        font.bold: true
                        color: "#FFD700"
                        style: Text.Outline
                        styleColor: "#000000"
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                Image {
                    source: "qrc:/resources/right-arrowMainMenu-svgrepo-com.svg"
                    Layout.preferredWidth: 40 * minRatio
                    Layout.preferredHeight: 40 * minRatio
                    opacity: statsRoot.selectedMode < 1 ? 1.0 : 0.3
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (statsRoot.selectedMode < 1) {
                                statsRoot.selectedMode++
                                if (AudioSettings.effectsEnabled) modeSwitchSound.play()
                            }
                        }
                    }
                }
            }

            Item { Layout.preferredHeight: 5 * minRatio }

            // ── Mode Coinche ──────────────────────────────────────────────
            ColumnLayout {
                Layout.fillWidth: true
                visible: statsRoot.selectedMode === 0
                spacing: 20 * minRatio

                // Section Victoires
                Row {
                    spacing: 8 * minRatio
                    Layout.topMargin: 10 * minRatio

                    Image {
                        source: "qrc:/resources/trophy-svgrepo-com.svg"
                        width: Math.min(28 * minRatio, 24)
                        height: Math.min(28 * minRatio, 24)
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "VICTOIRES"
                        font.pixelSize: Math.min(28 * minRatio, 24)
                        font.bold: true
                        color: "#FFD700"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                // Grid layout pour les cartes de stats
                GridLayout {
                    Layout.fillWidth: true
                    columns: 4  // Forcer 4 colonnes pour que tout tienne sur une ligne
                    columnSpacing: 10 * widthRatio
                    rowSpacing: 15 * heightRatio

                    // Carte: Parties
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 160 * heightRatio
                        Layout.minimumWidth: 180 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#FFD700"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 15 * minRatio
                            spacing: 8 * minRatio

                            Text {
                                text: "Parties jouées"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Item { Layout.fillHeight: true }

                            Text {
                                text: gamesPlayed.toString()
                                font.pixelSize: Math.min(48 * minRatio, 42)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Victoires
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 160 * heightRatio
                        Layout.minimumWidth: 180 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#FFD700"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 15 * minRatio
                            spacing: 8 * minRatio

                            Text {
                                text: "Victoires"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Item { Layout.fillHeight: true }

                            Text {
                                text: gamesWon.toString()
                                font.pixelSize: Math.min(48 * minRatio, 42)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Taux de victoire
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 160 * heightRatio
                        Layout.minimumWidth: 180 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#FFD700"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 15 * minRatio
                            spacing: 8 * minRatio

                            Text {
                                text: "Taux de victoire"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Item { Layout.fillHeight: true }

                            Text {
                                text: (winRatio * 100).toFixed(1) + "%"
                                font.pixelSize: Math.min(48 * minRatio, 42)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Série de victoire
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 160 * heightRatio
                        Layout.minimumWidth: 180 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#FFD700"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 15 * minRatio
                            spacing: 8 * minRatio

                            Text {
                                text: "Série de victoire"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Item { Layout.fillHeight: true }

                            Text {
                                text: maxWinStreak.toString()
                                font.pixelSize: Math.min(48 * minRatio, 42)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }
                }

                // Section Coinches
                Row {
                    spacing: 8 * minRatio
                    Layout.topMargin: 10 * minRatio

                    Image {
                        source: "qrc:/resources/target-svgrepo-com.svg"
                        width: Math.min(28 * minRatio, 24)
                        height: Math.min(28 * minRatio, 24)
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "COINCHES"
                        font.pixelSize: Math.min(28 * minRatio, 24)
                        font.bold: true
                        color: "#ff6666"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: Math.max(1, Math.floor(parent.width / (250 * widthRatio)))
                    columnSpacing: 15 * widthRatio
                    rowSpacing: 15 * heightRatio

                    // Carte: Coinches tentées
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#ff6666"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Tentatives"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: coincheAttempts.toString()
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Coinches réussies
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#ff6666"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Réussies"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: coincheSuccess.toString()
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Taux de réussite
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#ff6666"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Taux"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: coincheAttempts > 0 ? ((coincheSuccess / coincheAttempts) * 100).toFixed(1) + "%" : "0%"
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }
                }

                // Section Annonces Coinchées (coinches subies)
                Row {
                    spacing: 8 * minRatio
                    Layout.topMargin: 10 * minRatio

                    Image {
                        source: "qrc:/resources/shield-svgrepo-com.svg"
                        width: Math.min(28 * minRatio, 24)
                        height: Math.min(28 * minRatio, 24)
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "COINCHES SUBIES"
                        font.pixelSize: Math.min(28 * minRatio, 24)
                        font.bold: true
                        color: "#6699ff"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: Math.max(1, Math.floor(parent.width / (250 * widthRatio)))
                    columnSpacing: 15 * widthRatio
                    rowSpacing: 15 * heightRatio

                    // Carte: Annonces coinchées
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#6699ff"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Subies"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#6699ff"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: annoncesCoinchees.toString()
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#6699ff"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Annonces coinchées gagnées
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#6699ff"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Gagnées malgré tout"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#6699ff"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: annoncesCoincheesgagnees.toString()
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#6699ff"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Taux de victoire malgré coinche
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#6699ff"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Taux de victoire"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#6699ff"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: annoncesCoinchees > 0 ? ((annoncesCoincheesgagnees / annoncesCoinchees) * 100).toFixed(1) + "%" : "0%"
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#6699ff"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }
                }

                // Section Surcoinches
                Row {
                    spacing: 8 * minRatio
                    Layout.topMargin: 10 * minRatio

                    Image {
                        source: "qrc:/resources/fire-svgrepo-com.svg"
                        width: Math.min(28 * minRatio, 24)
                        height: Math.min(28 * minRatio, 24)
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "SURCOINCHES"
                        font.pixelSize: Math.min(28 * minRatio, 24)
                        font.bold: true
                        color: "#ff9900"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: Math.max(1, Math.floor(parent.width / (250 * widthRatio)))
                    columnSpacing: 15 * widthRatio
                    rowSpacing: 15 * heightRatio

                    // Carte: Surcoinches tentées
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#ff9900"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Tentatives"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#ff9900"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: surcoincheAttempts.toString()
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#ff9900"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Surcoinches réussies
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#ff9900"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Réussies"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#ff9900"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: surcoincheSuccess.toString()
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#ff9900"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Taux de réussite
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#ff9900"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Taux"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#ff9900"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: surcoincheAttempts > 0 ? ((surcoincheSuccess / surcoincheAttempts) * 100).toFixed(1) + "%" : "0%"
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#ff9900"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }
                }

                // Section Surcoinches Subies
                Row {
                    spacing: 8 * minRatio
                    Layout.topMargin: 10 * minRatio

                    Image {
                        source: "qrc:/resources/shield2-svgrepo-com.svg"
                        width: Math.min(28 * minRatio, 24)
                        height: Math.min(28 * minRatio, 24)
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "SURCOINCHES SUBIES"
                        font.pixelSize: Math.min(28 * minRatio, 24)
                        font.bold: true
                        color: "#cc66ff"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: Math.max(1, Math.floor(parent.width / (250 * widthRatio)))
                    columnSpacing: 15 * widthRatio
                    rowSpacing: 15 * heightRatio

                    // Carte: Surcoinches subies
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#cc66ff"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Subies"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#cc66ff"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: annoncesSurcoinchees.toString()
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#cc66ff"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Surcoinches subies réussies
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#cc66ff"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Subies réussies"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#cc66ff"
                                Layout.alignment: Qt.AlignHCenter
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: annoncesSurcoincheesGagnees.toString()
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#cc66ff"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Taux de réussite subies
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#cc66ff"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Taux de victoire"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#cc66ff"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: annoncesSurcoinchees > 0 ? ((annoncesSurcoincheesGagnees / annoncesSurcoinchees) * 100).toFixed(1) + "%" : "0%"
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#cc66ff"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }
                }

                // Section Capots
                Row {
                    spacing: 8 * minRatio
                    Layout.topMargin: 10 * minRatio

                    Image {
                        source: "qrc:/resources/bolt-svgrepo-com.svg"
                        width: Math.min(28 * minRatio, 24)
                        height: Math.min(28 * minRatio, 24)
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "CAPOTS"
                        font.pixelSize: Math.min(28 * minRatio, 24)
                        font.bold: true
                        color: "#ff6666"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: Math.max(1, Math.floor(parent.width / (240 * widthRatio)))
                    columnSpacing: 15 * widthRatio
                    rowSpacing: 15 * heightRatio

                    // Carte: Capots réalisés
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 180 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#ff6666"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Réalisés"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: capotRealises.toString()
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Capots annoncés tentés
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 180 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#ff6666"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Annoncés tentés"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: capotAnnoncesTentes.toString()
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Capots annoncés réussis
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 180 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#ff6666"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Annoncés réussis"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: capotAnnoncesRealises.toString()
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Taux de réussite capot annoncé
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 180 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#ff6666"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Taux annoncés"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: capotAnnoncesTentes > 0 ? ((capotAnnoncesRealises / capotAnnoncesTentes) * 100).toFixed(1) + "%" : "0%"
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }
                }

                // Section Générales
                Row {
                    spacing: 8 * minRatio
                    Layout.topMargin: 10 * minRatio

                    Image {
                        source: "qrc:/resources/star-svgrepo-com.svg"
                        width: Math.min(28 * minRatio, 24)
                        height: Math.min(28 * minRatio, 24)
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "GÉNÉRALES"
                        font.pixelSize: Math.min(28 * minRatio, 24)
                        font.bold: true
                        color: "#FF9800"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: Math.max(1, Math.floor(parent.width / (250 * widthRatio)))
                    columnSpacing: 15 * widthRatio
                    rowSpacing: 15 * heightRatio

                    // Carte: Générales tentées
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#FF9800"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Tentatives"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#FF9800"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: generaleAttempts.toString()
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#FF9800"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Générales réussies
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#FF9800"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Réussies"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#FF9800"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: generaleSuccess.toString()
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#FF9800"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Taux de réussite
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 200 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#FF9800"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Taux"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#FF9800"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: generaleAttempts > 0 ? ((generaleSuccess / generaleAttempts) * 100).toFixed(1) + "%" : "0%"
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#FF9800"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }
                }
            }
            // ── Fin mode Coinche ──────────────────────────────────────────

            // ── Mode Belote ───────────────────────────────────────────────
            ColumnLayout {
                Layout.fillWidth: true
                visible: statsRoot.selectedMode === 1
                spacing: 20 * minRatio

                // Section Victoires
                Row {
                    spacing: 8 * minRatio
                    Layout.topMargin: 10 * minRatio

                    Image {
                        source: "qrc:/resources/trophy-svgrepo-com.svg"
                        width: Math.min(28 * minRatio, 24)
                        height: Math.min(28 * minRatio, 24)
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "VICTOIRES"
                        font.pixelSize: Math.min(28 * minRatio, 24)
                        font.bold: true
                        color: "#FFD700"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 4
                    columnSpacing: 10 * widthRatio
                    rowSpacing: 15 * heightRatio

                    // Carte: Parties jouées
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 160 * heightRatio
                        Layout.minimumWidth: 180 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#FFD700"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 15 * minRatio
                            spacing: 8 * minRatio

                            Text {
                                text: "Parties jouées"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Item { Layout.fillHeight: true }

                            Text {
                                text: beloteGamesPlayed.toString()
                                font.pixelSize: Math.min(48 * minRatio, 42)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Victoires
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 160 * heightRatio
                        Layout.minimumWidth: 180 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#FFD700"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 15 * minRatio
                            spacing: 8 * minRatio

                            Text {
                                text: "Victoires"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Item { Layout.fillHeight: true }

                            Text {
                                text: beloteGamesWon.toString()
                                font.pixelSize: Math.min(48 * minRatio, 42)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Taux de victoire
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 160 * heightRatio
                        Layout.minimumWidth: 180 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#FFD700"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 15 * minRatio
                            spacing: 8 * minRatio

                            Text {
                                text: "Taux de victoire"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Item { Layout.fillHeight: true }

                            Text {
                                text: beloteGamesPlayed > 0 ? ((beloteGamesWon / beloteGamesPlayed) * 100).toFixed(1) + "%" : "0%"
                                font.pixelSize: Math.min(48 * minRatio, 42)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }

                    // Carte: Série de victoire
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 160 * heightRatio
                        Layout.minimumWidth: 180 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#FFD700"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 15 * minRatio
                            spacing: 8 * minRatio

                            Text {
                                text: "Série de victoire"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Item { Layout.fillHeight: true }

                            Text {
                                text: beloteMaxWinStreak.toString()
                                font.pixelSize: Math.min(48 * minRatio, 42)
                                font.bold: true
                                color: "#FFD700"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }
                }

                // Section Capots
                Row {
                    spacing: 8 * minRatio
                    Layout.topMargin: 10 * minRatio

                    Image {
                        source: "qrc:/resources/bolt-svgrepo-com.svg"
                        width: Math.min(28 * minRatio, 24)
                        height: Math.min(28 * minRatio, 24)
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "CAPOTS"
                        font.pixelSize: Math.min(28 * minRatio, 24)
                        font.bold: true
                        color: "#ff6666"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: Math.max(1, Math.floor(parent.width / (240 * widthRatio)))
                    columnSpacing: 15 * widthRatio
                    rowSpacing: 15 * heightRatio

                    // Carte: Capots réalisés
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140 * heightRatio
                        Layout.minimumWidth: 180 * widthRatio
                        color: "#2a2a2a"
                        radius: 15 * minRatio
                        border.color: "#ff6666"
                        border.width: 2 * minRatio
                        opacity: 0.5

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12 * minRatio
                            spacing: 5 * minRatio

                            Text {
                                text: "Réalisés"
                                font.pixelSize: Math.min(24 * minRatio, 22)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }

                            Text {
                                text: beloteCapots.toString()
                                font.pixelSize: Math.min(42 * minRatio, 36)
                                font.bold: true
                                color: "#ff6666"
                                Layout.alignment: Qt.AlignHCenter
                                style: Text.Outline
                                styleColor: "#000000"
                            }
                        }
                    }
                }
            }
            // ── Fin mode Belote ───────────────────────────────────────────

            Item { Layout.preferredHeight: 10 * minRatio }

            // Bouton Retour avec effet moderne
            AppButton {
                Layout.preferredWidth: 300 * widthRatio
                Layout.preferredHeight: 120 * heightRatio
                Layout.alignment: Qt.AlignHCenter

                background: Rectangle {
                    color: parent.down ? "#006699" : (parent.hovered ? "#0088cc" : "#0077bb")
                    radius: 30 * minRatio
                    border.color: "#FFD700"
                    border.width: 2 * minRatio

                    Rectangle {
                        anchors.fill: parent
                        radius: parent.radius
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#FFFFFF" }
                            GradientStop { position: 0.5; color: "transparent" }
                        }
                        opacity: parent.parent.hovered ? 0.2 : 0.1
                    }
                }

                contentItem: RowLayout {
                    spacing: 5 * minRatio

                    Item {}

                    Image {
                        source: "qrc:/resources/home-fireplace-svgrepo-com.svg"
                        Layout.preferredWidth: 32 * minRatio
                        Layout.preferredHeight: 32 * minRatio
                    }

                    Text {
                        text: " RETOUR AU MENU "
                        font.pixelSize: 30 * minRatio
                        font.bold: true
                        color: "white"
                        style: Text.Outline
                        styleColor: "#000000"
                    }

                    Item {}
                }

                onClicked: {
                    backToMenu()
                }

                scale: 1.0
                Behavior on scale { NumberAnimation { duration: 100 } }
                onPressed: scale = 0.95
                onReleased: scale = 1.0
            }

            Item { Layout.preferredHeight: 20 * minRatio }
        }
    }
}
