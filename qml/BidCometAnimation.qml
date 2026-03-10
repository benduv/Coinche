import QtQuick

Item {
    id: cometRoot
    anchors.fill: parent
    visible: false
    z: 100

    // Position cible (centre de l'indicateur d'annonce, en coordonnées du parent)
    property real targetX: 0
    property real targetY: 0

    // Taille de référence
    property real minRatio: Math.min(parent.width, parent.height) / 800

    // Position de départ (coin haut-droit)
    property real startX: parent.width + 30
    property real startY: -30

    // Rotation calculée automatiquement vers la cible
    property real cometRotation: 0

    // État interne
    property bool sparkPhase: false

    function start() {
        sparkPhase = false

        // Départ depuis le coin haut-droit
        startX = parent.width + 30
        startY = -30

        // Calculer l'angle de la traînée (pointe vers la cible)
        var dx = targetX - startX
        var dy = targetY - startY
        cometRotation = Math.atan2(dy, dx) * 180 / Math.PI

        cometHead.x = startX - cometHead.width / 2
        cometHead.y = startY - cometHead.height / 2
        cometHead.opacity = 0
        cometHead.rotation = cometRotation

        visible = true
        flyAnimation.start()
    }

    // Tête de la comète avec traînée
    Item {
        id: cometHead
        width: 140 * cometRoot.minRatio
        height: 6 * cometRoot.minRatio
        opacity: 0

        // Traînée
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 0.3; color: "#FFD70030" }
                GradientStop { position: 0.7; color: "#FFD700aa" }
                GradientStop { position: 1.0; color: "#FFFFFF" }
            }
            radius: height / 2
        }

        // Point lumineux
        Rectangle {
            x: parent.width - width / 2
            y: parent.height / 2 - height / 2
            width: 10 * cometRoot.minRatio
            height: 10 * cometRoot.minRatio
            radius: width / 2
            color: "#FFD700"

            Rectangle {
                anchors.centerIn: parent
                width: parent.width * 2.5
                height: parent.height * 2.5
                radius: width / 2
                color: "#FFD70040"
            }
        }
    }

    // Animation de vol
    SequentialAnimation {
        id: flyAnimation

        ParallelAnimation {
            NumberAnimation {
                target: cometHead; property: "opacity"
                from: 0; to: 1; duration: 60
                easing.type: Easing.OutQuad
            }
            NumberAnimation {
                target: cometHead; property: "x"
                from: cometRoot.startX - cometHead.width / 2
                to: cometRoot.targetX - cometHead.width / 2
                duration: 400
                easing.type: Easing.InCubic
            }
            NumberAnimation {
                target: cometHead; property: "y"
                from: cometRoot.startY - cometHead.height / 2
                to: cometRoot.targetY - cometHead.height / 2
                duration: 400
                easing.type: Easing.InCubic
            }
        }

        // Impact : cacher la comète, lancer les étincelles
        ScriptAction {
            script: {
                cometHead.opacity = 0
                cometRoot.sparkPhase = true
            }
        }

        PauseAnimation { duration: 900 }

        ScriptAction {
            script: {
                cometRoot.sparkPhase = false
                cometRoot.visible = false
            }
        }
    }

    // Flash d'impact
    Rectangle {
        id: impactFlash
        x: cometRoot.targetX - width / 2
        y: cometRoot.targetY - height / 2
        width: 0
        height: width
        radius: width / 2
        color: "#FFD700"
        opacity: 0

        ParallelAnimation {
            running: cometRoot.sparkPhase
            NumberAnimation { target: impactFlash; property: "width"; from: 0; to: 50 * cometRoot.minRatio; duration: 120; easing.type: Easing.OutQuad }
            NumberAnimation { target: impactFlash; property: "opacity"; from: 0.9; to: 0; duration: 250; easing.type: Easing.OutQuad }
        }
    }

    // Étincelles autour de l'indicateur d'annonce
    Repeater {
        model: 14
        Rectangle {
            id: sparkParticle
            property real sparkAngle: (index / 14) * 2 * Math.PI + (Math.random() - 0.5) * 0.5
            // Rayon de départ : les étincelles naissent sur le pourtour de l'indicateur
            property real startRadius: (20 + Math.random() * 15) * cometRoot.minRatio
            property real sparkDist: (35 + Math.random() * 45) * cometRoot.minRatio
            property real sparkSize: (2.5 + Math.random() * 3.5) * cometRoot.minRatio
            // Délai aléatoire pour un effet de crépitement échelonné
            property real sparkDelay: Math.random() * 300

            width: sparkSize
            height: sparkSize
            radius: width / 2
            color: index % 3 === 0 ? "#FFFFFF" : (index % 3 === 1 ? "#FFD700" : "#FFA500")
            x: cometRoot.targetX
            y: cometRoot.targetY
            opacity: 0

            ParallelAnimation {
                running: cometRoot.sparkPhase
                SequentialAnimation {
                    PauseAnimation { duration: sparkParticle.sparkDelay }
                    ParallelAnimation {
                        NumberAnimation {
                            target: sparkParticle; property: "x"
                            from: cometRoot.targetX - sparkParticle.width / 2 + Math.cos(sparkParticle.sparkAngle) * sparkParticle.startRadius
                            to: cometRoot.targetX - sparkParticle.width / 2 + Math.cos(sparkParticle.sparkAngle) * sparkParticle.sparkDist
                            duration: 700; easing.type: Easing.OutCubic
                        }
                        NumberAnimation {
                            target: sparkParticle; property: "y"
                            from: cometRoot.targetY - sparkParticle.height / 2 + Math.sin(sparkParticle.sparkAngle) * sparkParticle.startRadius
                            to: cometRoot.targetY - sparkParticle.height / 2 + Math.sin(sparkParticle.sparkAngle) * sparkParticle.sparkDist
                            duration: 700; easing.type: Easing.OutCubic
                        }
                        SequentialAnimation {
                            NumberAnimation { target: sparkParticle; property: "opacity"; from: 0; to: 1; duration: 60 }
                            NumberAnimation { target: sparkParticle; property: "opacity"; from: 1; to: 0; duration: 640; easing.type: Easing.InQuad }
                        }
                        NumberAnimation {
                            target: sparkParticle; property: "scale"
                            from: 1.3; to: 0.2; duration: 700; easing.type: Easing.OutQuad
                        }
                    }
                }
            }
        }
    }
}
