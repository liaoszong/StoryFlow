#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "core/net/apiservice.h"
#include "core/viewmodel/storyviewmodel.h"
#include "core/viewmodel/assetsviewmodel.h"
#include "core/video/videogenerator.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Basic");

    // 1. 创建网络服务
    ApiService apiService;

    // 2. 创建 ViewModel
    StoryViewModel storyViewModel(&apiService);
    AssetsViewModel assetsViewModel(&apiService);

    // 3. 创建视频生成器
    VideoGenerator videoGenerator;

    QQmlApplicationEngine engine;

    // 4. 注册上下文属性 (Context Property)
    engine.rootContext()->setContextProperty("storyViewModel", &storyViewModel);
    engine.rootContext()->setContextProperty("assetsViewModel", &assetsViewModel);
    engine.rootContext()->setContextProperty("videoGenerator", &videoGenerator);

    // 5. 加载 QML
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);

    engine.loadFromModule("StoryFlow", "Main");

    return app.exec();
}
