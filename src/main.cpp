#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "core/net/apiservice.h"
#include "core/viewmodel/storyviewmodel.h"
#include "FileManager/FileManager.h"
#include "core/viewmodel/assetsviewmodel.h"
#include "core/video/videogenerator.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Basic");

    // 1. 创建网络服务
    ApiService apiService;

    // 2. 创建文件管理器
    FileManager fileManager;

    // 3. 创建 ViewModel
    StoryViewModel storyViewModel(&apiService, &fileManager);
    AssetsViewModel assetsViewModel(&apiService);

    // 4. 创建视频生成器
    VideoGenerator videoGenerator;

    QQmlApplicationEngine engine;

    // 5. 注册上下文属性 (Context Property)
    engine.rootContext()->setContextProperty("storyViewModel", &storyViewModel);
    engine.rootContext()->setContextProperty("assetsViewModel", &assetsViewModel);
    engine.rootContext()->setContextProperty("videoGenerator", &videoGenerator);
    engine.rootContext()->setContextProperty("fileManager", &fileManager);

    // 6. 加载 QML
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);

    engine.loadFromModule("StoryFlow", "Main");

    return app.exec();
}
