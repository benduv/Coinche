import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    // Ratios pour le responsive
    property real widthRatio: width / 1920
    property real heightRatio: height / 1080
    property real minRatio: Math.min(widthRatio, heightRatio)

    Rectangle {
        anchors.fill: parent
        color: "#1a1a1a"
    }

    Rectangle { // Solution du pauvre pour mettre en noire la bar du haut de l'ecran (selfie)
        anchors.left: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width / 5
        height: parent.height
        color: "black"
    }

    Rectangle {
        anchors.right: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width / 5
        height: parent.height
        color: "black"
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 60 * root.minRatio

        // Titre
        Text {
            text: "Jouer avec des amis"
            font.pixelSize: 80 * root.minRatio
            font.bold: true
            color: "#FFD700"
            Layout.alignment: Qt.AlignHCenter
        }

        // Sous-titre
        Text {
            text: "Créez une partie privée ou rejoignez celle d'un ami"
            font.pixelSize: 42 * root.minRatio
            color: "#aaaaaa"
            Layout.alignment: Qt.AlignHCenter
        }

        // Conteneur des boutons
        Row {
            Layout.alignment: Qt.AlignHCenter
            spacing: 60 * root.minRatio

            // Bouton Hôte
            Button {
                width: 550 * root.widthRatio
                height: 400 * root.heightRatio

                background: Rectangle {
                    color: parent.down ? "#1a5c1a" : (parent.hovered ? "#2a7c2a" : "#1a6c1a")
                    radius: 15 * root.minRatio
                    border.color: "#00ff00"
                    border.width: 3 * root.minRatio

                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                contentItem: Column {
                    anchors.centerIn: parent
                    spacing: 15 * root.minRatio

                    Text {
                        text: "👑"
                        font.pixelSize: 80 * root.minRatio
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Hôte"
                        font.pixelSize: 80 * root.minRatio
                        font.bold: true
                        color: "white"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Créer un lobby"
                        font.pixelSize: 40 * root.minRatio
                        color: "#cccccc"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                onClicked: {
                    console.log("Creation d'un lobby prive")
                    networkManager.createPrivateLobby()
                }
            }

            // Bouton Rejoindre
            Button {
                width: 550 * root.widthRatio
                height: 400 * root.heightRatio

                background: Rectangle {
                    color: parent.down ? "#1a3c5c" : (parent.hovered ? "#2a5c8c" : "#1a4c7c")
                    radius: 15 * root.minRatio
                    border.color: "#00BFFF"
                    border.width: 3 * root.minRatio

                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                contentItem: Column {
                    anchors.centerIn: parent
                    spacing: 15 * root.minRatio

                    Text {
                        text: "🎮"
                        font.pixelSize: 80 * root.minRatio
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Rejoindre"
                        font.pixelSize: 80 * root.minRatio
                        font.bold: true
                        color: "white"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Entrer un code"
                        font.pixelSize: 40 * root.minRatio
                        color: "#cccccc"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                onClicked: {
                    joinLobbyPopup.open()
                }
            }
        }

        // Bouton Retour
        Button {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 40 * root.minRatio
            width: 400 * root.widthRatio
            height: 180 * root.heightRatio

            background: Rectangle {
                color: parent.down ? "#cc0000" : (parent.hovered ? "#ff3333" : "#ff0000")
                radius: 10 * root.minRatio
                Behavior on color { ColorAnimation { duration: 150 } }
            }

            contentItem: Text {
                text: "Retour"
                font.pixelSize: 50 * root.minRatio
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                stackView.pop()
            }
        }
    }

    // Popup pour rejoindre un lobby
    Popup {
        id: joinLobbyPopup
        anchors.centerIn: parent
        width: 700 * root.widthRatio
        height: 600 * root.heightRatio
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#2a2a2a"
            radius: 15 * root.minRatio
            border.color: "#00BFFF"
            border.width: 3 * root.minRatio
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 30 * root.minRatio
            spacing: 40 * root.minRatio

            Text {
                text: "Rejoindre un lobby"
                font.pixelSize: 42 * root.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "Entrez le code du lobby:"
                font.pixelSize: 36 * root.minRatio
                color: "white"
                Layout.alignment: Qt.AlignHCenter
            }

            TextField {
                id: lobbyCodeInput
                Layout.preferredWidth: 400 * root.widthRatio
                Layout.preferredHeight: 110 * root.heightRatio
                Layout.alignment: Qt.AlignHCenter
                font.pixelSize: 36 * root.minRatio
                horizontalAlignment: Text.AlignHCenter
                placeholderText: "XXXX"
                maximumLength: 4

                background: Rectangle {
                    color: "#1a1a1a"
                    radius: 8 * root.minRatio
                    border.color: lobbyCodeInput.focus ? "#00BFFF" : "#555555"
                    border.width: 2 * root.minRatio
                }

                onAccepted: {
                    if (text.length === 4) {
                        networkManager.joinPrivateLobby(text.toUpperCase())
                        joinLobbyPopup.close()
                        lobbyCodeInput.text = ""
                    }
                }
            }

            Row {
                Layout.alignment: Qt.AlignHCenter
                spacing: 40 * root.minRatio

                Button {
                    width: 200 * root.widthRatio
                    height: 120 * root.heightRatio

                    background: Rectangle {
                        color: parent.down ? "#1a5c1a" : (parent.hovered ? "#2a7c2a" : "#1a6c1a")
                        radius: 8 * root.minRatio
                    }

                    contentItem: Text {
                        text: "Rejoindre"
                        font.pixelSize: 36 * root.minRatio
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    enabled: lobbyCodeInput.text.length === 4

                    onClicked: {
                        networkManager.joinPrivateLobby(lobbyCodeInput.text.toUpperCase())
                        joinLobbyPopup.close()
                        lobbyCodeInput.text = ""
                    }
                }

                Button {
                    width: 200 * root.widthRatio
                    height: 120 * root.heightRatio

                    background: Rectangle {
                        color: parent.down ? "#cc0000" : (parent.hovered ? "#ff3333" : "#ff0000")
                        radius: 8 * root.minRatio
                    }

                    contentItem: Text {
                        text: "Annuler"
                        font.pixelSize: 36 * root.minRatio
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        joinLobbyPopup.close()
                        lobbyCodeInput.text = ""
                    }
                }
            }
        }
    }

    // Écouter les erreurs du serveur
    Connections {
        target: networkManager
        enabled: root.visible  // Désactiver quand la vue n'est plus visible

        function onLobbyError(errorMessage) {
            console.log("Erreur lobby (PrivateLobbyView):", errorMessage)
            errorPopup.errorText = errorMessage
            errorPopup.open()
        }
    }

    // Popup d'erreur
    Popup {
        id: errorPopup
        anchors.centerIn: parent
        width: 500 * root.widthRatio
        height: 350 * root.heightRatio
        modal: true
        focus: true

        property string errorText: ""

        background: Rectangle {
            color: "#2a2a2a"
            radius: 15 * root.minRatio
            border.color: "#ff0000"
            border.width: 3 * root.minRatio
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20 * root.minRatio
            spacing: 30 * root.minRatio

            Text {
                text: "Erreur"
                font.pixelSize: 36 * root.minRatio
                font.bold: true
                color: "#ff0000"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: errorPopup.errorText
                font.pixelSize: 36 * root.minRatio
                color: "white"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                width: 150 * root.widthRatio
                height: 120 * root.heightRatio

                background: Rectangle {
                    color: parent.down ? "#cc0000" : (parent.hovered ? "#ff3333" : "#ff0000")
                    radius: 8 * root.minRatio
                }

                contentItem: Text {
                    text: "OK"
                    font.pixelSize: 36 * root.minRatio
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: errorPopup.close()
            }
        }
    }
}  // Fin de Item root
