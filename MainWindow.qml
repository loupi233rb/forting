import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    visible: true
    width: 480
    height: 320
    title: "基础 QML 示例"
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 12
        Text {
            font.pixelSize: 28
            text: "value= "+ f.value
        }
        RowLayout {
            spacing: 8

            Button {text: "+1";onClicked: f.add1()}
            Button {text: "-1";onClicked: f.sub1()}
            Button {text: "*10";onClicked: f.mul10()}
            Button {text: "/10";onClicked: f.div10()}
        }
    }
}