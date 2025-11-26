import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: assetsPage
    anchors.fill: parent
    color: "#F0F2F5" // 浅灰背景
    bottomRightRadius: 20

    // 1.保存所有项目 (Array),等待 RightPage 注入数据
    property var allProjectsList: []

    signal navigateTo(string page, var data)

    // 2.给 GridView 用 (ListModel)
    ListModel {
        id: projectModel
    }

    // 监听数据源变化，一旦 RightPage 传数据进来，就刷新 UI
    onAllProjectsListChanged: {
        console.log("AssetsPage: 收到数据更新，数量:", allProjectsList.length)
        updateSearch(searchInput.text)
    }

    // 供外部强制调用的刷新接口
    function forceUpdateUI() {
        updateSearch(searchInput.text)
    }

    // 页面加载时初始化模拟数据
    Component.onCompleted: {
        // 模拟从后端拿到的 JSON 数据
        var mockData = [
            { id: "1", name: "勇敢猫咪的冒险", date: "2025-11-24", status: "completed", colorCode: "#FFCDD2" },
            { id: "2", name: "赛博朋克 2077", date: "2025-11-23", status: "draft", colorCode: "#BBDEFB" },
            { id: "3", name: "清晨的森林", date: "2025-11-22", status: "completed", colorCode: "#C8E6C9" },
            { id: "4", name: "未命名故事 01", date: "2025-11-20", status: "draft", colorCode: "#E1BEE7" },
            { id: "5", name: "机甲战士", date: "2025-11-19", status: "completed", colorCode: "#FFE0B2" }
        ]

        // 存入源数据
        assetsPage.allProjectsList = mockData
        // 初始显示全部
        updateSearch("")
    }

    // 3. 搜索与刷新逻辑
    function updateSearch(keyword) {
        projectModel.clear() // 清空旧的

        var query = (keyword || "").trim().toLowerCase()

        // 遍历父级传来的 list
        for (var i = 0; i < allProjectsList.length; i++) {
            var item = allProjectsList[i]

            // 搜索匹配
            if (query === "" || (item.name && item.name.toLowerCase().indexOf(query) !== -1)) {
                // 把数据转成 ListElement 格式塞进去
                projectModel.append({
                                        "name": item.name,
                                        "date": item.date,
                                        "status": item.status,
                                        "colorCode": item.colorCode || "#CCCCCC",
                                        "coverUrl": item.coverUrl || "",
                                        // 注意：ListModel 只能存简单数据类型，对象要拆开或转字符串
                                        // 如果点击需要跳转，可以通过 index 去 allProjectsList 里找原始对象
                                        "originalIndex": i
                                    })
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 40
        spacing: 20

        // --- 顶部栏 ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            Text {
                text: "Assets"
                font.pixelSize: 32
                font.weight: Font.Bold
                color: "#333333"
            }

            Item { Layout.fillWidth: true } // 占位弹簧

            // --- 搜索框组件 ---
            Rectangle {
                Layout.preferredWidth: 320
                Layout.preferredHeight: 44
                color: "#FFFFFF"
                radius: 22
                border.color: searchInput.activeFocus ? "#1976D2" : "#E0E0E0"
                border.width: searchInput.activeFocus ? 2 : 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 10

                    Text {
                        text: "🔍"; font.pixelSize: 14; color: "#999999"
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    TextField {
                        id: searchInput
                        Layout.fillWidth: true
                        placeholderText: "搜索故事名称..."
                        font.pixelSize: 14
                        color: "#333333"
                        background: null
                        selectByMouse: true
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter

                        // 当文字改变时，触发搜索函数
                        onTextChanged: {
                            assetsPage.updateSearch(text)
                        }
                    }
                }
            }
        }

        // --- 筛选标签 (UI展示) ---
        Row {
            spacing: 25
            Text {
                text: "全部项目"
                font.pixelSize: 15; font.weight: Font.Bold; color: "#1976D2"
                Rectangle { width: parent.width; height: 3; color: "#1976D2"; radius: 1.5; anchors.top: parent.bottom; anchors.topMargin: 4 }
            }
            Text { text: "草稿箱"; font.pixelSize: 15; color: "#666666" }
        }

        // --- 网格展示区 ---
        GridView {
            id: assetGrid
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            cellWidth: 260
            cellHeight: 300

            // 绑定到那个动态变化的 Model
            model: projectModel

            delegate: Rectangle {
                width: assetGrid.cellWidth - 20
                height: assetGrid.cellHeight - 20
                color: "#FFFFFF"
                radius: 12
                border.color: hoverHandler.hovered ? "#1976D2" : "#EEEEEE"
                border.width: hoverHandler.hovered ? 2 : 1

                MouseArea {
                    id: hoverHandler
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        console.log("点击项目:", name)
                        // 通过 originalIndex 找到原始的完整数据
                        var fullData = assetsPage.allProjectsList[originalIndex].fullData
                        // 发送信号（注意：需要 assetsPage 定义 signal navigateTo(page, payload)）
                        assetsPage.navigateTo("storyboard", fullData)
                    }
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    // 封面
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 150 // 保持高度不变
                        radius: 8
                        color: colorCode // 随机底色
                        clip: true       // 裁剪超出圆角的部分

                        // 1. 底色层 (当没有图片时显示这个颜色)
                        Rectangle {
                            anchors.fill: parent
                            color: colorCode
                            visible: img.status !== Image.Ready

                            // 默认首字母图标
                            Text {
                                anchors.centerIn: parent
                                text: name.charAt(0)
                                font.pixelSize: 40
                                color: "white"
                            }
                        }

                        // 2. 图片层
                        Image {
                            id: img
                            anchors.fill: parent
                            source: coverUrl
                            // 【关键】保持比例裁剪，填满整个区域，效果最好
                            fillMode: Image.PreserveAspectCrop
                            visible: source !== ""

                            // 异步加载，防止卡顿
                            asynchronous: true
                        }

                        // 3. 状态标签 (保持在最上层)
                        Rectangle {
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.margins: 8
                            width: status === "completed" ? 50 : 40
                            height: 22
                            radius: 11
                            color: "white"
                            opacity: 0.9

                            Text {
                                anchors.centerIn: parent
                                text: status === "completed" ? "完成" : "草稿"
                                font.pixelSize: 11
                                font.weight: Font.Medium
                                color: status === "completed" ? "#2E7D32" : "#EF6C00"
                            }
                        }
                    }

                    // 信息
                    Text { text: name; font.pixelSize: 16; font.weight: Font.Bold; color: "#333333"; Layout.fillWidth: true; elide: Text.ElideRight }
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: status === "completed" ? "✅ 完成" : "📝 草稿"; color: status === "completed" ? "green" : "orange"; font.pixelSize: 12 }
                        Item { Layout.fillWidth: true }
                        Text { text: date; color: "#999"; font.pixelSize: 12 }
                    }
                    Item { Layout.fillHeight: true }
                }
            }
        }
    }
}
