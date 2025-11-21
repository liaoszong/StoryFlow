import QtQuick
import QtQuick.Controls

// 左侧导航：垂直排列（宽度240px）
Rectangle{
    id: home_left
    width: 240
    anchors.top: home_top.bottom
    anchors.left: parent.left
    anchors.bottom: parent.bottom
    anchors.right: home_right.left
    color: "#E9EEF6"  // 淡蓝

    // 属性
    property string currentPage: "create"

    // 信号
    signal navigateTo(string page)



    // 垂直导航项
    Column {
        anchors.top: parent.top
        anchors.topMargin: 40
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 8

        // Create 导航项
        Rectangle {
            id: createNav
            width: 0.8*parent.width
            height: 50
            anchors.horizontalCenter: parent.horizontalCenter
            radius: 10
            color: currentPage === "create" ? "#D3E3FD" :
                    createMouseArea.containsMouse ? "#DCE1E9" : "transparent"

            Row {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 20
                spacing: 12

                Rectangle {
                    width: 24
                    height: 24
                    color: "transparent"
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        text: "📝"
                        font.pixelSize: 16
                        anchors.centerIn: parent
                    }
                }

                Text {
                    text: "Create"
                    font.pixelSize: 16
                    color: currentPage === "create" ? "#5775A9" : "#444746"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            MouseArea {
                id: createMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    home_left.navigateTo("create")
                    home_right.navigateTo("create")
                }
            }
        }

        // Assets 导航项
        Rectangle {
            id: assetsNav
            width: 0.8*parent.width
            height: 50
            anchors.horizontalCenter: parent.horizontalCenter
            radius: 10
            color: currentPage === "assets" ? "#D3E3FD" :
                    assetsMouseArea.containsMouse ? "#DCE1E9" : "transparent"

            Row {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 20
                spacing: 12

                Rectangle {
                    width: 24
                    height: 24
                    color: "transparent"
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        text: "📁"
                        font.pixelSize: 16
                        anchors.centerIn: parent
                    }
                }

                Text {
                    text: "Assets"
                    font.pixelSize: 16
                    color: currentPage === "assets" ? "#5775A9" : "#444746"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            MouseArea {
                id: assetsMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    home_left.navigateTo("assets")
                    home_right.navigateTo("assets")
                }
            }
        }
    }
}
