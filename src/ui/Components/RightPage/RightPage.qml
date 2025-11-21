import QtQuick
import QtQuick.Controls

Rectangle {
    id: home_right
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    color: "#FFFEFA"

    property string currentPage: "create"
    property string selectedStyle: ""
    property string storyText: ""
    property bool isGenerating: false
    property var currentProjectData: null

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
                case "shotDetail": return "../../Pages/ShotDetail.qml"
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

        // 【新增】监听子页面的跳转请求
        function onNavigateTo(page) {
            console.log("UI: 收到跳转请求 -> " + page);
            home_right.currentPage = page; // 真正的跳转发生在这里
        }
    }

    Connections {
        target: backendService

        function onStoryCreated(projectData) {
            // 4. 成功回来：隐藏遮罩
            home_right.isGenerating = false;

            // 跳转页面
            home_right.currentProjectData = projectData;
            home_right.currentPage = "storyboard";
        }

        function onErrorOccurred(msg) {
            // 5. 失败回来：隐藏遮罩
            home_right.isGenerating = false;
            console.error(msg);
        }
    }
}
