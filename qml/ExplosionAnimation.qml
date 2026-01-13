import QtQuick

Item {
    id: explosionRoot

    // Propriétés configurables
    property string text: "EXPLOSION !"
    property color mainColor: "#FF8800"
    property color secondaryColor: "#FFAA00"
    property color particleColor1: "#FF4400"
    property color particleColor2: "#FF8800"
    property color particleColor3: "#FFCC00"
    property bool running: false
    property real minRatio: 1.0

    visible: running

    // Cercle d'onde de choc 1
    Rectangle {
        id: shockwave1
        anchors.centerIn: parent
        width: 0
        height: width
        radius: width / 2
        color: "transparent"
        border.color: explosionRoot.mainColor
        border.width: 8
        opacity: 0

        SequentialAnimation {
            running: explosionRoot.running
            loops: 1
            ParallelAnimation {
                NumberAnimation { target: shockwave1; property: "width"; from: 0; to: explosionRoot.width * 1.5; duration: 600; easing.type: Easing.OutQuad }
                NumberAnimation { target: shockwave1; property: "opacity"; from: 1; to: 0; duration: 600; easing.type: Easing.OutQuad }
                NumberAnimation { target: shockwave1; property: "border.width"; from: 15; to: 2; duration: 600; easing.type: Easing.OutQuad }
            }
        }
    }

    // Cercle d'onde de choc 2 (décalé)
    Rectangle {
        id: shockwave2
        anchors.centerIn: parent
        width: 0
        height: width
        radius: width / 2
        color: "transparent"
        border.color: explosionRoot.secondaryColor
        border.width: 6
        opacity: 0

        SequentialAnimation {
            running: explosionRoot.running
            loops: 1
            PauseAnimation { duration: 100 }
            ParallelAnimation {
                NumberAnimation { target: shockwave2; property: "width"; from: 0; to: explosionRoot.width * 1.3; duration: 500; easing.type: Easing.OutQuad }
                NumberAnimation { target: shockwave2; property: "opacity"; from: 0.8; to: 0; duration: 500; easing.type: Easing.OutQuad }
                NumberAnimation { target: shockwave2; property: "border.width"; from: 10; to: 1; duration: 500; easing.type: Easing.OutQuad }
            }
        }
    }

    // Particules d'explosion
    Repeater {
        model: 24
        Rectangle {
            id: explosionParticle
            property real particleAngle: (index / 24) * 2 * Math.PI
            property real particleSpeed: 0.3 + Math.random() * 0.4
            property real particleSize: 8 + Math.random() * 12

            width: particleSize * explosionRoot.minRatio
            height: width
            radius: width / 2
            color: index % 3 === 0 ? explosionRoot.particleColor1 : (index % 3 === 1 ? explosionRoot.particleColor2 : explosionRoot.particleColor3)
            x: explosionRoot.width / 2 - width / 2
            y: explosionRoot.height / 2 - height / 2
            opacity: 0

            ParallelAnimation {
                running: explosionRoot.running
                NumberAnimation {
                    target: explosionParticle
                    property: "x"
                    from: explosionRoot.width / 2 - explosionParticle.width / 2
                    to: explosionRoot.width / 2 - explosionParticle.width / 2 + Math.cos(explosionParticle.particleAngle) * explosionRoot.width * explosionParticle.particleSpeed
                    duration: 700
                    easing.type: Easing.OutQuad
                }
                NumberAnimation {
                    target: explosionParticle
                    property: "y"
                    from: explosionRoot.height / 2 - explosionParticle.height / 2
                    to: explosionRoot.height / 2 - explosionParticle.height / 2 + Math.sin(explosionParticle.particleAngle) * explosionRoot.height * explosionParticle.particleSpeed
                    duration: 700
                    easing.type: Easing.OutQuad
                }
                SequentialAnimation {
                    NumberAnimation { target: explosionParticle; property: "opacity"; from: 0; to: 1; duration: 100 }
                    NumberAnimation { target: explosionParticle; property: "opacity"; from: 1; to: 0; duration: 600; easing.type: Easing.InQuad }
                }
                NumberAnimation {
                    target: explosionParticle
                    property: "scale"
                    from: 1.5
                    to: 0.3
                    duration: 700
                    easing.type: Easing.OutQuad
                }
            }
        }
    }

    // Étincelles secondaires
    Repeater {
        model: 16
        Rectangle {
            id: sparkParticle
            property real sparkAngle: (index / 16) * 2 * Math.PI + Math.random() * 0.5
            property real sparkSpeed: 0.15 + Math.random() * 0.25

            width: (4 + Math.random() * 6) * explosionRoot.minRatio
            height: width
            radius: width / 2
            color: "#FFFFFF"
            x: explosionRoot.width / 2 - width / 2
            y: explosionRoot.height / 2 - height / 2
            opacity: 0

            ParallelAnimation {
                running: explosionRoot.running
                NumberAnimation {
                    target: sparkParticle
                    property: "x"
                    from: explosionRoot.width / 2 - sparkParticle.width / 2
                    to: explosionRoot.width / 2 - sparkParticle.width / 2 + Math.cos(sparkParticle.sparkAngle) * explosionRoot.width * sparkParticle.sparkSpeed
                    duration: 500
                    easing.type: Easing.OutQuad
                }
                NumberAnimation {
                    target: sparkParticle
                    property: "y"
                    from: explosionRoot.height / 2 - sparkParticle.height / 2
                    to: explosionRoot.height / 2 - sparkParticle.height / 2 + Math.sin(sparkParticle.sparkAngle) * explosionRoot.height * sparkParticle.sparkSpeed
                    duration: 500
                    easing.type: Easing.OutQuad
                }
                SequentialAnimation {
                    PauseAnimation { duration: 50 }
                    NumberAnimation { target: sparkParticle; property: "opacity"; from: 0; to: 1; duration: 80 }
                    NumberAnimation { target: sparkParticle; property: "opacity"; from: 1; to: 0; duration: 370; easing.type: Easing.InQuad }
                }
            }
        }
    }

    // Flash central
    Rectangle {
        id: centralFlash
        anchors.centerIn: parent
        width: 0
        height: width
        radius: width / 2
        color: "#FFFFFF"
        opacity: 0

        SequentialAnimation {
            running: explosionRoot.running
            ParallelAnimation {
                NumberAnimation { target: centralFlash; property: "width"; from: 0; to: explosionRoot.height * 0.8; duration: 150; easing.type: Easing.OutQuad }
                NumberAnimation { target: centralFlash; property: "opacity"; from: 1; to: 0; duration: 300; easing.type: Easing.OutQuad }
            }
        }
    }

    // Texte avec effet d'impact
    Text {
        id: explosionText
        anchors.centerIn: parent
        text: explosionRoot.text
        font.pixelSize: parent.height * 0.28
        font.bold: true
        color: explosionRoot.mainColor
        style: Text.Outline
        styleColor: "#000000"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        scale: 0
        opacity: 0

        // Ombre portée
        Text {
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: 4
            anchors.verticalCenterOffset: 4
            text: parent.text
            font: parent.font
            color: "#40000000"
            z: -1
        }

        // Animation d'apparition explosive
        ParallelAnimation {
            running: explosionRoot.running
            SequentialAnimation {
                NumberAnimation { target: explosionText; property: "scale"; from: 3; to: 0.9; duration: 200; easing.type: Easing.OutQuad }
                NumberAnimation { target: explosionText; property: "scale"; from: 0.9; to: 1.1; duration: 150; easing.type: Easing.OutQuad }
                NumberAnimation { target: explosionText; property: "scale"; from: 1.1; to: 1; duration: 100; easing.type: Easing.InOutQuad }
            }
            NumberAnimation { target: explosionText; property: "opacity"; from: 0; to: 1; duration: 150 }
        }

        // Légère pulsation continue après l'apparition
        SequentialAnimation on scale {
            running: explosionRoot.running && explosionText.opacity > 0.9
            loops: Animation.Infinite
            NumberAnimation { from: 1; to: 1.05; duration: 400; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 1.05; to: 1; duration: 400; easing.type: Easing.InOutQuad }
        }
    }

    // Lueur derrière le texte
    Rectangle {
        id: textGlow
        anchors.centerIn: parent
        width: explosionText.paintedWidth * 1.4
        height: explosionText.paintedHeight * 1.6
        radius: 20
        color: explosionRoot.mainColor
        opacity: 0
        z: -1

        SequentialAnimation {
            running: explosionRoot.running
            PauseAnimation { duration: 100 }
            NumberAnimation { target: textGlow; property: "opacity"; from: 0; to: 0.3; duration: 200 }
            SequentialAnimation {
                loops: Animation.Infinite
                NumberAnimation { target: textGlow; property: "opacity"; from: 0.3; to: 0.15; duration: 500; easing.type: Easing.InOutQuad }
                NumberAnimation { target: textGlow; property: "opacity"; from: 0.15; to: 0.3; duration: 500; easing.type: Easing.InOutQuad }
            }
        }
    }
}
