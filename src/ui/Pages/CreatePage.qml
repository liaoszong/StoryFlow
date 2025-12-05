import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

/**
 * 创建页面 - Create
 * 用户输入故事内容并选择视觉风格
 */
Rectangle {
    id: createPage
    color: "#F8FAFC"
    bottomRightRadius: 16

    // ==================== 属性定义 ====================
    property string selectedStyle: "animation"  // 当前选中的风格
    property string storyText: ""               // 故事文本内容
    property var currentProjectData: null       // 当前项目数据
    property string savePath: ""                // 项目保存路径

    // ==================== 信号定义 ====================
    signal styleSelected(string style)

    // ==================== 文件夹选择对话框 ====================
    FolderDialog {
        id: folderDialog
        title: "选择项目保存位置"
        onAccepted: {
            createPage.savePath = selectedFolder.toString()
        }
    }

    // ==================== 主布局 ====================
    ScrollView {
        anchors.fill: parent
        contentWidth: parent.width
        clip: true

        ColumnLayout {
            width: parent.width
            spacing: 0

            // 页面标题区域
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                color: "transparent"

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 8

                    // 主标题
                    Text {
                        text: "Create"
                        font.pixelSize: 28
                        font.weight: Font.Bold
                        color: "#1E293B"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    // 副标题描述
                    Text {
                        text: "将您的创意转化为精彩的视觉故事"
                        font.pixelSize: 14
                        color: "#64748B"
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            // 内容区域
            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 48
                Layout.rightMargin: 48
                Layout.bottomMargin: 32
                spacing: 24

                // ==================== 故事输入卡片 ====================
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 240
                    radius: 12
                    color: "#FFFFFF"
                    border.width: 1
                    border.color: storyInput.activeFocus ? "#6366F1" : "#E2E8F0"

                    Behavior on border.color {
                        ColorAnimation { duration: 200 }
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 12

                        // 标签栏
                        Row {
                            spacing: 8

                            // 装饰条
                            Rectangle {
                                width: 4
                                height: 18
                                radius: 2
                                color: "#6366F1"
                            }

                            Text {
                                text: "故事内容"
                                font.pixelSize: 15
                                font.weight: Font.DemiBold
                                color: "#1E293B"
                            }
                        }

                        // 文本输入区
                        ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            TextArea {
                                id: storyInput
                                placeholderText: "在这里输入您的故事...\n\n例如：在一个遥远的王国里，勇敢的骑士踏上了寻找神秘宝藏的冒险旅程..."
                                placeholderTextColor: "#94A3B8"
                                font.pixelSize: 14
                                color: "#334155"
                                wrapMode: TextArea.Wrap
                                text: createPage.storyText

                                background: Rectangle {
                                    color: "transparent"
                                }

                                onTextChanged: {
                                    createPage.storyText = text
                                }
                            }
                        }

                        // 字数统计
                        Text {
                            text: storyInput.text.length + " 字符"
                            font.pixelSize: 11
                            color: "#94A3B8"
                            Layout.alignment: Qt.AlignRight
                        }
                    }
                }

                // ==================== 风格选择卡片 ====================
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 180
                    radius: 12
                    color: "#FFFFFF"
                    border.width: 1
                    border.color: "#E2E8F0"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 16

                        // 标签栏
                        Row {
                            spacing: 8

                            Rectangle {
                                width: 4
                                height: 18
                                radius: 2
                                color: "#6366F1"
                            }

                            Text {
                                text: "视觉风格"
                                font.pixelSize: 15
                                font.weight: Font.DemiBold
                                color: "#1E293B"
                            }
                        }

                        // 风格选项
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            // 电影风格
                            StyleCard {
                                emoji: "\uD83C\uDFAC"
                                label: "电影"
                                styleValue: "film"
                                isSelected: createPage.selectedStyle === "film"
                                onClicked: {
                                    createPage.selectedStyle = "film"
                                    createPage.styleSelected("film")
                                }
                            }

                            // 动画风格
                            StyleCard {
                                emoji: "\uD83C\uDFA8"
                                label: "动画"
                                styleValue: "animation"
                                isSelected: createPage.selectedStyle === "animation"
                                onClicked: {
                                    createPage.selectedStyle = "animation"
                                    createPage.styleSelected("animation")
                                }
                            }

                            // 写实风格
                            StyleCard {
                                emoji: "\uD83D\uDCF7"
                                label: "写实"
                                styleValue: "realistic"
                                isSelected: createPage.selectedStyle === "realistic"
                                onClicked: {
                                    createPage.selectedStyle = "realistic"
                                    createPage.styleSelected("realistic")
                                }
                            }
                        }
                    }
                }

                // ==================== 保存路径选择卡片 ====================
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 100
                    radius: 12
                    color: "#FFFFFF"
                    border.width: 1
                    border.color: "#E2E8F0"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 12

                        // 标签栏
                        Row {
                            spacing: 8

                            Rectangle {
                                width: 4
                                height: 18
                                radius: 2
                                color: "#6366F1"
                            }

                            Text {
                                text: "保存位置"
                                font.pixelSize: 15
                                font.weight: Font.DemiBold
                                color: "#1E293B"
                            }
                        }

                        // 路径选择行
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            // 路径显示框
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 40
                                radius: 8
                                color: "#F8FAFC"
                                border.width: 1
                                border.color: "#E2E8F0"

                                Text {
                                    anchors.fill: parent
                                    anchors.leftMargin: 12
                                    anchors.rightMargin: 12
                                    verticalAlignment: Text.AlignVCenter
                                    text: createPage.savePath ? createPage.savePath.replace("file:///", "") : "请选择项目保存位置..."
                                    font.pixelSize: 13
                                    color: createPage.savePath ? "#334155" : "#94A3B8"
                                    elide: Text.ElideMiddle
                                }
                            }

                            // 浏览按钮
                            Button {
                                Layout.preferredWidth: 80
                                Layout.preferredHeight: 40

                                background: Rectangle {
                                    radius: 8
                                    color: parent.down ? "#E0E7FF" : (parent.hovered ? "#EEF2FF" : "#F1F5F9")
                                    border.width: 1
                                    border.color: parent.hovered ? "#6366F1" : "#E2E8F0"
                                }

                                contentItem: Text {
                                    text: "浏览..."
                                    font.pixelSize: 13
                                    font.weight: Font.Medium
                                    color: "#4F46E5"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                onClicked: folderDialog.open()
                            }
                        }
                    }
                }

                // ==================== 生成按钮 ====================
                Button {
                    id: generateBtn
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 180
                    Layout.preferredHeight: 48
                    Layout.topMargin: 8

                    // 状态绑定：必须有故事内容且选择了保存路径
                    enabled: createPage.storyText.length > 0 && createPage.savePath.length > 0 && !storyViewModel.isGenerating

                    // 强制开启悬停，确保颜色变化生效
                    hoverEnabled: true

                    // 1. 背景修复：移除白色遮罩，直接用渐变
                    background: Rectangle {
                        radius: 10

                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop {
                                position: 0.0
                                // 逻辑：禁用(灰) -> 按下(深蓝) -> 悬停(中蓝) -> 默认(亮蓝)
                                color: !generateBtn.enabled ? "#CBD5E1" :
                                                              (generateBtn.down ? "#4338CA" :
                                                                                  (generateBtn.hovered ? "#4F46E5" : "#6366F1"))
                            }
                            GradientStop {
                                position: 1.0
                                color: !generateBtn.enabled ? "#CBD5E1" :
                                                              (generateBtn.down ? "#6D28D9" :
                                                                                  (generateBtn.hovered ? "#7C3AED" : "#8B5CF6"))
                            }
                        }
                    }

                    // 2. 用 Text 以保证完美居中
                    contentItem: Text {
                        text: storyViewModel.isGenerating ? "生成中..." : "生成故事"
                        font.pixelSize: 15
                        font.weight: Font.DemiBold
                        color: "#FFFFFF"

                        // 对齐属性
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }

                    onClicked: {
                        storyViewModel.createStory(createPage.storyText, createPage.selectedStyle, createPage.savePath)
                    }
                }
            }
        }
    }

    // ==================== 风格卡片组件 ====================
    component StyleCard : Rectangle {
        property string emoji       // 图标
        property string label       // 标签文字
        property string styleValue  // 风格值
        property bool isSelected    // 是否选中

        signal clicked()

        Layout.fillWidth: true
        Layout.preferredHeight: 90
        radius: 10
        color: {
            if (isSelected) return "#EEF2FF"
            if (cardMouse.containsMouse) return "#F8FAFC"
            return "#FFFFFF"
        }
        border.width: isSelected ? 2 : 1
        border.color: isSelected ? "#6366F1" : "#E2E8F0"

        Column {
            anchors.centerIn: parent
            spacing: 6

            // 图标
            Text {
                text: emoji
                font.pixelSize: 24
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // 标签
            Text {
                text: label
                font.pixelSize: 13
                font.weight: Font.DemiBold
                color: isSelected ? "#4F46E5" : "#1E293B"
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        // 选中标记
        Rectangle {
            visible: isSelected
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 6
            width: 18
            height: 18
            radius: 9
            color: "#6366F1"

            Text {
                anchors.centerIn: parent
                text: "\u2713"
                font.pixelSize: 10
                font.weight: Font.Bold
                color: "#FFFFFF"
            }
        }

        MouseArea {
            id: cardMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }
    }
}
