import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    visible: true
    width: 480
    height: 320
    title: "基础 QML 示例"

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: f
            delegate: Rectangle {
                width: ListView.view.width
                height: 44
                color: (index % 2 === 0) ? "#f6f6f6" : "#ffffff"
                border.color: "#dddddd"
            }
        }
    }
}