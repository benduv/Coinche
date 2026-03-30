import QtQuick
import QtMultimedia

Item {
    id: ufoAnimation
    anchors.fill: parent
    visible: false
    z: 150

    // Signal pour indiquer la fin de l'animation
    signal animationFinished()

    // Propriétés
    property real minRatio: 1.0

    // Données recap manche
    property int lastBidderIndex: -1
    property int bidValue: 0
    property bool contractSuccess: true
    property int scoreMancheTeam1: 0
    property int scoreMancheTeam2: 0

    // Données calculées pour l'affichage (perspective du joueur local)
    property bool myTeamAttacked: (networkManager.myPosition % 2) === (lastBidderIndex % 2)
    property bool myTeamIsTeam1: (networkManager.myPosition % 2) === 0
    property int myTeamScore: myTeamIsTeam1 ? scoreMancheTeam1 : scoreMancheTeam2
    property int otherTeamScore: myTeamIsTeam1 ? scoreMancheTeam2 : scoreMancheTeam1

    property string recapTitle: {
        if (myTeamAttacked) {
            return contractSuccess ? "Contrat rempli !" : "Vous chutez !"
        } else {
            return contractSuccess ? "Ils remplissent !" : "Ils chutent !"
        }
    }
    property string recapPoints: {
        if (myTeamAttacked) {
            if (contractSuccess) return "Vous marquez " + myTeamScore + " pts"
            else return "Ils marquent " + otherTeamScore + " pts"
        } else {
            if (contractSuccess) return "Ils marquent " + otherTeamScore + " pts"
            else return "Vous marquez " + myTeamScore + " pts"
        }
    }
    property color recapTitleColor: {
        if (myTeamAttacked) {
            return contractSuccess ? "#00ff88" : "#ff4444"
        } else {
            return contractSuccess ? "#ff4444" : "#00ff88"
        }
    }

    // Son UFO
    MediaPlayer {
        id: ufoSound
        source: "qrc:/resources/sons/727968__nomentero__ufo-flying-retro.wav"
        audioOutput: AudioOutput {}
    }

    // Fonction pour démarrer l'animation
    function start() {
        visible = true
        ufoImage.y = -ufoImage.height
        ufoImage.opacity = 1
        beam.opacity = 0
        beam.beamHeight = 0
        recapContainer.opacity = 0
        recapContainer.scale = 0.8
        newMancheTextContainer.opacity = 0
        newMancheTextContainer.scale = 0.5
        if(AudioSettings.effectsEnabled && Qt.application.state === Qt.ApplicationActive) {
            ufoSound.stop()
            ufoSound.play()
        }
        ufoDescendAnimation.start()
    }

    // Fonction pour arrêter l'animation
    function stop() {
        ufoDescendAnimation.stop()
        beamAppearAnimation.stop()
        recapAppearAnimation.stop()
        newMancheAppearAnimation.stop()
        ufoLeaveAnimation.stop()
        ufoSound.stop()
        visible = false
    }

    // Fond semi-transparent
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: 0.3
    }

    // UFO (GIF animé)
    AnimatedImage {
        id: ufoImage
        source: "qrc:/resources/animations/UFO_404.gif"
        width: 500 * ufoAnimation.minRatio
        height: 500 * ufoAnimation.minRatio
        fillMode: Image.PreserveAspectFit
        anchors.horizontalCenter: parent.horizontalCenter
        y: -height
        z: 10
        playing: ufoAnimation.visible
    }

    // Faisceau lumineux en forme de cône sous l'UFO
    Item {
        id: beam
        property real beamHeight: 0
        property real topWidth: 80 * ufoAnimation.minRatio
        property real bottomWidth: 350 * ufoAnimation.minRatio

        x: ufoImage.x + ufoImage.width / 2 - bottomWidth / 2
        y: ufoImage.y + ufoImage.height - 200 * ufoAnimation.minRatio
        width: bottomWidth
        height: beamHeight
        opacity: 0
        z: 5

        Canvas {
            id: beamCanvas
            anchors.fill: parent
            onPaint: {
                var ctx = getContext("2d")
                ctx.reset()
                if (beam.beamHeight <= 0) return
                var centerX = width / 2
                var topHalfWidth = beam.topWidth / 2
                var bottomHalfWidth = beam.bottomWidth / 2
                var gradient = ctx.createLinearGradient(centerX, 0, centerX, height)
                gradient.addColorStop(0, "rgba(0, 255, 255, 0.9)")
                gradient.addColorStop(0.3, "rgba(0, 220, 255, 0.7)")
                gradient.addColorStop(0.7, "rgba(0, 180, 255, 0.4)")
                gradient.addColorStop(1, "rgba(0, 150, 255, 0.1)")
                ctx.beginPath()
                ctx.moveTo(centerX - topHalfWidth, 0)
                ctx.lineTo(centerX + topHalfWidth, 0)
                ctx.lineTo(centerX + bottomHalfWidth, height)
                ctx.lineTo(centerX - bottomHalfWidth, height)
                ctx.closePath()
                ctx.fillStyle = gradient
                ctx.fill()
            }
            Connections {
                target: beam
                function onBeamHeightChanged() { beamCanvas.requestPaint() }
            }
        }

        // Particules dans le faisceau
        Repeater {
            model: 25
            Rectangle {
                id: particle
                property real particleProgress: 0
                property real particleXOffset: Math.random() - 0.5
                property real particleSpeed: 1200 + Math.random() * 800
                property real particleDelay: Math.random() * 600
                property real currentWidth: beam.topWidth + (beam.bottomWidth - beam.topWidth) * particleProgress
                x: beam.width / 2 + particleXOffset * currentWidth - width / 2
                y: particleProgress * (beam.beamHeight - 10)
                width: (3 + Math.random() * 4) * ufoAnimation.minRatio
                height: width
                radius: width / 2
                color: Qt.rgba(0.7 + Math.random() * 0.3, 1, 1, 0.8)
                visible: beam.opacity > 0 && beam.beamHeight > 10
                SequentialAnimation on particleProgress {
                    running: beam.opacity > 0 && beam.beamHeight > 10
                    loops: Animation.Infinite
                    PauseAnimation { duration: particle.particleDelay }
                    NumberAnimation { from: 0; to: 1; duration: particle.particleSpeed }
                    PropertyAction { value: 0 }
                }
                SequentialAnimation on opacity {
                    running: beam.opacity > 0 && beam.beamHeight > 10
                    loops: Animation.Infinite
                    PauseAnimation { duration: particle.particleDelay }
                    NumberAnimation { from: 0; to: 1; duration: 150 }
                    NumberAnimation { from: 1; to: 1; duration: particle.particleSpeed - 300 }
                    NumberAnimation { from: 1; to: 0; duration: 150 }
                }
            }
        }
    }

    // Recap dans le halo de l'UFO
    Item {
        id: recapContainer
        anchors.horizontalCenter: parent.horizontalCenter
        y: ufoImage.y + ufoImage.height + 20 * ufoAnimation.minRatio
        width: parent.width * 0.7
        height: recapColumn.height
        opacity: 0
        scale: 0.8
        z: 20
        transformOrigin: Item.Center
        visible: ufoAnimation.lastBidderIndex >= 0

        Column {
            id: recapColumn
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 12 * ufoAnimation.minRatio

            // Titre : "Contrat rempli!" / "Vous chutez!" etc.
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: ufoAnimation.recapTitle
                font.pixelSize: 52 * ufoAnimation.minRatio
                font.bold: true
                font.family: "Orbitron"
                color: ufoAnimation.recapTitleColor
                style: Text.Outline
                styleColor: Qt.darker(ufoAnimation.recapTitleColor, 2.0)
            }

            // Points
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: ufoAnimation.recapPoints
                font.pixelSize: 38 * ufoAnimation.minRatio
                font.bold: true
                font.family: "Orbitron"
                color: "#FFFFFF"
                style: Text.Outline
                styleColor: "#004466"
            }

            // Contrat annoncé
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Contrat : " + ufoAnimation.bidValue + " pts"
                font.pixelSize: 26 * ufoAnimation.minRatio
                font.family: "Orbitron"
                color: "#88DDFF"
                visible: ufoAnimation.bidValue > 0
            }
        }
    }

    // Texte "Nouvelle Manche"
    Item {
        id: newMancheTextContainer
        x: parent.width / 2 - width / 2
        y: ufoImage.y + ufoImage.height + 80 * ufoAnimation.minRatio
        width: newMancheText.width
        height: newMancheText.height
        opacity: 0
        scale: 0.5
        z: 20
        transformOrigin: Item.Center

        Text {
            id: newMancheGlow
            anchors.centerIn: parent
            text: "NOUVELLE MANCHE"
            font.pixelSize: 73 * ufoAnimation.minRatio
            font.bold: true
            font.family: "Orbitron"
            font.letterSpacing: 2 * ufoAnimation.minRatio
            color: "#00ffff"
            opacity: 0.5
            layer.enabled: true
            layer.smooth: true
        }

        Text {
            id: newMancheText
            anchors.centerIn: parent
            text: "NOUVELLE MANCHE"
            font.pixelSize: 70 * ufoAnimation.minRatio
            font.bold: true
            font.family: "Orbitron"
            font.letterSpacing: 2 * ufoAnimation.minRatio
            color: "#BFFBFF"
            style: Text.Outline
            styleColor: "#087780"
        }
    }

    // Animation 1: L'UFO descend
    SequentialAnimation {
        id: ufoDescendAnimation
        NumberAnimation {
            target: ufoImage
            property: "y"
            from: -ufoImage.height
            to: ufoAnimation.height * 0.085
            duration: 800
            easing.type: Easing.OutCubic
        }
        ScriptAction { script: beamAppearAnimation.start() }
    }

    // Animation 2: Le faisceau apparaît
    SequentialAnimation {
        id: beamAppearAnimation
        ParallelAnimation {
            NumberAnimation { target: beam; property: "opacity"; from: 0; to: 1; duration: 300 }
            NumberAnimation { target: beam; property: "beamHeight"; from: 0; to: 250 * ufoAnimation.minRatio; duration: 500; easing.type: Easing.OutQuad }
        }
        PauseAnimation { duration: 100 }
        ScriptAction {
            script: {
                if (ufoAnimation.lastBidderIndex >= 0)
                    recapAppearAnimation.start()
                else
                    newMancheAppearAnimation.start()
            }
        }
    }

    // Animation 3: Le recap apparaît dans le halo
    SequentialAnimation {
        id: recapAppearAnimation
        ParallelAnimation {
            NumberAnimation { target: recapContainer; property: "opacity"; from: 0; to: 1; duration: 400; easing.type: Easing.OutQuad }
            NumberAnimation { target: recapContainer; property: "scale"; from: 0.8; to: 1.0; duration: 500; easing.type: Easing.OutBack }
        }
        // Pause pour lire le recap
        PauseAnimation { duration: 3200 }
        // Fade out recap
        NumberAnimation { target: recapContainer; property: "opacity"; to: 0; duration: 300 }
        ScriptAction { script: newMancheAppearAnimation.start() }
    }

    // Animation 4: "Nouvelle Manche" apparaît
    SequentialAnimation {
        id: newMancheAppearAnimation
        ParallelAnimation {
            NumberAnimation { target: newMancheTextContainer; property: "opacity"; from: 0; to: 1; duration: 400; easing.type: Easing.OutQuad }
            NumberAnimation { target: newMancheTextContainer; property: "scale"; from: 0.5; to: 1.0; duration: 500; easing.type: Easing.OutBack }
        }
        PauseAnimation { duration: 1500 }
        ScriptAction { script: ufoLeaveAnimation.start() }
    }

    // Animation 5: L'UFO repart
    SequentialAnimation {
        id: ufoLeaveAnimation
        ParallelAnimation {
            NumberAnimation { target: beam; property: "opacity"; to: 0; duration: 300 }
            NumberAnimation { target: beam; property: "beamHeight"; to: 0; duration: 300 }
            NumberAnimation { target: newMancheTextContainer; property: "opacity"; to: 0; duration: 300 }
        }
        NumberAnimation { target: ufoImage; property: "y"; to: -ufoImage.height; duration: 600; easing.type: Easing.InCubic }
        ScriptAction {
            script: {
                ufoAnimation.visible = false
                ufoAnimation.animationFinished()
            }
        }
    }
}
