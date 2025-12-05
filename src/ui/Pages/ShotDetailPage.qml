import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * 分镜详情编辑页面
 * 用于编辑单个分镜的标题、提示词、旁白和转场效果
 */
Rectangle {
    id: shotDetailPage
    color: "#F8FAFC"
    bottomRightRadius: 16

    // ==================== 属性定义 ====================
    property var shotData: null         // 分镜数据
    property string projectId: ""       // 项目 ID
    property string selec_style: ""     // 选中的风格
    property int imageRefreshKey: 0     // 用于强制刷新图片

    // ==================== 信号定义 ====================
    signal navigateTo(string page)
    signal shotDataUpdated(var updatedShot)  // 通知父组件数据已更新

    // 转场效果选项
    readonly property var kTransitions: [
        { label: "Ken Burns (镜头推拉)", value: "kenBurns" },
        { label: "Crossfade (淡入淡出)", value: "crossfade" },
        { label: "Volume Mix (音量混合)", value: "volumeMix" }
    ]

    // 数据变化时回填表单
    onShotDataChanged: {
        if (shotData) {
            titleField.text = shotData.sceneTitle || ""
            promptArea.text = shotData.prompt || ""
            narrationArea.text = shotData.narration || ""

            // 设置转场下拉框
            var currentVal = shotData.transition || "kenBurns"
            var idx = 0
            for (var i = 0; i < kTransitions.length; i++) {
                if (kTransitions[i].value === currentVal) {
                    idx = i
                    break
                }
            }
            transitionCombo.currentIndex = idx

            // 回显运镜效果
            var currentEffect = shotData.effect || "zoom_in" // 默认放大
            var effIdx = effectCombo.values.indexOf(currentEffect)
            effectCombo.currentIndex = (effIdx >= 0) ? effIdx : 0
        }
    }

    // ==================== 顶部导航栏 ====================
    RowLayout {
        id: headerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 24
        spacing: 16

        // 返回按钮
        Button {
            id: backBtn
            text: "\u2190 返回分镜"

            background: Rectangle {
                color: {
                    if (backBtn.down) return "#E2E8F0"
                    if (backBtn.hovered) return "#F1F5F9"
                    return "#FFFFFF"
                }
                border.color: "#E2E8F0"
                border.width: 1
                radius: 8
            }

            contentItem: Text {
                text: parent.text
                color: "#475569"
                font.pixelSize: 13
                font.weight: Font.Medium
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                saveCurrentEdits()
                // 通知父组件数据已更新
                shotDetailPage.shotDataUpdated(shotData)
                shotDetailPage.navigateTo("storyboard")
            }
        }

        // 页面标题
        Text {
            text: "编辑分镜详情"
            font.pixelSize: 18
            font.weight: Font.Bold
            color: "#1E293B"
        }

        Item { Layout.fillWidth: true }

        // 状态标签
        Rectangle {
            visible: !!shotData
            width: statusRow.width + 16
            height: 28
            radius: 14
            color: getStatusColor(shotData ? shotData.status : "")

            Row {
                id: statusRow
                anchors.centerIn: parent
                spacing: 6

                // 状态指示点
                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: getStatusTextColor(shotData ? shotData.status : "")
                    anchors.verticalCenter: parent.verticalCenter
                }

                // 状态文字
                Text {
                    text: getStatusText(shotData ? shotData.status : "")
                    font.pixelSize: 11
                    font.weight: Font.Bold
                    color: getStatusTextColor(shotData ? shotData.status : "")
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    // ==================== 主内容区域 ====================
    RowLayout {
        anchors.top: headerBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 24
        anchors.topMargin: 20
        spacing: 24

        // 左侧：图片预览
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 5
            color: "#FFFFFF"
            radius: 12
            border.width: 1
            border.color: "#E2E8F0"
            clip: true

            // 预览图片
            Image {
                id: previewImage
                anchors.fill: parent
                anchors.margins: 2
                fillMode: Image.PreserveAspectFit
                // 添加时间戳强制刷新图片
                source: {
                    if (shotData && shotData.localImagePath) {
                        var path = shotData.localImagePath
                        // 如果是本地文件，添加时间戳强制刷新
                        if (path.indexOf("?") === -1) {
                            return path + "?t=" + imageRefreshKey
                        }
                        return path
                    }
                    return ""
                }
                visible: !!source
                cache: false
                asynchronous: true
            }

            // 占位符
            Column {
                anchors.centerIn: parent
                visible: !(shotData && shotData.localImagePath)
                spacing: 12

                Text {
                    text: "\uD83D\uDCF7"
                    font.pixelSize: 48
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    text: "等待生成图像..."
                    color: "#94A3B8"
                    font.pixelSize: 14
                }
            }

            // 生成中遮罩
            Rectangle {
                anchors.fill: parent
                color: "#80FFFFFF"
                visible: shotData && shotData.status === "generating"
                radius: parent.radius

                BusyIndicator {
                    anchors.centerIn: parent
                }
            }
        }

        // 右侧：编辑表单
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 4
            color: "transparent"

            ScrollView {
                anchors.fill: parent
                contentWidth: parent.width
                clip: true

                ColumnLayout {
                    width: parent.width
                    spacing: 20

                    // 场景标题
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: "场景标题"
                            color: "#475569"
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                        }

                        TextField {
                            id: titleField
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                            font.pixelSize: 14

                            background: Rectangle {
                                color: "#FFFFFF"
                                border.color: titleField.activeFocus ? "#6366F1" : "#E2E8F0"
                                border.width: 1
                                radius: 8
                            }
                        }
                    }

                    // 画面描述
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: "画面描述 (Prompt)"
                            color: "#475569"
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                        }

                        TextArea {
                            id: promptArea
                            Layout.fillWidth: true
                            Layout.preferredHeight: 100
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap

                            background: Rectangle {
                                color: "#FFFFFF"
                                border.color: promptArea.activeFocus ? "#6366F1" : "#E2E8F0"
                                border.width: 1
                                radius: 8
                            }
                        }
                    }

                    // 旁白配音
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: "旁白配音"
                            color: "#475569"
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                        }

                        TextArea {
                            id: narrationArea
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap

                            background: Rectangle {
                                color: "#FFFFFF"
                                border.color: narrationArea.activeFocus ? "#6366F1" : "#E2E8F0"
                                border.width: 1
                                radius: 8
                            }
                        }
                    }

                    // 转场效果
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: "转场效果"
                            color: "#475569"
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                        }

                        ComboBox {
                            id: transitionCombo
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                            model: kTransitions.map(t => t.label)

                            background: Rectangle {
                                color: "#FFFFFF"
                                border.color: "#E2E8F0"
                                border.width: 1
                                radius: 8
                            }
                        }
                    }

                    // 运镜效果 (Camera Movement)
                    ColumnLayout {
                        width: parent.width
                        spacing: 8
                        Text {
                            text: "运镜方式 (Camera Movement)"
                            font.pixelSize: 14; font.weight: Font.Bold; color: "#666666"
                        }
                        ComboBox {
                            id: effectCombo
                            Layout.fillWidth: true
                            // 显示给用户看的文字
                            model: ["Zoom In (放大)", "Zoom Out (缩小)", "Pan Left (左移)", "Pan Right (右移)", "Static (静止)"]

                            // 对应传给后端的 value
                            property var values: ["zoom_in", "zoom_out", "pan_left", "pan_right", "static"]

                            background: Rectangle {
                                color: "#FFFFFF"
                                border.color: "#E0E0E0"
                                radius: 8
                            }
                        }
                    }

                    Item { Layout.preferredHeight: 12 }

                    // 重新生成按钮
                    Button {
                        id: generateBtn
                        text: "重新生成图片"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 48
                        enabled: shotData && shotData.status !== "generating"

                        background: Rectangle {
                            radius: 10

                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop {
                                    position: 0.0
                                    color: generateBtn.enabled ? (generateBtn.down ? "#4F46E5" : "#6366F1") : "#CBD5E1"
                                }
                                GradientStop {
                                    position: 1.0
                                    color: generateBtn.enabled ? (generateBtn.down ? "#7C3AED" : "#8B5CF6") : "#CBD5E1"
                                }
                            }

                            Rectangle {
                                anchors.fill: parent
                                radius: parent.radius
                                color: "#FFFFFF"
                                opacity: generateBtn.hovered && generateBtn.enabled ? 0.1 : 0
                            }
                        }

                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 15
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (shotData) {
                                saveCurrentEdits()
                                // 使用 storyViewModel 中保存的 projectId
                                var projId = storyViewModel.currentProjectId || shotDetailPage.projectId || "temp_id"
                                var style = shotDetailPage.selec_style || "animation"

                                console.log("Regenerating image for shot:", shotData.shotId)
                                console.log("Project ID:", projId)
                                console.log("Prompt:", promptArea.text)
                                console.log("Style:", style)

                                // 更新状态为生成中
                                var temp = Object.assign({}, shotData)
                                temp.status = "generating"
                                shotData = temp

                                storyViewModel.regenerateImage(projId, shotData.shotId, promptArea.text, style)
                            }
                        }
                    }
                }
            }
        }
    }

    // ==================== 信号连接 ====================
    Connections {
        target: storyViewModel

        function onImageRegenerated(shotDataResult) {
            // 检查是否是当前分镜的图片
            if (shotData && shotDataResult.shotId === shotData.shotId) {
                console.log("Image regenerated for shot:", shotDataResult.shotId)
                console.log("New local path:", shotDataResult.localImagePath)

                // 保留用户编辑的内容，只更新图片路径和状态
                saveCurrentEdits()
                var updatedShot = Object.assign({}, shotData)
                updatedShot.localImagePath = shotDataResult.localImagePath
                updatedShot.status = "generated"
                shotData = updatedShot

                // 强制刷新图片显示
                imageRefreshKey = Date.now()

                // 通知父组件数据已更新
                shotDetailPage.shotDataUpdated(shotData)
            }
        }
    }

    // ==================== 辅助函数 ====================

    // 保存当前编辑内容
    function saveCurrentEdits() {
        if (!shotData) return
        shotData.sceneTitle = titleField.text
        shotData.prompt = promptArea.text
        shotData.narration = narrationArea.text
        shotData.transition = kTransitions[transitionCombo.currentIndex].value
        shotData.effect = effectCombo.values[effectCombo.currentIndex] // 保存运镜效果
    }

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
