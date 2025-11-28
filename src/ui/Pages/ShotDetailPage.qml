import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: shotDetailPage
    color: "#F0F2F5"
    bottomRightRadius: 20

    // 接收数据
    property var shotData: null
    property string projectId: ""
    property string selec_style: ""

    // 信号
    signal navigateTo(string page)

    // 转场选项
    readonly property var kTransitions: [
        { label: "Ken Burns (镜头推拉)", value: "kenBurns" },
        { label: "Crossfade (淡入淡出)", value: "crossfade" },
        { label: "Volume Mix (音量混合)", value: "volumeMix" }
    ]

    // 数据回显
    onShotDataChanged: {
        if (shotData) {
            titleField.text = shotData.sceneTitle || ""
            promptArea.text = shotData.prompt || ""
            narrationArea.text = shotData.narration || ""

            var currentVal = shotData.transition || "kenBurns"
            var idx = 0
            for(var i=0; i<kTransitions.length; i++) {
                if(kTransitions[i].value === currentVal) { idx = i; break; }
            }
            transitionCombo.currentIndex = idx
        }
    }

    // 顶部栏
    RowLayout {
        id: headerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 30
        spacing: 15

        // 返回按钮
        Button {
            id: backBtn
            text: "← 返回分镜"
            background: Rectangle {
                // 只有悬停时才变深一点灰，平时偏白
                color: backBtn.down ? "#D0D0D0" : (backBtn.hovered ? "#E0E0E0" : "#FAFAFA")
                border.color: "#CCCCCC"
                border.width: 1
                radius: 8
            }
            contentItem: Text {
                text: parent.text
                color: "#666666"
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: {
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

        Item { Layout.fillWidth: true } // 占位

        // 状态标签
        Rectangle {
            visible: !!shotData
            width: 100
            height: 28
            radius: 14
            color: getStatusColor(shotData ? shotData.status : "")
            Row {
                anchors.centerIn: parent
                spacing: 6
                Rectangle {
                    width: 8; height: 8; radius: 4
                    color: getStatusTextColor(shotData ? shotData.status : "")
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: shotData ? (shotData.status || "").toUpperCase() : ""
                    font.pixelSize: 12
                    font.weight: Font.Bold
                    color: getStatusTextColor(shotData ? shotData.status : "")
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    // 主内容区
    RowLayout {
        anchors.top: headerBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 30
        anchors.topMargin: 20
        spacing: 30

        // 左侧图片
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 4
            color: "#F5F5F5"
            radius: 12
            border.color: "#E0E0E0"
            border.width: 1
            clip: true

            Image {
                anchors.fill: parent
                anchors.margins: 2
                fillMode: Image.PreserveAspectFit
                source: (shotData && shotData.localImagePath) ? shotData.localImagePath : ""
                visible: !!source
                cache: false
                asynchronous: true
            }
            Column {
                anchors.centerIn: parent
                visible: !(shotData && shotData.localImagePath)
                spacing: 15
                Text { text: "📷"; font.pixelSize: 48; anchors.horizontalCenter: parent.horizontalCenter }
                Text { text: "等待生成图像..."; color: "#999999"; font.pixelSize: 14 }
            }
            Rectangle {
                anchors.fill: parent
                color: "#80FFFFFF"
                visible: shotData && shotData.status === "generating"
                BusyIndicator { anchors.centerIn: parent }
            }
        }

        // 右侧表单
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 3
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
                        Text { text: "场景标题"; color: "#666666"; font.pixelSize: 14; font.weight: Font.Bold }
                        TextField {
                            id: titleField
                            Layout.fillWidth: true
                            font.pixelSize: 16
                            background: Rectangle {
                                color: "#FFFFFF"
                                border.color: titleField.activeFocus ? "#1976D2" : "#E0E0E0"
                                radius: 8
                            }
                        }
                    }

                    // 2. 提示词
                    ColumnLayout {
                        width: parent.width
                        spacing: 8
                        Text { text: "画面描述 (Prompt)"; color: "#666666"; font.pixelSize: 14; font.weight: Font.Bold }
                        TextArea {
                            id: promptArea
                            Layout.fillWidth: true
                            Layout.preferredHeight: 100
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                            background: Rectangle {
                                color: "#FFFFFF"
                                border.color: promptArea.activeFocus ? "#1976D2" : "#E0E0E0"
                                radius: 8
                            }
                        }
                    }

                    // 3. 旁白
                    ColumnLayout {
                        width: parent.width
                        spacing: 8
                        Text { text: "旁白配音 (Narration)"; color: "#666666"; font.pixelSize: 14; font.weight: Font.Bold }
                        TextArea {
                            id: narrationArea
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                            background: Rectangle {
                                color: "#FFFFFF"
                                border.color: narrationArea.activeFocus ? "#1976D2" : "#E0E0E0"
                                radius: 8
                            }
                        }
                    }

                    // 4. 转场
                    ColumnLayout {
                        width: parent.width
                        spacing: 8
                        Text { text: "转场效果"; color: "#666666"; font.pixelSize: 14; font.weight: Font.Bold }
                        ComboBox {
                            id: transitionCombo
                            Layout.fillWidth: true
                            model: kTransitions.map(t => t.label)
                        }
                    }

                    Item { Layout.preferredHeight: 20 }

                    // 5. 重新生成按钮 (蓝色)
                    Button {
                        id: generateBtn
                        text: "重新生成图片"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 45
                        enabled: shotData && shotData.status !== "generating"

                        background: Rectangle {
                            // 蓝色按钮：默认蓝，悬停/按下变深蓝，禁用变灰
                            color: {
                                if (!generateBtn.enabled) return "#CCCCCC"; // 禁用：灰
                                if (generateBtn.down) return "#0D47A1";    // 按下：深蓝
                                if (generateBtn.hovered) return "#1565C0"; // 悬停：中深蓝 (绝对不是白色)
                                return "#1976D2";                           // 默认：亮蓝
                            }
                            radius: 8
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 16
                            font.weight: Font.Bold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            if (shotData) {
                                saveCurrentEdits()
                                var projId = shotDetailPage.projectId || "temp_id"
                                var style = shotDetailPage.selec_style || "animation"
                                // UI 立即反馈
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

    function saveCurrentEdits() {
        if (!shotData) return;
        shotData.sceneTitle = titleField.text
        shotData.prompt = promptArea.text
        shotData.narration = narrationArea.text
        shotData.transition = kTransitions[transitionCombo.currentIndex].value
    }

    function getStatusColor(status) {
        switch(status) { case "generated": return "#E8F5E9"; case "generating": return "#E3F2FD"; default: return "#FFF3E0"; }
    }
    function getStatusTextColor(status) {
        switch(status) { case "generated": return "#2E7D32"; case "generating": return "#1565C0"; default: return "#EF6C00"; }
    }
}
