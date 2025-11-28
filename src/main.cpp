#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "core/net/ApiService.h"
#include "core/viewmodel/StoryViewModel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 1. 创建网络服务 (全局唯一)
    ApiService apiService;
    // (如果已有 Token，可以在这里 setToken，或者后续通过登录 ViewModel 设置)

    // 2. 创建 ViewModel，并注入网络服务
    StoryViewModel storyViewModel(&apiService);

    QQmlApplicationEngine engine;

    // 3. 将 ViewModel 注册给 QML
    // 注意：以前是 backendService，现在我们按功能注册，更清晰
    engine.rootContext()->setContextProperty("storyViewModel", &storyViewModel);

    // 如果之后有 assetsViewModel，也是这样注册：
    // engine.rootContext()->setContextProperty("assetsViewModel", &assetsViewModel);

    engine.loadFromModule("StoryFlow", "Main");

    return app.exec();
}
