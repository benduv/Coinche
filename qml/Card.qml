import QtQuick

Item {
    id: root
    width: 80
    height: 120

    // Propriétés de la carte
    property int value: 7  // 7-14 (7,8,9,10,V,D,R,A)
    property int suit: 3   // 3=♥, 4=♦, 5=♣, 6=♠
    property bool faceUp: true
    property bool selected: false
    property bool enabled: true
    property bool isPlayable: true  // Nouvelle propriété

    // Fonction pour construire le chemin de l'image
    function getCardImagePath() {
        var suitName = ""
        switch(suit) {
            case 3: suitName = "coeur"; break
            case 4: suitName = "carreau"; break
            case 5: suitName = "trefle"; break
            case 6: suitName = "pique"; break
            default: suitName = "coeur"
        }

        var valueName = ""
        switch(value) {
            case 7: valueName = "7"; break
            case 8: valueName = "8"; break
            case 9: valueName = "9"; break
            case 10: valueName = "10"; break
            case 11: valueName = "valet"; break
            case 12: valueName = "dame"; break
            case 13: valueName = "roi"; break
            case 14: valueName = "as"; break
            default: valueName = "7"
        }

        // Format: resources/cards/7_hearts.png
        return "qrc:/resources/cards/" + suitName + "_" + valueName + ".png"
    }

    // Animation de sélection
    transform: [
        Scale {
            id: scaleTransform
            origin.x: root.width/2
            origin.y: root.height/2
            xScale: selected ? 1.1 : 1.0
            yScale: selected ? 1.1 : 1.0

            Behavior on xScale { NumberAnimation { duration: 100 } }
            Behavior on yScale { NumberAnimation { duration: 100 } }
        }
    ]

    // Conteneur avec bordure
    Rectangle {
        id: cardBorder
        anchors.fill: parent
        color: "transparent"
        border.color: selected ? "#FFD700" : "#000000"
        border.width: selected ? 3 : 1
        radius: 8

        Behavior on border.color { ColorAnimation { duration: 100 } }
        Behavior on border.width { NumberAnimation { duration: 100 } }

        // Face de la carte (image)
        Image {
            id: cardFace
            anchors.fill: parent
            anchors.margins: 1
            source: root.faceUp ? getCardImagePath() : ""
            visible: root.faceUp
            fillMode: Image.PreserveAspectFit
            smooth: true
            antialiasing: true
        }

        // Dos de la carte (image ou pattern)
        Image {
            id: cardBack
            anchors.fill: parent
            anchors.margins: 1
            source: "qrc:/resources/cards/back.png"
            visible: !root.faceUp
            fillMode: Image.PreserveAspectCrop
            smooth: true
            antialiasing: true

            // Fallback si pas d'image de dos
            Rectangle {
                anchors.fill: parent
                radius: parent.radius - 2
                color: "#000080"
                visible: cardBack.status !== Image.Ready

                Grid {
                    anchors.fill: parent
                    anchors.margins: 5
                    rows: 7
                    columns: 5
                    spacing: 4

                    Repeater {
                        model: 35
                        Rectangle {
                            width: 4
                            height: 4
                            radius: 2
                            color: "#4040FF"
                        }
                    }
                }
            }
        }
    }

    // Zone cliquable
    MouseArea {
        anchors.fill: parent
        enabled: root.enabled
        hoverEnabled: true
        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

        onClicked: root.selected = !root.selected
    }

    // Ombre portée pour effet de sélection
    //DropShadow {
     //   anchors.fill: cardBorder
     //   source: cardBorder
      //  horizontalOffset: 0
        //verticalOffset: selected ? 5 : 2
        //radius: selected ? 12 : 6
        //samples: 17
        //color: "#80000000"
        //visible: true

        //Behavior on verticalOffset { NumberAnimation { duration: 100 } }
      //  Behavior on radius { NumberAnimation { duration: 100 } }
    //}
}
