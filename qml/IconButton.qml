import QtQuick

Rectangle {
    id: root


    property string kind: "plus"


    signal activated()

    width: 30
    height: 26
    radius: 7


    color: !root.enabled ? "transparent"
          : mouse.pressed ? Qt.rgba(0, 0, 0, 0.12)
          : mouse.containsMouse ? Qt.rgba(0, 0, 0, 0.07)
          : "transparent"

    opacity: root.enabled ? 1.0 : 0.35

    Behavior on color {
        ColorAnimation { duration: 100 }
    }

    SafariIcon {
        anchors.centerIn: parent
        kind: root.kind
    }




    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.activated()
    }
}
