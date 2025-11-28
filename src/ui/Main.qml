import QtQuick
import QtQuick.Window
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
        radius: 20
        color: "#F9FAFB"
        border.width: 1
        border.color: "#E0E0E0"

        // 顶部栏
        TopPage {
            id: home_top
            height: 60
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
        }

        // 左侧导航
        LeftPage {
            id: home_left
            width: 240
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

            // 【MVVM 修正】删除 onStyleSelected 和 onGenerateStory
            // 因为 RightPage 已经删除了这些信号，保留会导致 crash。
            // 现在的逻辑是：子页面直接调 ViewModel，生成成功后 ViewModel 通知 RightPage 跳转。

            // 仅保留导航信号
            onNavigateTo: function(page) {
                mainWindow.currentPage = page
            }
        }
    }
}
