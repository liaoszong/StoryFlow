// AssetsPage.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: assetPage
    // 属性
    property string selectedStyle: ""
    property string storyText: ""

    // 信号,传递给RightPage
    signal styleSelected(string style)
    signal generateStory()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 40
        spacing: 30

        Text {
            text: "Assets"
            font.pixelSize: 32
            font.weight: Font.Bold
            color: "#333333"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "资源库页面\n(功能开发中)"
            font.pixelSize: 18
            color: "#666666"
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
        }

        // 占位内容
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#FAFAFA"
            radius: 12
            border.color: "#E0E0E0"
            border.width: 1

            Text {
                text: "这里将显示您生成的所有项目"
                font.pixelSize: 16
                color: "#999999"
                anchors.centerIn: parent
            }
        }
    }
}
