import QtQuick

Item {
    id: shootingStar
    width: starWidth
    height: starHeight

    property real startX: 0
    property real startY: 0
    property real minRatio: 1.0
    property real starWidth: 200
    property real starHeight: 4
    property real starRotation: 45

    // Distance de déplacement
    property real travelDistance: 500 * minRatio

    // Calculer le déplacement en X et Y en fonction de l'angle
    // L'angle est en degrés, il faut le convertir en radians
    property real radians: starRotation * Math.PI / 180
    property real deltaX: Math.cos(radians) * travelDistance
    property real deltaY: Math.sin(radians) * travelDistance

    x: startX
    y: startY
    opacity: 0

    // Angle de la trajectoire (aléatoire)
    rotation: starRotation

    function start() {
        shootingAnimation.start()
    }

    // Traînée de l'étoile filante (gradient)
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 0.3; color: "#ffffff40" }
            GradientStop { position: 0.7; color: "#ffffffcc" }
            GradientStop { position: 1.0; color: "white" }
        }
        radius: height / 2
    }

    // Point lumineux à la tête de l'étoile
    Rectangle {
        x: parent.width - width / 2
        y: parent.height / 2 - height / 2
        width: 8 * minRatio
        height: 8 * minRatio
        radius: width / 2
        color: "white"

        // Effet de halo
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 2
            height: parent.height * 2
            radius: width / 2
            color: "transparent"
            border.color: "#ffffff80"
            border.width: 1
        }
    }

    ParallelAnimation {
        id: shootingAnimation

        // Apparition rapide puis disparition
        SequentialAnimation {
            NumberAnimation {
                target: shootingStar
                property: "opacity"
                to: 1.0
                duration: 200
                easing.type: Easing.OutQuad
            }
            PauseAnimation { duration: 500 }
            NumberAnimation {
                target: shootingStar
                property: "opacity"
                to: 0.0
                duration: 300
                easing.type: Easing.InQuad
            }
        }

        // Déplacement selon l'angle de rotation
        NumberAnimation {
            target: shootingStar
            property: "x"
            to: startX + deltaX
            duration: 1000
            easing.type: Easing.Linear
        }

        NumberAnimation {
            target: shootingStar
            property: "y"
            to: startY + deltaY
            duration: 1000
            easing.type: Easing.Linear
        }

        onFinished: {
            // Détruire l'objet après l'animation
            shootingStar.destroy()
        }
    }
}
