#include "StoryEngine.h"
#include <QPainter>
#include <QDebug>
#include <QJsonArray>

StoryEngine::StoryEngine(QObject *parent) : QObject(parent) {}

void StoryEngine::loadConfig(const QJsonObject &config) {
    m_config = config;
    qDebug() << "Engine: 配置已加载，镜头数:" << config["track"].toArray().size();
}

void StoryEngine::setOutputSize(const QSize &size) {
    m_size = size;
}

// 【模拟渲染】这里随便画点什么，证明链路通了
QImage StoryEngine::getFrame(int timestamp) {
    if (m_size.isEmpty()) return QImage();

    // 创建一张画布
    QImage image(m_size, QImage::Format_ARGB32);

    // 根据时间变色 (模拟动画效果)
    int r = (timestamp / 10) % 255;
    int g = (timestamp / 20) % 255;
    int b = (timestamp / 30) % 255;

    image.fill(QColor(r, g, b));

    QPainter p(&image);
    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 40));
    p.drawText(image.rect(), Qt::AlignCenter,
               QString("预览模式\nTime: %1 ms").arg(timestamp));

    return image;
}

void StoryEngine::exportVideo(const QString &outputPath) {
    qDebug() << "Engine: 正在调用 FFmpeg 导出到..." << outputPath;
    // 真正的导出逻辑待实现
}
