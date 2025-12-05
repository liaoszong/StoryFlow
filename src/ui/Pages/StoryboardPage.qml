import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * 分镜预览页面 - Storyboard
 * 展示 AI 生成的故事分镜，支持编辑和生成视频
 */
Rectangle {
    id: storyboardPage
    color: "#F8FAFC"
    bottomRightRadius: 16

    // ==================== 属性定义 ====================
    property string selectedStyle: ""
    property string storyText: ""
    property var projectData: ({ "storyboards": [] })

    // ==================== 信号定义 ====================
    signal navigateTo(string page, var data)

    // ==================== 主布局 ====================
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32
        spacing: 24

        // 页面标题区域
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 8

            Text {
                text: "Storyboard"
                font.pixelSize: 28
                font.weight: Font.Bold
                color: "#1E293B"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "故事已生成！以下是您的分镜预览："
                font.pixelSize: 14
                color: "#64748B"
                Layout.alignment: Qt.AlignHCenter
            }
        }

        // ==================== 分镜列表 ====================
        ListView {
            id: shotList
            Layout.fillWidth: true
            Layout.fillHeight: true

            model: (storyboardPage.projectData && storyboardPage.projectData.storyboards)
                   ? storyboardPage.projectData.storyboards
                   : []

            orientation: ListView.Horizontal
            spacing: 16
            clip: true

            // 分镜卡片代理
            delegate: Rectangle {
                width: 280
                height: shotList.height - 20
                color: "#FFFFFF"
                radius: 12
                border.width: cardHover.containsMouse ? 2 : 1
                border.color: cardHover.containsMouse ? "#6366F1" : "#E2E8F0"

                MouseArea {
                    id: cardHover
                    anchors.fill: parent
                    hoverEnabled: true
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    // 图片预览区域
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 160
                        color: "#F1F5F9"
                        radius: 8
                        clip: true

                        // 图片
                        Image {
                            anchors.fill: parent
                            source: (modelData.localImagePath) ? modelData.localImagePath : ""
                            fillMode: Image.PreserveAspectCrop
                            visible: status === Image.Ready
                            asynchronous: true
                            cache: false
                        }

                        // 占位符
                        Column {
                            anchors.centerIn: parent
                            visible: !modelData.localImagePath
                            spacing: 8

                            Text {
                                text: "\uD83D\uDDBC"
                                font.pixelSize: 32
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: "生成中..."
                                color: "#94A3B8"
                                font.pixelSize: 12
                            }
                        }

                        // 加载指示器
                        BusyIndicator {
                            anchors.centerIn: parent
                            running: modelData.status === "generating"
                            visible: running
                            scale: 0.6
                        }

                        // 状态标签
                        Rectangle {
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.margins: 8
                            width: statusText.width + 16
                            height: 24
                            radius: 12
                            color: getStatusColor(modelData.status)

                            Text {
                                id: statusText
                                anchors.centerIn: parent
                                text: getStatusText(modelData.status)
                                color: getStatusTextColor(modelData.status)
                                font.pixelSize: 10
                                font.weight: Font.Bold
                            }
                        }
                    }

                    // 场景标题
                    Text {
                        text: modelData.sceneTitle || "未命名场景"
                        font.pixelSize: 16
                        font.weight: Font.Bold
                        color: "#1E293B"
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    // 旁白文字
                    Text {
                        text: modelData.narration || "暂无旁白..."
                        font.pixelSize: 13
                        color: "#64748B"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                        maximumLineCount: 3
                    }

                    // 编辑按钮
                    Button {
                        text: "编辑详情"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 36

                        background: Rectangle {
                            color: {
                                if (parent.down) return "#E2E8F0"
                                if (parent.hovered) return "#F1F5F9"
                                return "#F8FAFC"
                            }
                            radius: 8
                            border.width: 1
                            border.color: "#E2E8F0"
                        }

                        contentItem: Text {
                            text: parent.text
                            font.pixelSize: 13
                            font.weight: Font.Medium
                            color: "#475569"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            var shotPayload = {
                                "shotId": modelData.shotId,
                                "sceneTitle": modelData.sceneTitle,
                                "prompt": modelData.prompt,
                                "narration": modelData.narration,
                                "localImagePath": modelData.localImagePath,
                                "status": modelData.status,
                                "transition": modelData.transition || "kenBurns"
                            }
                            storyboardPage.navigateTo("shotDetail", shotPayload)
                        }
                    }
                }
            }
        }

        // ==================== 底部操作栏 ====================
        RowLayout {
            spacing: 16
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 8

            // 返回按钮
            Button {
                text: "返回修改"
                Layout.preferredWidth: 130
                Layout.preferredHeight: 44

                background: Rectangle {
                    color: "transparent"
                    border.color: parent.hovered ? "#94A3B8" : "#CBD5E1"
                    border.width: 1
                    radius: 10
                }

                contentItem: Text {
                    text: parent.text
                    color: "#64748B"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: storyboardPage.navigateTo("create", null)
            }

            // 生成视频按钮
            Button {
                id: genVideoBtn
                text: "生成视频"
                Layout.preferredWidth: 160
                Layout.preferredHeight: 44
                enabled: !storyViewModel.isGenerating

                background: Rectangle {
                    radius: 10

                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop {
                            position: 0.0
                            color: genVideoBtn.enabled ? (genVideoBtn.down ? "#4F46E5" : "#6366F1") : "#CBD5E1"
                        }
                        GradientStop {
                            position: 1.0
                            color: genVideoBtn.enabled ? (genVideoBtn.down ? "#7C3AED" : "#8B5CF6") : "#CBD5E1"
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: parent.radius
                        color: "#FFFFFF"
                        opacity: genVideoBtn.hovered && genVideoBtn.enabled ? 0.1 : 0
                    }
                }

                contentItem: Text {
                    text: parent.text
                    font.weight: Font.DemiBold
                    font.pixelSize: 14
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    // 检查是否有分镜数据
                    if (storyboardPage.projectData && storyboardPage.projectData.storyboards && storyboardPage.projectData.storyboards.length > 0) {
                        console.log("Navigating to preview with projectData:", JSON.stringify(storyboardPage.projectData))
                        // 跳转到预览页，传递完整的项目数据
                        storyboardPage.navigateTo("preview", storyboardPage.projectData)
                    } else {
                        console.error("No storyboards data available")
                    }
                }
            }
        }
    }

    // ==================== 辅助函数 ====================

    // 获取状态背景色
    function getStatusColor(status) {
        switch(status) {
            case "generated": return "#DCFCE7"
            case "generating": return "#DBEAFE"
            default: return "#FEF3C7"
        }
    }

    // 获取状态文字颜色
    function getStatusTextColor(status) {
        switch(status) {
            case "generated": return "#166534"
            case "generating": return "#1D4ED8"
            default: return "#B45309"
        }
    }

    // 获取状态显示文字
    function getStatusText(status) {
        switch(status) {
            case "generated": return "已完成"
            case "generating": return "生成中"
            default: return "等待中"
        }
    }
}
