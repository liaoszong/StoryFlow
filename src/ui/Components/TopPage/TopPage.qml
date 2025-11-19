import QtQuick
import QtQuick.Controls

// 顶部栏：带页面名称+右侧菜单（高度60px）
Rectangle{
    id: home_top
    height: 60
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    color: "#F5F5F0"  // 奶白
    border.color: "#EAEAEA"
    border.width: 1

    // 属性
    property string currentPage: "create"

    // 信号
    signal navigateTo(string page)
    signal minimizeClicked()
    signal maximizeClicked()
    signal closeClicked()

    //窗口拖动及放大还原功能
    MouseArea {
        anchors.fill: parent

        onPressed: function(mouse) {
            // 开始系统级窗口移动
            mainWindow.startSystemMove()
        }
        // 双击最大化/还原
        onDoubleClicked: {
            if (mainWindow.visibility === Window.Maximized) {
                mainWindow.showNormal()
            } else {
                mainWindow.showMaximized()
            }
        }
    }

    // 应用Logo
    Text {
        id: appTitle
        text: "StoryFlow"
        font.pixelSize: 20
        font.weight: Font.Bold
        color: "#333333"
        anchors.verticalCenter: parent.verticalCenter
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 20
        anchors.leftMargin: 0.02*mainWindow.width
    }


    // 窗口控制按钮 - 放在右上角
    Row {
        anchors.right: parent.right
        anchors.rightMargin: 0.02*mainWindow.width
        anchors.verticalCenter: parent.verticalCenter
        spacing: 15

        // 最小化按钮
        Rectangle {
            id: minBtn
            width: 30
            height: 30
            color: "transparent"
            border.color: "transparent" // 默认无边框
            border.width: 1

            Image {
                id: minImg
                anchors.centerIn: parent
                source: "/img/Resources/img/minus_sign.png"
                sourceSize: Qt.size(16, 16)
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: mainWindow.showMinimized()
                onEntered: {
                    minBtn.border.color = "#E0E0E0" // 鼠标悬停时显示淡灰色边框
                }
                onExited: {
                    minBtn.border.color = "transparent" // 鼠标离开时隐藏边框
                }
            }
        }
        // 全屏或窗口
        Rectangle {
            id: drec
            width: 30
            height: 30
            color: "transparent"
            border.color: "transparent" // 默认无边框
            border.width: 1
            Image {
                id: dsquare
                anchors.centerIn: parent
                source: "/img/Resources/img/Double_square.png"
                sourceSize: Qt.size(16, 16)
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    if (mainWindow.visibility === Window.Maximized) {
                        mainWindow.showNormal()
                    } else {
                        mainWindow.showMaximized()
                    }
                }
                onEntered: parent.border.color = "#E0E0E0"
                onExited: parent.border.color = "transparent"
            }
        }

        // 关闭按钮
        Rectangle {
            id: closerec
            width: 30
            height: 30
            color: "transparent"
            border.color: "transparent" // 默认无边框
            border.width: 1

            Image {
                id: closeImg
                anchors.centerIn: parent
                source: "/img/Resources/img/multiplication_sign.png"
                sourceSize: Qt.size(16, 16)
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: mainWindow.close()
                onEntered: {
                    parent.border.color = "#E0E0E0"
                }
                onExited: {
                    parent.border.color = "transparent"
                }
            }
        }
    }
}
