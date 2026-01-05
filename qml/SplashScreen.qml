import QtQuick
import QtQuick.Controls

Rectangle {
    id: splashRoot
    anchors.fill: parent
    color: "#000000"  // Fond noir pour style rétro

    signal loadingComplete()

    // Propriété pour suivre la progression (0.0 à 1.0)
    property real progress: 0.0

    // Ratio responsive
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Animation de fond style Matrix/rétro
    Repeater {
        model: 15
        Column {
            x: (index * splashRoot.width / 15)
            y: -splashRoot.height
            spacing: 20
            opacity: 0.15

            Repeater {
                model: 20
                Text {
                    text: ["0", "1", "♠", "♥", "♦", "♣"][Math.floor(Math.random() * 6)]
                    font.pixelSize: 20 * splashRoot.minRatio
                    font.family: "Courier New"
                    color: "#00ff00"
                }
            }

            SequentialAnimation on y {
                running: true
                loops: Animation.Infinite
                NumberAnimation {
                    from: -splashRoot.height
                    to: splashRoot.height * 1.5
                    duration: 8000 + (index * 500)
                }
            }
        }
    }

    // Contenu principal
    Column {
        anchors.centerIn: parent
        spacing: 60 * splashRoot.minRatio
        width: parent.width * 0.8

        // Nom du studio avec effet pixel/rétro
        Item {
            width: parent.width
            height: studioText.height
            anchors.horizontalCenter: parent.horizontalCenter

            // Ombre pixelisée
            Text {
                id: studioTextShadow
                text: "NEBULUDIK"
                font.pixelSize: 72 * splashRoot.minRatio
                font.family: "Courier New"
                font.bold: true
                font.letterSpacing: 8
                color: "#003300"
                anchors.centerIn: parent
                anchors.horizontalCenterOffset: 4 * splashRoot.minRatio
                anchors.verticalCenterOffset: 4 * splashRoot.minRatio
            }

            // Texte principal
            Text {
                id: studioText
                text: "NEBULUDIK"
                font.pixelSize: 72 * splashRoot.minRatio
                font.family: "Courier New"
                font.bold: true
                font.letterSpacing: 8
                color: "#00ff00"
                anchors.centerIn: parent

                // Effet de clignotement
                SequentialAnimation on opacity {
                    running: true
                    loops: Animation.Infinite
                    NumberAnimation { to: 1.0; duration: 800 }
                    NumberAnimation { to: 0.7; duration: 800 }
                }
            }

            // Effet de scan line rétro
            Rectangle {
                width: parent.width
                height: 2
                color: "#00ff00"
                opacity: 0.5
                anchors.centerIn: parent

                SequentialAnimation on y {
                    running: true
                    loops: Animation.Infinite
                    NumberAnimation {
                        from: -studioText.height / 2
                        to: studioText.height / 2
                        duration: 2000
                    }
                }
            }
        }

        // Texte "Studio"
        Text {
            text: "STUDIO"
            font.pixelSize: 24 * splashRoot.minRatio
            font.family: "Courier New"
            font.letterSpacing: 4
            color: "#00cc00"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // Barre de chargement rétro
        Column {
            width: parent.width
            spacing: 15 * splashRoot.minRatio
            anchors.horizontalCenter: parent.horizontalCenter

            // Texte "LOADING"
            Text {
                text: "LOADING..."
                font.pixelSize: 20 * splashRoot.minRatio
                font.family: "Courier New"
                font.letterSpacing: 2
                color: "#00ff00"
                anchors.horizontalCenter: parent.horizontalCenter

                // Animation de points
                SequentialAnimation on text {
                    running: true
                    loops: Animation.Infinite
                    PropertyAction { value: "LOADING" }
                    PauseAnimation { duration: 300 }
                    PropertyAction { value: "LOADING." }
                    PauseAnimation { duration: 300 }
                    PropertyAction { value: "LOADING.." }
                    PauseAnimation { duration: 300 }
                    PropertyAction { value: "LOADING..." }
                    PauseAnimation { duration: 300 }
                }
            }

            // Barre de progression style rétro
            Item {
                width: parent.width * 0.7
                height: 40 * splashRoot.minRatio
                anchors.horizontalCenter: parent.horizontalCenter

                // Cadre externe (style pixel art)
                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    border.color: "#00ff00"
                    border.width: 3 * splashRoot.minRatio
                }

                // Grille de fond (effet CRT)
                Grid {
                    anchors.fill: parent
                    anchors.margins: 5 * splashRoot.minRatio
                    columns: 20
                    rows: 1
                    spacing: 2 * splashRoot.minRatio

                    Repeater {
                        model: 20
                        Rectangle {
                            width: (parent.width - (19 * parent.spacing)) / 20
                            height: parent.height
                            color: index < (splashRoot.progress * 20) ? "#00ff00" : "#003300"
                            opacity: index < (splashRoot.progress * 20) ? 1.0 : 0.3

                            // Animation de remplissage
                            Behavior on color {
                                ColorAnimation { duration: 200 }
                            }
                        }
                    }
                }

                // Effet de brillance sur la barre
                Rectangle {
                    width: parent.width * splashRoot.progress
                    height: parent.height
                    anchors.left: parent.left
                    color: "transparent"
                    clip: true

                    Rectangle {
                        width: 30 * splashRoot.minRatio
                        height: parent.parent.height
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "transparent" }
                            GradientStop { position: 0.5; color: "#00ff0040" }
                            GradientStop { position: 1.0; color: "transparent" }
                        }

                        SequentialAnimation on x {
                            running: splashRoot.progress < 1.0
                            loops: Animation.Infinite
                            NumberAnimation {
                                from: -30 * splashRoot.minRatio
                                to: splashRoot.width * 0.7
                                duration: 1500
                            }
                            PauseAnimation { duration: 500 }
                        }
                    }
                }
            }

            // Pourcentage
            Text {
                text: Math.floor(splashRoot.progress * 100) + "%"
                font.pixelSize: 24 * splashRoot.minRatio
                font.family: "Courier New"
                font.bold: true
                font.letterSpacing: 2
                color: "#00ff00"
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }

    // Animation de progression automatique
    SequentialAnimation {
        id: progressAnimation
        running: true

        // Simulation du chargement
        NumberAnimation {
            target: splashRoot
            property: "progress"
            from: 0.0
            to: 1.0
            duration: 3000
            easing.type: Easing.InOutQuad
        }

        // Attendre un peu avant de terminer
        PauseAnimation { duration: 500 }

        // Déclencher la fin du chargement
        ScriptAction {
            script: splashRoot.loadingComplete()
        }
    }

    // Coin inférieur droit - copyright
    Text {
        text: "© 2026 NEBULUDIK"
        font.pixelSize: 14 * splashRoot.minRatio
        font.family: "Courier New"
        color: "#006600"
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20 * splashRoot.minRatio
    }
}
