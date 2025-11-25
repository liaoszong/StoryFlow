import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: previewPage
    color: "transparent" // 复用 RightPage 的背景色

    // 接收从 RightPage 传来的项目数据
    property var currentProjectData: null
    // 存储后端生成的渲染配置文件路径
    property string renderConfigPath: ""

    // 信号
    signal navigateTo(string page)

    // 页面加载完成后，请求后端生成播放器需要的 JSON 配置
    Component.onCompleted: {
        if (currentProjectData) {
            console.log("PreviewPage: 请求生成渲染配置...", currentProjectData.id)
            // 调用 Backend 函数 (需要在 C++ 中实现 generateRenderConfig)
            backendService.generateRenderConfig(currentProjectData)
        }
    }

    // 监听后端信号
    Connections {
        target: backendService
        function onRenderConfigReady(configPath) {
            console.log("PreviewPage: 收到配置文件路径 ->", configPath)
            previewPage.renderConfigPath = "file:///" + configPath

            // 如果此时播放器组件未被注释，可以在这里将 source 传给它
            // player.configSource = previewPage.renderConfigPath
            // player.play()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 40
        spacing: 20

        // 1. 标题区域
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 5
            Text {
                text: "Preview"
                font.pixelSize: 32
                font.weight: Font.Bold
                color: "#333333"
                Layout.alignment: Qt.AlignHCenter
            }
            Text {
                text: currentProjectData ? currentProjectData.name : "未命名项目"
                font.pixelSize: 16
                color: "#666666"
                Layout.alignment: Qt.AlignHCenter
            }
        }

        // 2. 播放器区域 (占位符)
        Rectangle {
            id: playerContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumWidth: 800 // 限制最大宽度，保持 16:9 比例更佳
            Layout.alignment: Qt.AlignHCenter

            color: "#000000"
            radius: 12
            clip: true

            // --- 实际播放器组件 (已注释) ---
            /*
            StoryPlayer {
                id: player
                anchors.fill: parent
                // 将后端生成的 json 路径传给播放器
                configSource: previewPage.renderConfigPath

                onPlaybackFinished: {
                    console.log("播放结束")
                }
            }
            */

            // --- 临时占位显示 ---
            Column {
                anchors.centerIn: parent
                spacing: 15
                visible: true // 当播放器被注释时显示这个

                Text {
                    text: "▶️"
                    font.pixelSize: 64
                    color: "#666666"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    text: previewPage.renderConfigPath ? "渲染配置已就绪\n" + previewPage.renderConfigPath : "正在准备渲染数据..."
                    color: "#999999"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            // 加载指示器
            BusyIndicator {
                anchors.centerIn: parent
                running: previewPage.renderConfigPath === ""
                visible: running
            }
        }

        // 3. 底部控制栏
        RowLayout {
            spacing: 20
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 10

            // 导出视频按钮
            Button {
                text: "导出成品视频 (Export)"

                background: Rectangle {
                    color: parent.down ? "#1565C0" : (parent.hovered ? "#1565C0" : "#1976D2")
                    radius: 8
                }

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 16
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    console.log("Exporting video for:", currentProjectData.id)
                    // 这里未来可以调用 backendService.exportVideo(...)
                }
            }

            // 返回按钮
            Button {
                text: "返回资产库"

                background: Rectangle {
                    color: parent.down ? "#E0E0E0" : (parent.hovered ? "#F5F5F5" : "#FAFAFA")
                    border.color: "#E0E0E0"
                    radius: 8
                }

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 16
                    color: "#666666"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    // 通常预览页是从 Assets 或 Storyboard 来的，这里默认回 Assets，或者根据需求改
                    previewPage.navigateTo("assets")
                }
            }
        }
    }
}
