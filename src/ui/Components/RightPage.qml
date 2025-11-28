import QtQuick
import QtQuick.Controls

// 路由容器

Rectangle {
    id: home_right
    bottomRightRadius: 20
    color: "#FFFEFA"

    // =========================================================
    // 1. UI 状态属性 (View State)
    // =========================================================
    property string currentPage: "create"
    property string selectedStyle: ""
    property string storyText: ""

    // *直接绑定 ViewModel 的状态，无需手动控制 true/false
    property bool isGenerating: storyViewModel.isGenerating

    // *用于在页面间传递 (路由参数)
    // 理想情况下 C++ ViewModel 应该有一个 currentProject 属性，但现在先由 UI 暂存
    property var currentProjectData: null
    property var currentShotData: null

    // =========================================================
    // 2. 信号定义 (仅保留导航相关)
    // =========================================================
    signal navigateTo(string page)

    // =========================================================
    // 3. 页面加载器 (Router)
    // =========================================================
    Loader {
        id: contentLoader
        anchors.fill: parent
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

        // *只传递数据 (Props)，不连接业务信号 (Actions)
        onLoaded: {
            if (!item) return;

            // --- A. 数据注入 (Data Injection) ---
            // 类似于 React/Vue 的 Props 传递

            // 1. 通用数据
            if (item.hasOwnProperty("currentProjectData")) {
                item.currentProjectData = home_right.currentProjectData
            }
            if (item.hasOwnProperty("projectData")) { // 兼容不同命名
                item.projectData = home_right.currentProjectData
            }
            if (item.hasOwnProperty("shotData")) {
                item.shotData = home_right.currentShotData
            }

            // 2. CreatePage 状态保持
            if (item.hasOwnProperty("selectedStyle")) {
                item.selectedStyle = home_right.selectedStyle
            }
            if (item.hasOwnProperty("storyText")) {
                item.storyText = home_right.storyText
                // 双向绑定输入框内容
                item.storyTextChanged.connect(function() {
                    home_right.storyText = item.storyText
                })
            }

            // 3. AssetsPage 自动刷新逻辑 (属于 View 的生命周期管理)
            if (home_right.currentPage === "assets" && item.hasOwnProperty("allProjectsList")) {
                assetsViewModel.loadAssets()
            }
        }
    }

    // =========================================================
    // 4. 全局 Loading 遮罩 (Global UI)
    // =========================================================
    Rectangle {
        id: loadingOverlay
        anchors.fill: parent
        color: "#80FFFFFF"
        visible: storyViewModel.isGenerating // 自动响应 C++ 状态
        z: 100

        BusyIndicator {
            anchors.centerIn: parent
            running: storyViewModel.isGenerating
        }
        Text {
            anchors.top: parent.verticalCenter
            anchors.topMargin: 40
            anchors.horizontalCenter: parent.horizontalCenter
            text: storyViewModel.statusMessage // 自动响应 C++ 消息
            font.pixelSize: 16
            color: "#666666"
        }
        MouseArea { anchors.fill: parent } // 拦截点击
    }

    // =========================================================
    // 5. 信号处理 (Controller Logic)
    // =========================================================

    // A. 监听子页面的导航请求 (Router Logic)
    Connections {
        target: contentLoader.item
        ignoreUnknownSignals: true

        function onNavigateTo(page, payload) {
            console.log("Router: 导航 -> " + page);

            // 如果跳转带了参数（如点击分镜详情），暂存到 Router
            if (page === "shotDetail" && payload) {
                home_right.currentShotData = payload;
            }

            // 执行跳转
            home_right.navigateTo(page);
        }

        // 注意：这里不再处理 onGenerateStory，子页面自己去调 ViewModel
    }

    // B. 监听 ViewModel 的业务结果 (Data Sync)
    Connections {
        target: storyViewModel

        // 故事生成成功 -> 更新数据并跳转
        function onStoryCreated(projectData) {
            console.log("ViewModel: 故事生成完毕");
            home_right.currentProjectData = projectData;

            // 通知资产页刷新 (后台)
            assetsViewModel.loadAssets()

            // 路由跳转
            home_right.navigateTo("storyboard");
        }

        // 图片重绘成功 -> 局部更新 UI 数据
        function onImageRegenerated(updatedShotPayload) {
            console.log("ViewModel: 图片更新 ->", updatedShotPayload.shotId);
            updateShotStatus(
                updatedShotPayload.shotId,
                updatedShotPayload.status,
                updatedShotPayload.localImagePath
            );
        }

        // 错误处理
        function onErrorOccurred(msg) {
            console.error("ViewModel Error:", msg);
            // 这里建议未来加一个 Toast 组件显示错误
        }
    }

    // =========================================================
    // 6. 辅助函数 (Local Helper)
    // =========================================================

    // 用于在不重新请求 C++ 全量数据的情况下，局部刷新界面显示
    function updateShotStatus(shotId, newStatus, newUrl) {
        // 更新当前选中的分镜 (ShotDetail)
        if (home_right.currentShotData && home_right.currentShotData.shotId === shotId) {
            var tempShot = Object.assign({}, home_right.currentShotData);
            tempShot.status = newStatus;
            if (newUrl) tempShot.localImagePath = newUrl;
            home_right.currentShotData = tempShot;
        }

        // 更新项目列表中的分镜 (Storyboard)
        if (home_right.currentProjectData && home_right.currentProjectData.storyboards) {
            var proj = home_right.currentProjectData;
            var list = proj.storyboards;
            var found = false;
            for (var i = 0; i < list.length; i++) {
                if (list[i].shotId === shotId) {
                    list[i].status = newStatus;
                    if (newUrl) list[i].localImagePath = newUrl;
                    found = true;
                    break;
                }
            }
            // 强制触发 QML 绑定更新 (Hack: 只有对象引用变了 QML 才会重绘)
            if (found) {
                home_right.currentProjectData = JSON.parse(JSON.stringify(proj));
            }
        }
    }
}
