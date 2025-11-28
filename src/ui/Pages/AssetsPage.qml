import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * 资产管理页面 - Assets
 * 展示所有已创建的故事项目，支持搜索和筛选
 */
Rectangle {
    id: assetsPage
    anchors.fill: parent
    color: "#F8FAFC"
    bottomRightRadius: 16

    // ==================== 属性定义 ====================
    property var allProjectsList: assetsViewModel.projectList  // 绑定 C++ 数据

    // ==================== 信号定义 ====================
    signal navigateTo(string page, var data)

    // 项目列表模型
    ListModel {
        id: projectModel
    }

    // 监听数据源变化
    onAllProjectsListChanged: {
        updateSearch(searchInput.text)
    }

    // 强制刷新接口
    function forceUpdateUI() {
        updateSearch(searchInput.text)
    }

    // 页面加载时获取数据
    Component.onCompleted: {
        assetsViewModel.loadAssets()
    }

    // 搜索过滤函数
    function updateSearch(keyword) {
        projectModel.clear()
        var query = (keyword || "").trim().toLowerCase()

        for (var i = 0; i < allProjectsList.length; i++) {
            var item = allProjectsList[i]
            // 增加 item 的空值检查
            if (!item) {
                console.warn("AssetsPage: 发现空项目数据，已跳过。");
                continue; // 跳过此项，防止崩溃
            }
            if (query === "" || (item.name && item.name.toLowerCase().indexOf(query) !== -1)) {
                projectModel.append({
                    "name": item.name || "无名称",
                    "date": item.date,
                    "status": item.status,
                    "colorCode": item.colorCode || "#6366F1",
                    "coverUrl": item.coverUrl || "",
                    "originalIndex": i
                })
            }
        }
    }

    // ==================== 主布局 ====================
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32
        spacing: 24

        // 顶部标题栏
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            // 标题区域
            ColumnLayout {
                spacing: 4

                Text {
                    text: "Assets"
                    font.pixelSize: 28
                    font.weight: Font.Bold
                    color: "#1E293B"
                }

                Text {
                    text: projectModel.count + " 个项目"
                    font.pixelSize: 13
                    color: "#64748B"
                }
            }

            Item { Layout.fillWidth: true }

            // 搜索框
            Rectangle {
                Layout.preferredWidth: 280
                Layout.preferredHeight: 40
                color: "#FFFFFF"
                radius: 10
                border.color: searchInput.activeFocus ? "#6366F1" : "#E2E8F0"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 8

                    // 搜索图标
                    Text {
                        text: "\uD83D\uDD0D"
                        font.pixelSize: 14
                        color: "#94A3B8"
                    }

                    TextField {
                        id: searchInput
                        Layout.fillWidth: true
                        placeholderText: "搜索项目..."
                        placeholderTextColor: "#94A3B8"
                        font.pixelSize: 13
                        color: "#1E293B"
                        background: null
                        selectByMouse: true

                        onTextChanged: {
                            assetsPage.updateSearch(text)
                        }
                    }
                }
            }
        }

        // 筛选标签栏
        Row {
            spacing: 24

            // 全部项目标签
            TabItem {
                text: "全部项目"
                isActive: true
            }

            // 草稿箱标签
            TabItem {
                text: "草稿箱"
                isActive: false
            }
        }

        // ==================== 项目网格 ====================
        GridView {
            id: assetGrid
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            cellWidth: 260
            cellHeight: 280

            model: projectModel

            // 项目卡片代理
            delegate: Rectangle {
                width: assetGrid.cellWidth - 16
                height: assetGrid.cellHeight - 16
                color: "#FFFFFF"
                radius: 12
                border.color: hoverHandler.containsMouse ? "#6366F1" : "#E2E8F0"
                border.width: hoverHandler.containsMouse ? 2 : 1

                MouseArea {
                    id: hoverHandler
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor

                    onClicked: {
                        var fullData = assetsPage.allProjectsList[originalIndex].fullData
                        assetsPage.navigateTo("storyboard", fullData)
                    }
                }

                Button {
                    width: 30
                    height: 30
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: 8 // 稍微大一点间距
                    z: 10 // 确保在最上层

                    background: Rectangle {
                        color: parent.hovered ? "#FFEBEE" : "white"
                        radius: 15
                        opacity: 0.9
                        border.color: "#E2E8F0" // 加个边框更明显
                        border.width: 1
                    }

                    contentItem: Text {
                        text: "🗑️"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        // 【防御编程】使用 model.name 替代 modelData.name
                        var n = name || "未知项目"
                        console.log("删除项目:", n)

                        // 获取 ID 并删除
                        var pid = assetsPage.allProjectsList[index].id
                        assetsViewModel.deleteProject(pid)
                    }
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    // 封面图片
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140
                        radius: 8
                        color: colorCode
                        clip: true

                        // 默认背景（无图片时显示首字母）
                        Rectangle {
                            anchors.fill: parent
                            color: colorCode
                            visible: img.status !== Image.Ready

                            Text {
                                anchors.centerIn: parent
                                text: name.charAt(0).toUpperCase()
                                font.pixelSize: 36
                                font.weight: Font.Bold
                                color: "white"
                            }
                        }

                        // 封面图片
                        Image {
                            id: img
                            anchors.fill: parent
                            source: coverUrl
                            fillMode: Image.PreserveAspectCrop
                            visible: source !== ""
                            asynchronous: true
                        }

                        // 状态标签
                        Rectangle {
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.margins: 8
                            width: badgeText.width + 12
                            height: 22
                            radius: 11
                            color: "#FFFFFF"

                            Text {
                                id: badgeText
                                anchors.centerIn: parent
                                text: status === "completed" ? "已完成" : "草稿"
                                font.pixelSize: 10
                                font.weight: Font.Bold
                                color: status === "completed" ? "#166534" : "#B45309"
                            }
                        }
                    }

                    // 项目信息
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        // 项目名称
                        Text {
                            text: name
                            font.pixelSize: 15
                            font.weight: Font.DemiBold
                            color: "#1E293B"
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }

                        // 状态和日期
                        RowLayout {
                            Layout.fillWidth: true

                            // 状态指示点
                            Rectangle {
                                width: 6
                                height: 6
                                radius: 3
                                color: status === "completed" ? "#22C55E" : "#F59E0B"
                            }

                            Text {
                                text: status === "completed" ? "已完成" : "进行中"
                                color: "#64748B"
                                font.pixelSize: 12
                            }

                            Item { Layout.fillWidth: true }

                            // 日期
                            Text {
                                text: date
                                color: "#94A3B8"
                                font.pixelSize: 11
                            }
                        }
                    }

                    Item { Layout.fillHeight: true }
                }
            }
        }
    }

    // ==================== 标签页组件 ====================
    component TabItem : Rectangle {
        property string text      // 标签文字
        property bool isActive    // 是否激活

        width: tabText.width
        height: tabText.height + 8
        color: "transparent"

        Text {
            id: tabText
            text: parent.text
            font.pixelSize: 14
            font.weight: isActive ? Font.DemiBold : Font.Normal
            color: isActive ? "#6366F1" : "#64748B"
        }

        // 激活指示条
        Rectangle {
            visible: isActive
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 2
            radius: 1
            color: "#6366F1"
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
        }
    }
}
