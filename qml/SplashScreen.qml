import QtQuick
import QtQuick.Controls

Rectangle {
    id: splashRoot
    anchors.fill: parent

    // Dégradé spatial pour le fond
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#0a0a2e" }  // Bleu très foncé
        GradientStop { position: 0.5; color: "#16213e" }  // Bleu-gris
        GradientStop { position: 1.0; color: "#1a1a3e" }  // Bleu nuit
    }

    signal loadingComplete()

    // Propriété pour suivre la progression (0.0 à 1.0)
    property real progress: 0.0

    // Ratio responsive
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Étoiles scintillantes en arrière-plan
    Repeater {
        model: 80
        Rectangle {
            x: Math.random() * splashRoot.width
            y: Math.random() * splashRoot.height
            width: (Math.random() * 2 + 1) * splashRoot.minRatio
            height: width
            radius: width / 2
            color: "white"
            opacity: 0.3

            SequentialAnimation on opacity {
                running: true
                loops: Animation.Infinite
                PauseAnimation { duration: Math.random() * 2000 }
                NumberAnimation { to: 0.8; duration: 1000 + Math.random() * 1000 }
                NumberAnimation { to: 0.3; duration: 1000 + Math.random() * 1000 }
            }
        }
    }

    // Effet de nébuleuse (cercles colorés avec flou)
    Repeater {
        model: 5
        Rectangle {
            x: Math.random() * splashRoot.width
            y: Math.random() * splashRoot.height
            width: (100 + Math.random() * 200) * splashRoot.minRatio
            height: width
            radius: width / 2
            color: {
                var colors = ["#4a148c80", "#1a237e80", "#0d47a180", "#01579b80", "#4a00e080"]
                return colors[index]
            }
            opacity: 0.15

            SequentialAnimation on opacity {
                running: true
                loops: Animation.Infinite
                NumberAnimation { to: 0.3; duration: 3000 + (index * 500) }
                NumberAnimation { to: 0.1; duration: 3000 + (index * 500) }
            }

            NumberAnimation on rotation {
                from: 0
                to: 360
                duration: 20000 + (index * 5000)
                loops: Animation.Infinite
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
                color: "#1a1a3e"
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
                color: "white"
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
                color: "white"
                opacity: 0.3
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
            color: "#cccccc"
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
                color: "white"
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
                    border.color: "white"
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
                            color: index < (splashRoot.progress * 20) ? "white" : "#2a2a4e"
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
                            GradientStop { position: 0.5; color: "#ffffff40" }
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
                color: "white"
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
