import QtQuick
import QtQuick.Controls

Rectangle {
    id: friendsRoot
    anchors.fill: parent
    color: "transparent"

    property bool isPortrait: height > width
    property real widthRatio: isPortrait ? width / 600 : width / 1024
    property real heightRatio: isPortrait ? height / 1024 : height / 768
    property real minRatio: Math.min(widthRatio, heightRatio)

    signal backToMenu()
    signal openContact()

    ListModel { id: friendsModel }
    ListModel { id: pendingRequestsModel }

    // Fond étoilé
    StarryBackground {
        anchors.fill: parent
        minRatio: friendsRoot.minRatio
        z: -1
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
            onClicked: backToMenu()
            onEntered: parent.scale = 1.1
            onExited: parent.scale = 1.0
        }
    }

    // Titre
    Text {
        id: titleText
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 50 * minRatio
        text: "AMIS"
        font.pixelSize: 42 * minRatio
        font.bold: true
        color: "#FFD700"
    }

    // Contenu scrollable
    Flickable {
        id: friendsFlickable
        anchors.top: titleText.bottom
        anchors.topMargin: 30 * minRatio
        anchors.bottom: contactButton.top
        anchors.bottomMargin: 20 * minRatio
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width * 0.9, 700)
        contentHeight: mainColumn.height
        clip: true
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: mainColumn
            width: parent.width
            spacing: 15 * minRatio

            // Section: Demandes en attente
            Rectangle {
                width: parent.width
                height: 40 * minRatio
                color: "#2a2a2a"
                radius: 5
                visible: pendingRequestsModel.count > 0

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 15
                    anchors.verticalCenter: parent.verticalCenter
                    text: "DEMANDES EN ATTENTE"
                    font.pixelSize: 20 * minRatio
                    font.bold: true
                    color: "#FF9800"
                }
            }

            Repeater {
                model: pendingRequestsModel

                Rectangle {
                    width: mainColumn.width
                    height: 60 * minRatio
                    color: "#1a1a1a"
                    radius: 10
                    border.color: "#FF9800"
                    border.width: 1

                    Row {
                        anchors.fill: parent
                        anchors.margins: 10 * minRatio
                        spacing: 15 * minRatio

                        // Avatar rond
                        Rectangle {
                            width: 40 * minRatio
                            height: 40 * minRatio
                            radius: width / 2
                            color: "#3a3a3a"
                            anchors.verticalCenter: parent.verticalCenter
                            clip: true

                            Image {
                                anchors.fill: parent
                                source: model.avatar ? "qrc:/resources/" + model.avatar : ""
                                fillMode: Image.PreserveAspectFit
                                visible: model.avatar !== ""
                            }
                        }

                        // Pseudo
                        Text {
                            text: model.pseudo
                            font.pixelSize: 20 * minRatio
                            font.bold: true
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width - 200 * minRatio
                            elide: Text.ElideRight
                        }

                        // Bouton Accepter
                        Rectangle {
                            width: 80 * minRatio
                            height: 35 * minRatio
                            radius: 8
                            color: "#006600"
                            anchors.verticalCenter: parent.verticalCenter

                            Text {
                                anchors.centerIn: parent
                                text: "Accepter"
                                font.pixelSize: 14 * minRatio
                                font.bold: true
                                color: "white"
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: networkManager.acceptFriendRequest(model.pseudo)
                            }
                        }

                        // Bouton Refuser
                        Rectangle {
                            width: 80 * minRatio
                            height: 35 * minRatio
                            radius: 8
                            color: "#cc0000"
                            anchors.verticalCenter: parent.verticalCenter

                            Text {
                                anchors.centerIn: parent
                                text: "Refuser"
                                font.pixelSize: 14 * minRatio
                                font.bold: true
                                color: "white"
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: networkManager.rejectFriendRequest(model.pseudo)
                            }
                        }
                    }
                }
            }

            // Section: Mes amis
            Rectangle {
                width: parent.width
                height: 40 * minRatio
                color: "#2a2a2a"
                radius: 5

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 15
                    anchors.verticalCenter: parent.verticalCenter
                    text: "MES AMIS"
                    font.pixelSize: 20 * minRatio
                    font.bold: true
                    color: "#4CAF50"
                }
            }

            // Message si aucun ami
            Text {
                visible: friendsModel.count === 0
                text: "Aucun ami pour le moment"
                font.pixelSize: 18 * minRatio
                color: "#888888"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Repeater {
                model: friendsModel

                Rectangle {
                    width: mainColumn.width
                    height: 75 * minRatio
                    color: "#1a1a1a"
                    radius: 10
                    border.color: "#3a3a3a"
                    border.width: 1

                    Row {
                        anchors.left: parent.left
                        anchors.right: deleteButton.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.margins: 10 * minRatio
                        spacing: 15 * minRatio

                        // Pastille en ligne / hors ligne
                        Rectangle {
                            width: 14 * minRatio
                            height: 14 * minRatio
                            radius: width / 2
                            color: model.online ? "#4CAF50" : "#666666"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        // Avatar rond
                        Rectangle {
                            width: 50 * minRatio
                            height: 50 * minRatio
                            radius: width / 2
                            color: "#3a3a3a"
                            anchors.verticalCenter: parent.verticalCenter
                            clip: true

                            Image {
                                anchors.fill: parent
                                source: model.avatar ? "qrc:/resources/" + model.avatar : ""
                                fillMode: Image.PreserveAspectFit
                                visible: model.avatar !== ""
                            }
                        }

                        // Pseudo
                        Text {
                            text: model.pseudo
                            font.pixelSize: 22 * minRatio
                            font.bold: true
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        // Statut texte
                        Text {
                            text: model.online ? "En ligne" : "Hors ligne"
                            font.pixelSize: 15 * minRatio
                            color: model.online ? "#4CAF50" : "#666666"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    // Bouton supprimer ami
                    Rectangle {
                        id: deleteButton
                        anchors.right: parent.right
                        anchors.rightMargin: 10 * minRatio
                        anchors.verticalCenter: parent.verticalCenter
                        width: 35 * minRatio
                        height: 35 * minRatio
                        radius: width / 2
                        color: "#cc0000"

                        Image {
                            anchors.fill: parent
                            anchors.margins: 8 * minRatio
                            source: "qrc:/resources/cross-small-svgrepo-com.svg"
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                deleteFriendPopup.friendToDelete = model.pseudo
                                deleteFriendPopup.visible = true
                            }
                        }
                    }
                }
            }
        }
    }

    // Bouton Contacter Nebuludik
    Rectangle {
        id: contactButton
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 30 * minRatio
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width * 0.7, 400)
        height: 50 * minRatio
        radius: 10
        color: "#3a3a3a"
        border.color: "#FFD700"
        border.width: 2

        Text {
            anchors.centerIn: parent
            text: "Contacter Nebuludik"
            font.pixelSize: 20 * minRatio
            font.bold: true
            color: "#FFD700"
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: friendsRoot.openContact()
        }
    }

    // Popup de confirmation de suppression d'ami
    Rectangle {
        id: deleteFriendPopup
        anchors.fill: parent
        color: "#CC000000"
        visible: false
        z: 500

        property string friendToDelete: ""

        MouseArea {
            anchors.fill: parent
            onClicked: deleteFriendPopup.visible = false
        }

        Rectangle {
            anchors.centerIn: parent
            width: Math.min(parent.width * 0.8, 400)
            height: 180 * minRatio
            color: "#1a1a1a"
            radius: 15
            border.color: "#FFD700"
            border.width: 2

            MouseArea {
                anchors.fill: parent
                onClicked: {} // bloquer propagation
            }

            Column {
                anchors.centerIn: parent
                spacing: 25 * minRatio

                Text {
                    text: "Supprimer " + deleteFriendPopup.friendToDelete + " de vos amis ?"
                    font.pixelSize: 20 * minRatio
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Row {
                    spacing: 30 * minRatio
                    anchors.horizontalCenter: parent.horizontalCenter

                    Rectangle {
                        width: 100 * minRatio
                        height: 45 * minRatio
                        radius: 10
                        color: "#cc0000"

                        Text {
                            anchors.centerIn: parent
                            text: "Oui"
                            font.pixelSize: 18 * minRatio
                            font.bold: true
                            color: "white"
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                networkManager.removeFriend(deleteFriendPopup.friendToDelete)
                                deleteFriendPopup.visible = false
                            }
                        }
                    }

                    Rectangle {
                        width: 100 * minRatio
                        height: 45 * minRatio
                        radius: 10
                        color: "#3a3a3a"
                        border.color: "#FFD700"
                        border.width: 2

                        Text {
                            anchors.centerIn: parent
                            text: "Non"
                            font.pixelSize: 18 * minRatio
                            font.bold: true
                            color: "#FFD700"
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: deleteFriendPopup.visible = false
                        }
                    }
                }
            }
        }
    }

    // Connexions réseau
    Connections {
        target: networkManager

        function onFriendsListReceived(friends, pendingRequests) {
            friendsModel.clear()
            for (var i = 0; i < friends.length; i++) {
                friendsModel.append(friends[i])
            }
            pendingRequestsModel.clear()
            for (var j = 0; j < pendingRequests.length; j++) {
                pendingRequestsModel.append(pendingRequests[j])
            }
        }

        function onFriendRequestAccepted(pseudo) {
            networkManager.getFriendsList()
        }

        function onFriendRequestRejected() {
            networkManager.getFriendsList()
        }

        function onFriendRequestReceived(fromPseudo, fromAvatar) {
            networkManager.getFriendsList()
        }

        function onFriendRemoved(pseudo) {
            networkManager.getFriendsList()
        }
    }

    Component.onCompleted: {
        networkManager.getFriendsList()
    }
}
