import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 1024
    height: 768
    title: "Jeu de Coinche"
    color: "#2d5016"  // Couleur verte de tapis de jeu

    Rectangle {
        anchors.fill: parent
        color: "#2d5016"

        GridLayout {
            anchors.fill: parent
            anchors.margins: 20
            rows: 3
            columns: 3
            rowSpacing: 20
            columnSpacing: 20

            // Espace vide en haut à gauche
            Item { Layout.fillWidth: true; Layout.fillHeight: true }

            // Joueur Nord (Player 2) - en haut
            /*ColumnLayout {
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                Layout.fillWidth: true

                Text {
                    text: "Joueur Nord (Partenaire)"
                    color: "white"
                    font.pixelSize: 16
                    Layout.alignment: Qt.AlignHCenter
                }

                Row {
                    spacing: -40  // Cartes qui se chevauchent
                    Layout.alignment: Qt.AlignHCenter

                    Repeater {
                        model: gameModel.player2Hand
                        Card {
                            value: model.value
                            suit: model.suit
                            faceUp: model.faceUp
                            width: 60
                            height: 90
                            enabled: false
                        }
                    }
                }*/
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                Layout.fillWidth: true

                Text {
                    text: "Joueur Nord (Partenaire)"
                    color: "white"
                    font.pixelSize: 16
                    Layout.alignment: Qt.AlignHCenter
                }

                Row {
                    spacing: 5
                    Layout.alignment: Qt.AlignHCenter

                    Repeater {
                        model: gameModel.player2Hand
                        Card {
                            value: model.value
                            suit: model.suit
                            faceUp: true //model.faceUp
                            isPlayable: model.isPlayable  // Binding avec le modèle
                            width: 80
                            height: 120
                            enabled: gameModel.currentPlayer === 2

                            // Quand on clique sur une carte
                            MouseArea {
                                anchors.fill: parent
                                enabled: gameModel.currentPlayer === 2 && model.isPlayable
                                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor

                                onClicked: {
                                    gameModel.playCard(2, index)
                                }
                            }
                        }
                    }
                }
            }

            // Espace vide en haut à droite
            Item { Layout.fillWidth: true; Layout.fillHeight: true }

            // Joueur Ouest (Player 1) - à gauche
 //           ColumnLayout {
   //             Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
     //           Layout.fillHeight: true
//
  //              Text {
    //                text: "Joueur Ouest"
      //              color: "white"
        //            font.pixelSize: 16
          //      }

            //    Column {
              //      spacing: -60

//                    Repeater {
  //                      model: gameModel.player1Hand
    //                    Card {
      //                      value: model.value
        //                    suit: model.suit
          //                  faceUp: model.faceUp
            //                width: 60
              //              height: 90
                //            enabled: false
                  //          rotation: 90
                    //    }
                    //}
                //}
            //}
            ColumnLayout {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                Layout.fillHeight: true

                Text {
                        text: "Joueur Ouest"
                        color: "white"
                        font.pixelSize: 16
                    }

                Column {
                    spacing: -30
                    //Layout.alignment: Qt.AlignHCenter

                    Repeater {
                        model: gameModel.player1Hand
                        Card {
                            value: model.value
                            suit: model.suit
                            faceUp: true//model.faceUp
                            isPlayable: model.isPlayable  // Binding avec le modèle
                            width: 60
                            height: 90
                            rotation: 90
                            enabled: gameModel.currentPlayer === 1

                            // Quand on clique sur une carte
                            MouseArea {
                                anchors.fill: parent
                                enabled: gameModel.currentPlayer === 1 && model.isPlayable
                                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor

                                onClicked: {
                                    gameModel.playCard(1, index)
                                }
                            }
                        }
                    }
                }
            }

            // Centre - Zone de jeu
            Rectangle {
                id: playArea
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignCenter
                color: "#1a3d0f"
                radius: 10
                border.color: "#8b6914"
                border.width: 3

                //Text {
                  //  anchors.centerIn: parent
                    //text: "Pli en cours\n(à implémenter)"
                    //color: "white"
                    //font.pixelSize: 18
                    //horizontalAlignment: Text.AlignHCenter
                //}

                // Représentation du pli actuel
                Item {
                    id: pliArea
                    anchors.centerIn: parent
                    width: parent.width
                    height: parent.height

                    Repeater {
                        id: pliRepeater
                        model: gameModel.currentPli

                        Card {
                            width: 80
                            height: 120
                            value: modelData.value
                            suit: modelData.suit
                            faceUp: true

                            // Position selon le joueur
                            x: {
                                switch (modelData.playerId) {
                                    case 0: return pliArea.width / 2 - width / 2;     // Sud
                                    case 1: return pliArea.width / 2 - width / 2 - 150; // Ouest
                                    case 2: return pliArea.width / 2 - width / 2;     // Nord
                                    case 3: return pliArea.width / 2 - width / 2 + 150; // Est
                                }
                            }
                            y: {
                                switch (modelData.playerId) {
                                    case 0: return pliArea.height / 2 + 100; // Sud
                                    case 1: return pliArea.height / 2;       // Ouest
                                    case 2: return pliArea.height / 2 - 200; // Nord
                                    case 3: return pliArea.height / 2;       // Est
                                }
                            }

                            Behavior on x { NumberAnimation { duration: 400; easing.type: Easing.InOutQuad } }
                            Behavior on y { NumberAnimation { duration: 400; easing.type: Easing.InOutQuad } }
                            Component.onCompleted: console.log("currentPli =", JSON.stringify(gameModel.currentPli))
                        }
                    }
                }
            }

            // Joueur Est (Player 3) - à droite
            /*ColumnLayout {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                Layout.fillHeight: true

                Text {
                    text: "Joueur Est"
                    color: "white"
                    font.pixelSize: 16
                }

                Column {
                    spacing: -60

                    Repeater {
                        model: gameModel.player3Hand
                        Card {
                            value: model.value
                            suit: model.suit
                            faceUp: model.faceUp
                            width: 60
                            height: 90
                            enabled: false
                            rotation: -90
                        }
                    }
                }
            }*/

            ColumnLayout {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                Layout.fillHeight: true

                Text {
                        text: "Joueur Est"
                        color: "white"
                        font.pixelSize: 16
                    }

                Column {
                    spacing: -30
                    //Layout.alignment: Qt.AlignHCenter

                    Repeater {
                        model: gameModel.player3Hand
                        Card {
                            value: model.value
                            suit: model.suit
                            faceUp: true //model.faceUp
                            isPlayable: model.isPlayable  // Binding avec le modèle
                            width: 60
                            height: 90
                            rotation: 90
                            enabled: gameModel.currentPlayer === 3

                            // Quand on clique sur une carte
                            MouseArea {
                                anchors.fill: parent
                                enabled: gameModel.currentPlayer === 3 && model.isPlayable
                                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor

                                onClicked: {
                                    gameModel.playCard(3, index)
                                }
                            }
                        }
                    }
                }
            }

            // Espace vide en bas à gauche
            Item { Layout.fillWidth: true; Layout.fillHeight: true }

            // Joueur Sud (Player 0) - en bas (VOUS)
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                Layout.fillWidth: true

                Row {
                    spacing: 5
                    Layout.alignment: Qt.AlignHCenter

                    Repeater {
                        model: gameModel.player0Hand
                        Card {
                            value: model.value
                            suit: model.suit
                            faceUp: model.faceUp
                            isPlayable: model.isPlayable  // Binding avec le modèle
                            width: 80
                            height: 120
                            enabled: gameModel.currentPlayer === 0

                            // Quand on clique sur une carte
                            MouseArea {
                                anchors.fill: parent
                                enabled: gameModel.currentPlayer === 0 && model.isPlayable
                                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor

                                onClicked: {
                                    gameModel.playCard(0, index)
                                }
                            }
                        }
                    }
                }

                Text {
                    text: "Vos cartes" + (gameModel.currentPlayer === 0 ? " - À vous de jouer!" : "")
                    color: gameModel.currentPlayer === 0 ? "#ffff00" : "white"
                    font.pixelSize: 18
                    font.bold: gameModel.currentPlayer === 0
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            // Espace vide en bas à droite
            Item { Layout.fillWidth: true; Layout.fillHeight: true }
        }

        // Panneau d'informations
        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 10
            width: 200
            height: 150
            color: "#1a1a1a"
            opacity: 0.8
            radius: 5

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10

                Text {
                    text: "Score"
                    color: "white"
                    font.pixelSize: 18
                    font.bold: true
                }

                Text {
                    text: "Équipe 1: 0"
                    color: "white"
                    font.pixelSize: 14
                }

                Text {
                    text: "Équipe 2: 0"
                    color: "white"
                    font.pixelSize: 14
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "gray"
                }

                Text {
                    text: "Joueur actuel: " + (gameModel.currentPlayer + 1)
                    color: "yellow"
                    font.pixelSize: 14
                }
            }
        }
    }
}
