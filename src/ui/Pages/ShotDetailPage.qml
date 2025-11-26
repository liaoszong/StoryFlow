import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: shotDetailPage
    color: "#F0F2F5"
    bottomRightRadius: 20

    // 接收从 Storyboard 传来的单个分镜数据
    property var shotData: null
    property string projectId: ""
    property string selec_style: ""

    // 信号
    signal navigateTo(string page)
    signal regenerateImage(string shotId, string newPrompt)
    signal updateShotData(var updatedData) // 用于将修改后的数据存回主数据

    // 当 shotData 改变时，刷新界面上的输入框内容
    onShotDataChanged: {
        if (shotData) {
            titleField.text = shotData.sceneTitle || ""
            promptArea.text = shotData.prompt || ""
            narrationArea.text = shotData.narration || ""
            // 设置转场 (简单映射，默认 kenBurns)
            var transitions = ["kenBurns", "crossfade", "volumeMix"]
            var idx = transitions.indexOf(shotData.transition)
            transitionCombo.currentIndex = (idx >= 0) ? idx : 0
        }
    }

    // 顶部导航栏
    RowLayout {
        id: headerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 30
        spacing: 15

        // 返回按钮 (统一风格：白底灰边)
        Button {
            id: backBtn
            text: "← 返回分镜"
            background: Rectangle {
                color: backBtn.down ? "#E0E0E0" : (backBtn.hovered ? "#F5F5F5" : "#FAFAFA")
                border.color: "#E0E0E0"
                border.width: 1
                radius: 8
            }
            contentItem: Text {
                text: parent.text
                font.pixelSize: 14
                color: "#666666"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: {
                // 保存当前修改
                saveCurrentEdits()
                shotDetailPage.navigateTo("storyboard")
            }
        }

        Text {
            text: "编辑分镜详情"
            font.pixelSize: 20
            font.weight: Font.Bold
            color: "#333333"
        }

        Item { Layout.fillWidth: true } // 占位符

        // 状态标签
        Rectangle {
            visible: shotData !== null
            width: 100
            height: 28
            radius: 14
            color: {
                if (!shotData) return "transparent"
                return shotData.status === "generated" ? "#E8F5E9" : (shotData.status === "generating" ? "#E3F2FD" : "#FFF3E0")
            }

            Row {
                anchors.centerIn: parent
                spacing: 6
                Rectangle {
                    width: 8; height: 8; radius: 4
                    color: {
                        if (!shotData) return "transparent"
                        return shotData.status === "generated" ? "#2E7D32" : (shotData.status === "generating" ? "#1565C0" : "#EF6C00")
                    }
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: shotData ? shotData.status.toUpperCase() : ""
                    font.pixelSize: 12
                    font.weight: Font.Bold
                    color: {
                        if (!shotData) return "transparent"
                        return shotData.status === "generated" ? "#2E7D32" : (shotData.status === "generating" ? "#1565C0" : "#EF6C00")
                    }
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    // 主内容区：左图右表单
    RowLayout {
        anchors.top: headerBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 30
        anchors.topMargin: 20
        spacing: 30

        // --- 左侧：图片预览区 ---
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 4 // 比例 4
            color: "#F5F5F5"
            radius: 12
            border.color: "#E0E0E0"
            border.width: 1
            clip: true

            // 图片
            Image {
                anchors.fill: parent
                anchors.margins: 2
                fillMode: Image.PreserveAspectFit
                source: (shotData && shotData.localImagePath) ? shotData.localImagePath : ""
                visible: (shotData && shotData.localImagePath)
                cache: false
            }

            // 占位图
            Column {
                anchors.centerIn: parent
                visible: !(shotData && shotData.localImagePath)
                spacing: 15
                Text { text: "📷"; font.pixelSize: 48; anchors.horizontalCenter: parent.horizontalCenter }
                Text { text: "等待生成图像..."; color: "#999999"; font.pixelSize: 14 }
            }

            // 加载中遮罩
            Rectangle {
                anchors.fill: parent
                color: "#80FFFFFF"
                visible: shotData && shotData.status === "generating"
                BusyIndicator { anchors.centerIn: parent }
            }
        }

        // --- 右侧：编辑表单区 ---
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 3 // 比例 3
            color: "transparent"

            ScrollView {
                anchors.fill: parent
                contentWidth: parent.width
                clip: true

                ColumnLayout {
                    width: parent.width
                    spacing: 20

                    // 1. 标题
                    ColumnLayout {
                        width: parent.width
                        spacing: 8
                        Text { text: "场景标题 (Scene Title)"; font.pixelSize: 14; font.weight: Font.Medium; color: "#666666" }
                        TextField {
                            id: titleField
                            Layout.fillWidth: true
                            font.pixelSize: 16
                            placeholderText: "输入场景标题"
                            background: Rectangle {
                                color: "#FAFAFA"
                                border.color: titleField.activeFocus ? "#1976D2" : "#E0E0E0"
                                radius: 8
                            }
                        }
                    }

                    // 2. 提示词 (Prompt)
                    ColumnLayout {
                        width: parent.width
                        spacing: 8
                        Text { text: "画面描述 (Prompt)"; font.pixelSize: 14; font.weight: Font.Medium; color: "#666666" }
                        TextArea {
                            id: promptArea
                            Layout.fillWidth: true
                            Layout.preferredHeight: 100
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                            placeholderText: "输入用于生成画面的提示词..."
                            background: Rectangle {
                                color: "#FAFAFA"
                                border.color: promptArea.activeFocus ? "#1976D2" : "#E0E0E0"
                                radius: 8
                            }
                        }
                    }

                    // 3. 旁白 (Narration)
                    ColumnLayout {
                        width: parent.width
                        spacing: 8
                        Text { text: "旁白配音 (Narration)"; font.pixelSize: 14; font.weight: Font.Medium; color: "#666666" }
                        TextArea {
                            id: narrationArea
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                            placeholderText: "输入该镜头的旁白台词..."
                            background: Rectangle {
                                color: "#FAFAFA"
                                border.color: narrationArea.activeFocus ? "#1976D2" : "#E0E0E0"
                                radius: 8
                            }
                        }
                    }

                    // 4. 转场效果 (Transition)
                    ColumnLayout {
                        width: parent.width
                        spacing: 8
                        Text { text: "转场效果 (Transition)"; font.pixelSize: 14; font.weight: Font.Medium; color: "#666666" }
                        ComboBox {
                            id: transitionCombo
                            Layout.fillWidth: true
                            model: ["Ken Burns (镜头推拉)", "Crossfade (淡入淡出)", "Volume Mix (音量混合)"]
                            // 自定义背景以匹配风格
                            background: Rectangle {
                                color: "#FAFAFA"
                                border.color: "#E0E0E0"
                                radius: 8
                            }
                        }
                    }

                    // 5. 底部按钮区
                    Item { Layout.preferredHeight: 20 } // 间距

                    Button {
                        id: generateBtn
                        text: "重新生成图片"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 45

                        // 统一使用蓝色风格
                        background: Rectangle {
                            color: generateBtn.down ? "#1565C0" : (generateBtn.hovered ? "#1565C0" : "#1976D2")
                            radius: 8
                            // 添加阴影效果
                            layer.enabled: true
                        }

                        contentItem: Text {
                            text: parent.text
                            font.pixelSize: 16
                            font.weight: Font.Medium
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (shotData) {
                                // 触发重新生成信号
                                saveCurrentEdits() // 先保存文字修改
                                shotDetailPage.regenerateImage(shotData.shotId, promptArea.text)
                                shotDetailPage.navigateTo("storyboard")
                            }
                        }
                    }
                }
            }
        }
    }

    // 辅助函数：保存当前编辑到 shotData 对象（内存中）
    function saveCurrentEdits() {
        if (!shotData) return;

        shotData.sceneTitle = titleField.text
        shotData.prompt = promptArea.text
        shotData.narration = narrationArea.text

        // 映射下拉框回数据字段
        var values = ["kenBurns", "crossfade", "volumeMix"]
        shotData.transition = values[transitionCombo.currentIndex]

        console.log("UI: 已保存分镜修改 ->", shotData.sceneTitle)
    }
}
