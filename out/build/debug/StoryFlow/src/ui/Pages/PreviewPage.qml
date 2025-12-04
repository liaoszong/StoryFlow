// StoryboardPage.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: storyboardPage
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
            text: "Storyboard"
            font.pixelSize: 32
            font.weight: Font.Bold
            color: "#333333"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "故事生成成功！这是您的分镜预览："
            font.pixelSize: 16
            color: "#666666"
            Layout.alignment: Qt.AlignHCenter
        }

        // 三个分镜区域
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 20

            // 分镜1
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#E3F2FD"
                radius: 12
                border.color: "#BBDEFB"
                border.width: 2

                Column {
                    anchors.centerIn: parent
                    spacing: 10

                    Text {
                        text: "🎬"
                        font.pixelSize: 32
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "分镜 1"
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        color: "#1976D2"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "开场场景"
                        font.pixelSize: 14
                        color: "#666666"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }

            // 分镜2
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#F3E5F5"
                radius: 12
                border.color: "#E1BEE7"
                border.width: 2

                Column {
                    anchors.centerIn: parent
                    spacing: 10

                    Text {
                        text: "⚔️"
                        font.pixelSize: 32
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "分镜 2"
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        color: "#7B1FA2"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "冒险开始"
                        font.pixelSize: 14
                        color: "#666666"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }

            // 分镜3
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#E8F5E8"
                radius: 12
                border.color: "#C8E6C9"
                border.width: 2

                Column {
                    anchors.centerIn: parent
                    spacing: 10

                    Text {
                        text: "🏆"
                        font.pixelSize: 32
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "分镜 3"
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        color: "#388E3C"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "胜利时刻"
                        font.pixelSize: 14
                        color: "#666666"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }
        }

        // 返回按钮
        Button {
            text: "返回创建页面"
            Layout.alignment: Qt.AlignHCenter

            background: Rectangle {
                color: parent.down ? "#E0E0E0" :
                       parent.hovered ? "#F5F5F5" : "#FAFAFA"
                border.color: "#E0E0E0"
                border.width: 1
                radius: 8
            }

            contentItem: Text {
                text: parent.text
                font.pixelSize: 14
                color: "#666666"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                home_right.navigateTo("create")
            }
        }
    }
}
