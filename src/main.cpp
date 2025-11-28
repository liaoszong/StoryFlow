#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "core/net/ApiService.h"
#include "core/viewmodel/StoryViewModel.h"
#include "core/viewmodel/AssetsViewModel.h" // 【新增】引入头文件

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Basic");

    // 1. 创建网络服务
    ApiService apiService;

    // 2. 创建 ViewModel
    StoryViewModel storyViewModel(&apiService);
    AssetsViewModel assetsViewModel(&apiService); // 【新增】实例化

    QQmlApplicationEngine engine;

    // 3. 注册上下文属性 (Context Property)
    engine.rootContext()->setContextProperty("storyViewModel", &storyViewModel);
    engine.rootContext()->setContextProperty("assetsViewModel", &assetsViewModel); // 【新增】注册

    // 4. 加载 QML
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);

    engine.loadFromModule("StoryFlow", "Main");

    return app.exec();
}
