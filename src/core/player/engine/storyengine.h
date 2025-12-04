#ifndef STORYENGINE_H
#define STORYENGINE_H

#include <QObject>
#include <QJsonObject>
#include <QImage>
#include <QSize>

// 请在此类中实现核心渲染逻辑
class StoryEngine : public QObject
{
    Q_OBJECT
public:
    explicit StoryEngine(QObject *parent = nullptr);

    // 1. 加载配置 (解析你传来的 huge JSON)
    void loadConfig(const QJsonObject &config);

    // 2. 更新画布尺寸
    void setOutputSize(const QSize &size);

    // 3. 【核心】获取当前这一帧的图像 (预览用)
    // timestamp: 当前播放时间(毫秒)
    QImage getFrame(int timestamp);

    // 4. 导出视频 (调用 FFmpeg)
    void exportVideo(const QString &outputPath);

private:
    QJsonObject m_config;
    QSize m_size;
    // 这里未来会存放他的解码器、OpenGL上下文等
};

#endif // STORYENGINE_H
