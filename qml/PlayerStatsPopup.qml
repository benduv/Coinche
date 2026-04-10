import QtQuick
import QtQuick.Controls
import QtMultimedia

Rectangle {
    id: statsPopup
    anchors.fill: parent
    color: "#CC000000"  // Fond sombre semi-transparent
    visible: false
    opacity: 0.8
    z: 1000

    property bool isPortrait: height > width
    property real widthRatio: isPortrait ? width / 600 : width / 1024
    property real heightRatio: isPortrait ? height / 1024 : height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    property string playerName: ""
    property bool isFriend: false
    property int gamesPlayed: 0
    property int gamesWon: 0
    property int winRate: 0
    property int maxWinStreak: 0
    property int annoncesTentees: 0
    property int annoncesReussies: 0
    property int tauxReussite: 0
    property int coinches: 0
    property int coinchesReussies: 0
    property int tauxCoincheReussite: 0
    property int surcoinchesTentees: 0
    property int surcoincheReussies: 0
    property int tauxSurcoincheReussite: 0
    property int annoncesCoinchees: 0
    property int annoncesCoincheeGagnees: 0
    property int tauxCoincheeReussite: 0
    property int annoncesSurcoinchees: 0
    property int annoncesSurcoincheesGagnees: 0
    property int tauxSurcoincheesReussite: 0
    property int capotsAnnonces: 0
    property int capotsReussis: 0
    property int tauxCapotReussite: 0
    property int generalesTentees: 0
    property int generalesReussies: 0
    property int tauxGeneraleReussite: 0
    property bool hideFriendButton: false

    // Stats Belote
    property int beloteGamesPlayed: 0
    property int beloteGamesWon: 0
    property int beloteWinRate: 0
    property int beloteMaxWinStreak: 0
    property int beloteCapots: 0

    // Sélecteur de mode affiché
    property int selectedMode: 0  // 0 = Coinche, 1 = Belote

    SoundEffect {
        id: modeSwitchSound
        source: "qrc:/resources/sons/742832__sadiquecat__woosh-metal-tea-strainer-1.wav"
    }

    signal closePopup()
    signal addFriend(string playerName)

    // Fonction pour charger les stats depuis le serveur
    function loadPlayerStats(name) {
        if (!name || name === "") {
            console.error("PlayerStatsPopup - Nom du joueur vide!")
            return
        }
        playerName = name
        // Les stats seront chargées via WebSocket
        networkManager.requestStats(name)
    }

    // Zone cliquable pour fermer la popup
    MouseArea {
        anchors.fill: parent
        onClicked: statsPopup.closePopup()
    }

    // Contenu de la popup
    Rectangle {
        id: popupContent
        anchors.centerIn: parent
        width: parent.width * 0.9
        height: parent.height * 0.9
        color: "#1a1a1a"
        radius: 30 * minRatio
        border.color: "#FFD700"
        border.width: 6 * minRatio

        // Empêcher la fermeture quand on clique dans la popup
        MouseArea {
            anchors.fill: parent
            onClicked: {}
        }

        // Bouton X fermer en haut à droite
        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 20 * minRatio
            width: 80 * minRatio
            height: 80 * minRatio
            radius: 16 * minRatio
            color: "#cc0000"
            z: 10

            Image {
                anchors.fill: parent
                anchors.margins: 3 * minRatio
                source: "qrc:/resources/cross-small-svgrepo-com.svg"
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: statsPopup.closePopup()
            }
        }

        // Header : icône + nom du joueur
        Rectangle {
            id: popupHeader
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 40 * minRatio
            height: 120 * minRatio
            color: "#2a2a2a"
            radius: 20 * minRatio

            Row {
                anchors.centerIn: parent
                spacing: 30 * minRatio

                Image {
                    source: "qrc:/resources/increase-stats-svgrepo-com.svg"
                    width: 54 * minRatio
                    height: 54 * minRatio
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: "Statistiques de " + statsPopup.playerName
                    font.pixelSize: 48 * minRatio
                    font.bold: true
                    color: "#FFD700"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // Sélecteur Coinche / Belote
        Row {
            id: modeSelector
            anchors.top: popupHeader.bottom
            anchors.topMargin: 20 * minRatio
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 20 * minRatio

            Image {
                source: "qrc:/resources/left-arrowMainMenu-svgrepo-com.svg"
                width: 40 * minRatio
                height: 40 * minRatio
                anchors.verticalCenter: parent.verticalCenter
                opacity: statsPopup.selectedMode > 0 ? 1.0 : 0.3
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (statsPopup.selectedMode > 0) {
                            statsPopup.selectedMode--
                            if (AudioSettings.effectsEnabled) modeSwitchSound.play()
                        }
                    }
                }
            }

            Item {
                width: 200 * minRatio
                height: 44 * minRatio
                anchors.verticalCenter: parent.verticalCenter
                Text {
                    anchors.centerIn: parent
                    text: statsPopup.selectedMode === 0 ? "Coinche" : "Belote"
                    font.pixelSize: 40 * minRatio
                    font.bold: true
                    color: "#FFD700"
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            Image {
                source: "qrc:/resources/right-arrowMainMenu-svgrepo-com.svg"
                width: 40 * minRatio
                height: 40 * minRatio
                anchors.verticalCenter: parent.verticalCenter
                opacity: statsPopup.selectedMode < 1 ? 1.0 : 0.3
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (statsPopup.selectedMode < 1) {
                            statsPopup.selectedMode++
                            if (AudioSettings.effectsEnabled) modeSwitchSound.play()
                        }
                    }
                }
            }
        }

        // Bouton demander en ami / déjà ami
        AppButton {
            id: friendButton
            visible: !statsPopup.hideFriendButton
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 40 * minRatio
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.4
            height: 100 * minRatio
            enabled: !statsPopup.isFriend

            background: Rectangle {
                color: statsPopup.isFriend ? "#555555" : (parent.pressed ? "#004400" : "#006600")
                radius: 20 * minRatio
                border.color: statsPopup.isFriend ? "#888888" : "#00cc00"
                border.width: 4 * minRatio
            }

            contentItem: Text {
                text: statsPopup.isFriend ? "Déjà ami" : "Demander en ami"
                font.pixelSize: 40 * minRatio
                font.bold: true
                color: statsPopup.isFriend ? "#cccccc" : "#ffffff"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                if (!statsPopup.isFriend) {
                    statsPopup.addFriend(statsPopup.playerName)
                    statsPopup.closePopup()
                }
            }
        }

        // Scrollable content
        ScrollView {
            anchors.top: modeSelector.bottom
            anchors.topMargin: 20 * minRatio
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 40 * minRatio
            anchors.rightMargin: 40 * minRatio
            anchors.bottom: statsPopup.hideFriendButton ? parent.bottom : friendButton.top
            anchors.bottomMargin: statsPopup.hideFriendButton ? 40 * minRatio : 20 * minRatio
            clip: true
            contentWidth: availableWidth

            // Un seul enfant direct obligatoire pour ScrollView
            Column {
                width: parent.width
                spacing: 40 * minRatio

                // Vue Coinche
                Column {
                    width: parent.width
                    spacing: 40 * minRatio
                    visible: statsPopup.selectedMode === 0
                    // height doit être explicite quand visible=false pour ne pas prendre de place
                    height: visible ? implicitHeight : 0

                    StatsSection { title: "🏆 VICTOIRES"; titleColor: "#4CAF50" }
                    Grid {
                        columns: 2; columnSpacing: 80 * minRatio; rowSpacing: 20 * minRatio; width: parent.width
                        StatItem { label: "Parties jouées"; value: statsPopup.gamesPlayed }
                        StatItem { label: "Victoires"; value: statsPopup.gamesWon }
                        StatItem { label: "Taux de victoire"; value: statsPopup.winRate + " %" }
                        StatItem { label: "Série de victoires"; value: statsPopup.maxWinStreak }
                    }

                    StatsSection { title: "🎯️ COINCHES"; titleColor: "#FF9800" }
                    Grid {
                        columns: 3; columnSpacing: 20 * minRatio; rowSpacing: 20 * minRatio; width: parent.width
                        StatItem { label: "Coinches tentées"; value: statsPopup.coinches }
                        StatItem { label: "Coinches réussies"; value: statsPopup.coinchesReussies }
                        StatItem { label: "Taux coinche"; value: statsPopup.tauxCoincheReussite + " %" }
                    }

                    StatsSection { title: "🛡️ COINCHES SUBIES"; titleColor: "#9C27B0" }
                    Grid {
                        columns: 3; columnSpacing: 20 * minRatio; rowSpacing: 20 * minRatio; width: parent.width
                        StatItem { label: "Coinches subies"; value: statsPopup.annoncesCoinchees }
                        StatItem { label: "Victoires"; value: statsPopup.annoncesCoincheeGagnees }
                        StatItem { label: "Taux de victoire"; value: statsPopup.tauxCoincheeReussite + " %" }
                    }

                    StatsSection { title: "🔥️ SURCOINCHES"; titleColor: "#FF9800" }
                    Grid {
                        columns: 3; columnSpacing: 20 * minRatio; rowSpacing: 20 * minRatio; width: parent.width
                        StatItem { label: "Surcoinches tentées"; value: statsPopup.surcoinchesTentees }
                        StatItem { label: "Surcoinches réussies"; value: statsPopup.surcoincheReussies }
                        StatItem { label: "Taux surcoinche"; value: statsPopup.tauxSurcoincheReussite + " %" }
                    }

                    StatsSection { title: "🛡️ SURCOINCHES SUBIES"; titleColor: "#9C27B0" }
                    Grid {
                        columns: 3; columnSpacing: 20 * minRatio; rowSpacing: 20 * minRatio; width: parent.width
                        StatItem { label: "Surcoinches subies"; value: statsPopup.annoncesSurcoinchees }
                        StatItem { label: "Victoires"; value: statsPopup.annoncesSurcoincheesGagnees }
                        StatItem { label: "Taux de victoire"; value: statsPopup.tauxSurcoincheesReussite + " %" }
                    }

                    StatsSection { title: "💥 CAPOTS"; titleColor: "#2196F3" }
                    Grid {
                        columns: 3; columnSpacing: 20 * minRatio; rowSpacing: 20 * minRatio; width: parent.width
                        StatItem { label: "Capots annoncés"; value: statsPopup.capotsAnnonces }
                        StatItem { label: "Capots réussis"; value: statsPopup.capotsReussis }
                        StatItem { label: "Taux capot"; value: statsPopup.tauxCapotReussite + " %" }
                    }

                    StatsSection { title: "⭐ GÉNÉRALE"; titleColor: "#E91E63" }
                    Grid {
                        columns: 3; columnSpacing: 20 * minRatio; rowSpacing: 20 * minRatio; width: parent.width
                        StatItem { label: "Générales tentées"; value: statsPopup.generalesTentees }
                        StatItem { label: "Générales réussies"; value: statsPopup.generalesReussies }
                        StatItem { label: "Taux générale"; value: statsPopup.tauxGeneraleReussite + " %" }
                    }
                }

                // Vue Belote
                Column {
                    width: parent.width
                    spacing: 40 * minRatio
                    visible: statsPopup.selectedMode === 1
                    height: visible ? implicitHeight : 0

                    StatsSection { title: "🏆 VICTOIRES"; titleColor: "#4CAF50" }
                    Grid {
                        columns: 2; columnSpacing: 80 * minRatio; rowSpacing: 20 * minRatio; width: parent.width
                        StatItem { label: "Parties jouées"; value: statsPopup.beloteGamesPlayed }
                        StatItem { label: "Victoires"; value: statsPopup.beloteGamesWon }
                        StatItem { label: "Taux de victoire"; value: statsPopup.beloteWinRate + " %" }
                        StatItem { label: "Série de victoires"; value: statsPopup.beloteMaxWinStreak }
                    }

                    StatsSection { title: "💥 CAPOTS"; titleColor: "#2196F3" }
                    Grid {
                        columns: 1; rowSpacing: 20 * minRatio; width: parent.width
                        StatItem { label: "Capots réalisés"; value: statsPopup.beloteCapots }
                    }
                }
            }
        }
    }

    // Composant pour les titres de section
    component StatsSection: Rectangle {
        property string title: ""
        property color titleColor: "#FFD700"

        width: parent.width
        height: 80 * minRatio
        color: "#2a2a2a"
        radius: 10 * minRatio

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 30 * minRatio
            anchors.verticalCenter: parent.verticalCenter
            text: title
            font.pixelSize: 40 * minRatio
            font.bold: true
            color: titleColor
        }
    }

    // Composant pour un élément de statistique
    component StatItem: Row {
        property string label: ""
        property var value: 0

        spacing: 20 * minRatio
        width: (parent.width - parent.columnSpacing) / 3

        Text {
            text: label + ":"
            font.pixelSize: 32 * minRatio
            color: "#CCCCCC"
        }

        Text {
            text: value.toString()
            font.pixelSize: 32 * minRatio
            font.bold: true
            color: "#FFD700"
        }
    }

    // Recevoir les stats du serveur
    Connections {
        target: networkManager

        function onMessageReceived(message) {
            try {
                var msg = JSON.parse(message)
                if (msg.type === "statsData") {
                    statsPopup.isFriend = msg.isFriend || false
                    statsPopup.gamesPlayed = msg.gamesPlayed || 0
                    statsPopup.gamesWon = msg.gamesWon || 0
                    statsPopup.winRate = Math.round((msg.winRatio || 0) * 100)
                    statsPopup.maxWinStreak = msg.maxWinStreak || 0

                    // Coinches
                    statsPopup.coinches = msg.coincheAttempts || 0
                    statsPopup.coinchesReussies = msg.coincheSuccess || 0
                    statsPopup.tauxCoincheReussite = statsPopup.coinches > 0 ? Math.round((statsPopup.coinchesReussies / statsPopup.coinches) * 100) : 0

                    // Surcoinches
                    statsPopup.surcoinchesTentees = msg.surcoincheAttempts || 0
                    statsPopup.surcoincheReussies = msg.surcoincheSuccess || 0
                    statsPopup.tauxSurcoincheReussite = statsPopup.surcoinchesTentees > 0 ? Math.round((statsPopup.surcoincheReussies / statsPopup.surcoinchesTentees) * 100) : 0

                    // Coinches subies
                    statsPopup.annoncesCoinchees = msg.annoncesCoinchees || 0
                    statsPopup.annoncesCoincheeGagnees = msg.annoncesCoincheesgagnees || 0
                    statsPopup.tauxCoincheeReussite = statsPopup.annoncesCoinchees > 0 ? Math.round((statsPopup.annoncesCoincheeGagnees / statsPopup.annoncesCoinchees) * 100) : 0

                    // Surcoinches subies
                    statsPopup.annoncesSurcoinchees = msg.annoncesSurcoinchees || 0
                    statsPopup.annoncesSurcoincheesGagnees = msg.annoncesSurcoincheesGagnees || 0
                    statsPopup.tauxSurcoincheesReussite = statsPopup.annoncesSurcoinchees > 0 ? Math.round((statsPopup.annoncesSurcoincheesGagnees / statsPopup.annoncesSurcoinchees) * 100) : 0

                    // Capots
                    statsPopup.capotsAnnonces = msg.capotAnnoncesTentes || 0
                    statsPopup.capotsReussis = msg.capotAnnoncesRealises || 0
                    statsPopup.tauxCapotReussite = statsPopup.capotsAnnonces > 0 ? Math.round((statsPopup.capotsReussis / statsPopup.capotsAnnonces) * 100) : 0

                    // Générales
                    statsPopup.generalesTentees = msg.generaleAttempts || 0
                    statsPopup.generalesReussies = msg.generaleSuccess || 0
                    statsPopup.tauxGeneraleReussite = statsPopup.generalesTentees > 0 ? Math.round((statsPopup.generalesReussies / statsPopup.generalesTentees) * 100) : 0

                    // Stats Belote
                    statsPopup.beloteGamesPlayed = msg.beloteGamesPlayed || 0
                    statsPopup.beloteGamesWon = msg.beloteGamesWon || 0
                    statsPopup.beloteWinRate = statsPopup.beloteGamesPlayed > 0 ? Math.round((statsPopup.beloteGamesWon / statsPopup.beloteGamesPlayed) * 100) : 0
                    statsPopup.beloteMaxWinStreak = msg.beloteMaxWinStreak || 0
                    statsPopup.beloteCapots = msg.beloteCapots || 0
                }
            } catch (e) {
                console.error("Erreur parsing stats:", e)
            }
        }
    }
}
