import QtQuick

// SafariIcon draws one small, Safari-style line icon ("back", "forward",
// "share", "plus", "tabs" or "lock") as vector strokes.
//
// Canvas is a built-in QML type that gives us an immediate-mode 2D drawing
// surface (like a small <canvas> from the web). Unlike Rectangle/Text, which
// are retained-mode items the scene graph keeps alive, everything drawn here
// is produced by our onPaint() callback each time we call requestPaint().
// We use it because Safari's toolbar icons are thin geometric glyphs that
// are painful to fake with stacked Rectangles but trivial with a few
// moveTo()/lineTo() calls on a Context2D.
Canvas {
    id: root

    // Which glyph to draw. Kept as a string instead of an enum so the icon
    // set can grow (download, bookmark, ...) without touching the API.
    property string kind: "plus"
    // Stroke color of the glyph; the toolbar passes its own grey here.
    property color strokeColor: "#4A4A4A"
    // Stroke thickness in logical pixels; 1.5 mimics SF Symbols' fine weight.
    property real strokeWidth: 1.5

    width: 18
    height: 18
    antialiasing: true

    onPaint: {
        // Context2D is the drawing API of Canvas: set stroke style/width,
        // build a path, then stroke() it. Coordinates are in this item's
        // local 18x18 grid.
        const ctx = getContext("2d");
        ctx.reset();
        ctx.strokeStyle = root.strokeColor;
        ctx.lineWidth = root.strokeWidth;
        ctx.lineCap = "round";
        ctx.lineJoin = "round";

        switch (root.kind) {
        case "back":
            ctx.beginPath();
            ctx.moveTo(11.5, 4);
            ctx.lineTo(6, 9);
            ctx.lineTo(11.5, 14);
            ctx.stroke();
            break;
        case "forward":
            ctx.beginPath();
            ctx.moveTo(6.5, 4);
            ctx.lineTo(12, 9);
            ctx.lineTo(6.5, 14);
            ctx.stroke();
            break;
        case "plus":
            ctx.beginPath();
            ctx.moveTo(9, 3.5);
            ctx.lineTo(9, 14.5);
            ctx.moveTo(3.5, 9);
            ctx.lineTo(14.5, 9);
            ctx.stroke();
            break;
        case "share":
            // Tray: a rounded-bottom box whose top edge has a gap in the
            // middle where the arrow passes through, iOS share-style.
            ctx.beginPath();
            ctx.moveTo(11.25, 8.5);
            ctx.lineTo(12, 8.5);
            ctx.quadraticCurveTo(14, 8.5, 14, 10.5);
            ctx.lineTo(14, 14);
            ctx.quadraticCurveTo(14, 16, 12, 16);
            ctx.lineTo(6, 16);
            ctx.quadraticCurveTo(4, 16, 4, 14);
            ctx.lineTo(4, 10.5);
            ctx.quadraticCurveTo(4, 8.5, 6, 8.5);
            ctx.lineTo(6.75, 8.5);
            ctx.stroke();
            // Arrow: shaft up + chevron head.
            ctx.beginPath();
            ctx.moveTo(9, 11);
            ctx.lineTo(9, 2.5);
            ctx.moveTo(6.75, 4.75);
            ctx.lineTo(9, 2.5);
            ctx.lineTo(11.25, 4.75);
            ctx.stroke();
            break;
        case "tabs":
            // Two overlapping rounded squares = Safari's tab overview glyph.
            ctx.beginPath();
            traceRoundedRect(ctx, 3.5, 3, 8.5, 8.5, 2);
            traceRoundedRect(ctx, 6, 6.5, 8.5, 8.5, 2);
            ctx.stroke();
            break;
        case "lock":
            // Shackle: the top half of a circle sitting on the body.
            ctx.beginPath();
            ctx.arc(9, 7.75, 2.75, Math.PI, 2 * Math.PI, false);
            ctx.stroke();
            // Body: small rounded rectangle below the shackle.
            ctx.beginPath();
            traceRoundedRect(ctx, 4.75, 7.75, 8.5, 7.5, 2);
            ctx.stroke();
            break;
        }
    }

    
    function traceRoundedRect(ctx, x, y, w, h, r) {
        ctx.moveTo(x + r, y);
        ctx.lineTo(x + w - r, y);
        ctx.quadraticCurveTo(x + w, y, x + w, y + r);
        ctx.lineTo(x + w, y + h - r);
        ctx.quadraticCurveTo(x + w, y + h, x + w - r, y + h);
        ctx.lineTo(x + r, y + h);
        ctx.quadraticCurveTo(x, y + h, x, y + h - r);
        ctx.lineTo(x, y + r);
        ctx.quadraticCurveTo(x, y, x + r, y);
    }

    onKindChanged: requestPaint()
    onStrokeColorChanged: requestPaint()
    onStrokeWidthChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
}
