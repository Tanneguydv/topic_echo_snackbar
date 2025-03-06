import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Rectangle {
    id: topicEchoSnackBar
    objectName: "topicEchoSnackBar"
    Layout.minimumWidth: 300
    Layout.minimumHeight: 300
    color: "transparent"

    property int tooltipDelay: 500
    property int tooltipTimeout: 1000

    Column {
        anchors.fill: parent
        anchors.margins: 10

        Row {
            Column {
                Label {
                    text: "Topic"
                }

                TextField {
                    id: topicField
                    objectName: "topicField"
                    text: TopicEchoSnackBar.topic
                    selectByMouse: true
                }
            }

            Switch {
                id: echoSwitch
                objectName: "echoSwitch"
                text: qsTr("Echo")
                checked: true // Checked by default
                onToggled: {
                    TopicEchoSnackBar.topic = topicField.text
                    TopicEchoSnackBar.OnEcho(checked);
                }
                ToolTip.visible: hovered
                ToolTip.delay: tooltipDelay
                ToolTip.timeout: tooltipTimeout
                ToolTip.text: checked ? qsTr("Stop echoing") : qsTr("Start echoing")
            }
        }

        Label {
            text: "Buffer"
        }

        SpinBox {
            id: bufferField
            objectName: "bufferField"
            value: 10
            onValueChanged: {
                TopicEchoSnackBar.OnBuffer(value)
            }
        }

        CheckBox {
            objectName: "pauseCheck"
            text: qsTr("Pause")
            checked: TopicEchoSnackBar.paused
            onClicked: {
                TopicEchoSnackBar.SetPaused(checked)
            }
        }

        Label {
            id: msgsLabel
            text: "Messages"
        }

        Rectangle {
            width: topicEchoSnackBar.parent !== null ? topicEchoSnackBar.parent.width - 20 : 50
            height: topicEchoSnackBar.parent !== null ? topicEchoSnackBar.parent.height - 200 : 50
            color: "transparent"

            ListView {
                id: listView
                objectName: "listView"
                clip: true
                anchors.fill: parent

                focus: true
                currentIndex: -1

                delegate: ItemDelegate {
                    width: listView.width
                    Label {
                        text: model.display
                        textFormat: Text.RichText
                        anchors.fill: parent
                    }
                }

                model: TopicEchoSnackBarMsgList

                ScrollIndicator.vertical: ScrollIndicator {
                    active: true;
                    onActiveChanged: {
                        active = true;
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        TopicEchoSnackBar.topic = topicField.text;
        TopicEchoSnackBar.OnEcho(echoSwitch.checked); // Call OnEcho after component is loaded
    }
}