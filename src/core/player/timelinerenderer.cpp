#include "TimelineRenderer.h"
#include "engine/StoryEngine.h" // 引用上面的引擎
#include <QPainter>
#include <QDebug>

// 构造函数
TimelineRenderer::TimelineRenderer(QQuickItem *parent)
    : QQuickPaintedItem(parent), m_isPlaying(false), m_currentTime(0)
{
    // 实例化引擎
    m_engine = new StoryEngine(this);

    // 设置刷新定时器 (约 30 FPS)
    m_renderTimer = new QTimer(this);
    m_renderTimer->setInterval(33);
    connect(m_renderTimer, &QTimer::timeout, this, &TimelineRenderer::onTimerTick);
}

// 接收 QML 的 config 属性
void TimelineRenderer::setConfig(const QJsonObject &config)
{
    if (m_config == config) return;
    m_config = config;
    emit configChanged();

    // 1. 把配置传给引擎
    m_engine->loadConfig(config);

    // 2. 重置播放进度
    m_currentTime = 0;
    update(); // 触发重绘 (paint)
}

// 播放控制
void TimelineRenderer::play() {
    m_isPlaying = true;
    m_renderTimer->start();
    emit isPlayingChanged();
}

void TimelineRenderer::pause() {
    m_isPlaying = false;
    m_renderTimer->stop();
    emit isPlayingChanged();
}

void TimelineRenderer::exportVideo(const QString &path) {
    // 转发给引擎
    if(m_engine) {
        m_engine->exportVideo(path);
    }
}

// 定时器回调：推进时间 -> 触发重绘
void TimelineRenderer::onTimerTick() {
    if (m_isPlaying) {
        m_currentTime += 33; // 每次加 33ms
        update(); // 这一句会触发 paint() 函数
    }
}

// 【核心绘制函数】Qt 会自动调用这个函数把画面画在 QML 上
void TimelineRenderer::paint(QPainter *painter)
{
    if (!m_engine) return;

    // 1. 告诉引擎当前窗口有多大 (响应窗口缩放)
    m_engine->setOutputSize(QSize(width(), height()));

    // 2. 找引擎要当前这一帧的图像
    QImage frame = m_engine->getFrame(m_currentTime);

    // 3. 画在屏幕上
    if (!frame.isNull()) {
        painter->drawImage(boundingRect(), frame);
    } else {
        // 如果没图，画个黑底
        painter->fillRect(boundingRect(), Qt::black);
    }
}
