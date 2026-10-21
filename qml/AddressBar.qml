import QtQuick
Rectangle {
    id: pillRoot

    property var backend

    readonly property int collapsedWidth:
        Math.min(560, Math.max(320, parent ? parent.width * 0.42 : 320))
    property real expandProgress: field.activeFocus ? 1 : 0

    height: 30
    radius: height / 2
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    width: collapsedWidth + expandProgress * ((parent.width - 200) - collapsedWidth)

    color: field.activeFocus ? "#FFFFFF" : "#E6E4E1"
    border.width: field.activeFocus ? 2 : 0
    border.color: "#007AFF"
    clip: true

    Behavior on expandProgress {
        NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
    }

    Row {
        id: contentRow
        spacing: 5
        anchors.verticalCenter: parent.verticalCenter
        x: field.activeFocus ? 14 : (pillRoot.width - contentRow.width) / 2

        Behavior on x {
            NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
        }

        SafariIcon {
            kind: "lock"
            width: 12
            height: 12
            strokeColor: "#6E6E73"
            anchors.verticalCenter: parent.verticalCenter
            visible: field.text.length > 0
        }



        TextInput {
            id: field

    
    
            width: field.activeFocus ? pillRoot.width - 28 : implicitWidth + 2
            Behavior on width {
                NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
            }

            font.pixelSize: 13
            color: "#1D1D1F"
            clip: true
            selectByMouse: true
    
    
            horizontalAlignment: field.activeFocus
                                 ? TextInput.AlignLeft : TextInput.AlignHCenter
            verticalAlignment: TextInput.AlignVCenter
            selectionColor: Qt.rgba(0, 122 / 255, 1.0, 0.30)
            selectedTextColor: "#000000"

    
    
            onActiveFocusChanged: {
                if (activeFocus && text.length > 0) {
                    selectAll()
                }
            }

            Keys.onEscapePressed: {
                text = ""
                focus = false
            }

            onAccepted: {
                pillRoot.backend.url = field.text
                focus = false
            }
        }

        Text {
    
    
            text: qsTr("Search or enter website name")
            visible: field.text.length === 0 && !field.activeFocus
            font.pixelSize: 13
            color: "#86868B"
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.IBeamCursor
        onPressed: {
            field.forceActiveFocus()
            if (field.text.length > 0) {
                field.selectAll()
            }
        }
    }

    Connections {
        target: pillRoot.backend
        function onUrlChanged(url) {
            if (!field.activeFocus) {
                field.text = url
            }
        }
    }
}
