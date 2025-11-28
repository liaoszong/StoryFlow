import QtQuick
import QtQuick.Window
import "./Components/LeftPage"
import "./Components/RightPage"
import "./Components/TopPage"

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

    // 主内容容器 - 带圆角的矩形
    Rectangle {
        id: backgroundRect
        anchors.fill: parent
        radius: 20  // 设置圆角半径
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
            currentPage: mainWindow.currentPage
            selectedStyle: mainWindow.selectedStyle
            storyText: mainWindow.storyText

            onStyleSelected: function(style) {
                mainWindow.selectedStyle = style
            }

            onGenerateStory: {
                // 模拟生成成功，导航到Storyboard
                console.log("生成故事:", storyText, "风格:", selectedStyle)
                mainWindow.currentPage = "storyboard"
            }

            // 添加导航信号处理
            onNavigateTo: function(page) {
                mainWindow.currentPage = page
            }
        }
    }
}

