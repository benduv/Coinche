import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#000000"
    opacity: 0.95
    radius: 10
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 20
        
        // Titre
        Text {
            text: "Phase d'annonces"
            font.pixelSize: 32
            font.bold: true
            color: "#FFD700"
            Layout.alignment: Qt.AlignHCenter
        }
        
        // Annonce actuelle
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 100
            color: "#1a1a1a"
            radius: 8
            border.color: "#FFD700"
            border.width: 2
            
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 5
                
                Text {
                    text: "Dernière annonce"
                    font.pixelSize: 16
                    color: "#aaaaaa"
                    Layout.alignment: Qt.AlignHCenter
                }
                
                Text {
                    text: gameModel.lastBid
                    font.pixelSize: 28
                    font.bold: true
                    color: "#FFD700"
                    Layout.alignment: Qt.AlignHCenter
                }
                
                Text {
                    text: gameModel.lastBidSuit
                    font.pixelSize: 20
                    color: "#ffffff"
                    Layout.alignment: Qt.AlignHCenter
                    visible: gameModel.lastBidValue > 0
                }
            }
        }
        
        // Tour du joueur
        Text {
            text: gameModel.biddingPlayer === 0 ? 
                  "À vous d'annoncer !" : 
                  "Joueur " + (gameModel.biddingPlayer + 1) + " annonce..."
            font.pixelSize: 20
            color: gameModel.biddingPlayer === 0 ? "#00ff00" : "#ffffff"
            Layout.alignment: Qt.AlignHCenter
        }
        
        Item { Layout.fillHeight: true }
        
        // Choix de l'annonce (visible seulement pour le joueur humain)
        /*ColumnLayout {
            Layout.fillWidth: true
            //anchors.horizontalCenter: parent
            //: 15
            visible: gameModel.biddingPlayer === 0*/
            
            Text {
                text: "Choisissez votre annonce :"
                font.pixelSize: 18
                color: "#ffffff"
                Layout.alignment: Qt.AlignHCenter
            }
            
            // Grille des annonces
            RowLayout {
                //Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                //columns: 4
                //spacing: 0
                //columnSpacing: 10
                
                // 80-100
                BidButton {
                    text: "80"
                    bidValue: 1
                    enabled: 1 > gameModel.lastBidValue
                    onClicked: showSuitSelector(1)
                }
                BidButton {
                    text: "90"
                    bidValue: 2
                    enabled: 2 > gameModel.lastBidValue
                    onClicked: showSuitSelector(2)
                }
                BidButton {
                    text: "100"
                    bidValue: 3
                    enabled: 3 > gameModel.lastBidValue
                    onClicked: showSuitSelector(3)
                }
                BidButton {
                    text: "110"
                    bidValue: 4
                    enabled: 4 > gameModel.lastBidValue
                    onClicked: showSuitSelector(4)
                }
                
                // 120-150
                BidButton {
                    text: "120"
                    bidValue: 5
                    enabled: 5 > gameModel.lastBidValue
                    onClicked: showSuitSelector(5)
                }
                BidButton {
                    text: "130"
                    bidValue: 6
                    enabled: 6 > gameModel.lastBidValue
                    onClicked: showSuitSelector(6)
                }
                BidButton {
                    text: "140"
                    bidValue: 7
                    enabled: 7 > gameModel.lastBidValue
                    onClicked: showSuitSelector(7)
                }
                BidButton {
                    text: "150"
                    bidValue: 8
                    enabled: 8 > gameModel.lastBidValue
                    onClicked: showSuitSelector(8)
                }
                
                // 160, Capot, Générale
                BidButton {
                    text: "160"
                    bidValue: 9
                    enabled: 9 > gameModel.lastBidValue
                    onClicked: showSuitSelector(9)
                }
                BidButton {
                    text: "Capot"
                    bidValue: 10
                    enabled: 10 > gameModel.lastBidValue
                    onClicked: showSuitSelector(10)
                }
                BidButton {
                    text: "Générale"
                    bidValue: 11
                    enabled: 11 > gameModel.lastBidValue
                    onClicked: showSuitSelector(11)
                }
            //}
            

        }

        // Bouton Passer
        Button {
            text: "Passer"
            font.pixelSize: 18
            Layout.preferredWidth: 200
            Layout.preferredHeight: 50
            Layout.alignment: Qt.AlignHCenter

            background: Rectangle {
                color: parent.down ? "#cc0000" : (parent.hovered ? "#ff3333" : "#ff0000")
                radius: 5
            }

            contentItem: Text {
                text: parent.text
                font: parent.font
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: gameModel.passBid()
        }
        
        Item { Layout.fillHeight: true }
    }
    
    // Popup pour choisir la couleur
    Popup {
        id: suitSelector
        anchors.centerIn: parent
        width: 400
        height: 250
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape
        
        property int selectedBidValue: 0
        
        background: Rectangle {
            color: "#2a2a2a"
            radius: 10
            border.color: "#FFD700"
            border.width: 2
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15
            
            Text {
                text: "Choisissez la couleur d'atout :"
                font.pixelSize: 20
                color: "#ffffff"
                Layout.alignment: Qt.AlignHCenter
            }
            
            Row {
                Layout.alignment: Qt.AlignHCenter
                spacing: 15
                
                SuitButton {
                    text: "♥"
                    suitColor: "#E60000"
                    suitValue: 3
                    onClicked: {
                        gameModel.makeBid(suitSelector.selectedBidValue, 3)
                        suitSelector.close()
                    }
                }
                
                SuitButton {
                    text: "♦"
                    suitColor: "#E60000"
                    suitValue: 4
                    onClicked: {
                        gameModel.makeBid(suitSelector.selectedBidValue, 4)
                        suitSelector.close()
                    }
                }
                
                SuitButton {
                    text: "♣"
                    suitColor: "#000000"
                    suitValue: 5
                    onClicked: {
                        gameModel.makeBid(suitSelector.selectedBidValue, 5)
                        suitSelector.close()
                    }
                }
                
                SuitButton {
                    text: "♠"
                    suitColor: "#000000"
                    suitValue: 6
                    onClicked: {
                        gameModel.makeBid(suitSelector.selectedBidValue, 6)
                        suitSelector.close()
                    }
                }
            }
        }
    }
    
    function showSuitSelector(bidValue) {
        suitSelector.selectedBidValue = bidValue
        suitSelector.open()
    }
    
    // Composant pour les boutons d'annonce
    component BidButton: Button {
        property int bidValue
        
        /*Layout.preferredWidth: 40
        Layout.preferredHeight: 40*/
        
        background: Rectangle {
            color: parent.enabled ? 
                   (parent.down ? "#0066cc" : (parent.hovered ? "#0080ff" : "#0099ff")) : 
                   "#333333"
            radius: 5
            border.color: parent.enabled ? "#FFD700" : "#555555"
            border.width: 1
        }
        
        contentItem: Text {
            text: parent.text
            font.pixelSize: 16
            font.bold: true
            color: parent.enabled ? "white" : "#666666"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        //Component.onCompleted: console.log("Layout.preferredWidth = " + Layout.preferredWidth)
    }
    
    // Composant pour les boutons de couleur
    component SuitButton: Button {
        property color suitColor
        property int suitValue
        
        width: 80
        height: 80
        
        background: Rectangle {
            color: parent.down ? "#444444" : (parent.hovered ? "#555555" : "#333333")
            radius: 10
            border.color: suitColor
            border.width: 3
        }
        
        contentItem: Text {
            text: parent.text
            font.pixelSize: 48
            color: suitColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
