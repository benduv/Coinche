import QtQuick

Rectangle {
    id: toast
    anchors.horizontalCenter: parent.horizontalCenter
    width: parent.width * 0.6
    height: parent.height * 0.09
    radius: 15 * minRatio
    color: "#8EEDF5"
    border.color: "white"
    border.width: 1
    visible: false
    z: 2000
    y: -height

    // Ratio autonome basé sur 1920x1080 (paysage)
    property real minRatio: Math.min(parent.width / 1920, parent.height / 1080)
    property string fromPseudo: ""
    property string lobbyCode: ""
    property real targetY: parent.height * 0.01

    signal accepted(string lobbyCode)
    signal rejected()

    function show() {
        y = -height
        visible = true
        slideIn.start()
    }

    function hide() {
        slideOut.start()
    }

    NumberAnimation on y {
        id: slideIn
        to: toast.targetY
        duration: 400
        easing.type: Easing.OutBack
        running: false
    }

    NumberAnimation on y {
        id: slideOut
        to: -toast.height
        duration: 300
        easing.type: Easing.InQuad
        running: false
        onFinished: toast.visible = false
    }

    Row {
        anchors.centerIn: parent
        spacing: toast.width * 0.03

        Text {
            text: toast.fromPseudo + " vous invite à rejoindre sa partie privée"
            font.pixelSize: 40 * toast.minRatio
            font.bold: true
            color: "black"
            anchors.verticalCenter: parent.verticalCenter
        }

        Item {
            width: 1
            height: 1
        }

        Item {
            width: 1
            height: 1
        }

        Rectangle {
            width: toast.width * 0.08
            height: toast.height * 0.65
            radius: 8 * toast.minRatio
            color: "#25BA31"
            anchors.verticalCenter: parent.verticalCenter

            Image {
                source: "qrc:/resources/check-svgrepo-com.svg"
                anchors.fill: parent
                anchors.margins: parent.width * 0.06
                fillMode: Image.PreserveAspectFit
                sourceSize: Qt.size(width, height)
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    toast.accepted(toast.lobbyCode)
                    toast.hide()
                    autoHideTimer.stop()
                }
            }
        }

        Rectangle {
            width: toast.width * 0.08
            height: toast.height * 0.65
            radius: 8 * toast.minRatio
            color: "#BA3125"
            anchors.verticalCenter: parent.verticalCenter

            Image {
                source: "qrc:/resources/cross-small-svgrepo-com.svg"
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                sourceSize: Qt.size(width, height)
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    toast.rejected()
                    toast.hide()
                    autoHideTimer.stop()
                }
            }
        }
    }

    Timer {
        id: autoHideTimer
        interval: 15000
        onTriggered: toast.hide()
    }

    function showInvite(pseudo, code) {
        fromPseudo = pseudo
        lobbyCode = code
        show()
        autoHideTimer.restart()
    }
}
