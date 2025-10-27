import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root
    width: 1024
    height: 768
    visible: true
    title: qsTr("Coinche")
    color: "#035605" // Green background for card table

    // Main game area
    Rectangle {
        anchors.fill: parent
        color: "transparent"

        // North player area
        Item {
            id: northPlayer
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.3
            height: parent.height * 0.2
        }

        // West player area
        Item {
            id: westPlayer
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width * 0.2
            height: parent.height * 0.3
        }

        // East player area
        Item {
            id: eastPlayer
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width * 0.2
            height: parent.height * 0.3
        }

        // South player area (current player)
        Item {
            id: southPlayer
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.3
            height: parent.height * 0.2
        }

        // Central playing area
        Rectangle {
            id: playArea
            anchors.centerIn: parent
            width: parent.width * 0.3
            height: parent.height * 0.3
            color: "#034504" // Slightly darker green
            radius: 10
        }
    }
}