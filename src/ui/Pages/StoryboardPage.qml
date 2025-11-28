import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: storyboardPage
    color: "#F0F2F5"
    bottomRightRadius: 20

    // =========================================================
    // 1. UI 状态 (View State)
    // =========================================================
    property string selectedStyle: ""
    property string storyText: ""

    // 【关键】给予一个安全的默认值，防止 ListView 初始化时报错
    property var projectData: ({ "storyboards": [] })

    // =========================================================
    // 2. 信号 (Signals)
    // =========================================================
    // 仅保留页面跳转信号，这是 View 层的核心职责
    signal navigateTo(string page, var data)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 40
        spacing: 30

        // 标题区
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 10

            Text {
                text: "Storyboard"
                font.pixelSize: 32
                font.weight: Font.Bold
                color: "#333333"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "故事生成成功！这是您的分镜预览："
                font.pixelSize: 16
                color: "#666666"
                Layout.alignment: Qt.AlignHCenter
            }
        }

        // 分镜列表
        ListView {
            id: shotList
            Layout.fillWidth: true
            Layout.fillHeight: true

            // 数据源绑定
            model: (storyboardPage.projectData && storyboardPage.projectData.storyboards)
                   ? storyboardPage.projectData.storyboards
                   : []

            orientation: ListView.Horizontal
            spacing: 20
            clip: true

            // 卡片模板
            delegate: Rectangle {
                width: 300
                height: shotList.height
                color: "#FFFFFF"
                radius: 12
                border.color: "#E0E0E0"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 10

                    // A. 图片区域
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 160
                        color: "#F5F5F5"
                        radius: 8
                        clip: true

                        Image {
                            anchors.fill: parent
                            // 确保 URL 有效才加载
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
                            spacing: 5
                            Text {
                                text: "🖼️"
                                font.pixelSize: 30;
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Text {
                                text: "等待生成..."
                                color: "#999999"
                                font.pixelSize: 12
                            }
                        }

                        // 加载指示器 (当状态为 generating 时显示)
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
                            width: 80
                            height: 24
                            radius: 12

                            // 提取颜色逻辑
                            color: getStatusColor(modelData.status)

                            Text {
                                anchors.centerIn: parent
                                text: modelData.status ? modelData.status.toUpperCase() : "PENDING"
                                color: getStatusTextColor(modelData.status)
                                font.pixelSize: 10
                                font.weight: Font.Bold
                            }
                        }
                    }

                    // B. 标题
                    Text {
                        text: modelData.sceneTitle || "未命名场景"
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        color: "#333333"
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    // C. 旁白
                    Text {
                        text: modelData.narration || "暂无旁白..."
                        font.pixelSize: 14
                        color: "#666666"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                    }

                    // D. 编辑按钮
                    Button {
                        text: "编辑详情"
                        Layout.fillWidth: true
                        // 使用浅色背景风格
                        background: Rectangle {
                            color: parent.down ? "#E0E0E0" : "#F5F5F5"
                            radius: 6
                        }
                        onClicked: {
                            // 构造完整的 Payload
                            var shotPayload = {
                                "shotId": modelData.shotId,
                                "sceneTitle": modelData.sceneTitle,
                                "prompt": modelData.prompt,
                                "narration": modelData.narration,
                                "localImagePath": modelData.localImagePath,
                                "status": modelData.status,
                                // 确保 transition 字段存在
                                "transition": modelData.transition || "kenBurns"
                            };
                            console.log("Router: 跳转详情 ->", shotPayload.shotId);
                            storyboardPage.navigateTo("shotDetail", shotPayload);
                        }
                    }
                }
            }
        }

        // 底部操作栏
        RowLayout{
            spacing: 20
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 10

            // 返回按钮
            Button {
                text: "返回修改"
                Layout.preferredWidth: 120
                Layout.preferredHeight: 44

                background: Rectangle {
                    color: "transparent"
                    border.color: "#CCCCCC"
                    border.width: 1
                    radius: 22
                }
                contentItem: Text {
                    text: parent.text
                    color: "#666666"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: storyboardPage.navigateTo("create", null)
            }

            // 生成视频按钮
            Button {
                id: genVideoBtn
                text: "生成最终视频"
                Layout.preferredWidth: 160
                Layout.preferredHeight: 44
                enabled: !storyViewModel.isGenerating

                background: Rectangle {
                    radius: 22
                    // 修正颜色：按下/悬停变深蓝，平时亮蓝
                    color: {
                        if (!genVideoBtn.enabled) return "#CCCCCC";
                        if (genVideoBtn.down) return "#0D47A1";
                        if (genVideoBtn.hovered) return "#1565C0";
                        return "#1976D2";
                    }
                }

                contentItem: Text {
                    text: parent.text
                    font.weight: Font.Bold
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    if (storyboardPage.projectData && storyboardPage.projectData.id) {
                        storyViewModel.generateVideo(storyboardPage.projectData.id);
                        storyboardPage.navigateTo("preview", null);
                    }
                }
            }
        }
    }

    // =========================================================
    // 3. 辅助函数 (View Helpers)
    // =========================================================

    function getStatusColor(status) {
        switch(status) {
            case "generated": return "#E8F5E9"; // 浅绿
            case "generating": return "#E3F2FD"; // 浅蓝
            default: return "#FFF3E0"; // 浅橙 (pending)
        }
    }

    function getStatusTextColor(status) {
        switch(status) {
            case "generated": return "#2E7D32"; // 深绿
            case "generating": return "#1565C0"; // 深蓝
            default: return "#EF6C00"; // 深橙
        }
    }
}
