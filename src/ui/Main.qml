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

    // ==================== 错误提示弹窗 ====================
    Rectangle {
        id: errorToast
        width: toastContent.width + 48
        height: toastContent.height + 28
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: errorToast.opacity > 0 ? 24 : -100
        radius: 12
        color: "#FEF2F2"
        border.width: 1
        border.color: "#FECACA"
        visible: opacity > 0
        opacity: 0
        z: 1000

        Behavior on anchors.topMargin {
            NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
        }

        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }

        Row {
            id: toastContent
            anchors.centerIn: parent
            spacing: 12

            // 错误图标
            Text {
                text: "⚠"
                font.pixelSize: 20
                color: "#DC2626"
                anchors.verticalCenter: parent.verticalCenter
            }

            // 错误信息
            Text {
                id: errorText
                text: ""
                font.pixelSize: 14
                color: "#991B1B"
                anchors.verticalCenter: parent.verticalCenter
                wrapMode: Text.NoWrap
            }

            // 关闭按钮
            Rectangle {
                width: 24
                height: 24
                radius: 12
                color: closeArea.containsMouse ? "#FEE2E2" : "transparent"
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    anchors.centerIn: parent
                    text: "✕"
                    font.pixelSize: 12
                    color: "#DC2626"
                }

                MouseArea {
                    id: closeArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: hideErrorToast()
                }
            }
        }
    }

    // 显示错误提示函数
    function showErrorToast(message) {
        errorText.text = message
        errorToast.opacity = 1
        errorHideTimer.restart()
    }

    // 隐藏错误提示函数
    function hideErrorToast() {
        errorToast.opacity = 0
    }

    // 自动隐藏定时器
    Timer {
        id: errorHideTimer
        interval: 5000
        onTriggered: hideErrorToast()
    }

    // 连接 ViewModel 错误信号
    Connections {
        target: storyViewModel
        function onErrorOccurred(message) {
            showErrorToast(message)
        }
    }

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
