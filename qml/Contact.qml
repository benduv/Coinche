import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: contactRoot
    anchors.fill: parent
    color: "#1a1a1a"

    // Ratio responsive
    property real widthRatio: width / 1024
    property real heightRatio: height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    // Email de contact (pour affichage uniquement)
    property string contactEmail: "contact@nebuludik.fr"

    // Etat de l'envoi
    property bool isSending: false

    signal backToMenu()

    // Connexion aux signaux du networkManager
    Connections {
        target: networkManager
        function onContactMessageSuccess() {
            contactRoot.isSending = false
            successPopup.open()
        }
        function onContactMessageFailed(error) {
            contactRoot.isSending = false
            errorText.text = error
            errorPopup.open()
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
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                backToMenu()
            }
            onEntered: {
                parent.scale = 1.1
            }
            onExited: {
                parent.scale = 1.0
            }
        }

        Behavior on scale {
            NumberAnimation { duration: 100 }
        }
    }

    // MouseArea pour fermer le clavier en cliquant ailleurs
    MouseArea {
        anchors.fill: parent
        onClicked: {
            // Retirer le focus des champs de texte pour fermer le clavier
            contactRoot.forceActiveFocus()
        }
    }

    // Contenu principal scrollable
    Flickable {
        id: mainFlickable
        anchors.fill: parent
        anchors.topMargin: 120 * contactRoot.minRatio
        anchors.leftMargin: 40 * contactRoot.minRatio
        anchors.rightMargin: 40 * contactRoot.minRatio
        anchors.bottomMargin: 40 * contactRoot.minRatio
        contentHeight: contentColumn.height
        clip: true

        Column {
            id: contentColumn
            width: parent.width
            spacing: 30 * contactRoot.minRatio

            // Titre
            Text {
                text: "NOUS CONTACTER"
                font.pixelSize: 48 * contactRoot.minRatio
                font.bold: true
                color: "#FFD700"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // Description
            Text {
                width: parent.width
                text: "Vous avez une suggestion, un bug à signaler ou simplement envie de nous faire part de vos impressions ? N'hésitez pas à nous écrire via ce formulaire ou bien directement via contact@nebuludik.fr"
                font.pixelSize: 28 * contactRoot.minRatio
                color: "white"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Item { height: 20 * contactRoot.minRatio; width: 1 }

            // Formulaire
            Rectangle {
                width: Math.min(parent.width, 750 * contactRoot.minRatio)
                height: formColumn.height + 40 * contactRoot.minRatio
                color: "#2a2a2a"
                radius: 15 * contactRoot.minRatio
                border.color: "#FFD700"
                border.width: 2 * contactRoot.minRatio
                anchors.horizontalCenter: parent.horizontalCenter

                Column {
                    id: formColumn
                    anchors.centerIn: parent
                    width: parent.width - 40 * contactRoot.minRatio
                    spacing: 20 * contactRoot.minRatio

                    // Champ Sujet
                    Column {
                        width: parent.width
                        spacing: 8 * contactRoot.minRatio

                        Text {
                            text: "Sujet"
                            font.pixelSize: 24 * contactRoot.minRatio
                            font.bold: true
                            color: "#FFD700"
                        }

                        Rectangle {
                            width: parent.width
                            height: 50 * contactRoot.minRatio
                            color: "#3a3a3a"
                            radius: 8 * contactRoot.minRatio
                            border.color: subjectField.activeFocus ? "#FFD700" : "#555555"
                            border.width: 2 * contactRoot.minRatio

                            TextInput {
                                id: subjectField
                                anchors.fill: parent
                                anchors.margins: 10 * contactRoot.minRatio
                                font.pixelSize: 22 * contactRoot.minRatio
                                color: "white"
                                clip: true
                                // Bouton "Suivant" pour passer au champ message
                                inputMethodHints: Qt.ImhNone
                                Keys.onReturnPressed: messageArea.forceActiveFocus()

                                Text {
                                    anchors.fill: parent
                                    text: "Ex: Suggestion, Bug, Question..."
                                    font.pixelSize: 22 * contactRoot.minRatio
                                    color: "#888888"
                                    visible: !subjectField.text && !subjectField.activeFocus
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                        }
                    }

                    // Champ Message
                    Column {
                        width: parent.width
                        spacing: 8 * contactRoot.minRatio

                        Text {
                            text: "Message"
                            font.pixelSize: 24 * contactRoot.minRatio
                            font.bold: true
                            color: "#FFD700"
                        }

                        Rectangle {
                            id: messageFieldRect
                            width: parent.width
                            height: 150 * contactRoot.minRatio
                            color: "#3a3a3a"
                            radius: 8 * contactRoot.minRatio
                            border.color: messageArea.activeFocus ? "#FFD700" : "#555555"
                            border.width: 2 * contactRoot.minRatio

                            Flickable {
                                id: messageFlickable
                                anchors.fill: parent
                                anchors.margins: 10 * contactRoot.minRatio
                                contentWidth: messageArea.width
                                contentHeight: messageArea.height
                                clip: true
                                flickableDirection: Flickable.VerticalFlick
                                boundsBehavior: Flickable.StopAtBounds

                                TextEdit {
                                    id: messageArea
                                    width: messageFlickable.width
                                    height: Math.max(contentHeight, messageFlickable.height)
                                    font.pixelSize: 22 * contactRoot.minRatio
                                    color: "white"
                                    wrapMode: TextEdit.Wrap
                                    // Combinaison de flags pour éviter le mode extract sur Android
                                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhHiddenText

                                    Text {
                                        anchors.top: parent.top
                                        width: parent.width
                                        text: "Ecrivez votre message ici..."
                                        font.pixelSize: 22 * contactRoot.minRatio
                                        color: "#888888"
                                        visible: !messageArea.text && !messageArea.activeFocus
                                    }

                                    // Scroll automatique vers le curseur dans le TextEdit
                                    onCursorRectangleChanged: {
                                        if (cursorRectangle.y + cursorRectangle.height > messageFlickable.contentY + messageFlickable.height) {
                                            messageFlickable.contentY = cursorRectangle.y + cursorRectangle.height - messageFlickable.height
                                        } else if (cursorRectangle.y < messageFlickable.contentY) {
                                            messageFlickable.contentY = cursorRectangle.y
                                        }
                                    }

                                    // Scroll automatique du formulaire vers le champ message quand il obtient le focus
                                    onActiveFocusChanged: {
                                        if (activeFocus) {
                                            // Calculer la position Y du champ message dans le contentColumn
                                            var targetY = messageFieldRect.mapToItem(contentColumn, 0, 0).y
                                            // Scroller pour que le champ message soit visible en haut de la zone visible
                                            // avec une petite marge
                                            var scrollTarget = targetY - 20 * contactRoot.minRatio
                                            if (scrollTarget > 0) {
                                                mainFlickable.contentY = Math.min(scrollTarget, mainFlickable.contentHeight - mainFlickable.height)
                                            }
                                        }
                                    }
                                }
                            }

                            // Bouton OK intégré en bas à droite du champ message
                            Rectangle {
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                anchors.margins: 8 * contactRoot.minRatio
                                width: 70 * contactRoot.minRatio
                                height: 35 * contactRoot.minRatio
                                color: okMouseArea.pressed ? "#cc9900" : "#FFD700"
                                radius: 6 * contactRoot.minRatio
                                visible: messageArea.activeFocus
                                z: 10

                                Text {
                                    anchors.centerIn: parent
                                    text: "OK"
                                    font.pixelSize: 18 * contactRoot.minRatio
                                    font.bold: true
                                    color: "#1a1a1a"
                                }

                                MouseArea {
                                    id: okMouseArea
                                    anchors.fill: parent
                                    onClicked: {
                                        contactRoot.forceActiveFocus()
                                    }
                                }
                            }
                        }
                    }

                    // Bouton Envoyer
                    Button {
                        id: sendButton
                        width: parent.width
                        height: 75 * contactRoot.minRatio
                        enabled: subjectField.text.trim() !== "" && messageArea.text.trim() !== "" && !contactRoot.isSending && networkManager.connected

                        background: Rectangle {
                            color: sendButton.enabled ?
                                   (sendButton.down ? "#00aa00" : (sendButton.hovered ? "#00dd00" : "#00cc00")) :
                                   "#555555"
                            radius: 10 * contactRoot.minRatio
                            border.color: sendButton.enabled ? "#FFD700" : "#888888"
                            border.width: 2 * contactRoot.minRatio
                        }

                        contentItem: Row {
                            anchors.centerIn: parent
                            spacing: 10 * contactRoot.minRatio

                            // Indicateur de chargement
                            BusyIndicator {
                                id: busyIndicator
                                width: 30 * contactRoot.minRatio
                                height: 30 * contactRoot.minRatio
                                running: contactRoot.isSending
                                visible: contactRoot.isSending
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: contactRoot.isSending ? "Envoi en cours..." : "Envoyer"
                                font.pixelSize: 28 * contactRoot.minRatio
                                font.bold: true
                                color: sendButton.enabled ? "white" : "#aaaaaa"
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }

                        onClicked: {
                            contactRoot.isSending = true
                            // Envoyer via le serveur
                            networkManager.sendContactMessage(
                                "", // senderName vide (anonyme)
                                subjectField.text.trim(),
                                messageArea.text.trim()
                            )
                        }
                    }

                    // Note sur la connexion
                    Text {
                        width: parent.width
                        text: networkManager.connected ? "" : "Connexion au serveur requise pour envoyer un message"
                        font.pixelSize: 18 * contactRoot.minRatio
                        color: "#ff6666"
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        visible: !networkManager.connected
                    }
                }
            }

            Item { height: 40 * contactRoot.minRatio; width: 1 }
        }
    }

    // Popup de succes
    Popup {
        id: successPopup
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.8, 400 * contactRoot.minRatio)
        height: successColumn.height + 60 * contactRoot.minRatio
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#2a2a2a"
            radius: 15 * contactRoot.minRatio
            border.color: "#00cc00"
            border.width: 3 * contactRoot.minRatio
        }

        Column {
            id: successColumn
            anchors.centerIn: parent
            width: parent.width - 40 * contactRoot.minRatio
            spacing: 20 * contactRoot.minRatio

            Text {
                text: "Message envoyé !"
                font.pixelSize: 32 * contactRoot.minRatio
                font.bold: true
                color: "#00cc00"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                width: parent.width
                text: "Merci pour votre message ! Nous prendrons en compte vos remarques au plus vite"
                font.pixelSize: 22 * contactRoot.minRatio
                color: "white"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
                width: parent.width * 0.6
                height: 75 * contactRoot.minRatio
                anchors.horizontalCenter: parent.horizontalCenter

                background: Rectangle {
                    color: parent.down ? "#444444" : (parent.hovered ? "#555555" : "#333333")
                    radius: 8 * contactRoot.minRatio
                    border.color: "#FFD700"
                    border.width: 2 * contactRoot.minRatio
                }

                contentItem: Text {
                    text: "OK"
                    font.pixelSize: 24 * contactRoot.minRatio
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    successPopup.close()
                    // Effacer les champs
                    subjectField.text = ""
                    messageArea.text = ""
                }
            }
        }
    }

    // Popup d'erreur
    Popup {
        id: errorPopup
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.8, 400 * contactRoot.minRatio)
        height: errorColumn.height + 60 * contactRoot.minRatio
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#2a2a2a"
            radius: 15 * contactRoot.minRatio
            border.color: "#ff6666"
            border.width: 3 * contactRoot.minRatio
        }

        Column {
            id: errorColumn
            anchors.centerIn: parent
            width: parent.width - 40 * contactRoot.minRatio
            spacing: 20 * contactRoot.minRatio

            Text {
                text: "Erreur d'envoi"
                font.pixelSize: 32 * contactRoot.minRatio
                font.bold: true
                color: "#ff6666"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                id: errorText
                width: parent.width
                text: ""
                font.pixelSize: 22 * contactRoot.minRatio
                color: "white"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                width: parent.width
                text: "Vous pouvez aussi nous contacter directement à : " + contactRoot.contactEmail
                font.pixelSize: 18 * contactRoot.minRatio
                color: "#aaaaaa"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
                width: parent.width * 0.6
                height: 75 * contactRoot.minRatio
                anchors.horizontalCenter: parent.horizontalCenter

                background: Rectangle {
                    color: parent.down ? "#444444" : (parent.hovered ? "#555555" : "#333333")
                    radius: 8 * contactRoot.minRatio
                    border.color: "#FFD700"
                    border.width: 2 * contactRoot.minRatio
                }

                contentItem: Text {
                    text: "OK"
                    font.pixelSize: 24 * contactRoot.minRatio
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    errorPopup.close()
                }
            }
        }
    }
}
