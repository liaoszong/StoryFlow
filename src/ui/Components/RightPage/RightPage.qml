import QtQuick
import QtQuick.Controls

Rectangle {
    id: home_right
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    color: "#FFFFFF"
    border.color: "#EAEAEA"
    border.width: 1

    property string currentPage: "create"
    property string selectedStyle: ""
    property string storyText: ""

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
                // 设置公共属性
                if (item.hasOwnProperty("selectedStyle")) {
                    item.selectedStyle = Qt.binding(function() { return home_right.selectedStyle })
                }
                if (item.hasOwnProperty("storyText")) {
                    item.storyText = Qt.binding(function() { return home_right.storyText })
                }

                // 连接信号
                if (item.hasOwnProperty("styleSelected")) {
                    item.styleSelected.connect(home_right.styleSelected)
                }
                if (item.hasOwnProperty("generateStory")) {
                    item.generateStory.connect(home_right.generateStory)
                }
                if (item.hasOwnProperty("navigateTo")) {
                    item.navigateTo.connect(home_right.navigateTo)
                }
            }
        }
    }
}
