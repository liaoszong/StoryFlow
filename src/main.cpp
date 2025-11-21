#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "Backend.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 设置样式为 "Basic"，允许自定义按钮和输入框的背景色
    QQuickStyle::setStyle("Basic");

    // 创建后端实例
    Backend backend;

    QQmlApplicationEngine engine;

    // 把 backend 注册给 QML，起名叫 "backendService"
    engine.rootContext()->setContextProperty("backendService", &backend);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // 加载 QML
    engine.loadFromModule("StoryFlow", "Main");

    return app.exec();
}
