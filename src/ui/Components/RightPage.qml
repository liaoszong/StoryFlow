import QtQuick
import QtQuick.Controls

/**
 * 右侧内容区域 - 路由容器
 * 负责页面切换和全局状态管理
 */
Rectangle {
    id: home_right
    bottomRightRadius: 16
    color: "#F8FAFC"

    // ==================== 属性定义 ====================
    property string currentPage: "create"           // 当前页面
    property string selectedStyle: ""               // 选中的风格
    property string storyText: ""                   // 故事文本
    property bool isGenerating: storyViewModel.isGenerating  // 是否正在生成
    property var currentProjectData: null           // 当前项目数据
    property var currentShotData: null              // 当前分镜数据

    // ==================== 信号定义 ====================
    signal navigateTo(string page)

    // ==================== 页面加载器 ====================
    Loader {
        id: contentLoader
        anchors.fill: parent

        // 根据当前页面加载对应 QML
        source: {
            switch(currentPage) {
                case "create": return "../Pages/CreatePage.qml"
                case "storyboard": return "../Pages/StoryboardPage.qml"
                case "assets": return "../Pages/AssetsPage.qml"
                case "shotDetail": return "../Pages/ShotDetailPage.qml"
                case "preview": return "../Pages/PreviewPage.qml"
                default: return "../Pages/CreatePage.qml"
            }
        }

        // 页面加载完成后注入数据
        onLoaded: {
            if (!item) return

            console.log("RightPage onLoaded: currentPage =", home_right.currentPage)
            console.log("RightPage onLoaded: currentProjectData =", JSON.stringify(home_right.currentProjectData))

            // 注入项目数据
            if (item.hasOwnProperty("currentProjectData")) {
                console.log("RightPage: Injecting currentProjectData to page")
                item.currentProjectData = home_right.currentProjectData
            }
            if (item.hasOwnProperty("projectData")) {
                item.projectData = home_right.currentProjectData
            }

            // 注入分镜数据
            if (item.hasOwnProperty("shotData")) {
                item.shotData = home_right.currentShotData
            }

            // 注入风格设置
            if (item.hasOwnProperty("selectedStyle")) {
                item.selectedStyle = home_right.selectedStyle
            }

            // 双向绑定故事文本
            if (item.hasOwnProperty("storyText")) {
                item.storyText = home_right.storyText
                item.storyTextChanged.connect(function() {
                    home_right.storyText = item.storyText
                })
            }

            // Assets 页面自动刷新
            if (home_right.currentPage === "assets" && item.hasOwnProperty("allProjectsList")) {
                assetsViewModel.loadAssets()
            }
        }
    }

    // ==================== 加载遮罩 ====================
    Rectangle {
        id: loadingOverlay
        anchors.fill: parent
        color: Qt.rgba(255, 255, 255, 0.95)
        // preview 页面有自己的加载状态，不显示全局遮罩
        visible: storyViewModel.isGenerating && currentPage !== "preview"
        z: 100

        Column {
            anchors.centerIn: parent
            spacing: 20

            // 加载指示器
            BusyIndicator {
                running: storyViewModel.isGenerating
                anchors.horizontalCenter: parent.horizontalCenter
                scale: 1.2
            }

            // 状态消息
            Text {
                text: storyViewModel.statusMessage
                font.pixelSize: 15
                color: "#475569"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // 提示文字
            Text {
                text: "请稍候..."
                font.pixelSize: 13
                color: "#94A3B8"
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        // 拦截点击事件
        MouseArea {
            anchors.fill: parent
        }
    }

    // ==================== 信号连接 ====================

    // 监听子页面导航请求
    Connections {
        target: contentLoader.item
        ignoreUnknownSignals: true

        function onNavigateTo(page, payload) {
            console.log("RightPage Connections.onNavigateTo: page =", page, "payload =", JSON.stringify(payload))

            // 保存分镜数据
            if (page === "shotDetail" && payload) {
                home_right.currentShotData = payload
            }
            // 保存项目数据（用于 preview 页面）
            if (page === "preview" && payload) {
                home_right.currentProjectData = payload
                console.log("RightPage: 设置 currentProjectData for preview")
            }

            // 发出导航信号，让 Main.qml 更新 currentPage
            home_right.navigateTo(page)
        }

        // 监听分镜详情页数据更新
        function onShotDataUpdated(updatedShot) {
            console.log("RightPage: shotDataUpdated received:", JSON.stringify(updatedShot))
            if (updatedShot && updatedShot.shotId) {
                // 更新 currentShotData
                home_right.currentShotData = updatedShot

                // 同步更新到 projectData 中的 storyboards 列表
                if (home_right.currentProjectData && home_right.currentProjectData.storyboards) {
                    var proj = JSON.parse(JSON.stringify(home_right.currentProjectData))
                    var list = proj.storyboards
                    for (var i = 0; i < list.length; i++) {
                        if (list[i].shotId === updatedShot.shotId) {
                            // 更新所有字段
                            list[i] = updatedShot
                            break
                        }
                    }
                    home_right.currentProjectData = proj
                    console.log("RightPage: projectData updated with shot changes")
                }
            }
        }
    }

    // 监听 ViewModel 事件
    Connections {
        target: storyViewModel

        // 故事生成完成
        function onStoryCreated(projectData) {
            home_right.currentProjectData = projectData
            assetsViewModel.loadAssets()
            home_right.navigateTo("storyboard")
        }

        // 图片重新生成完成
        function onImageRegenerated(updatedShotPayload) {
            updateShotStatus(
                updatedShotPayload.shotId,
                updatedShotPayload.status,
                updatedShotPayload.localImagePath
            )
        }

        // 错误处理
        function onErrorOccurred(msg) {
            console.error("ViewModel 错误:", msg)
        }
    }

    // ==================== 辅助函数 ====================

    // 更新分镜状态
    function updateShotStatus(shotId, newStatus, newUrl) {
        // 更新当前分镜详情
        if (home_right.currentShotData && home_right.currentShotData.shotId === shotId) {
            var tempShot = Object.assign({}, home_right.currentShotData)
            tempShot.status = newStatus
            if (newUrl) tempShot.localImagePath = newUrl
            home_right.currentShotData = tempShot
        }

        // 更新项目中的分镜列表
        if (home_right.currentProjectData && home_right.currentProjectData.storyboards) {
            var proj = home_right.currentProjectData
            var list = proj.storyboards
            var found = false

            for (var i = 0; i < list.length; i++) {
                if (list[i].shotId === shotId) {
                    list[i].status = newStatus
                    if (newUrl) list[i].localImagePath = newUrl
                    found = true
                    break
                }
            }

            // 触发 QML 重新绑定
            if (found) {
                home_right.currentProjectData = JSON.parse(JSON.stringify(proj))
            }
        }
    }
}
