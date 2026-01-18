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
    Button {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 20 * contactRoot.minRatio
        anchors.topMargin: 20 * contactRoot.minRatio
        width: 80 * contactRoot.minRatio
        height: 80 * contactRoot.minRatio
        z: 10

        background: Rectangle {
            color: parent.down ? "#444444" : (parent.hovered ? "#555555" : "#333333")
            radius: 10 * contactRoot.minRatio
            border.color: "#FFD700"
            border.width: 2 * contactRoot.minRatio
        }

        contentItem: Image {
            source: "qrc:/resources/back-square-svgrepo-com.svg"
            fillMode: Image.PreserveAspectFit
            anchors.fill: parent
            anchors.margins: 10 * contactRoot.minRatio
        }

        onClicked: {
            contactRoot.backToMenu()
        }
    }

    // Contenu principal scrollable
    Flickable {
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
                width: Math.min(parent.width, 700 * contactRoot.minRatio)
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
                            width: parent.width
                            height: 200 * contactRoot.minRatio
                            color: "#3a3a3a"
                            radius: 8 * contactRoot.minRatio
                            border.color: messageArea.activeFocus ? "#FFD700" : "#555555"
                            border.width: 2 * contactRoot.minRatio

                            ScrollView {
                                id: messageScrollView
                                anchors.fill: parent
                                anchors.margins: 10 * contactRoot.minRatio
                                clip: true

                                TextArea {
                                    id: messageArea
                                    width: messageScrollView.availableWidth
                                    height: Math.max(messageScrollView.availableHeight, contentHeight)
                                    font.pixelSize: 22 * contactRoot.minRatio
                                    color: "white"
                                    wrapMode: TextArea.Wrap
                                    background: null
                                    placeholderText: "Ecrivez votre message ici..."
                                    placeholderTextColor: "#888888"
                                }
                            }
                        }
                    }

                    // Bouton Envoyer
                    Button {
                        id: sendButton
                        width: parent.width
                        height: 70 * contactRoot.minRatio
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
                text: "Message envoye !"
                font.pixelSize: 32 * contactRoot.minRatio
                font.bold: true
                color: "#00cc00"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                width: parent.width
                text: "Merci pour votre message ! Nous vous repondrons dans les plus brefs delais."
                font.pixelSize: 22 * contactRoot.minRatio
                color: "white"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
                width: parent.width * 0.6
                height: 50 * contactRoot.minRatio
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
                text: "Vous pouvez aussi nous contacter directement a : " + contactRoot.contactEmail
                font.pixelSize: 18 * contactRoot.minRatio
                color: "#aaaaaa"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
                width: parent.width * 0.6
                height: 50 * contactRoot.minRatio
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
