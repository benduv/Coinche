import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

Rectangle {
    id: rulesRoot

    SoundEffect {
        id: backSound
        source: "qrc:/resources/sons/742832__sadiquecat__woosh-metal-tea-strainer-1.wav"
    }

    SoundEffect {
        id: modeSwitchSound
        source: "qrc:/resources/sons/742832__sadiquecat__woosh-metal-tea-strainer-1.wav"
    }

    anchors.fill: parent
    color: "transparent"

    signal backToMenu()

    // Ratio responsive
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    property int selectedMode: 0  // 0 = Coinche, 1 = Belote

    // Fond étoilé
    StarryBackground {
        anchors.fill: parent
        minRatio: rulesRoot.minRatio
        z: -1
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
                rulesRoot.backToMenu()
            }
        }
    }

    // ScrollView pour les règles
    ScrollView {
        id: rulesScrollView
        anchors.top: parent.top
        anchors.topMargin: 40 * rulesRoot.minRatio
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 80 * rulesRoot.minRatio
        anchors.rightMargin: 80 * rulesRoot.minRatio
        anchors.bottomMargin: 40 * rulesRoot.minRatio
        clip: true

        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        contentWidth: availableWidth

        Column {
            width: rulesScrollView.availableWidth
            spacing: 25 * rulesRoot.minRatio

            // Titre
            Text {
                text: "Règles"
                font.pixelSize: 56 * rulesRoot.minRatio
                font.bold: true
                color: "#FFD700"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // Sélecteur Coinche / Belote
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 20 * rulesRoot.minRatio

                Image {
                    source: "qrc:/resources/left-arrowMainMenu-svgrepo-com.svg"
                    width: 40 * rulesRoot.minRatio
                    height: 40 * rulesRoot.minRatio
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: rulesRoot.selectedMode > 0 ? 1.0 : 0.3
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (rulesRoot.selectedMode > 0) {
                                rulesRoot.selectedMode--
                                if (AudioSettings.effectsEnabled) modeSwitchSound.play()
                            }
                        }
                    }
                }

                Item {
                    width: 180 * rulesRoot.minRatio
                    height: 44 * rulesRoot.minRatio
                    anchors.verticalCenter: parent.verticalCenter
                    Text {
                        anchors.centerIn: parent
                        text: rulesRoot.selectedMode === 0 ? "Coinche" : "Belote"
                        font.pixelSize: 36 * rulesRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
                        style: Text.Outline
                        styleColor: "#000000"
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                Image {
                    source: "qrc:/resources/right-arrowMainMenu-svgrepo-com.svg"
                    width: 40 * rulesRoot.minRatio
                    height: 40 * rulesRoot.minRatio
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: rulesRoot.selectedMode < 1 ? 1.0 : 0.3
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (rulesRoot.selectedMode < 1) {
                                rulesRoot.selectedMode++
                                if (AudioSettings.effectsEnabled) modeSwitchSound.play()
                            }
                        }
                    }
                }
            }

            // ── Règles Coinche ────────────────────────────────────────────
            Column {
                width: parent.width
                spacing: 25 * rulesRoot.minRatio
                visible: rulesRoot.selectedMode === 0

                // But du jeu
                Text {
                    width: parent.width
                    text: "BUT DU JEU"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "La Coinche est un jeu de cartes qui se joue à 4 joueurs répartis en 2 équipes de 2. L'objectif est de réaliser le contrat annoncé lors des enchères en marquant un maximum de points."
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                }

                // Les cartes
                Text {
                    width: parent.width
                    text: "LES CARTES ET LEUR VALEUR"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Le jeu comprend 32 cartes (7, 8, 9, 10, Valet, Dame, Roi, As) dans 4 couleurs (Pique, Cœur, Carreau, Trèfle)."
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Valeur des cartes à l'atout :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "• Valet : 20 points\n• 9 : 14 points\n• As : 11 points\n• 10 : 10 points\n• Roi : 4 points\n• Dame : 3 points\n• 8 : 0 point\n• 7 : 0 point"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                Text {
                    width: parent.width
                    text: "Valeur des cartes hors atout :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "• As : 11 points\n• 10 : 10 points\n• Roi : 4 points\n• Dame : 3 points\n• Valet : 2 points\n• 9, 8, 7 : 0 point"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                // Les enchères
                Text {
                    width: parent.width
                    text: "LES ENCHÈRES"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Chaque joueur, à tour de rôle, peut :\n• Annoncer un contrat (nombre de points qu'il pense réaliser) et choisir l'atout\n• Passer\n\nLes contrats vont de 80 à 160 points par paliers de 10."
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                Text {
                    width: parent.width
                    text: "Annonces spéciales :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "• Coinche : Double les points du contrat adverse si vous pensez qu'ils ne réussiront pas\n\n• Surcoinche : Redouble les points si vous êtes coinché et sûr de réussir\n\n• Capot : Annoncer que votre équipe va remporter tous les plis (250 points)\n\n• Générale : Annoncer que vous allez remporter tous les plis (500 points)\n\n• Tout Atout (TA) : Toutes les couleurs deviennent atout. Ordre des cartes : Valet (14), 9 (9), As (6), 10 (4), Roi (3), Dame (2), 8 (0), 7 (0)\n\n• Sans Atout (SA) : Aucune couleur n'est atout. Ordre des cartes : As (19), 10 (10), Roi (4), Dame (3), Valet (2), 9 (0), 8 (0), 7 (0)"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.4
                }

                // Déroulement d'un pli
                Text {
                    width: parent.width
                    text: "DÉROULEMENT D'UN PLI"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Le joueur à la gauche du Dealer débute la partie."
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Règles obligatoires :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "1. Vous devez fournir la couleur demandée si vous l'avez\n\n2. Si vous n'avez pas la couleur demandée :\n   • Si votre partenaire est maître du pli : vous pouvez jouer ce que vous voulez (défausser)\n   • Si votre partenaire n'est pas maître : vous devez couper avec l'atout si possible\n   • Si un atout a déjà été joué et que vous devez couper : vous devez surmonter si possible\n\n3. Si vous ne pouvez ni fournir ni couper, vous défaussez\n\n4. Le joueur qui pose la carte la plus forte remporte le pli\n\nLe gagnant du pli joue la carte suivante."
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.4
                }

                // Le comptage des points
                Text {
                    width: parent.width
                    text: "COMPTAGE DES POINTS"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "À la fin de la manche :\n• Total des points dans le jeu : 162 points (152 + 10 de dix de der)\n• L'équipe qui remporte le dernier pli gagne 10 points bonus (dix de der)"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                Text {
                    width: parent.width
                    text: "Si le contrat est réussi :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "• L'équipe qui a pris marque le nombre de points annoncés\n• L'équipe adverse marque ses points réalisés"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                Text {
                    width: parent.width
                    text: "Si le contrat échoue :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "• L'équipe qui a pris ne marque rien\n• L'équipe adverse marque 162 points + le contrat annoncé"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                Text {
                    width: parent.width
                    text: "En cas de coinche/surcoinche :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "• Les points du contrat sont multipliés par 2 (coinche) ou 4 (surcoinche)"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                // Coinche à la volée
                Text {
                    width: parent.width
                    text: "COINCHE À LA VOLÉE"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Dans cette version de la Coinche, la coinche à la volée est autorisée :\n\n• N'importe quel joueur de l'équipe adverse peut coincher immédiatement après l'annonce d'un contrat\n\n• Il n'est pas nécessaire d'attendre son tour pour coincher\n\n• Le bouton \"Coinche\" apparaît pendant quelques secondes après chaque annonce adverse, permettant à tout joueur de l'équipe défensive de coincher\n\n• Une fois qu'un joueur a coinché, seule l'équipe attaquante peut surcoincher\n\nCette règle ajoute du dynamisme aux enchères et permet de réagir rapidement à une annonce jugée trop ambitieuse !"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.4
                }

                // Belote et rebelote
                Text {
                    width: parent.width
                    text: "BELOTE ET REBELOTE"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Si vous possédez le Roi et la Dame d'atout, vous pouvez annoncer :\n• \"Belote\" en jouant la première carte\n• \"Rebelote\" en jouant la seconde\n\nCela rapporte 20 points bonus à votre équipe."
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                Text {
                    width: parent.width
                    text: "Important : Si l'équipe qui a pris le contrat possède la belote, ces 20 points comptent comme des points en moins à réaliser dans leur contrat.\n• Par exemple, avec un contrat de 100 points et la belote, l'équipe ne doit réaliser que 80 points pour réussir son contrat.\n• Il n'y a pas de belote en TA et en SA."
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                // Fin de partie
                Text {
                    width: parent.width
                    text: "FIN DE PARTIE"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "La partie se termine lorsqu'une équipe atteint 1000 points ou plus.\nL'équipe avec le plus de points gagne la partie !"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                // Déconnexion et reconnexion
                Text {
                    width: parent.width
                    text: "DÉCONNEXION ET RECONNEXION"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "En cas de déconnexion involontaire :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "• Si vous perdez la connexion (problème réseau, fermeture accidentelle de l'app), votre place est temporairement prise par un bot\n\n• Vous pouvez rejoindre la partie en cours automatiquement en vous reconnectant\n\n• Vos cartes et votre position sont conservées\n\n• Si vous ne revenez pas avant la fin de la partie, une défaite est comptabilisée dans vos statistiques (SAUF en mode Entraînement), quel que soit le résultat de votre équipe"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.4
                }

                Text {
                    width: parent.width
                    text: "En cas d'abandon volontaire (bouton Quitter) :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "• Si vous cliquez sur le bouton \"Quitter\" pendant une partie, vous abandonnez définitivement\n\n• Votre place est remplacée par un bot pour le reste de la partie\n\n• Une défaite est comptabilisée immédiatement dans vos statistiques (SAUF en mode Entraînement)\n\n• Vous ne pourrez pas rejoindre cette partie, mais vous pouvez en commencer une nouvelle"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.4
                }

                Text {
                    width: parent.width
                    text: "Impact sur les statistiques :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "• Victoire : comptabilisée uniquement si vous êtes présent à la fin de la partie quand votre équipe gagne\n\n• Défaite par abandon : comptabilisée immédiatement lorsque vous cliquez sur \"Quitter\" (SAUF en mode Entraînement)\n\n• Déconnexion sans retour : comptabilisée comme une défaite, même si votre équipe gagne grâce au bot qui vous a remplacé (SAUF en mode Entraînement)"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    bottomPadding: 40 * rulesRoot.minRatio
                    lineHeight: 1.4
                }
            }
            // ── Fin règles Coinche ────────────────────────────────────────

            // ── Règles Belote ─────────────────────────────────────────────
            Column {
                width: parent.width
                spacing: 25 * rulesRoot.minRatio
                visible: rulesRoot.selectedMode === 1

                // But du jeu
                Text {
                    width: parent.width
                    text: "BUT DU JEU"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "La Belote est un jeu de cartes qui se joue à 4 joueurs répartis en 2 équipes de 2. L'objectif est de prendre le contrat et de réaliser plus de 81 points sur les 162 disponibles."
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                }

                // Les cartes
                Text {
                    width: parent.width
                    text: "LES CARTES ET LEUR VALEUR"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Le jeu comprend 32 cartes (7, 8, 9, 10, Valet, Dame, Roi, As) dans 4 couleurs (Pique, Cœur, Carreau, Trèfle). Les valeurs sont identiques à la Coinche."
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Valeur des cartes à l'atout :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "• Valet : 20 points\n• 9 : 14 points\n• As : 11 points\n• 10 : 10 points\n• Roi : 4 points\n• Dame : 3 points\n• 8 : 0 point\n• 7 : 0 point"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                Text {
                    width: parent.width
                    text: "Valeur des cartes hors atout :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "• As : 11 points\n• 10 : 10 points\n• Roi : 4 points\n• Dame : 3 points\n• Valet : 2 points\n• 9, 8, 7 : 0 point"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                // Distribution
                Text {
                    width: parent.width
                    text: "DISTRIBUTION DES CARTES"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "• Chaque joueur reçoit 5 cartes\n• Une carte est retournée face visible au centre : c'est la carte proposée pour l'atout\n• Les 11 cartes restantes sont mises en réserve pour la distribution complémentaire"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                // Les enchères
                Text {
                    width: parent.width
                    text: "LES ENCHÈRES"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Tour 1 — La carte retournée :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Chaque joueur à tour de rôle peut :\n• Prendre : accepter la couleur de la carte retournée comme atout\n• Passer : refuser\n\nSi un joueur prend, il reçoit la carte retournée dans sa main. Chaque joueur reçoit ensuite les cartes restantes (le preneur reçoit 2 cartes, les autres en reçoivent 3 chacun) pour compléter sa main à 8 cartes."
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                Text {
                    width: parent.width
                    text: "Tour 2 — Si tout le monde a passé :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Chaque joueur peut à nouveau :\n• Choisir une autre couleur comme atout (autre que la couleur retournée)\n• Passer\n\nSi tout le monde passe à nouveau, les cartes sont redistribuées et le dealer change."
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                // Déroulement d'un pli
                Text {
                    width: parent.width
                    text: "DÉROULEMENT D'UN PLI"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Les règles de jeu sont identiques à la Coinche :\n\n1. Vous devez fournir la couleur demandée si vous l'avez\n\n2. Si vous n'avez pas la couleur demandée :\n   • Si votre partenaire est maître du pli : vous pouvez défausser\n   • Sinon : vous devez couper avec l'atout si possible, et surmonter si un atout est déjà joué\n\n3. Le joueur qui pose la carte la plus forte remporte le pli\n\nLe gagnant du pli joue la carte suivante."
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.4
                }

                // Comptage des points
                Text {
                    width: parent.width
                    text: "COMPTAGE DES POINTS"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Total des points dans le jeu : 162 points (152 + 10 de dix de der)."
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Si le contrat est réussi (preneur > 81 points) :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "• Chaque équipe marque ses points réels (+ 20 si elle détient la Belote)"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                Text {
                    width: parent.width
                    text: "Si le contrat échoue (preneur ≤ 81 points) :"
                    font.pixelSize: 32 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "• L'équipe qui a pris ne marque rien\n• L'équipe qui défend marque 162 points (+ 20 si elle détient la Belote)"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                // Capot
                Text {
                    width: parent.width
                    text: "CAPOT"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Si une équipe remporte tous les plis :\n• Elle marque 250 points (+ 20 si elle détient la Belote)\n• L'équipe adverse ne marque rien"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                // Belote et rebelote
                Text {
                    width: parent.width
                    text: "BELOTE ET REBELOTE"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "Si vous possédez le Roi et la Dame d'atout, vous pouvez annoncer :\n• \"Belote\" en jouant la première carte\n• \"Rebelote\" en jouant la seconde\n\nCela rapporte 20 points bonus à votre équipe, quoi qu'il arrive (même en cas de chute)."
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }

                // Fin de partie
                Text {
                    width: parent.width
                    text: "FIN DE PARTIE"
                    font.pixelSize: 40 * rulesRoot.minRatio
                    font.bold: true
                    color: "#FFD700"
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: "La partie se termine lorsqu'une équipe atteint 500 points ou plus.\nL'équipe avec le plus de points gagne la partie !"
                    font.pixelSize: 28 * rulesRoot.minRatio
                    color: "white"
                    wrapMode: Text.WordWrap
                    bottomPadding: 40 * rulesRoot.minRatio
                    lineHeight: 1.3
                }
            }
            // ── Fin règles Belote ─────────────────────────────────────────
        }
    }
}
