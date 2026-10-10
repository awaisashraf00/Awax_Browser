import QtQuick 2.15
import QtQuick.Window 2.15


Window {
    id: appWindow
    width: 1280
    height: 1280
    visible: true
    title: "Awax"
    
    SearchArea{
        id: searchArea
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }
}
