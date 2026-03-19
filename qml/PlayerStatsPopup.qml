import QtQuick
import QtQuick.Controls

Rectangle {
    id: statsPopup
    anchors.fill: parent
    color: "#CC000000"  // Fond sombre semi-transparent
    visible: false
    opacity: 0.8
    z: 1000

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
        width: Math.min(parent.width * 0.9, 800)
        height: Math.min(parent.height * 0.9, 600)
        color: "#1a1a1a"
        radius: 15
        border.color: "#FFD700"
        border.width: 3

        // Empêcher la fermeture quand on clique dans la popup
        MouseArea {
            anchors.fill: parent
            onClicked: {
                // Ne rien faire - bloquer la propagation
            }
        }

        // Bouton X fermer en haut à droite
        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 10
            width: 40
            height: 40
            radius: 20
            color: "#cc0000"
            z: 10

            Image {
                anchors.fill: parent
                anchors.margins: 8
                source: "qrc:/resources/cross-small-svgrepo-com.svg"
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: statsPopup.closePopup()
            }
        }

        Column {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            // Header avec nom du joueur
            Rectangle {
                width: parent.width
                height: 60
                color: "#2a2a2a"
                radius: 10

                Row {
                    anchors.centerIn: parent
                    spacing: 15

                    Text {
                        text: "📊"
                        font.pixelSize: 32
                        color: "#FFD700"
                    }

                    Text {
                        text: "Statistiques de " + statsPopup.playerName
                        font.pixelSize: 28
                        font.bold: true
                        color: "#FFD700"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            // Scrollable content
            ScrollView {
                width: parent.width
                height: parent.height - 120
                clip: true

                Column {
                    width: parent.width
                    spacing: 20

                    // Section: Parties
                    StatsSection {
                        title: "🏆 VICTOIRES"
                        titleColor: "#4CAF50"
                    }

                    Grid {
                        columns: 2
                        columnSpacing: 40
                        rowSpacing: 10
                        width: parent.width

                        StatItem { label: "Parties jouées"; value: statsPopup.gamesPlayed }
                        StatItem { label: "Victoires"; value: statsPopup.gamesWon }
                        StatItem { label: "Taux de victoire"; value: statsPopup.winRate + " %" }
                        StatItem { label: "Série de victoires"; value: statsPopup.maxWinStreak }
                    }

                    // Section: Coinches
                    StatsSection {
                        title: "🎯️ COINCHES"
                        titleColor: "#FF9800"
                    }

                    Grid {
                        columns: 3
                        columnSpacing: 10
                        rowSpacing: 10
                        width: parent.width

                        StatItem { label: "Coinches tentées"; value: statsPopup.coinches }
                        StatItem { label: "Coinches réussies"; value: statsPopup.coinchesReussies }
                        StatItem { label: "Taux coinche"; value: statsPopup.tauxCoincheReussite + " %" }
                    }

                    // Section: Coinches subies
                    StatsSection {
                        title: "🛡️ COINCHES SUBIES"
                        titleColor: "#9C27B0"
                    }

                    Grid {
                        columns: 3
                        columnSpacing: 10
                        rowSpacing: 10
                        width: parent.width

                        StatItem { label: "Coinches subies"; value: statsPopup.annoncesCoinchees }
                        StatItem { label: "Victoires"; value: statsPopup.annoncesCoincheeGagnees }
                        StatItem { label: "Taux de victoire"; value: statsPopup.tauxCoincheeReussite + " %" }
                    }

                    // Section: Surcoinches
                    StatsSection {
                        title: "🔥️ SURCOINCHES"
                        titleColor: "#FF9800"
                    }

                    Grid {
                        columns: 3
                        columnSpacing: 10
                        rowSpacing: 10
                        width: parent.width

                        StatItem { label: "Surcoinches tentées"; value: statsPopup.surcoinchesTentees }
                        StatItem { label: "Surcoinches réussies"; value: statsPopup.surcoincheReussies }
                        StatItem { label: "Taux surcoinche"; value: statsPopup.tauxSurcoincheReussite + " %" }
                    }

                    // Section: Surcoinches subies
                    StatsSection {
                        title: "🛡️ SURCOINCHES SUBIES"
                        titleColor: "#9C27B0"
                    }

                    Grid {
                        columns: 3
                        columnSpacing: 10
                        rowSpacing: 10
                        width: parent.width

                        StatItem { label: "Surcoinches subies"; value: statsPopup.annoncesSurcoinchees }
                        StatItem { label: "Victoires"; value: statsPopup.annoncesSurcoincheesGagnees }
                        StatItem { label: "Taux de victoire"; value: statsPopup.tauxSurcoincheesReussite + " %" }
                    }

                    // Section: Capot
                    StatsSection {
                        title: "💥 CAPOTS"
                        titleColor: "#2196F3"
                    }

                    Grid {
                        columns: 3
                        columnSpacing: 10
                        rowSpacing: 10
                        width: parent.width

                        StatItem { label: "Capots annoncés"; value: statsPopup.capotsAnnonces }
                        StatItem { label: "Capots réussis"; value: statsPopup.capotsReussis }
                        StatItem { label: "Taux capot"; value: statsPopup.tauxCapotReussite + " %" }
                    }

                    // Section: Générale
                    StatsSection {
                        title: "⭐ GÉNÉRALE"
                        titleColor: "#E91E63"
                    }

                    Grid {
                        columns: 3
                        columnSpacing: 10
                        rowSpacing: 10
                        width: parent.width

                        StatItem { label: "Générales tentées"; value: statsPopup.generalesTentees }
                        StatItem { label: "Générales réussies"; value: statsPopup.generalesReussies }
                        StatItem { label: "Taux générale"; value: statsPopup.tauxGeneraleReussite + " %" }
                    }
                }
            }

            // Bouton demander en ami / déjà ami
            Button {
                width: parent.width
                height: 50
                enabled: !statsPopup.isFriend

                background: Rectangle {
                    color: statsPopup.isFriend ? "#555555" : (parent.pressed ? "#004400" : "#006600")
                    radius: 10
                    border.color: statsPopup.isFriend ? "#888888" : "#00cc00"
                    border.width: 2
                }

                contentItem: Text {
                    text: statsPopup.isFriend ? "Déjà ami" : "Demander en ami"
                    font.pixelSize: 20
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
        }
    }

    // Composant pour les titres de section
    component StatsSection: Rectangle {
        property string title: ""
        property color titleColor: "#FFD700"

        width: parent.width
        height: 40
        color: "#2a2a2a"
        radius: 5

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            text: title
            font.pixelSize: 20
            font.bold: true
            color: titleColor
        }
    }

    // Composant pour un élément de statistique
    component StatItem: Row {
        property string label: ""
        property var value: 0

        spacing: 10
        width: (parent.width - parent.columnSpacing) / 3

        Text {
            id: labelText
            text: label + ":"
            font.pixelSize: 16
            color: "#CCCCCC"
        }

        Text {
            text: value.toString()
            font.pixelSize: 16
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
                    // winRatio est un ratio (0.0 à 1.0), on le convertit en pourcentage
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
                }
            } catch (e) {
                console.error("Erreur parsing stats:", e)
            }
        }
    }
}
