import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2 as OriginalDialogs

import "controls-uit"
import "styles-uit"
import "windows"

ModalWindow {
    id: root
    objectName: "ConnectionFailureDialog"
    HifiConstants { id: hifi }
    visible: false
    destroyOnCloseButton: false
    destroyOnHidden: false
    width: 680
    height: 200

    property int icon: OriginalDialogs.StandardIcon.NoIcon
    property string iconText: ""
    property int iconSize: 50

    Item {
        id: messageBox
        clip: true
        width: pane.width
        height: pane.height
        QtObject {
            id: d
            readonly property int minWidth: 480
            readonly property int maxWidth: 1280
            readonly property int minHeight: 120
            readonly property int maxHeight: 720
        }


        RalewaySemiBold {
            id: mainTextContainer

            wrapMode: Text.WordWrap
            text: "No Connection"

            size: hifi.fontSizes.menuItem
            color: hifi.colors.baseGrayHighlight
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                margins: 0
                topMargin: hifi.dimensions.contentSpacing.y
            }
            lineHeight: 2
            lineHeightMode: Text.ProportionalHeight
            horizontalAlignment: Text.AlignHCenter
        }
        RalewaySemiBold {
            id: informativeTextContainer
            height:100
            width:500
            text: "Unable to connect to this domain. Click the 'GO TO' button on the toolbar to visit another domain."
            wrapMode: Text.WordWrap
            size: hifi.fontSizes.menuItem
            color: hifi.colors.baseGrayHighlight
            anchors {
                top: mainTextContainer.bottom
                left: parent.left
                right: parent.right
                margins: 0
                topMargin: hifi.dimensions.contentSpacing.y
            }
        }


        Button {
            id: button
            text: qsTr("OK")
            width: 160
            anchors {
                top: informativeTextContainer.bottom
                horizontalCenter: parent.horizontalCenter
            }
            onClicked: { root.visible = false }
        }
    }
}
