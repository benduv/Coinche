import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: statsRoot
    anchors.fill: parent
    color: "#1a1a1a"

    // Ratio responsive pour adapter la taille des composants
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Propriétés pour les statistiques
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
    property string playerName: ""

    signal backToMenu()

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
                console.log("Stats reçues:", JSON.stringify(data))
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
                console.log("Stats affichées - Parties:", gamesPlayed, "Victoires:", gamesWon)
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

            // Titre avec effet
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 80 * heightRatio
                color: "transparent"

                Rectangle {
                    anchors.fill: parent
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#FFD700" }
                        GradientStop { position: 1.0; color: "#FFA500" }
                    }
                    radius: 15 * minRatio
                    opacity: 0.2
                }

                Text {
                    anchors.centerIn: parent
                    text: "📊 STATISTIQUES DE JEU"
                    font.pixelSize: Math.min(42 * minRatio, 36)
                    font.bold: true
                    color: "#FFD700"
                    style: Text.Outline
                    styleColor: "#000000"
                }
            }

            // Nom du joueur avec badge
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 50 * heightRatio
                color: "#2a2a2a"
                radius: 25 * minRatio
                border.color: "#FFD700"
                border.width: 2 * minRatio

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 10 * minRatio

                    Text {
                        text: "👤"
                        font.pixelSize: 24 * minRatio
                    }

                    Text {
                        text: playerName
                        font.pixelSize: Math.min(24 * minRatio, 20)
                        font.bold: true
                        color: "#FFD700"
                    }
                }
            }

            // Grid layout pour les cartes de stats
            GridLayout {
                Layout.fillWidth: true
                columns: Math.max(1, Math.floor(parent.width / (300 * widthRatio)))
                columnSpacing: 15 * widthRatio
                rowSpacing: 15 * heightRatio

                // Carte: Parties
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 160 * heightRatio
                    Layout.minimumWidth: 250 * widthRatio
                    color: "#2a2a2a"
                    radius: 15 * minRatio
                    border.color: "#4CAF50"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 15 * minRatio
                        spacing: 8 * minRatio

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8 * minRatio

                            Text {
                                text: "🎮"
                                font.pixelSize: 28 * minRatio
                            }

                            Text {
                                text: "Parties jouées"
                                font.pixelSize: Math.min(20 * minRatio, 18)
                                font.bold: true
                                color: "#4CAF50"
                                Layout.fillWidth: true
                            }
                        }

                        Item { Layout.fillHeight: true }

                        Text {
                            text: gamesPlayed.toString()
                            font.pixelSize: Math.min(48 * minRatio, 42)
                            font.bold: true
                            color: "#4CAF50"
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
                    Layout.minimumWidth: 250 * widthRatio
                    color: "#2a2a2a"
                    radius: 15 * minRatio
                    border.color: "#00ff00"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 15 * minRatio
                        spacing: 8 * minRatio

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8 * minRatio

                            Text {
                                text: "🏆"
                                font.pixelSize: 28 * minRatio
                            }

                            Text {
                                text: "Victoires"
                                font.pixelSize: Math.min(20 * minRatio, 18)
                                font.bold: true
                                color: "#00ff00"
                                Layout.fillWidth: true
                            }
                        }

                        Item { Layout.fillHeight: true }

                        Text {
                            text: gamesWon.toString()
                            font.pixelSize: Math.min(48 * minRatio, 42)
                            font.bold: true
                            color: "#00ff00"
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
                    Layout.minimumWidth: 250 * widthRatio
                    color: "#2a2a2a"
                    radius: 15 * minRatio
                    border.color: "#FFD700"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 15 * minRatio
                        spacing: 8 * minRatio

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8 * minRatio

                            Text {
                                text: "📈"
                                font.pixelSize: 28 * minRatio
                            }

                            Text {
                                text: "Taux de victoire"
                                font.pixelSize: Math.min(20 * minRatio, 18)
                                font.bold: true
                                color: "#FFD700"
                                Layout.fillWidth: true
                            }
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
            }

            // Section Coinches
            Text {
                text: "🎯 COINCHES"
                font.pixelSize: Math.min(28 * minRatio, 24)
                font.bold: true
                color: "#ff6666"
                Layout.topMargin: 10 * minRatio
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

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Tentatives"
                            font.pixelSize: Math.min(18 * minRatio, 16)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: coincheAttempts.toString()
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#ff6666"
                            Layout.alignment: Qt.AlignHCenter
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
                    border.color: "#00dd00"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Réussies"
                            font.pixelSize: Math.min(18 * minRatio, 16)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: coincheSuccess.toString()
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#00dd00"
                            Layout.alignment: Qt.AlignHCenter
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
                    border.color: "#FFD700"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Taux"
                            font.pixelSize: Math.min(18 * minRatio, 16)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: coincheAttempts > 0 ? ((coincheSuccess / coincheAttempts) * 100).toFixed(1) + "%" : "0%"
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#FFD700"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }

            // Section Annonces Coinchées (coinches subies)
            Text {
                text: "🛡️ ANNONCES COINCHÉES"
                font.pixelSize: Math.min(28 * minRatio, 24)
                font.bold: true
                color: "#9966ff"
                Layout.topMargin: 10 * minRatio
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
                    border.color: "#9966ff"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Subies"
                            font.pixelSize: Math.min(18 * minRatio, 16)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: annoncesCoinchees.toString()
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#9966ff"
                            Layout.alignment: Qt.AlignHCenter
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
                    border.color: "#00dd00"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Gagnées malgré tout"
                            font.pixelSize: Math.min(18 * minRatio, 16)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: annoncesCoincheesgagnees.toString()
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#00dd00"
                            Layout.alignment: Qt.AlignHCenter
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
                    border.color: "#FFD700"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Taux de victoire"
                            font.pixelSize: Math.min(18 * minRatio, 16)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: annoncesCoinchees > 0 ? ((annoncesCoincheesgagnees / annoncesCoinchees) * 100).toFixed(1) + "%" : "0%"
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#FFD700"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }

            // Section Surcoinches
            Text {
                text: "🔥 SURCOINCHES"
                font.pixelSize: Math.min(28 * minRatio, 24)
                font.bold: true
                color: "#ff9900"
                Layout.topMargin: 10 * minRatio
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

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Tentatives"
                            font.pixelSize: Math.min(18 * minRatio, 16)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: surcoincheAttempts.toString()
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#ff9900"
                            Layout.alignment: Qt.AlignHCenter
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
                    border.color: "#00dd00"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Réussies"
                            font.pixelSize: Math.min(18 * minRatio, 16)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: surcoincheSuccess.toString()
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#00dd00"
                            Layout.alignment: Qt.AlignHCenter
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
                    border.color: "#FFD700"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Taux"
                            font.pixelSize: Math.min(18 * minRatio, 16)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: surcoincheAttempts > 0 ? ((surcoincheSuccess / surcoincheAttempts) * 100).toFixed(1) + "%" : "0%"
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#FFD700"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }

            // Section Capots
            Text {
                text: "💥 CAPOTS"
                font.pixelSize: Math.min(28 * minRatio, 24)
                font.bold: true
                color: "#9C27B0"
                Layout.topMargin: 10 * minRatio
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
                    border.color: "#9C27B0"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Réalisés"
                            font.pixelSize: Math.min(18 * minRatio, 16)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: capotRealises.toString()
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#9C27B0"
                            Layout.alignment: Qt.AlignHCenter
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
                    border.color: "#E91E63"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Annoncés tentés"
                            font.pixelSize: Math.min(16 * minRatio, 14)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Text {
                            text: capotAnnoncesTentes.toString()
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#E91E63"
                            Layout.alignment: Qt.AlignHCenter
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
                    border.color: "#00ff88"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Annoncés réussis"
                            font.pixelSize: Math.min(16 * minRatio, 14)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Text {
                            text: capotAnnoncesRealises.toString()
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#00ff88"
                            Layout.alignment: Qt.AlignHCenter
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
                    border.color: "#FFD700"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Taux annoncés"
                            font.pixelSize: Math.min(16 * minRatio, 14)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Text {
                            text: capotAnnoncesTentes > 0 ? ((capotAnnoncesRealises / capotAnnoncesTentes) * 100).toFixed(1) + "%" : "0%"
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#FFD700"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }

            // Section Générales
            Text {
                text: "⭐ GÉNÉRALES"
                font.pixelSize: Math.min(28 * minRatio, 24)
                font.bold: true
                color: "#FF9800"
                Layout.topMargin: 10 * minRatio
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

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Tentatives"
                            font.pixelSize: Math.min(18 * minRatio, 16)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: generaleAttempts.toString()
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#FF9800"
                            Layout.alignment: Qt.AlignHCenter
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
                    border.color: "#00dd00"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Réussies"
                            font.pixelSize: Math.min(18 * minRatio, 16)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: generaleSuccess.toString()
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#00dd00"
                            Layout.alignment: Qt.AlignHCenter
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
                    border.color: "#FFD700"
                    border.width: 2 * minRatio

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12 * minRatio
                        spacing: 5 * minRatio

                        Text {
                            text: "Taux"
                            font.pixelSize: Math.min(18 * minRatio, 16)
                            color: "#aaaaaa"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: generaleAttempts > 0 ? ((generaleSuccess / generaleAttempts) * 100).toFixed(1) + "%" : "0%"
                            font.pixelSize: Math.min(42 * minRatio, 36)
                            font.bold: true
                            color: "#FFD700"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }

            Item { Layout.preferredHeight: 10 * minRatio }

            // Bouton Retour avec effet moderne
            Button {
                Layout.preferredWidth: Math.min(300 * widthRatio, parent.width * 0.8)
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
                    spacing: 10 * minRatio

                    Item { Layout.fillWidth: true }

                    Text {
                        text: "🏠"
                        font.pixelSize: Math.min(28 * minRatio, 24)
                    }

                    Text {
                        text: "RETOUR AU MENU"
                        font.pixelSize: Math.min(24 * minRatio, 20)
                        font.bold: true
                        color: "white"
                        style: Text.Outline
                        styleColor: "#000000"
                    }

                    Item { Layout.fillWidth: true }
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
