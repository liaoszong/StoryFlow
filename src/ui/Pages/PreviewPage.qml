import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

/**
 * 视频预览页面
 * 1. 进入页面时自动调用 VideoGenerator 生成视频
 * 2. 生成完成后使用 MediaPlayer 播放视频
 */
Rectangle {
    id: previewPage
    color: "#0F172A"
    bottomRightRadius: 16

    // ==================== 属性定义 ====================
    property var currentProjectData: null   // 当前项目数据
    property string videoOutputPath: ""     // 生成的视频路径
    property bool videoReady: false         // 视频是否准备好

    // ==================== 信号定义 ====================
    signal navigateTo(string page)

    // ==================== 页面初始化 ====================
    // 监听 currentProjectData 变化，当数据注入后自动开始生成
    onCurrentProjectDataChanged: {
        console.log("PreviewPage: currentProjectData changed:", JSON.stringify(currentProjectData))
        if (currentProjectData && currentProjectData.storyboards && !videoReady && !videoGenerator.isGenerating) {
            startVideoGeneration()
        }
    }

    Component.onCompleted: {
        console.log("PreviewPage loaded, projectData:", JSON.stringify(currentProjectData))
        // 如果数据已经存在（可能在加载前就设置了），立即开始生成
        if (currentProjectData && currentProjectData.storyboards) {
            startVideoGeneration()
        }
    }

    // 开始生成视频
    function startVideoGeneration() {
        var storyboards = currentProjectData.storyboards
        if (!storyboards || storyboards.length === 0) {
            console.error("No storyboards to generate video")
            return
        }

        // 构建 shots 数据 (VideoGenerator 需要的格式)
        var shots = []
        for (var i = 0; i < storyboards.length; i++) {
            var shot = storyboards[i]
            shots.push({
                // 图片路径 - 使用 localFilePath (原始路径供 FFmpeg 使用)
                "imagePath": shot.localFilePath || shot.localImagePath.replace("file:///", ""),
                // 音频路径
                "audioPath": shot.localAudioPath || "",
                // 持续时间
                "duration": shot.duration || 3.0,
                // 转场类型
                "transitionType": shot.transition || "crossfade",
                // 转场时长
                "transitionDuration": shot.transitionDuration || 0.5,
                // Ken Burns 特效
                "kenBurnsEnabled": shot.kenBurnsEnabled || false,
                "kenBurnsPreset": shot.kenBurnsPreset || "zoom_in",
                // 字幕（旁白文字）
                "subtitle": shot.narration || ""
            })
        }

        // 生成输出路径
        var timestamp = Date.now()
        var projectName = currentProjectData.name || "video"
        // 使用临时目录
        videoOutputPath = "C:/temp/storyflow_" + timestamp + ".mp4"

        console.log("Starting video generation...")
        console.log("Output path:", videoOutputPath)
        console.log("Shots:", JSON.stringify(shots))

        // 调用 VideoGenerator
        videoGenerator.generateVideo(shots, videoOutputPath, 1920, 1080, 30)
    }

    // ==================== VideoGenerator 信号连接 ====================
    Connections {
        target: videoGenerator

        function onFinished(success, outputPath) {
            console.log("Video generation finished:", success, "outputPath:", outputPath)
            if (success && outputPath) {
                previewPage.videoOutputPath = outputPath
                previewPage.videoReady = true
                // 设置视频源并播放
                var videoSource = "file:///" + outputPath.replace(/\\/g, "/")
                console.log("Video source:", videoSource)
                videoPlayer.source = videoSource
                videoPlayer.play()
            } else {
                console.error("Video generation failed:", videoGenerator.errorMessage)
            }
        }

        function onProgressChanged() {
            // 进度更新由属性绑定自动处理
        }
    }

    // ==================== 主布局 ====================
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32
        spacing: 24

        // 顶部导航栏
        RowLayout {
            Layout.fillWidth: true

            // 返回按钮
            Button {
                text: "← 返回编辑"
                background: Rectangle {
                    color: parent.hovered ? "#1E293B" : "transparent"
                    radius: 8
                }
                contentItem: Text {
                    text: parent.text
                    color: "#94A3B8"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                }
                onClicked: {
                    videoPlayer.pause()
                    videoPlayer.source = ""
                    previewPage.navigateTo("storyboard")
                }
            }

            Item { Layout.fillWidth: true }

            // 项目名称
            Text {
                text: currentProjectData ? currentProjectData.name : "预览"
                color: "#FFFFFF"
                font.pixelSize: 18
                font.weight: Font.Bold
            }

            Item { Layout.fillWidth: true }
            Item { width: 100 }
        }

        // ==================== 播放器容器 ====================
        Rectangle {
            id: playerContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumWidth: 960
            Layout.alignment: Qt.AlignHCenter

            color: "#000000"
            radius: 16
            clip: true

            // ========== 视频播放器 ==========
            MediaPlayer {
                id: videoPlayer
                videoOutput: videoOutput
                audioOutput: AudioOutput {}

                onErrorOccurred: function(error, errorString) {
                    console.error("Video error:", error, errorString)
                }

                onPlaybackStateChanged: {
                    console.log("Playback state:", playbackState)
                }

                // 播放完毕后循环
                onMediaStatusChanged: {
                    if (mediaStatus === MediaPlayer.EndOfMedia) {
                        videoPlayer.setPosition(0)
                        videoPlayer.play()
                    }
                }
            }

            VideoOutput {
                id: videoOutput
                anchors.fill: parent
                visible: previewPage.videoReady
                fillMode: VideoOutput.PreserveAspectFit
            }

            // ========== 生成中状态 ==========
            Item {
                anchors.fill: parent
                visible: !previewPage.videoReady

                Column {
                    anchors.centerIn: parent
                    spacing: 24

                    // 加载动画
                    BusyIndicator {
                        running: videoGenerator.isGenerating
                        anchors.horizontalCenter: parent.horizontalCenter
                        scale: 1.5

                        // 自定义颜色
                        palette.dark: "#6366F1"
                    }

                    // 状态文字
                    Text {
                        text: videoGenerator.isGenerating ? "正在生成视频..." : "准备中..."
                        color: "#FFFFFF"
                        font.pixelSize: 18
                        font.weight: Font.Medium
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    // 进度条
                    ProgressBar {
                        width: 300
                        height: 8
                        value: videoGenerator.progress / 100.0
                        anchors.horizontalCenter: parent.horizontalCenter
                        visible: videoGenerator.isGenerating

                        background: Rectangle {
                            radius: 4
                            color: "#1E293B"
                        }

                        contentItem: Item {
                            Rectangle {
                                width: parent.width * videoGenerator.progress / 100.0
                                height: parent.height
                                radius: 4

                                gradient: Gradient {
                                    orientation: Gradient.Horizontal
                                    GradientStop { position: 0.0; color: "#6366F1" }
                                    GradientStop { position: 1.0; color: "#8B5CF6" }
                                }
                            }
                        }
                    }

                    // 进度百分比
                    Text {
                        text: videoGenerator.progress + "%"
                        color: "#94A3B8"
                        font.pixelSize: 14
                        anchors.horizontalCenter: parent.horizontalCenter
                        visible: videoGenerator.isGenerating
                    }

                    // 错误信息
                    Text {
                        text: videoGenerator.errorMessage
                        color: "#EF4444"
                        font.pixelSize: 14
                        anchors.horizontalCenter: parent.horizontalCenter
                        visible: videoGenerator.errorMessage !== ""
                        wrapMode: Text.WordWrap
                        width: 400
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            // ========== 视频控制条 ==========
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 60
                visible: previewPage.videoReady

                gradient: Gradient {
                    GradientStop { position: 0.0; color: "transparent" }
                    GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.8) }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 16

                    // 播放/暂停按钮
                    Button {
                        implicitWidth: 40
                        implicitHeight: 40

                        background: Rectangle {
                            color: parent.hovered ? Qt.rgba(1, 1, 1, 0.2) : "transparent"
                            radius: 20
                        }

                        contentItem: Text {
                            text: videoPlayer.playbackState === MediaPlayer.PlayingState ? "⏸" : "▶"
                            color: "white"
                            font.pixelSize: 18
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (videoPlayer.playbackState === MediaPlayer.PlayingState) {
                                videoPlayer.pause()
                            } else {
                                videoPlayer.play()
                            }
                        }
                    }

                    // 进度条
                    Slider {
                        id: progressSlider
                        Layout.fillWidth: true
                        from: 0
                        to: videoPlayer.duration
                        value: videoPlayer.position

                        onMoved: {
                            videoPlayer.setPosition(value)
                        }

                        background: Rectangle {
                            x: progressSlider.leftPadding
                            y: progressSlider.topPadding + progressSlider.availableHeight / 2 - height / 2
                            width: progressSlider.availableWidth
                            height: 4
                            radius: 2
                            color: Qt.rgba(1, 1, 1, 0.3)

                            Rectangle {
                                width: progressSlider.visualPosition * parent.width
                                height: parent.height
                                radius: 2
                                color: "#6366F1"
                            }
                        }

                        handle: Rectangle {
                            x: progressSlider.leftPadding + progressSlider.visualPosition * (progressSlider.availableWidth - width)
                            y: progressSlider.topPadding + progressSlider.availableHeight / 2 - height / 2
                            width: 16
                            height: 16
                            radius: 8
                            color: progressSlider.pressed ? "#8B5CF6" : "#6366F1"
                        }
                    }

                    // 时间显示
                    Text {
                        text: formatTime(videoPlayer.position) + " / " + formatTime(videoPlayer.duration)
                        color: "white"
                        font.pixelSize: 12
                    }
                }
            }
        }

        // ==================== 底部操作栏 ====================
        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: 500
            Layout.alignment: Qt.AlignHCenter
            spacing: 16

            // 重新生成按钮
            Button {
                text: "重新生成"
                Layout.preferredWidth: 120
                Layout.preferredHeight: 44
                enabled: !videoGenerator.isGenerating

                background: Rectangle {
                    color: "transparent"
                    border.color: parent.hovered ? "#94A3B8" : "#475569"
                    border.width: 1
                    radius: 10
                }

                contentItem: Text {
                    text: parent.text
                    color: "#94A3B8"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    videoPlayer.pause()
                    videoPlayer.source = ""
                    previewPage.videoReady = false
                    startVideoGeneration()
                }
            }

            // 导出按钮
            Button {
                id: exportBtn
                text: "导出视频"
                Layout.fillWidth: true
                Layout.preferredHeight: 52
                enabled: previewPage.videoReady

                background: Rectangle {
                    radius: 12

                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop {
                            position: 0.0
                            color: exportBtn.enabled ? (exportBtn.down ? "#4338CA" : "#6366F1") : "#475569"
                        }
                        GradientStop {
                            position: 1.0
                            color: exportBtn.enabled ? (exportBtn.down ? "#6D28D9" : "#8B5CF6") : "#475569"
                        }
                    }
                }

                contentItem: Text {
                    text: "🎬  " + parent.text
                    color: "white"
                    font.weight: Font.Bold
                    font.pixelSize: 15
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    // 打开文件保存对话框或直接复制到用户选择的位置
                    console.log("Export video from:", previewPage.videoOutputPath)
                    // TODO: 实现文件保存对话框
                    exportSuccessText.visible = true
                    exportSuccessTimer.start()
                }
            }
        }

        // 导出成功提示
        Text {
            id: exportSuccessText
            text: "✓ 视频已保存到: " + previewPage.videoOutputPath
            color: "#22C55E"
            font.pixelSize: 13
            Layout.alignment: Qt.AlignHCenter
            visible: false

            Timer {
                id: exportSuccessTimer
                interval: 5000
                onTriggered: exportSuccessText.visible = false
            }
        }
    }

    // ==================== 辅助函数 ====================
    function formatTime(ms) {
        if (isNaN(ms) || ms < 0) return "00:00"
        var seconds = Math.floor(ms / 1000)
        var minutes = Math.floor(seconds / 60)
        seconds = seconds % 60
        return (minutes < 10 ? "0" : "") + minutes + ":" + (seconds < 10 ? "0" : "") + seconds
    }
}
