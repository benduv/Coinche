import QtQuick 2.15

// Deux anneaux concentriques dorés qui s'expansent une seule fois
// quand le joueur doit annoncer. S'utilise à l'intérieur d'un Rectangle avatar :
//   BidWave { active: gameModel.biddingPhase && gameModel.biddingPlayer === idx }
Item {
    id: root

    property bool active: false
    property real waveRadius: 5   // doit correspondre au radius du Rectangle parent

    anchors.fill: parent

    onActiveChanged: {
        if (active) {
            wave1.scale = 1.0; wave1.opacity = 0
            wave2.scale = 1.0; wave2.opacity = 0
            anim1.restart()
            anim2.restart()
        } else {
            anim1.stop()
            anim2.stop()
            wave1.opacity = 0; wave1.scale = 1.0
            wave2.opacity = 0; wave2.scale = 1.0
        }
    }

    // Onde 1
    Rectangle {
        id: wave1
        anchors.centerIn: parent
        width: parent.width; height: parent.height
        radius: root.waveRadius
        color: "transparent"
        border.color: "#FFD700"; border.width: 2
        opacity: 0; scale: 1.0

        SequentialAnimation {
            id: anim1
            ParallelAnimation {
                NumberAnimation { target: wave1; property: "scale";   from: 1.0; to: 1.9; duration: 900; easing.type: Easing.OutCubic }
                NumberAnimation { target: wave1; property: "opacity"; from: 0.55; to: 0;  duration: 900; easing.type: Easing.OutCubic }
            }
        }
    }

    // Onde 2 (décalée de 400 ms)
    Rectangle {
        id: wave2
        anchors.centerIn: parent
        width: parent.width; height: parent.height
        radius: root.waveRadius
        color: "transparent"
        border.color: "#FFD700"; border.width: 2
        opacity: 0; scale: 1.0

        SequentialAnimation {
            id: anim2
            PauseAnimation { duration: 400 }
            ParallelAnimation {
                NumberAnimation { target: wave2; property: "scale";   from: 1.0; to: 1.9; duration: 900; easing.type: Easing.OutCubic }
                NumberAnimation { target: wave2; property: "opacity"; from: 0.55; to: 0;  duration: 900; easing.type: Easing.OutCubic }
            }
        }
    }
}
