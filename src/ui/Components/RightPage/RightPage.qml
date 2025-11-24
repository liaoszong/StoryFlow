import QtQuick
import QtQuick.Controls

Rectangle {
    id: home_right
    // anchors.top: parent.top
    // anchors.left: home_left.right
    // anchors.right: parent.right
    // anchors.bottom: parent.bottom
    color: "#FFFEFA"

    property string currentPage: "create"
    property string selectedStyle: ""
    property string storyText: ""
    property bool isGenerating: false
    property var currentProjectData: null
    property var currentShotData: null

    signal styleSelected(string style)
    signal generateStory()
    signal navigateTo(string page)

    // 动态加载不同页面内容
    Loader {
        id: contentLoader
        anchors.fill: parent
        source: {
            switch(currentPage) {
                case "create": return "../../Pages/CreatePage.qml"
                case "storyboard": return "../../Pages/StoryboardPage.qml"
                case "assets": return "../../Pages/AssetsPage.qml"
                case "shotDetail": return "../../Pages/ShotDetailPage.qml"
                case "preview": return "../../Pages/PreviewPage.qml"
                default: return "../../Pages/CreatePage.qml"
            }
        }

        onLoaded: {
            if (item) {
                // 1. 传递数据给子页面（分镜页需要的数据）
                if (item.hasOwnProperty("currentProjectData")) {
                    item.currentProjectData = home_right.currentProjectData
                }

                // 2. 把选中的风格往下传
                if (item.hasOwnProperty("selectedStyle")) {
                    item.selectedStyle = home_right.selectedStyle
                }
                // 分镜详细页
                if (item.hasOwnProperty("shotData")) {
                    item.shotData = home_right.currentShotData
                }

                // 3. 监听子页面的输入
                // 如果子页面里有 storyText 属性，一旦它变了，就同步给父页面
                if (item.hasOwnProperty("storyText")) {
                    // 初始化：把父页面的值给子页面（防止切回来文字没了）
                    item.storyText = home_right.storyText
                    // 监听：子页面变了 -> 父页面跟着变
                    item.storyTextChanged.connect(function() {
                        home_right.storyText = item.storyText
                    })
                }

                // 4. 信号连接：监听 Regenerate Image (ShotDetailPage)
                if (item.hasOwnProperty("regenerateImage")) {
                    item.regenerateImage.connect(function(shotId, newPrompt){
                        console.log("UI: 收到重绘请求 ->", shotId);

                        // 立即将状态改为 "generating" 以显示 loading
                        updateShotStatus(shotId, "generating");

                        // 调用 C++ 后端
                        // 此时我们需要 ProjectID，假设它存储在 currentProjectData.id 中
                        var projId = home_right.currentProjectData.id || "temp_id";
                        var style = home_right.selectedStyle || "animation";

                        // 调用 backend.cpp 中的 regenerateImage
                        backendService.regenerateImage(projId, shotId, newPrompt, style);
                    });
                }

                // 连接 generateStory 等信号
                if (item.hasOwnProperty("generateStory")) {
                    // item.generateStory.connect(home_right.generateStory)
                }

                if (item.hasOwnProperty("projectData")) {
                    item.projectData = home_right.currentProjectData
                }
            }
        }
    }

    // 辅助函数：更新本地数据状态（避免等待服务器时界面无反应）
        function updateShotStatus(shotId, newStatus, newUrl) {
            // A. 更新 ShotDetail 当前查看的数据
            if (home_right.currentShotData && home_right.currentShotData.shotId === shotId) {
                var tempShot = Object.assign({}, home_right.currentShotData);
                tempShot.status = newStatus;
                if (newUrl) tempShot.localImagePath = newUrl;
                home_right.currentShotData = tempShot; // 触发绑定更新
            }

            // B. 更新 Storyboard 列表中的数据
            if (home_right.currentProjectData && home_right.currentProjectData.storyboards) {
                var proj = home_right.currentProjectData;
                var list = proj.storyboards;
                for (var i = 0; i < list.length; i++) {
                    if (list[i].shotId === shotId) {
                        list[i].status = newStatus;
                        if (newUrl) list[i].localImagePath = newUrl;
                        break;
                    }
                }
                // 强制触发 QML 更新 (QML 对数组内部属性变化不敏感，需要重置引用)
                home_right.currentProjectData = JSON.parse(JSON.stringify(proj));
            }
        }

    // 2. 加载遮罩 (放在 Loader 下面写，就会盖在 Loader 上面)
    Rectangle {
        id: loadingOverlay
        anchors.fill: parent
        color: "#80FFFFFF" // 半透明白色
        visible: home_right.isGenerating // 由属性控制显示/隐藏
        z: 100 // 在最上层

        // 忙碌指示器（转圈圈动画）
        BusyIndicator {
            anchors.centerIn: parent
            running: home_right.isGenerating
            width: 60
            height: 60
        }

        // 提示文字
        Text {
            anchors.top: parent.verticalCenter
            anchors.topMargin: 40
            anchors.horizontalCenter: parent.horizontalCenter
            text: "AI 正在构思故事，请稍候..."
            font.pixelSize: 16
            color: "#666666"
        }

        // 遮罩出现时，拦截鼠标点击，防止用户乱点
        MouseArea {
            anchors.fill: parent
        }
    }

    Connections {
        target: contentLoader.item
        ignoreUnknownSignals: true

        function onGenerateStory() {
            // 3. 开始生成：显示遮罩
            home_right.isGenerating = true;

            // 调用后端
            var prompt = home_right.storyText;
            var style = home_right.selectedStyle;
            backendService.createStory(prompt, style);
        }

        function onNavigateTo(page, payload) {
            console.log("UI: 跳转请求 -> " + page);
            console.log("DEBUG: 接收到的 payload ID:", payload ? payload.shotId : "NULL");

            // 去详情页，先把数据存起来
            if (page === "shotDetail" && payload) {
                home_right.currentShotData = payload;
            }

            // 同时更新 MainWindow 的当前页面，这样 LeftPage 的选中状态才会更新
            home_right.navigateTo(page); // 这会触发 Main.qml 中的处理
        }
    }

    Connections {
        target: backendService

        function onStoryCreated(projectData) {
            // 4. 成功回来：隐藏遮罩
            home_right.isGenerating = false;

            // 跳转页面
            home_right.currentProjectData = projectData;
            home_right.navigateTo("storyboard");
        }

        // 2. 单图重绘成功
        function onImageRegenerated(updatedShotPayload) {
            console.log("UI: 图片重绘成功 ->", updatedShotPayload.shotId);
            // 调用辅助函数更新数据模型
            home_right.updateShotStatus(
                        updatedShotPayload.shotId,
                        updatedShotPayload.status,
                        updatedShotPayload.localImagePath
                        );
        }

        function onErrorOccurred(msg) {
            // 5. 失败回来：隐藏遮罩
            home_right.isGenerating = false;
            console.error(msg);
        }
    }
}
