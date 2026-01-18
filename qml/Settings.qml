import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: settingsRoot
    anchors.fill: parent
    color: "#1a1a1a"

    // Ratio responsive
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Propriétés passées par le parent
    property string playerName: ""
    property string accountType: ""

    signal backToMenu()
    signal accountDeleted()

    // Flag pour éviter de sauvegarder pendant l'initialisation
    property bool initialized: false

    // Synchroniser avec les paramètres globaux
    Component.onCompleted: {
        musicEnabled = AudioSettings.musicEnabled
        effectsEnabled = AudioSettings.effectsEnabled
        initialized = true
    }

    // Écouter les signaux de suppression de compte
    Connections {
        target: networkManager
        function onDeleteAccountSuccess() {
            deleteConfirmPopup.close()
            settingsRoot.accountDeleted()
        }
        function onDeleteAccountFailed(error) {
            deleteErrorText.text = error
            deleteErrorText.visible = true
        }
    }

    // Propriétés pour l'état des sons
    property bool musicEnabled: AudioSettings.musicEnabled
    property bool effectsEnabled: AudioSettings.effectsEnabled

    // Watcher pour mettre à jour les paramètres globaux (seulement après initialisation)
    onMusicEnabledChanged: {
        if (initialized) {
            AudioSettings.saveMusicEnabled(musicEnabled)
        }
    }

    onEffectsEnabledChanged: {
        if (initialized) {
            AudioSettings.saveEffectsEnabled(effectsEnabled)
        }
    }

    // Bouton retour
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 40 * minRatio
        width: 100 * minRatio
        height: 100 * minRatio
        color: "transparent"
        z: 100

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.6
            height: parent.height * 0.6
            color: "lightgrey"
        }

        Image {
            anchors.fill: parent
            source: "qrc:/resources/back-square-svgrepo-com.svg"
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                settingsRoot.backToMenu()
            }
        }
    }

    // Titre
    Text {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 50 * minRatio
        text: "RÉGLAGES"
        font.pixelSize: 60 * minRatio
        font.bold: true
        color: "#FFD700"
    }

    // Contenu des réglages
    ScrollView {
        anchors.centerIn: parent
        width: parent.width * 0.8
        height: parent.height * 0.6
        clip: true

        Column {
            width: parent.width
            spacing: 30 * settingsRoot.minRatio

            // Section Audio
            Rectangle {
                width: parent.width
                height: audioColumn.height + 40 * settingsRoot.minRatio
                color: "#2a2a2a"
                radius: 10 * settingsRoot.minRatio
                border.color: "#FFD700"
                border.width: 2 * settingsRoot.minRatio

                Column {
                    id: audioColumn
                    anchors.centerIn: parent
                    width: parent.width - 40 * settingsRoot.minRatio
                    spacing: 20 * settingsRoot.minRatio

                    Text {
                        text: "🔊 AUDIO"
                        font.pixelSize: 40 * settingsRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
                    }

                    // Musique ON/OFF
                    Row {
                        width: parent.width
                        spacing: 20 * settingsRoot.minRatio

                        Text {
                            text: "Musique:"
                            font.pixelSize: 30 * settingsRoot.minRatio
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width * 0.5
                        }

                        Button {
                            id: musicToggleButton
                            width: 150 * settingsRoot.minRatio
                            height: 100 * settingsRoot.minRatio
                            anchors.verticalCenter: parent.verticalCenter

                            background: Rectangle {
                                color: settingsRoot.musicEnabled ? "#00aa00" : "#aa0000"
                                radius: 10 * settingsRoot.minRatio
                                border.color: "#FFD700"
                                border.width: 2 * settingsRoot.minRatio
                            }

                            contentItem: Text {
                                text: settingsRoot.musicEnabled ? "ON" : "OFF"
                                font.pixelSize: 30 * settingsRoot.minRatio
                                font.bold: true
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                settingsRoot.musicEnabled = !settingsRoot.musicEnabled
                            }
                        }
                    }

                    // Effets sonores ON/OFF
                    Row {
                        width: parent.width
                        spacing: 20 * settingsRoot.minRatio

                        Text {
                            text: "Effets sonores:"
                            font.pixelSize: 30 * settingsRoot.minRatio
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width * 0.5
                        }

                        Button {
                            id: effectsToggleButton
                            width: 150 * settingsRoot.minRatio
                            height: 100 * settingsRoot.minRatio
                            anchors.verticalCenter: parent.verticalCenter

                            background: Rectangle {
                                color: settingsRoot.effectsEnabled ? "#00aa00" : "#aa0000"
                                radius: 10 * settingsRoot.minRatio
                                border.color: "#FFD700"
                                border.width: 2 * settingsRoot.minRatio
                            }

                            contentItem: Text {
                                text: settingsRoot.effectsEnabled ? "ON" : "OFF"
                                font.pixelSize: 30 * settingsRoot.minRatio
                                font.bold: true
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                settingsRoot.effectsEnabled = !settingsRoot.effectsEnabled
                            }
                        }
                    }
                }
            }

            // Section Affichage (pour futures options)
            Rectangle {
                width: parent.width
                height: displayColumn.height + 40 * settingsRoot.minRatio
                color: "#2a2a2a"
                radius: 10 * settingsRoot.minRatio
                border.color: "#FFD700"
                border.width: 2 * settingsRoot.minRatio

                Column {
                    id: displayColumn
                    anchors.centerIn: parent
                    width: parent.width - 40 * settingsRoot.minRatio
                    spacing: 20 * settingsRoot.minRatio

                    Text {
                        text: "🎨 AFFICHAGE"
                        font.pixelSize: 40 * settingsRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
                    }

                    Text {
                        text: "(Options d'affichage à venir...)"
                        font.pixelSize: 25 * settingsRoot.minRatio
                        color: "#888888"
                        font.italic: true
                    }
                }
            }

            // Section Compte (visible uniquement pour les comptes enregistrés)
            Rectangle {
                width: parent.width
                height: accountColumn.height + 40 * settingsRoot.minRatio
                color: "#2a2a2a"
                radius: 10 * settingsRoot.minRatio
                border.color: "#FFD700"
                border.width: 2 * settingsRoot.minRatio
                visible: settingsRoot.accountType !== "guest"

                Column {
                    id: accountColumn
                    anchors.centerIn: parent
                    width: parent.width - 40 * settingsRoot.minRatio
                    spacing: 20 * settingsRoot.minRatio

                    Text {
                        text: "👤 COMPTE"
                        font.pixelSize: 40 * settingsRoot.minRatio
                        font.bold: true
                        color: "#FFD700"
                    }

                    Text {
                        text: "Connecté en tant que : " + settingsRoot.playerName
                        font.pixelSize: 28 * settingsRoot.minRatio
                        color: "white"
                    }

                    // Bouton Supprimer mon compte
                    Button {
                        width: parent.width
                        height: 100 * settingsRoot.minRatio

                        background: Rectangle {
                            color: parent.down ? "#cc0000" : (parent.hovered ? "#bb0000" : "#aa0000")
                            radius: 10 * settingsRoot.minRatio
                            border.color: "#ff6666"
                            border.width: 2 * settingsRoot.minRatio
                        }

                        contentItem: Text {
                            text: "Supprimer mon compte"
                            font.pixelSize: 34 * settingsRoot.minRatio
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            deleteErrorText.visible = false
                            deleteConfirmPopup.open()
                        }
                    }
                }
            }
        }
    }

    // Popup de confirmation de suppression
    Popup {
        id: deleteConfirmPopup
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.8, 500 * settingsRoot.minRatio)
        height: deletePopupColumn.height + 60 * settingsRoot.minRatio
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#2a2a2a"
            radius: 15 * settingsRoot.minRatio
            border.color: "#ff6666"
            border.width: 3 * settingsRoot.minRatio
        }

        Column {
            id: deletePopupColumn
            anchors.centerIn: parent
            width: parent.width - 40 * settingsRoot.minRatio
            spacing: 20 * settingsRoot.minRatio

            Text {
                text: "Supprimer le compte ?"
                font.pixelSize: 36 * settingsRoot.minRatio
                font.bold: true
                color: "#ff6666"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                width: parent.width
                text: "Cette action est irréversible. Toutes vos données seront supprimées :"
                font.pixelSize: 26 * settingsRoot.minRatio
                color: "white"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                width: parent.width
                text: "• Toutes vos statistiques de jeu\n• Votre adresse email\n• Votre mot de passe"
                font.pixelSize: 24 * settingsRoot.minRatio
                color: "#cccccc"
                wrapMode: Text.WordWrap
                lineHeight: 1.3
            }

            // Message d'erreur
            Text {
                id: deleteErrorText
                width: parent.width
                text: ""
                font.pixelSize: 22 * settingsRoot.minRatio
                color: "#ff6666"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                visible: false
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 20 * settingsRoot.minRatio

                // Bouton Annuler
                Button {
                    width: 170 * settingsRoot.minRatio
                    height: 100 * settingsRoot.minRatio

                    background: Rectangle {
                        color: parent.down ? "#444444" : (parent.hovered ? "#555555" : "#333333")
                        radius: 10 * settingsRoot.minRatio
                        border.color: "#888888"
                        border.width: 2 * settingsRoot.minRatio
                    }

                    contentItem: Text {
                        text: "Annuler"
                        font.pixelSize: 24 * settingsRoot.minRatio
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        deleteConfirmPopup.close()
                    }
                }

                // Bouton Confirmer
                Button {
                    width: 170 * settingsRoot.minRatio
                    height: 100 * settingsRoot.minRatio

                    background: Rectangle {
                        color: parent.down ? "#cc0000" : (parent.hovered ? "#bb0000" : "#aa0000")
                        radius: 10 * settingsRoot.minRatio
                        border.color: "#ff6666"
                        border.width: 2 * settingsRoot.minRatio
                    }

                    contentItem: Text {
                        text: "Supprimer"
                        font.pixelSize: 24 * settingsRoot.minRatio
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        console.log("Suppression du compte:" + settingsRoot.playerName)
                        networkManager.deleteAccount(settingsRoot.playerName)
                    }
                }
            }
        }
    }
}
