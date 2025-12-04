import QtQuick
import QtQuick.Controls

/**
 * 顶部导航栏组件
 * - 包含应用 Logo 和标题
 * - 窗口拖拽移动功能
 * - 最小化、最大化、关闭按钮
 */
Rectangle {
    id: home_top
    height: 56
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    topLeftRadius: 16
    topRightRadius: 16
    color: "#FFFFFF"

    // 底部分割线
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: "#F0F0F0"
    }

    // 窗口拖动区域
    MouseArea {
        anchors.fill: parent

        onPressed: function(mouse) {
            mainWindow.startSystemMove()
        }

        // 双击切换最大化/还原
        onDoubleClicked: {
            if (mainWindow.visibility === Window.Maximized) {
                mainWindow.showNormal()
            } else {
                mainWindow.showMaximized()
            }
        }
    }

    // Logo 区域
    Row {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 24
        spacing: 10

        // Logo 图标 - 渐变色背景
        Rectangle {
            width: 32
            height: 32
            radius: 8
            anchors.verticalCenter: parent.verticalCenter

            gradient: Gradient {
                GradientStop { position: 0.0; color: "#667EEA" }
                GradientStop { position: 1.0; color: "#764BA2" }
            }

            Text {
                anchors.centerIn: parent
                text: "S"
                font.pixelSize: 18
                font.weight: Font.Bold
                color: "#FFFFFF"
            }
        }

        // 应用标题
        Text {
            id: appTitle
            text: "StoryFlow"
            font.pixelSize: 18
            font.weight: Font.DemiBold
            color: "#1A1A2E"
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    // 窗口控制按钮组
    Row {
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        spacing: 4

        // 最小化按钮
        Rectangle {
            id: minBtn
            width: 36
            height: 36
            radius: 8
            color: minMouseArea.containsMouse ? "#F3F4F6" : "transparent"

            Image {
                anchors.centerIn: parent
                source: "/img/Resources/img/minus_sign.png"
                sourceSize: Qt.size(14, 14)
                opacity: minMouseArea.containsMouse ? 0.8 : 0.5
            }

            MouseArea {
                id: minMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: mainWindow.showMinimized()
            }
        }

        // 最大化/还原按钮
        Rectangle {
            id: maxBtn
            width: 36
            height: 36
            radius: 8
            color: maxMouseArea.containsMouse ? "#F3F4F6" : "transparent"

            Image {
                anchors.centerIn: parent
                source: "/img/Resources/img/Double_square.png"
                sourceSize: Qt.size(14, 14)
                opacity: maxMouseArea.containsMouse ? 0.8 : 0.5
            }

            MouseArea {
                id: maxMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (mainWindow.visibility === Window.Maximized) {
                        mainWindow.showNormal()
                    } else {
                        mainWindow.showMaximized()
                    }
                }
            }
        }

        // 关闭按钮
        Rectangle {
            id: closeBtn
            width: 36
            height: 36
            radius: 8
            color: closeMouseArea.containsMouse ? "#FEE2E2" : "transparent"

            Image {
                anchors.centerIn: parent
                source: "/img/Resources/img/multiplication_sign.png"
                sourceSize: Qt.size(14, 14)
                opacity: closeMouseArea.containsMouse ? 1.0 : 0.5
            }

            MouseArea {
                id: closeMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: mainWindow.close()
            }
        }
    }
}
