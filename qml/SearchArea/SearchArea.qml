import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick

Rectangle {
    id: searchAreaRoot
    height: 150
    color: "Black"
    property var backend
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 100
        anchors.rightMargin: 10
        height: 50
        color: "#C7C5C5"
        radius: 15
        border.color: "#888888"
        border.width: 1

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 14
            anchors.verticalCenter: parent.verticalCenter
            text: "Search anything"
            color: "#525252"
            visible: searchInput.text.length === 0
        }

        TextInput {
            id: searchInput
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            color: "#202020"
            font.pixelSize: 14
            verticalAlignment: TextInput.AlignVCenter
            clip: true
            selectByMouse: true

            Keys.onEscapePressed: {
                text = ""
                focus = false
            }
            onAccepted: {
                searchAreaRoot.backend.url = searchInput.text
                searchInput.focus = false
            }
        }
    }
}