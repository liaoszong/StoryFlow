import QtQuick
import QtQuick.Window
import QtQuick.Effects
import "./Components"

Window {
    id: mainWindow
    width: 1280
    height: 720
    minimumWidth: 800
    minimumHeight: 600
    visible: true
    title: qsTr("StoryFlow")
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.Window

    // 全局状态
    property string currentPage: "create"
    property string selectedStyle: ""
    property string storyText: ""

    // 主内容容器
    Rectangle {
        id: backgroundRect
        anchors.fill: parent
        radius: 16
        color: "#FFFFFF"

        // 微妙的边框
        border.width: 1
        border.color: Qt.rgba(0, 0, 0, 0.08)

        // 内阴影效果模拟
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.width: 1
            border.color: Qt.rgba(255, 255, 255, 0.8)
            z: -1
        }

        // 顶部栏
        TopPage {
            id: home_top
            height: 56
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
        }

        // 左侧导航
        LeftPage {
            id: home_left
            width: 220
            anchors.top: home_top.bottom
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            currentPage: mainWindow.currentPage
            onNavigateTo: function(page) {
                mainWindow.currentPage = page
            }
        }

        // 右侧主内容区域
        RightPage {
            id: home_right
            anchors.top: home_top.bottom
            anchors.left: home_left.right
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            // 属性绑定 (Props Down)
            currentPage: mainWindow.currentPage
            selectedStyle: mainWindow.selectedStyle
            storyText: mainWindow.storyText

            // 仅保留导航信号
            onNavigateTo: function(page) {
                mainWindow.currentPage = page
            }
        }
    }
}
