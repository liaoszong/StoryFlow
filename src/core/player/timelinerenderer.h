#ifndef TIMELINERENDERER_H
#define TIMELINERENDERER_H

#include <QQuickPaintedItem> // 注意：改用 PaintedItem 方便画图
#include <QJsonObject>
#include <QTimer>

class StoryEngine; // 前置声明

class TimelineRenderer : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QJsonObject config READ config WRITE setConfig NOTIFY configChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)

public:
    explicit TimelineRenderer(QQuickItem *parent = nullptr);

    QJsonObject config() const { return m_config; }
    void setConfig(const QJsonObject &config);

    bool isPlaying() const { return m_isPlaying; }

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void exportVideo(const QString &path);

    // 重写绘制函数
    void paint(QPainter *painter) override;

signals:
    void configChanged();
    void isPlayingChanged();

private slots:
    void onTimerTick();

private:
    QJsonObject m_config;
    bool m_isPlaying;
    int m_currentTime; // 当前播放时间 (ms)

    StoryEngine *m_engine; // 持有引擎实例
    QTimer *m_renderTimer; // 驱动动画的定时器
};

#endif
