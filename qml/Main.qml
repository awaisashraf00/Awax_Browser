import QtQuick
import QtQuick.Window

// Top-level window: Safari-style toolbar docked at the top, white page
// viewport below. The viewport is where retrieved documents will eventually
// be displayed once the rendering stage (Phase 8) exists.
Window {
    id: appWindow
    width: 1280
    height: 800
    minimumWidth: 940
    minimumHeight: 600
    visible: true
    title: "Awax"
    color: "#FFFFFF"

    BrowserToolbar {
        id: toolbar
        backend: Main_Url
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    // Empty page surface reserved for the renderer's output.
    Rectangle {
        anchors.top: toolbar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "#FFFFFF"
    }
}
