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
    color: "#F8F9FA"  // 浅米白
    border.color: "#EAEAEA"
    border.width: 1

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
            width: parent.width
            height: 50
            color: currentPage === "create" ? "#E3F2FD" :
                    createMouseArea.containsMouse ? "#F5F5F5" : "transparent"

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
                    color: currentPage === "create" ? "#1976D2" : "#666666"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            MouseArea {
                id: createMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: home_left.navigateTo("create")
            }
        }

        // Assets 导航项
        Rectangle {
            id: assetsNav
            width: parent.width
            height: 50
            color: currentPage === "assets" ? "#E3F2FD" :
                    assetsMouseArea.containsMouse ? "#F5F5F5" : "transparent"

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
                    color: currentPage === "assets" ? "#1976D2" : "#666666"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            MouseArea {
                id: assetsMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: home_left.navigateTo("assets")
            }
        }
    }
}
