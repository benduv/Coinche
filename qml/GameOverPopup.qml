import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: gameOverRoot
    anchors.fill: parent
    color: "#000000CC"  // Fond semi-transparent

    property int winnerTeam: 1
    property int scoreTeam1: 0
    property int scoreTeam2: 0
    property int myPosition: 0  // Position du joueur local (0-3)
    property var playerNames: []  // Tableau des 4 noms de joueurs

    // Ratio responsive
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Calculer si le joueur local a gagné
    property int myTeam: (myPosition % 2 === 0) ? 1 : 2
    property bool iWon: myTeam === winnerTeam

    // Obtenir les noms des joueurs de chaque équipe
    property string team1Player1: playerNames.length > 0 ? playerNames[0] : ""
    property string team1Player2: playerNames.length > 2 ? playerNames[2] : ""
    property string team2Player1: playerNames.length > 1 ? playerNames[1] : ""
    property string team2Player2: playerNames.length > 3 ? playerNames[3] : ""

    signal returnToMenu()

    // Animation d'apparition
    opacity: 0
    NumberAnimation on opacity {
        from: 0
        to: 1
        duration: 300
        easing.type: Easing.OutCubic
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            // Empêche les clics de passer à travers
        }
    }

    // Boîte de dialogue centrale
    Rectangle {
        id: dialogBox
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.75, 750 * widthRatio)
        height: Math.min(parent.height * 0.85, 650 * heightRatio)
        color: "#1a1a1a"
        radius: 20 * minRatio
        border.color: "#FFD700"
        border.width: 4 * minRatio

        // Animation d'entrée (scale)
        scale: 0.7
        NumberAnimation on scale {
            from: 0.7
            to: 1.0
            duration: 400
            easing.type: Easing.OutBack
        }

        // Effet de brillance sur la bordure
        Rectangle {
            anchors.fill: parent
            anchors.margins: -2 * minRatio
            color: "transparent"
            radius: parent.radius
            border.color: "#FFD700"
            border.width: 2 * minRatio
            opacity: 0.3

            SequentialAnimation on opacity {
                loops: Animation.Infinite
                NumberAnimation { from: 0.3; to: 0.8; duration: 1000; easing.type: Easing.InOutQuad }
                NumberAnimation { from: 0.8; to: 0.3; duration: 1000; easing.type: Easing.InOutQuad }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 25 * minRatio
            spacing: 10 * minRatio

            // Icône (Trophée si gagné, Larme si perdu)
            Text {
                text: iWon ? "🏆" : "😢"
                font.pixelSize: 60 * minRatio
                Layout.alignment: Qt.AlignHCenter

                SequentialAnimation on scale {
                    running: iWon
                    loops: Animation.Infinite
                    NumberAnimation { from: 1.0; to: 1.2; duration: 800; easing.type: Easing.InOutQuad }
                    NumberAnimation { from: 1.2; to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
                }
            }

            // Titre
            Text {
                text: iWon ? "VICTOIRE !" : "DÉFAITE"
                font.pixelSize: 42 * minRatio
                font.bold: true
                color: iWon ? "#00ff00" : "#ff6666"
                Layout.alignment: Qt.AlignHCenter

                style: Text.Outline
                styleColor: "#000000"
            }

            // Message de victoire/défaite avec noms des joueurs
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 70 * heightRatio
                color: "transparent"
                radius: 15 * minRatio

                // Gradient de fond
                Rectangle {
                    anchors.fill: parent
                    radius: parent.radius
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: iWon ? "#003300" : "#330000" }
                        GradientStop { position: 1.0; color: "#1a1a1a" }
                    }
                }

                // Bordure animée
                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    radius: parent.radius
                    border.color: iWon ? "#00ff00" : "#ff6666"
                    border.width: 3 * minRatio

                    SequentialAnimation on border.width {
                        running: iWon
                        loops: Animation.Infinite
                        NumberAnimation { from: 3 * minRatio; to: 5 * minRatio; duration: 600 }
                        NumberAnimation { from: 5 * minRatio; to: 3 * minRatio; duration: 600 }
                    }
                }

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 8 * minRatio

                    Text {
                        text: iWon ? "🎉 VOTRE ÉQUIPE A GAGNÉ ! 🎉" : "VOTRE ÉQUIPE A PERDU"
                        font.pixelSize: 28 * minRatio
                        font.bold: true
                        color: iWon ? "#00ff00" : "#ff6666"
                        Layout.alignment: Qt.AlignHCenter

                        style: Text.Outline
                        styleColor: "#000000"
                    }

                    /*Text {
                        text: "👥 " + (myTeam === 1 ? team1Player1 + " et " + team1Player2 : team2Player1 + " et " + team2Player2)
                        font.pixelSize: 20 * minRatio
                        color: "#cccccc"
                        Layout.alignment: Qt.AlignHCenter
                    }*/
                }
            }

            // Scores finaux avec design amélioré
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 160 * heightRatio
                color: "#2a2a2a"
                radius: 15 * minRatio
                border.color: "#FFD700"
                border.width: 2 * minRatio

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15 * minRatio
                    spacing: 5 * minRatio

                    Text {
                        text: "📊 SCORES FINAUX"
                        font.pixelSize: 24 * minRatio
                        font.bold: true
                        color: "#FFD700"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 15 * widthRatio

                        // Score Équipe 1
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: winnerTeam === 1 ? "#002200" : "#220000"
                            radius: 10 * minRatio
                            border.color: winnerTeam === 1 ? "#00ff00" : "#ff6666"
                            border.width: 2 * minRatio

                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 3 * minRatio
                                width: parent.width * 0.9

                                Text {
                                    text: team1Player1 + " & " + team1Player2
                                    font.pixelSize: 16 * minRatio
                                    font.bold: true
                                    color: "#cccccc"
                                    Layout.alignment: Qt.AlignHCenter
                                    wrapMode: Text.WordWrap
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                    maximumLineCount: 2
                                }

                                Text {
                                    text: scoreTeam1.toString()
                                    font.pixelSize: 40 * minRatio
                                    font.bold: true
                                    color: winnerTeam === 1 ? "#00ff00" : "#ff6666"
                                    Layout.alignment: Qt.AlignHCenter

                                    style: Text.Outline
                                    styleColor: "#000000"
                                }
                            }
                        }

                        // Séparateur stylisé
                        Rectangle {
                            width: 3 * minRatio
                            Layout.fillHeight: true
                            radius: 2 * minRatio
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: "#FFD700" }
                                GradientStop { position: 0.5; color: "#FFFFFF" }
                                GradientStop { position: 1.0; color: "#FFD700" }
                            }
                        }

                        // Score Équipe 2
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: winnerTeam === 2 ? "#002200" : "#220000"
                            radius: 10 * minRatio
                            border.color: winnerTeam === 2 ? "#00ff00" : "#ff6666"
                            border.width: 2 * minRatio

                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 3 * minRatio
                                width: parent.width * 0.9

                                Text {
                                    text: team2Player1 + " & " + team2Player2
                                    font.pixelSize: 16 * minRatio
                                    font.bold: true
                                    color: "#cccccc"
                                    Layout.alignment: Qt.AlignHCenter
                                    wrapMode: Text.WordWrap
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                    maximumLineCount: 2
                                }

                                Text {
                                    text: scoreTeam2.toString()
                                    font.pixelSize: 40 * minRatio
                                    font.bold: true
                                    color: winnerTeam === 2 ? "#00ff00" : "#ff6666"
                                    Layout.alignment: Qt.AlignHCenter

                                    style: Text.Outline
                                    styleColor: "#000000"
                                }
                            }
                        }
                    }
                }
            }

            Item { height: 1 * minRatio }

            // Bouton Retour au menu amélioré
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 140 * heightRatio
                Layout.maximumHeight: 140 * heightRatio

                background: Rectangle {
                    color: parent.down ? "#006699" : (parent.hovered ? "#0088cc" : "#0077bb")
                    radius: 12 * minRatio
                    border.color: "#FFD700"
                    border.width: 2 * minRatio

                    // Effet de brillance au survol
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
                    spacing: 8 * minRatio

                    Item { Layout.fillWidth: true }

                    Text {
                        text: "🏠"
                        font.pixelSize: 40 * minRatio
                    }

                    Text {
                        text: "RETOUR AU MENU"
                        font.pixelSize: 40 * minRatio
                        font.bold: true
                        color: "white"

                        style: Text.Outline
                        styleColor: "#000000"
                    }

                    Item { Layout.fillWidth: true }
                }

                onClicked: {
                    returnToMenu()
                }

                // Animation au clic
                scale: 1.0
                Behavior on scale {
                    NumberAnimation { duration: 100 }
                }
                onPressed: scale = 0.95
                onReleased: scale = 1.0
            }
        }
    }

    // Particules de confettis (seulement si victoire)
    Repeater {
        model: iWon ? 30 : 0

        Rectangle {
            width: 8 * minRatio
            height: 8 * minRatio
            radius: 4 * minRatio
            color: Qt.hsla(Math.random(), 0.8, 0.6, 0.8)
            x: Math.random() * gameOverRoot.width
            y: -20

            NumberAnimation on y {
                from: -20
                to: gameOverRoot.height + 20
                duration: 3000 + Math.random() * 2000
                loops: Animation.Infinite
            }

            NumberAnimation on rotation {
                from: 0
                to: 360
                duration: 1000 + Math.random() * 1000
                loops: Animation.Infinite
            }
        }
    }
}
