import QtQuick

// IconButton is a Safari-style toolbar button: a borderless hit area with a
// thin vector glyph that gains a soft rounded-square highlight on hover.
//
// All current buttons are visual chrome only -- the browser core has no
// history/tabs/share functionality yet (Phase 10 in the roadmap) -- so the
// toolbar instantiates them with enabled: false, which dims the glyph to
// match how macOS renders disabled toolbar items.
Rectangle {
    id: root

    // Which SafariIcon glyph to draw ("back", "forward", "share", ...).
    property string kind: "plus"
    // Emitted on click so real functionality can be wired in later with
    // onActivated: ... without changing this component.
    signal activated()

    width: 30
    height: 26
    radius: 7
    // Qt.rgba() builds a color from 0..1 float channels; here it is a very
    // light overlay of black so the highlight works on the grey toolbar.
    color: !root.enabled ? "transparent"
          : mouse.pressed ? Qt.rgba(0, 0, 0, 0.12)
          : mouse.containsMouse ? Qt.rgba(0, 0, 0, 0.07)
          : "transparent"
    // Disabled toolbar items render at reduced opacity, like macOS.
    opacity: root.enabled ? 1.0 : 0.35

    Behavior on color {
        ColorAnimation { duration: 100 }
    }

    SafariIcon {
        anchors.centerIn: parent
        kind: root.kind
    }

    // MouseArea is QML's basic pointer-input handler. hoverEnabled makes it
    // report containsMouse without a button held down, which a plain
    // onClicked handler would not give us.
    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.activated()
    }
}
