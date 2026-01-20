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
        textAppearAnimation.stop()
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
        y: -height  // Commence hors écran en haut
        z: 10

        // Le GIF est toujours animé quand visible
        playing: ufoAnimation.visible
    }

    // Faisceau lumineux en forme de cône sous l'UFO
    Item {
        id: beam
        property real beamHeight: 0
        property real topWidth: 80 * ufoAnimation.minRatio    // Largeur en haut (sous l'UFO)
        property real bottomWidth: 350 * ufoAnimation.minRatio // Largeur en bas (plus large)

        x: ufoImage.x + ufoImage.width / 2 - bottomWidth / 2
        y: ufoImage.y + ufoImage.height - 200 * ufoAnimation.minRatio
        width: bottomWidth
        height: beamHeight
        opacity: 0
        z: 5

        // Canvas pour dessiner le cône avec gradient
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

                // Créer un gradient linéaire du haut vers le bas
                var gradient = ctx.createLinearGradient(centerX, 0, centerX, height)
                gradient.addColorStop(0, "rgba(0, 255, 255, 0.9)")    // Cyan vif en haut
                gradient.addColorStop(0.3, "rgba(0, 220, 255, 0.7)")
                gradient.addColorStop(0.7, "rgba(0, 180, 255, 0.4)")
                gradient.addColorStop(1, "rgba(0, 150, 255, 0.1)")    // Presque transparent en bas

                // Dessiner le trapèze (cône)
                ctx.beginPath()
                ctx.moveTo(centerX - topHalfWidth, 0)           // Haut gauche
                ctx.lineTo(centerX + topHalfWidth, 0)           // Haut droite
                ctx.lineTo(centerX + bottomHalfWidth, height)   // Bas droite
                ctx.lineTo(centerX - bottomHalfWidth, height)   // Bas gauche
                ctx.closePath()

                ctx.fillStyle = gradient
                ctx.fill()
            }

            // Redessiner quand la hauteur change
            Connections {
                target: beam
                function onBeamHeightChanged() {
                    beamCanvas.requestPaint()
                }
            }
        }

        // Particules dans le faisceau (adaptées au cône)
        Repeater {
            model: 25
            Rectangle {
                id: particle
                property real particleProgress: 0  // 0 = haut, 1 = bas
                property real particleXOffset: Math.random() - 0.5  // -0.5 à 0.5
                property real particleSpeed: 1200 + Math.random() * 800
                property real particleDelay: Math.random() * 600

                // Calcul de la position X en fonction de la progression (suit le cône)
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
                    NumberAnimation {
                        from: 0
                        to: 1
                        duration: particle.particleSpeed
                    }
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

    // Texte "Nouvelle Manche" avec style futuriste
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

        // Effet de glow derrière le texte
        Text {
            id: newMancheGlow
            anchors.centerIn: parent
            text: "NOUVELLE MANCHE"
            font.pixelSize: 73 * ufoAnimation.minRatio
            font.bold: true
            font.family: "Consolas, Courier New, monospace"
            font.letterSpacing: 2 * ufoAnimation.minRatio
            color: "#00ffff"
            opacity: 0.5

            // Légèrement flou pour effet glow
            layer.enabled: true
            layer.smooth: true
        }

        // Texte principal
        Text {
            id: newMancheText
            anchors.centerIn: parent
            text: "NOUVELLE MANCHE"
            font.pixelSize: 70 * ufoAnimation.minRatio
            font.bold: true
            font.family: "Consolas, Courier New, monospace"
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

        ScriptAction {
            script: beamAppearAnimation.start()
        }
    }

    // Animation 2: Le faisceau apparaît
    SequentialAnimation {
        id: beamAppearAnimation

        ParallelAnimation {
            NumberAnimation {
                target: beam
                property: "opacity"
                from: 0
                to: 1
                duration: 300
            }
            NumberAnimation {
                target: beam
                property: "beamHeight"
                from: 0
                to: 250 * ufoAnimation.minRatio
                duration: 500
                easing.type: Easing.OutQuad
            }
        }

        PauseAnimation { duration: 100 }

        ScriptAction {
            script: textAppearAnimation.start()
        }
    }

    // Animation 3: Le texte apparaît
    SequentialAnimation {
        id: textAppearAnimation

        ParallelAnimation {
            NumberAnimation {
                target: newMancheTextContainer
                property: "opacity"
                from: 0
                to: 1
                duration: 400
                easing.type: Easing.OutQuad
            }
            NumberAnimation {
                target: newMancheTextContainer
                property: "scale"
                from: 0.5
                to: 1.0
                duration: 500
                easing.type: Easing.OutBack
            }
        }

        // Pause pour laisser le temps de lire
        PauseAnimation { duration: 1700 }

        ScriptAction {
            script: ufoLeaveAnimation.start()
        }
    }

    // Animation 4: L'UFO repart
    SequentialAnimation {
        id: ufoLeaveAnimation

        // Le faisceau et le texte disparaissent
        ParallelAnimation {
            NumberAnimation {
                target: beam
                property: "opacity"
                to: 0
                duration: 300
            }
            NumberAnimation {
                target: beam
                property: "beamHeight"
                to: 0
                duration: 300
            }
            NumberAnimation {
                target: newMancheTextContainer
                property: "opacity"
                to: 0
                duration: 300
            }
        }

        // L'UFO remonte et sort de l'écran
        NumberAnimation {
            target: ufoImage
            property: "y"
            to: -ufoImage.height
            duration: 600
            easing.type: Easing.InCubic
        }

        ScriptAction {
            script: {
                ufoAnimation.visible = false
                ufoAnimation.animationFinished()
            }
        }
    }
}
