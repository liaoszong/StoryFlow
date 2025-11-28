// StoryboardPage.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: storyboardPage
    color: "#F0F2F5"
    bottomRightRadius: 20
    // 属性
    property string selectedStyle: ""
    property string storyText: ""
    property var projectData: ({ "storyboards": [] })

    // 信号,传递给RightPage
    signal styleSelected(string style)
    signal generateStory()
    signal navigateTo(string page, var data)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 40
        spacing: 30

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

        // 把原来的 RowLayout 删掉，换成这个 ListView
        ListView {
            id: shotList
            Layout.fillWidth: true
            Layout.fillHeight: true

            // 1. 数据源
            // 告诉列表：你的数据在 projectData 里的 storyboards 字段里
            // 如果为空，就给个空数组 []
            model: (storyboardPage.projectData && storyboardPage.projectData.storyboards)
                   ? storyboardPage.projectData.storyboards
                   : []

            // 2. 排列方式】横向排列
            orientation: ListView.Horizontal
            spacing: 20
            clip: true // 防止卡片滑出边界

            // 3. 卡片模板 (Delegate)
            // 每一个分镜数据都会套用这个模板渲染一次
            delegate: Rectangle {
                width: 300  // 卡片宽度
                height: shotList.height // 高度占满列表
                color: "#FFFFFF"
                radius: 12
                border.color: "#E0E0E0"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 10

                    // A. 顶部：图片或占位符
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 160
                        color: "#F5F5F5"
                        radius: 8
                        clip: true

                        // 如果有图片 URL 就显示图片 (Image)，没有就显示文字
                        // modelData 代表当前这一条分镜数据
                        Image {
                            anchors.fill: parent
                            source: modelData.localImagePath ? modelData.localImagePath : ""
                            fillMode: Image.PreserveAspectCrop
                            visible: modelData.localImagePath !== ""
                            // 异步加载，列表滑动更流畅
                            asynchronous: true
                            // 如果图片会重绘，关掉缓存防止显示旧图
                            cache: false
                        }

                        // 没图片时显示的占位文字
                        Text {
                            anchors.centerIn: parent
                            text: modelData.localImagePath ? "" : "Waiting for Image..."
                            color: "#999999"
                            visible: !modelData.localImagePath
                        }

                        // 状态标签 (比如 "generating")
                        Rectangle {
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.margins: 8
                            width: 80
                            height: 24
                            radius: 12
                            // 【优化】根据状态变色
                            color: modelData.status === "generated" ? "#E8F5E9" :
                                                                      (modelData.status === "generating" ? "#E3F2FD" : "#FFF3E0")

                            Text {
                                anchors.centerIn: parent
                                text: modelData.status ? modelData.status.toUpperCase() : "PENDING"
                                // 【优化】文字颜色也跟着变
                                color: modelData.status === "generated" ? "#2E7D32" :
                                                                          (modelData.status === "generating" ? "#1565C0" : "#EF6C00")
                                font.pixelSize: 10
                                font.weight: Font.Bold
                            }
                        }
                    }

                    // B. 中间：标题
                    Text {
                        // 使用数据里的 sceneTitle
                        text: modelData.sceneTitle
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        color: "#333333"
                        Layout.fillWidth: true
                        elide: Text.ElideRight //文字太长自动显示省略号
                    }

                    // C. 中间：旁白摘要
                    Text {
                        // 使用数据里的 narration
                        text: modelData.narration
                        font.pixelSize: 14
                        color: "#666666"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                    }

                    // D. 底部：详情按钮
                    Button {
                        text: "编辑详情"
                        Layout.fillWidth: true
                        onClicked: {
                            var shotPayload = {
                                "shotId": modelData.shotId,
                                "sceneTitle": modelData.sceneTitle,
                                "prompt": modelData.prompt,
                                "narration": modelData.narration,
                                "localImagePath": modelData.localImagePath,
                                "status": modelData.status,
                                "transition": modelData.transition || "kenBurns"
                            };

                            console.log("准备跳转，数据ID:", shotPayload.shotId);
                            storyboardPage.navigateTo("shotDetail", shotPayload);
                        }
                    }
                }
            }
        }

        RowLayout{
            spacing: 10
            Layout.alignment: Qt.AlignHCenter

            // 生成视频按钮
            Button {
                text: "生成视频"

                background: Rectangle {
                    color: parent.down ? "#E0E0E0" :
                           parent.hovered ? "#1565C0" : "#1976D2"
                    radius: 8
                }

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 14
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    // 调用 ViewModel 生成视频
                    // 假设 projectData 里有 id 字段
                    if (storyboardPage.projectData && storyboardPage.projectData.id) {
                        storyViewModel.generateVideo(storyboardPage.projectData.id)

                        // 跳转去预览页
                        storyboardPage.navigateTo("preview", null)
                    } else {
                        console.log("没有项目ID，无法生成视频")
                    }
                }
            }
            // 返回按钮
            Button {
                text: "返回创建页面"

                background: Rectangle {
                    color: parent.down ? "#E0E0E0" :
                           parent.hovered ? "#F5F5F5" : "#FAFAFA"
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
                    storyboardPage.navigateTo("create", null)
                }
            }
        }
    }
}
