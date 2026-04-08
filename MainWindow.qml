import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FortingStruct 1.0

ApplicationWindow {
    visible: true
    width: 480
    height: 320
    title: "Forting"

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: "正序"
                onClicked: {
                    f.sort_by(Forting.name, Forting.Asc)
                }
            }

            Button {
                text: "倒序"
                onClicked: {
                    f.sort_by(Forting.name, Forting.Desc)
                }
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: f

            delegate: Rectangle {
                width: ListView.view.width
                height: 20
                color: (index % 2 === 0) ? "#f6f6f6" : "#ffffff"
                border.color: "#dddddd"

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: name
                }
            }
        }
    }
}