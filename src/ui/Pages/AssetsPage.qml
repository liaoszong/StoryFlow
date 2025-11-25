import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: assetsPage
    anchors.fill: parent
    color: "#F0F2F5" // 浅灰背景
    bottomRightRadius: 20

    // 1.保存所有项目 (Array)
    property var allProjectsList: []

    // 2.给 GridView 用 (ListModel)
    ListModel {
        id: projectModel
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

    // 【3. 核心搜索逻辑】
    function updateSearch(keyword) {
        projectModel.clear() // 先清空显示

        var query = keyword.trim().toLowerCase() // 转小写，忽略大小写差异

        for (var i = 0; i < allProjectsList.length; i++) {
            var item = allProjectsList[i]

            // 如果搜索词为空，或者名字里包含搜索词
            if (query === "" || item.name.toLowerCase().indexOf(query) !== -1) {
                projectModel.append(item)
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
                    onClicked: console.log("点击项目:", name)
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    // 封面
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 150
                        radius: 8
                        color: colorCode // 使用数据里的颜色

                        Text {
                            anchors.centerIn: parent
                            text: name.charAt(0) // 取首字做图标
                            font.pixelSize: 40
                            color: "white"
                        }
                    }

                    // 标题
                    Text {
                        text: name
                        font.pixelSize: 16
                        font.weight: Font.Bold
                        color: "#333333"
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    // 状态与时间
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: status === "completed" ? "✅ 完成" : "📝 草稿"
                            color: status === "completed" ? "green" : "orange"
                            font.pixelSize: 12
                        }
                        Item { Layout.fillWidth: true }
                        Text { text: date; color: "#999"; font.pixelSize: 12 }
                    }

                    Item { Layout.fillHeight: true }
                }
            }
        }
    }
}
