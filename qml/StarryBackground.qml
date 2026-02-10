import QtQuick

Item {
    id: starryBackground
    anchors.fill: parent

    // Propriétés configurables
    property color backgroundColor: "#0a0a2e"
    property int starCount: 80
    property real minRatio: 1.0

    // Fond coloré
    Rectangle {
        anchors.fill: parent
        color: starryBackground.backgroundColor
        z: -1
    }

    Rectangle { // Solution du pauvre pour mettre en noire la bar du haut de l'ecran (selfie)
        anchors.left: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width / 5
        height: parent.height
        color: "black"
        z: 100
    }

    Rectangle {
        anchors.right: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width / 5
        height: parent.height
        color: "black"
        z: 100
    }

    // Étoiles scintillantes
    Repeater {
        model: starryBackground.starCount
        Rectangle {
            property real starX: Math.random()
            property real starY: Math.random()
            property real starSize: 1 + Math.random() * 2
            property real starDelay: Math.random() * 2000

            x: starX * starryBackground.width
            y: starY * starryBackground.height
            width: starSize * starryBackground.minRatio
            height: width
            radius: width / 2
            color: "white"
            opacity: 0.3
            z: 0

            SequentialAnimation on opacity {
                running: true
                loops: Animation.Infinite
                PauseAnimation { duration: starDelay }
                NumberAnimation { to: 0.8; duration: 1000 + Math.random() * 1000 }
                NumberAnimation { to: 0.3; duration: 1000 + Math.random() * 1000 }
            }
        }
    }
}
