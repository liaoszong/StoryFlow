import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: createPage
    color: "transparent"

    // 属性
    property string selectedStyle: ""
    property string storyText: ""
    property var currentProjectData: null

    // 信号
    signal styleSelected(string style)
    signal generateStory()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 40
        spacing: 30

        // 页面标题
        Text {
            text: "Create"
            font.pixelSize: 32
            font.weight: Font.Bold
            color: "#333333"
            Layout.alignment: Qt.AlignHCenter
        }

        // 故事输入区域
        ColumnLayout {
            width: parent.width
            spacing: 15

            Text {
                text: "输入您的故事"
                font.pixelSize: 18
                font.weight: Font.Medium
                color: "#333333"
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 200

                TextArea {
                    id: storyInput
                    placeholderText: "在这里输入您的故事内容...\n例如：在一个遥远的王国里，勇敢的骑士踏上了寻找神秘宝藏的冒险旅程..."
                    font.pixelSize: 16
                    wrapMode: TextArea.Wrap
                    background: Rectangle {
                        color: "#FAFAFA"
                        border.color: "#E0E0E0"
                        border.width: 1
                        radius: 8
                    }

                    onTextChanged: {
                        createPage.storyText = text
                    }
                }
            }
        }

        // 风格选择区域
        ColumnLayout {
            width: parent.width
            spacing: 15

            Text {
                text: "选择视频风格"
                font.pixelSize: 18
                font.weight: Font.Medium
                color: "#333333"
            }

            RowLayout {
                spacing: 20

                // 电影风格
                Rectangle {
                    id: filmStyle
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 120
                    color: createPage.selectedStyle === "film" ? "#E3F2FD" : "#FAFAFA"
                    border.color: createPage.selectedStyle === "film" ? "#1976D2" : "#E0E0E0"
                    border.width: createPage.selectedStyle === "film" ? 2 : 1
                    radius: 12

                    Column {
                        anchors.centerIn: parent
                        spacing: 8

                        Text {
                            text: "🎬"
                            font.pixelSize: 24
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "电影"
                            font.pixelSize: 14
                            font.weight: Font.Medium
                            color: createPage.selectedStyle === "film" ? "#1976D2" : "#666666"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            createPage.selectedStyle = "film"
                            createPage.styleSelected("film")
                        }
                    }
                }

                // 动画风格
                Rectangle {
                    id: animationStyle
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 120
                    color: createPage.selectedStyle === "animation" ? "#E3F2FD" : "#FAFAFA"
                    border.color: createPage.selectedStyle === "animation" ? "#1976D2" : "#E0E0E0"
                    border.width: createPage.selectedStyle === "animation" ? 2 : 1
                    radius: 12

                    Column {
                        anchors.centerIn: parent
                        spacing: 8

                        Text {
                            text: "🖌️"
                            font.pixelSize: 24
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "动画"
                            font.pixelSize: 14
                            font.weight: Font.Medium
                            color: createPage.selectedStyle === "animation" ? "#1976D2" : "#666666"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            createPage.selectedStyle = "animation"
                            createPage.styleSelected("animation")
                        }
                    }
                }

                // 写实风格
                Rectangle {
                    id: realisticStyle
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 120
                    color: createPage.selectedStyle === "realistic" ? "#E3F2FD" : "#FAFAFA"
                    border.color: createPage.selectedStyle === "realistic" ? "#1976D2" : "#E0E0E0"
                    border.width: createPage.selectedStyle === "realistic" ? 2 : 1
                    radius: 12

                    Column {
                        anchors.centerIn: parent
                        spacing: 8

                        Text {
                            text: "📷"
                            font.pixelSize: 24
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "写实"
                            font.pixelSize: 14
                            font.weight: Font.Medium
                            color: createPage.selectedStyle === "realistic" ? "#1976D2" : "#666666"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            createPage.selectedStyle = "realistic"
                            createPage.styleSelected("realistic")
                        }
                    }
                }
            }
        }

        // 生成故事
        Rectangle{
            id: generatestory
            width: 100
            height: 40
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 20
            color: createPage.storyText.length > 0 ?
                              (mouse_gstory.containsMouse ? "#1565C0" : "#1976D2")
                                                          : "#CCCCCC"
            radius: 8
            Text {
                id: t_generatestory
                text: "生成故事"
                font.pixelSize: 16
                font.weight: Font.Medium
                color: "white"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
            MouseArea{
                id: mouse_gstory
                hoverEnabled: true
                anchors.fill: parent
                onClicked: {
                    if (createPage.storyText.length > 0) {
                        createPage.generateStory()
                    }
                }
            }
        }
    }
}
