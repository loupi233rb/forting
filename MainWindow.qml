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
        width: 300

        TextField {
            id: nameInput
            Layout.fillWidth: true
            placeholderText: "请输入文本"
        }

        ComboBox {
            id: optionBox
            Layout.fillWidth: true
            model: ["选项 A", "选项 B", "选项 C"]
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: "确认"
                Layout.fillWidth: true
                onClicked: {
                    console.log("输入内容:", nameInput.text)
                    console.log("选择项:", optionBox.currentText)
                }
            }

            Button {
                text: "清空"
                Layout.fillWidth: true
                onClicked: {
                    nameInput.text = ""
                    optionBox.currentIndex = 0
                }
            }
        }

        Text {
            id: resultText
            Layout.fillWidth: true
            text: "请填写信息后点击确认"
            wrapMode: Text.WordWrap
        }
    }
}