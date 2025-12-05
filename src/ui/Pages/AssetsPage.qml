import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Rectangle {
    id: assetsPage
    anchors.fill: parent
    color: "#F8FAFC"
    bottomRightRadius: 16

    property var allProjectsList: assetsViewModel.projectList
    property int currentFilter: 0

    signal navigateTo(string page, var data)

    ListModel { id: projectModel }

    FolderDialog {
        id: folderDialog
        title: "选择项目文件夹"
        onAccepted: {
            var path = selectedFolder.toString()
            assetsViewModel.scanProjects(path)
        }
    }

    onAllProjectsListChanged: updateProjectList()
    function forceUpdateUI() { updateProjectList() }
    Component.onCompleted: assetsViewModel.loadAssets()

    function isDateInRange(dateStr, filter) {
        if (filter === 0) return true
        if (!dateStr) return false
        var itemDate = new Date(dateStr)
        var now = new Date()
        var today = new Date(now.getFullYear(), now.getMonth(), now.getDate())
        if (filter === 1) return itemDate >= today
        if (filter === 2) {
            var weekStart = new Date(today)
            weekStart.setDate(today.getDate() - today.getDay())
            return itemDate >= weekStart
        }
        if (filter === 3) {
            var monthStart = new Date(now.getFullYear(), now.getMonth(), 1)
            return itemDate >= monthStart
        }
        return true
    }

    function updateProjectList() {
        projectModel.clear()
        var query = (searchInput.text || "").trim().toLowerCase()
        for (var i = 0; i < allProjectsList.length; i++) {
            var item = allProjectsList[i]
            if (!item) continue
            var matchName = query === "" || (item.name && item.name.toLowerCase().indexOf(query) !== -1)
            var matchTime = isDateInRange(item.updated_at || item.created_at, currentFilter)
            if (matchName && matchTime) {
                projectModel.append({
                    "name": item.name || "无名称",
                    "date": formatDate(item.updated_at || item.created_at),
                    "status": item.status || "draft",
                    "colorCode": getColorCode(i),
                    "coverUrl": item.thumbnail || "",
                    "originalIndex": i
                })
            }
        }
    }

    function formatDate(dateStr) {
        if (!dateStr) return ""
        return new Date(dateStr).toLocaleDateString()
    }

    function getColorCode(index) {
        var colors = ["#6366F1", "#8B5CF6", "#EC4899", "#F59E0B", "#10B981", "#3B82F6"]
        return colors[index % colors.length]
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32
        spacing: 24

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            ColumnLayout {
                spacing: 4
                Text { text: "Assets"; font.pixelSize: 28; font.weight: Font.Bold; color: "#1E293B" }
                Text { text: projectModel.count + " 个项目"; font.pixelSize: 13; color: "#64748B" }
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                Layout.preferredWidth: 240
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
                    Text { text: "🔍"; font.pixelSize: 14; color: "#94A3B8" }
                    TextField {
                        id: searchInput
                        Layout.fillWidth: true
                        placeholderText: "搜索项目..."
                        placeholderTextColor: "#94A3B8"
                        font.pixelSize: 13
                        color: "#1E293B"
                        background: null
                        selectByMouse: true
                        onTextChanged: assetsPage.updateProjectList()
                    }
                }
            }

            Button {
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
                background: Rectangle { color: parent.hovered ? "#F1F5F9" : "#FFFFFF"; radius: 10; border.color: "#E2E8F0"; border.width: 1 }
                contentItem: Text { text: "🔄"; font.pixelSize: 16; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                onClicked: assetsViewModel.loadAssets()
                ToolTip.visible: hovered
                ToolTip.text: "刷新列表"
            }

            Button {
                Layout.preferredHeight: 40
                background: Rectangle { color: parent.hovered ? "#EEF2FF" : "#FFFFFF"; radius: 10; border.color: "#6366F1"; border.width: 1 }
                contentItem: RowLayout {
                    spacing: 6
                    Text { text: "📁"; font.pixelSize: 14 }
                    Text { text: "选择文件夹"; font.pixelSize: 13; color: "#6366F1" }
                }
                leftPadding: 12
                rightPadding: 12
                onClicked: folderDialog.open()
            }
        }

        RowLayout {
            spacing: 16
            Repeater {
                model: [{ text: "全部", filter: 0 }, { text: "今天", filter: 1 }, { text: "本周", filter: 2 }, { text: "本月", filter: 3 }]
                TabItem {
                    required property var modelData
                    text: modelData.text
                    isActive: currentFilter === modelData.filter
                    onClicked: { currentFilter = modelData.filter; updateProjectList() }
                }
            }
            Item { Layout.fillWidth: true }
            Text {
                visible: assetsViewModel.scanPath !== ""
                text: "📂 " + assetsViewModel.scanPath
                font.pixelSize: 12
                color: "#94A3B8"
                elide: Text.ElideMiddle
                Layout.maximumWidth: 300
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Column {
                anchors.centerIn: parent
                spacing: 16
                visible: projectModel.count === 0
                Text { text: "📭"; font.pixelSize: 64; anchors.horizontalCenter: parent.horizontalCenter }
                Text { text: assetsViewModel.scanPath === "" ? "请选择项目文件夹" : "没有找到项目"; font.pixelSize: 18; font.weight: Font.DemiBold; color: "#64748B"; anchors.horizontalCenter: parent.horizontalCenter }
                Text { text: assetsViewModel.scanPath === "" ? "点击上方「选择文件夹」按钮" : "尝试更改搜索条件"; font.pixelSize: 13; color: "#94A3B8"; anchors.horizontalCenter: parent.horizontalCenter }
                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: assetsViewModel.scanPath === "" ? "选择文件夹" : "刷新"
                    background: Rectangle { color: parent.hovered ? "#4F46E5" : "#6366F1"; radius: 8 }
                    contentItem: Text { text: parent.text; font.pixelSize: 14; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    leftPadding: 24; rightPadding: 24; topPadding: 10; bottomPadding: 10
                    onClicked: { if (assetsViewModel.scanPath === "") folderDialog.open(); else assetsViewModel.loadAssets() }
                }
            }

            GridView {
                id: assetGrid
                anchors.fill: parent
                clip: true
                visible: projectModel.count > 0
                cellWidth: 260
                cellHeight: 280
                model: projectModel

                delegate: Rectangle {
                    width: assetGrid.cellWidth - 16
                    height: assetGrid.cellHeight - 16
                    color: "#FFFFFF"
                    radius: 12
                    border.color: cardMouse.containsMouse ? "#6366F1" : "#E2E8F0"
                    border.width: cardMouse.containsMouse ? 2 : 1

                    MouseArea {
                        id: cardMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            var fullData = assetsPage.allProjectsList[originalIndex].fullData
                            assetsPage.navigateTo("preview", fullData)
                        }
                    }

                    Button {
                        width: 30; height: 30
                        anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 8
                        z: 10
                        background: Rectangle { color: parent.hovered ? "#FFEBEE" : "white"; radius: 15; border.color: "#E2E8F0"; border.width: 1 }
                        contentItem: Text { text: "🗑️"; font.pixelSize: 14; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                        onClicked: { var projectPath = assetsPage.allProjectsList[originalIndex].path; assetsViewModel.deleteProject(projectPath) }
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 140
                            radius: 8
                            color: colorCode
                            clip: true

                            Rectangle {
                                anchors.fill: parent
                                color: colorCode
                                visible: coverImg.status !== Image.Ready
                                Text { anchors.centerIn: parent; text: name.charAt(0).toUpperCase(); font.pixelSize: 36; font.weight: Font.Bold; color: "white" }
                            }

                            Image { id: coverImg; anchors.fill: parent; source: coverUrl; fillMode: Image.PreserveAspectCrop; visible: source !== ""; asynchronous: true }

                            Rectangle {
                                anchors.top: parent.top; anchors.left: parent.left; anchors.margins: 8
                                width: statusText.width + 12; height: 22; radius: 11; color: "#FFFFFF"
                                Text { id: statusText; anchors.centerIn: parent; text: status === "completed" ? "已完成" : "草稿"; font.pixelSize: 10; font.weight: Font.Bold; color: status === "completed" ? "#166534" : "#B45309" }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Text { text: name; font.pixelSize: 15; font.weight: Font.DemiBold; color: "#1E293B"; Layout.fillWidth: true; elide: Text.ElideRight }
                            RowLayout {
                                Layout.fillWidth: true
                                Rectangle { width: 6; height: 6; radius: 3; color: status === "completed" ? "#22C55E" : "#F59E0B" }
                                Text { text: status === "completed" ? "已完成" : "进行中"; color: "#64748B"; font.pixelSize: 12 }
                                Item { Layout.fillWidth: true }
                                Text { text: date; color: "#94A3B8"; font.pixelSize: 11 }
                            }
                        }
                        Item { Layout.fillHeight: true }
                    }
                }
            }
        }
    }

    component TabItem : Rectangle {
        property string text
        property bool isActive
        signal clicked()
        width: tabLabel.width + 16
        height: tabLabel.height + 12
        color: isActive ? "#EEF2FF" : "transparent"
        radius: 6
        Text { id: tabLabel; anchors.centerIn: parent; text: parent.text; font.pixelSize: 14; font.weight: isActive ? Font.DemiBold : Font.Normal; color: isActive ? "#6366F1" : "#64748B" }
        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: parent.clicked() }
    }
}
