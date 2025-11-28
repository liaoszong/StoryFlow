import QtQuick
import QtQuick.Controls

/**
 * 左侧导航栏组件
 * - 显示主要功能入口：Create 和 Assets
 * - 支持选中状态高亮
 * - 悬停交互效果
 */
Rectangle {
    id: home_left
    width: 220
    anchors.top: home_top.bottom
    anchors.left: parent.left
    anchors.bottom: parent.bottom
    anchors.right: home_right.left
    bottomLeftRadius: 16
    color: "#FAFBFC"

    // 右侧分割线
    Rectangle {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 1
        color: "#F0F0F0"
    }

    // 当前选中的页面
    property string currentPage: "create"

    // 导航信号
    signal navigateTo(string page)

    // 导航菜单区域
    Column {
        anchors.top: parent.top
        anchors.topMargin: 24
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 6

        // 区域标题
        Text {
            text: "工作区"
            font.pixelSize: 11
            font.weight: Font.DemiBold
            color: "#9CA3AF"
            leftPadding: 12
            bottomPadding: 8
            font.letterSpacing: 1
        }

        // Create 导航项
        Rectangle {
            id: createNav
            width: parent.width
            height: 44
            radius: 10
            color: {
                if (currentPage === "create") return "#EEF2FF"
                if (createMouseArea.containsMouse) return "#F3F4F6"
                return "transparent"
            }

            Row {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 14
                spacing: 12

                // 图标容器
                Rectangle {
                    width: 28
                    height: 28
                    radius: 6
                    color: currentPage === "create" ? "#6366F1" : "transparent"
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        anchors.centerIn: parent
                        text: "\u270F"
                        font.pixelSize: 14
                        color: currentPage === "create" ? "#FFFFFF" : "#6B7280"
                    }
                }

                Text {
                    text: "Create"
                    font.pixelSize: 14
                    font.weight: currentPage === "create" ? Font.DemiBold : Font.Medium
                    color: currentPage === "create" ? "#4F46E5" : "#374151"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // 左侧选中指示条
            Rectangle {
                visible: currentPage === "create"
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                width: 3
                height: 20
                radius: 2
                color: "#6366F1"
            }

            MouseArea {
                id: createMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: home_left.navigateTo("create")
            }
        }

        // Assets 导航项
        Rectangle {
            id: assetsNav
            width: parent.width
            height: 44
            radius: 10
            color: {
                if (currentPage === "assets") return "#EEF2FF"
                if (assetsMouseArea.containsMouse) return "#F3F4F6"
                return "transparent"
            }

            Row {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 14
                spacing: 12

                // 图标容器
                Rectangle {
                    width: 28
                    height: 28
                    radius: 6
                    color: currentPage === "assets" ? "#6366F1" : "transparent"
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        anchors.centerIn: parent
                        text: "\uD83D\uDCC1"
                        font.pixelSize: 14
                        color: currentPage === "assets" ? "#FFFFFF" : "#6B7280"
                    }
                }

                Text {
                    text: "Assets"
                    font.pixelSize: 14
                    font.weight: currentPage === "assets" ? Font.DemiBold : Font.Medium
                    color: currentPage === "assets" ? "#4F46E5" : "#374151"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // 左侧选中指示条
            Rectangle {
                visible: currentPage === "assets"
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                width: 3
                height: 20
                radius: 2
                color: "#6366F1"
            }

            MouseArea {
                id: assetsMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: home_left.navigateTo("assets")
            }
        }
    }

    // 底部版本信息
    Text {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        text: "v1.0.0"
        font.pixelSize: 11
        color: "#D1D5DB"
    }
}
