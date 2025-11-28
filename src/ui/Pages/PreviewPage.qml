import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: previewPage
    color: "#121212" // 沉浸式深色背景
    bottomRightRadius: 20

    // 1. 数据输入
    property var currentProjectData: null
    property var renderConfig: null // 播放器配置

    // 信号
    signal navigateTo(string page)

    // 2. 初始化逻辑
    Component.onCompleted: {
        if (currentProjectData) {
            console.log("Preview: 构建实时渲染配置...")
            // 【MVVM】调用 C++ 纯逻辑函数，获取播放器需要的 JSON
            var config = storyViewModel.buildRenderConfig(currentProjectData)
            previewPage.renderConfig = config
        }
    }

    // 3. 界面布局
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 40
        spacing: 20

        // 顶部栏
        RowLayout {
            Layout.fillWidth: true
            Button {
                text: "← 返回编辑"
                background: Rectangle { color: "transparent" }
                contentItem: Text { text: parent.text; color: "#AAAAAA"; font.pixelSize: 16 }
                onClicked: previewPage.navigateTo("storyboard")
            }
            Item { Layout.fillWidth: true }
            Text {
                text: currentProjectData ? currentProjectData.name : "预览"
                color: "white"; font.pixelSize: 18; font.weight: Font.Bold
            }
            Item { Layout.fillWidth: true }
            Item { width: 80 }
        }

        // 播放器区域
        Rectangle {
            id: playerContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumWidth: 500 // 9:16 比例
            Layout.alignment: Qt.AlignHCenter
            color: "black"
            radius: 12
            clip: true

            // =================================================
            // 【集成点】多媒体同学的播放器组件
            // =================================================
            // 假设组件名叫 StoryPlayer
            /*
            StoryPlayer {
                id: realTimePlayer
                anchors.fill: parent

                // 核心：传入 ViewModel 构建好的配置
                config: previewPage.renderConfig

                // 自动播放
                autoPlay: true

                onPlaybackFinished: {
                    playBtn.visible = true
                }
            }
            */

            // --- 临时模拟播放器 (当组件没写好时用这个调试) ---
            Item {
                anchors.fill: parent
                visible: true // 如果 StoryPlayer 好了，设为 false

                Image {
                    id: previewImg
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectCrop
                    // 简单的幻灯片演示逻辑
                    property int currentIndex: 0
                    source: {
                        if (renderConfig && renderConfig.track && renderConfig.track.length > 0) {
                             return renderConfig.track[currentIndex].assets.image
                        }
                        return ""
                    }

                    // 模拟播放定时器
                    Timer {
                        running: previewPage.renderConfig !== null
                        interval: 3000 // 3秒切一张
                        repeat: true
                        onTriggered: {
                            if (!previewPage.renderConfig) return
                            var len = previewPage.renderConfig.track.length
                            previewImg.currentIndex = (previewImg.currentIndex + 1) % len
                        }
                    }
                }

                // 字幕层
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 40
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width * 0.9
                    height: 60
                    color: "#80000000"
                    radius: 8
                    Text {
                        anchors.centerIn: parent
                        color: "white"
                        width: parent.width - 20
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        // 模拟字幕
                        text: {
                            if (renderConfig && renderConfig.track) {
                                return renderConfig.track[previewImg.currentIndex].assets.text
                            }
                            return "Loading..."
                        }
                    }
                }
            }
        }

        // 底部栏
        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: 500
            Layout.alignment: Qt.AlignHCenter
            spacing: 20

            Button {
                text: "导出成品视频 (MP4)"
                Layout.fillWidth: true
                Layout.preferredHeight: 50

                background: Rectangle {
                    color: "#1976D2"
                    radius: 25
                }
                contentItem: Text {
                    text: parent.text; color: "white"; font.weight: Font.Bold; font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    // 调用后端真实渲染
                    if (currentProjectData && currentProjectData.id) {
                        console.log("请求后端渲染 MP4...")
                        storyViewModel.exportVideo(currentProjectData.id)
                    }
                }
            }
        }
    }
}
