import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: createPage
    color: "#F0F2F5"
    bottomRightRadius: 20

    // =========================================================
    // 1. UI 状态 (View State)
    // =========================================================
    // 默认选中 'animation'，避免空值
    property string selectedStyle: "animation"
    property string storyText: ""

    // 兼容 RightPage 的传参（如果有）
    property var currentProjectData: null

    // =========================================================
    // 2. 信号 (Signals)
    // =========================================================
    // 保留这个信号仅用于 UI 内部状态同步（如果父组件需要感知）
    // 如果父组件不关心具体选了啥，这个也可以删掉
    signal styleSelected(string style)

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

                    // 直接绑定外部属性，方便双向同步
                    text: createPage.storyText

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

                // 封装一个简单的 StyleCard 组件避免代码重复 (可选优化，这里先保持直观)

                // 1. 电影风格
                StyleCard {
                    emoji: "🎬"
                    label: "电影"
                    styleValue: "film"
                    isSelected: createPage.selectedStyle === "film"
                    onClicked: {
                        createPage.selectedStyle = "film"
                        createPage.styleSelected("film")
                    }
                }

                // 2. 动画风格
                StyleCard {
                    emoji: "🖌️"
                    label: "动画"
                    styleValue: "animation"
                    isSelected: createPage.selectedStyle === "animation"
                    onClicked: {
                        createPage.selectedStyle = "animation"
                        createPage.styleSelected("animation")
                    }
                }

                // 3. 写实风格
                StyleCard {
                    emoji: "📷"
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

        // 生成故事按钮
        Button {
            id: generateBtn
            width: 120
            height: 44
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 20

            text: "生成故事"

            // 绑定 enabled 状态
            enabled: createPage.storyText.length > 0 && !storyViewModel.isGenerating

            background: Rectangle {
                radius: 8
                color: {
                    if (!generateBtn.enabled) return "#CCCCCC";
                    if (generateBtn.down) return "#0D47A1";
                    if (generateBtn.hovered) return "#1565C0";
                    return "#1976D2";
                }
            }

            contentItem: Text {
                text: generateBtn.text
                font.pixelSize: 16
                font.weight: Font.Medium
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            // 直接调用 ViewModel
            onClicked: {
                console.log("UI: 请求生成故事 ->", createPage.selectedStyle);
                storyViewModel.createStory(createPage.storyText, createPage.selectedStyle);
            }
        }
    }

    // 内部组件,把重复的样式卡片提取出来
    // 如果你不想单独建文件，可以在这里定义内联组件
    component StyleCard : Rectangle {
        property string emoji
        property string label
        property string styleValue
        property bool isSelected
        signal clicked()

        Layout.preferredWidth: 120
        Layout.preferredHeight: 120
        color: isSelected ? "#E3F2FD" : "#FAFAFA"
        border.color: isSelected ? "#1976D2" : "#E0E0E0"
        border.width: isSelected ? 2 : 1
        radius: 12

        Column {
            anchors.centerIn: parent
            spacing: 8
            Text {
                text: emoji
                font.pixelSize: 24
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: label
                font.pixelSize: 14
                font.weight: Font.Medium
                color: isSelected ? "#1976D2" : "#666666"
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }
    }
}
