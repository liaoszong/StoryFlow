import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * 视频预览页面
 * 沉浸式深色背景，用于预览生成的视频效果
 */
Rectangle {
    id: previewPage
    color: "#0F172A"
    bottomRightRadius: 16

    // ==================== 属性定义 ====================
    property var currentProjectData: null   // 当前项目数据
    property var renderConfig: null         // 渲染配置

    // ==================== 信号定义 ====================
    signal navigateTo(string page)

    // 页面加载时构建渲染配置
    Component.onCompleted: {
        if (currentProjectData) {
            var config = storyViewModel.buildRenderConfig(currentProjectData)
            previewPage.renderConfig = config
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
                onClicked: previewPage.navigateTo("storyboard")
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
            // 移除固定宽度限制，允许自适应，但设置最大宽度防止在超宽屏上太扁
            Layout.maximumWidth: 800
            Layout.alignment: Qt.AlignHCenter

            color: "#000000"
            radius: 16
            clip: true

            // 模拟播放器
            Item {
                anchors.fill: parent
                visible: true

                // 图片轮播
                Image {
                    id: previewImg
                    anchors.fill: parent

                    // 改为 Fit 模式，确保画面完整显示不裁剪
                    // 这样横屏/竖屏视频都能完美适配，多余部分显示黑色背景
                    fillMode: Image.PreserveAspectFit

                    property int currentIndex: 0

                    source: {
                        if (renderConfig && renderConfig.track && renderConfig.track.length > 0) {
                            return renderConfig.track[currentIndex].assets.image
                        }
                        return ""
                    }

                    // 自动播放定时器
                    Timer {
                        running: previewPage.renderConfig !== null
                        interval: 3000
                        repeat: true
                        onTriggered: {
                            if (!previewPage.renderConfig) return
                            var len = previewPage.renderConfig.track.length
                            previewImg.currentIndex = (previewImg.currentIndex + 1) % len
                        }
                    }
                }

                // 底部渐变遮罩 (优化高度)
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 140 //稍微加高一点，容纳字幕

                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "transparent" }
                        GradientStop { position: 1.0; color: "#000000" }
                    }
                }

                // 字幕区域
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 32 // 抬高一点
                    anchors.horizontalCenter: parent.horizontalCenter

                    // 自适应宽度，最大 80%
                    width: Math.min(parent.width * 0.8, subtitleText.implicitWidth + 48)
                    height: subtitleText.implicitHeight + 24

                    color: Qt.rgba(0, 0, 0, 0.6) // 半透明黑底
                    radius: 8

                    Text {
                        id: subtitleText
                        anchors.centerIn: parent
                        width: parent.width - 32
                        color: "white"
                        font.pixelSize: 16 // 稍微加大字号
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter

                        text: {
                            if (renderConfig && renderConfig.track && renderConfig.track.length > 0) {
                                return renderConfig.track[previewImg.currentIndex].assets.text
                            }
                            return "加载中..."
                        }
                    }
                }

                // 进度指示器
                Row {
                    anchors.top: parent.top
                    anchors.topMargin: 20
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 6
                    visible: renderConfig && renderConfig.track

                    Repeater {
                        model: renderConfig ? renderConfig.track.length : 0

                        Rectangle {
                            width: previewImg.currentIndex === index ? 32 : 8 // 激活态更长
                            height: 4
                            radius: 2
                            color: previewImg.currentIndex === index ? "#FFFFFF" : Qt.rgba(1, 1, 1, 0.3)

                            // 加个动画让进度切换更平滑
                            Behavior on width { NumberAnimation { duration: 200 } }
                        }
                    }
                }
            }
        }

        // ==================== 底部操作栏 ====================
        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: 400
            Layout.alignment: Qt.AlignHCenter
            spacing: 16

            // 导出按钮
            Button {
                id: exportBtn
                text: "导出视频 (MP4)"
                Layout.fillWidth: true
                Layout.preferredHeight: 52

                hoverEnabled: true // 强制开启悬停

                background: Rectangle {
                    radius: 12

                    // 移除白色覆盖层，使用纯净渐变逻辑
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop {
                            position: 0.0;
                            color: exportBtn.down ? "#4338CA" : (exportBtn.hovered ? "#4F46E5" : "#6366F1")
                        }
                        GradientStop {
                            position: 1.0;
                            color: exportBtn.down ? "#6D28D9" : (exportBtn.hovered ? "#7C3AED" : "#8B5CF6")
                        }
                    }
                }

                // 使用 Text 替代 Row，保证文字完美居中
                contentItem: Text {
                    text: "\uD83C\uDFAC  " + parent.text
                    color: "white"
                    font.weight: Font.Bold
                    font.pixelSize: 15
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    if (currentProjectData && currentProjectData.id) {
                        storyViewModel.exportVideo(currentProjectData.id)
                    }
                }
            }
        }
    }
}
