import QtQuick

// BrowserToolbar reproduces Safari's single-row toolbar: navigation chevrons
// on the left, the centered address pill, share/new-tab/tab-overview glyphs
// on the right, and a 1px hairline separating it from the page area.
//
// Only the address pill is functional today; the surrounding buttons are
// disabled chrome placeholders until history (Phase 10) and tabs exist.
Rectangle {
    id: toolbarRoot

    // The C++ URL object exposed as "Main_Url" by main(); forwarded to the
    // AddressBar so navigation goes through the same path as before.
    property var backend

    height: 52
    color: "#F4F3F1"

    Row {
        id: navButtons
        spacing: 4
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter

        // Disabled: NavigationController/history does not exist yet.
        IconButton { kind: "back"; enabled: false }
        IconButton { kind: "forward"; enabled: false }
    }

    // Self-positioning: AddressBar centers and sizes itself inside the
    // toolbar and animates its expansion when focused.
    AddressBar {
        id: addressBar
        backend: toolbarRoot.backend
    }

    Row {
        id: toolButtons
        spacing: 4
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter

        // Disabled: no share/new-tab/tab-overview features yet.
        IconButton { kind: "share"; enabled: false }
        IconButton { kind: "plus"; enabled: false }
        IconButton { kind: "tabs"; enabled: false }
    }

    // Hairline: a 1px translucent black line, the standard macOS way to
    // separate a toolbar from the content below it.
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: Qt.rgba(0, 0, 0, 0.15)
    }
}
