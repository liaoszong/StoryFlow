#ifndef VIDEOGENERATOR_H
#define VIDEOGENERATOR_H

#include <QObject>
#include <QVariantList>
#include <QString>
#include <QThread>

class VideoGeneratorWorker;

/**
 * VideoGenerator - 视频生成桥接类
 *
 * 用于将 StoryFlow 的分镜数据转换为 VideoCreator 需要的 JSON 配置，
 * 并调用 VideoCreator::RenderFromJsonString() 生成视频。
 *
 * QML 调用示例：
 *   videoGenerator.generateVideo(shots, "C:/output/video.mp4")
 */
class VideoGenerator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isGenerating READ isGenerating NOTIFY isGeneratingChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit VideoGenerator(QObject *parent = nullptr);
    ~VideoGenerator();

    bool isGenerating() const { return m_isGenerating; }
    int progress() const { return m_progress; }
    QString errorMessage() const { return m_errorMessage; }

    /**
     * 生成视频
     * @param shots 分镜列表，每个元素是一个 QVariantMap，包含:
     *   - imagePath: QString 分镜图片路径
     *   - audioPath: QString 音频路径
     *   - duration: double 持续时间（秒）
     *   - transitionType: QString 转场类型 (可选: "crossfade", "wipe", "slide")
     *   - transitionDuration: double 转场时长（秒，可选，默认0.5）
     * @param outputPath 输出视频路径
     * @param width 视频宽度（可选，默认1920）
     * @param height 视频高度（可选，默认1080）
     * @param fps 帧率（可选，默认30）
     */
    Q_INVOKABLE void generateVideo(const QVariantList &shots,
                                   const QString &outputPath,
                                   int width = 1920,
                                   int height = 1080,
                                   int fps = 30);

    /**
     * 取消当前生成任务
     */
    Q_INVOKABLE void cancel();

signals:
    void isGeneratingChanged();
    void progressChanged();
    void errorMessageChanged();

    // 生成完成信号
    void finished(bool success, const QString &outputPath);

private slots:
    void onWorkerProgress(int percent);
    void onWorkerFinished(bool success, const QString &error);

private:
    QString buildJsonConfig(const QVariantList &shots,
                           const QString &outputPath,
                           int width, int height, int fps);

    bool m_isGenerating = false;
    int m_progress = 0;
    QString m_errorMessage;
    QString m_outputPath;

    QThread *m_workerThread = nullptr;
    VideoGeneratorWorker *m_worker = nullptr;
};

/**
 * 工作线程类 - 在后台执行视频渲染
 */
class VideoGeneratorWorker : public QObject
{
    Q_OBJECT
public:
    explicit VideoGeneratorWorker(QObject *parent = nullptr);

public slots:
    void doRender(const QString &jsonConfig);
    void cancel();

signals:
    void progress(int percent);
    void finished(bool success, const QString &error);

private:
    bool m_cancelled = false;
};

#endif // VIDEOGENERATOR_H
