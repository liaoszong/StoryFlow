#include "videogenerator.h"
#include "VideoCreatorAPI.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

// ============== VideoGenerator ==============

VideoGenerator::VideoGenerator(QObject *parent)
    : QObject(parent)
{
}

VideoGenerator::~VideoGenerator()
{
    cancel();
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
    }
}

void VideoGenerator::generateVideo(const QVariantList &shots,
                                   const QString &outputPath,
                                   int width, int height, int fps)
{
    if (m_isGenerating) {
        qWarning() << "VideoGenerator: Already generating video";
        return;
    }

    if (shots.isEmpty()) {
        m_errorMessage = "No shots provided";
        emit errorMessageChanged();
        emit finished(false, QString());
        return;
    }

    // 保存输出路径
    m_outputPath = outputPath;

    // 确保输出目录存在
    QFileInfo outputInfo(outputPath);
    QDir dir = outputInfo.absoluteDir();
    if (!dir.exists()) {
        qDebug() << "VideoGenerator: Creating output directory:" << dir.absolutePath();
        dir.mkpath(".");
    }

    // 构建 JSON 配置
    QString jsonConfig = buildJsonConfig(shots, outputPath, width, height, fps);
    qDebug() << "VideoGenerator: Output path:" << outputPath;
    qDebug() << "VideoGenerator: JSON config:" << jsonConfig;

    // 设置状态
    m_isGenerating = true;
    m_progress = 0;
    m_errorMessage.clear();
    emit isGeneratingChanged();
    emit progressChanged();
    emit errorMessageChanged();

    // 创建工作线程
    m_workerThread = new QThread(this);
    m_worker = new VideoGeneratorWorker();
    m_worker->moveToThread(m_workerThread);

    // 连接信号
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &VideoGeneratorWorker::progress, this, &VideoGenerator::onWorkerProgress);
    connect(m_worker, &VideoGeneratorWorker::finished, this, &VideoGenerator::onWorkerFinished);

    // 启动线程并开始渲染
    m_workerThread->start();
    QMetaObject::invokeMethod(m_worker, "doRender", Qt::QueuedConnection,
                              Q_ARG(QString, jsonConfig));
}

void VideoGenerator::cancel()
{
    if (m_worker) {
        QMetaObject::invokeMethod(m_worker, "cancel", Qt::QueuedConnection);
    }
}

void VideoGenerator::onWorkerProgress(int percent)
{
    m_progress = percent;
    emit progressChanged();
}

void VideoGenerator::onWorkerFinished(bool success, const QString &error)
{
    m_isGenerating = false;
    m_progress = success ? 100 : m_progress;
    m_errorMessage = error;

    emit isGeneratingChanged();
    emit progressChanged();
    if (!error.isEmpty()) {
        emit errorMessageChanged();
    }

    // 清理线程
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        m_workerThread->deleteLater();
        m_workerThread = nullptr;
        m_worker = nullptr;
    }

    emit finished(success, success ? m_outputPath : QString());
}

QString VideoGenerator::buildJsonConfig(const QVariantList &shots,
                                        const QString &outputPath,
                                        int width, int height, int fps)
{
    QJsonObject root;

    // 项目配置
    QJsonObject project;
    project["name"] = "StoryFlow Video";
    project["output_path"] = outputPath;
    project["width"] = width;
    project["height"] = height;
    project["fps"] = fps;
    project["background_color"] = "#000000";
    root["project"] = project;

    // 场景列表
    QJsonArray scenes;
    int sceneId = 1;

    for (int i = 0; i < shots.size(); ++i) {
        QVariantMap shot = shots[i].toMap();

        // 图片场景
        QJsonObject scene;
        scene["id"] = sceneId;
        scene["type"] = "IMAGE_SCENE";
        scene["duration"] = shot.value("duration", 3.0).toDouble();

        // 资源配置
        QJsonObject resources;

        // 图片
        QJsonObject image;
        image["path"] = shot.value("imagePath").toString();
        image["x"] = 0;
        image["y"] = 0;
        image["scale"] = 1.0;
        image["rotation"] = 0.0;
        resources["image"] = image;

        // 音频
        QString audioPath = shot.value("audioPath").toString();
        if (!audioPath.isEmpty()) {
            QJsonObject audio;
            audio["path"] = audioPath;
            audio["volume"] = shot.value("volume", 1.0).toDouble();
            audio["start_offset"] = 0.0;
            resources["audio"] = audio;
        }

        scene["resources"] = resources;

        // 特效配置（可选的 Ken Burns 效果）
        QJsonObject effects;
        QJsonObject kenBurns;
        kenBurns["enabled"] = shot.value("kenBurnsEnabled", false).toBool();
        if (kenBurns["enabled"].toBool()) {
            kenBurns["preset"] = shot.value("kenBurnsPreset", "zoom_in").toString();
            kenBurns["start_scale"] = shot.value("kenBurnsStartScale", 1.0).toDouble();
            kenBurns["end_scale"] = shot.value("kenBurnsEndScale", 1.2).toDouble();
        }
        effects["ken_burns"] = kenBurns;

        // 字幕配置
        QString subtitleText = shot.value("subtitle", "").toString();
        if (!subtitleText.isEmpty()) {
            QJsonObject subtitle;
            subtitle["text"] = subtitleText;
            subtitle["font_size"] = shot.value("subtitleFontSize", 48).toInt();
            subtitle["font_color"] = shot.value("subtitleFontColor", "white").toString();
            subtitle["bg_color"] = shot.value("subtitleBgColor", "black@0.5").toString();
            subtitle["margin_bottom"] = shot.value("subtitleMarginBottom", 60).toInt();
            effects["subtitle"] = subtitle;
        }

        scene["effects"] = effects;

        scenes.append(scene);
        int currentSceneId = sceneId;
        sceneId++;

        // 添加转场（如果不是最后一个镜头）
        if (i < shots.size() - 1) {
            QString transitionType = shot.value("transitionType", "crossfade").toString().toUpper();
            double transitionDuration = shot.value("transitionDuration", 0.5).toDouble();

            if (transitionDuration > 0) {
                QJsonObject transition;
                transition["id"] = sceneId;
                transition["type"] = "TRANSITION";
                transition["duration"] = transitionDuration;
                transition["transition_type"] = transitionType;
                transition["from_scene"] = currentSceneId;
                transition["to_scene"] = sceneId + 1;  // 下一个场景的ID

                scenes.append(transition);
                sceneId++;
            }
        }
    }

    root["scenes"] = scenes;

    // 全局效果配置
    QJsonObject globalEffects;

    QJsonObject audioNorm;
    audioNorm["enabled"] = false;
    audioNorm["target_level"] = -16.0;
    globalEffects["audio_normalization"] = audioNorm;

    QJsonObject videoEnc;
    videoEnc["codec"] = "libx264";
    videoEnc["bitrate"] = "5000k";
    videoEnc["preset"] = "medium";
    videoEnc["crf"] = 23;
    globalEffects["video_encoding"] = videoEnc;

    QJsonObject audioEnc;
    audioEnc["codec"] = "aac";
    audioEnc["bitrate"] = "192k";
    audioEnc["channels"] = 2;
    globalEffects["audio_encoding"] = audioEnc;

    root["global_effects"] = globalEffects;

    // 转换为 JSON 字符串
    QJsonDocument doc(root);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

// ============== VideoGeneratorWorker ==============

VideoGeneratorWorker::VideoGeneratorWorker(QObject *parent)
    : QObject(parent)
{
}

void VideoGeneratorWorker::doRender(const QString &jsonConfig)
{
    m_cancelled = false;

    qDebug() << "VideoGeneratorWorker: Starting render...";

    // 调用 VideoCreator API
    std::string error;
    bool success = VideoCreator::RenderFromJsonString(jsonConfig.toStdString(), &error);

    if (m_cancelled) {
        emit finished(false, "Cancelled by user");
        return;
    }

    if (success) {
        qDebug() << "VideoGeneratorWorker: Render completed successfully";
        emit finished(true, QString());
    } else {
        QString errorMsg = QString::fromStdString(error);
        qWarning() << "VideoGeneratorWorker: Render failed:" << errorMsg;
        emit finished(false, errorMsg);
    }
}

void VideoGeneratorWorker::cancel()
{
    m_cancelled = true;
    // 注意：VideoCreator API 目前不支持取消，这里只是设置标志
    // 未来可以扩展 VideoCreator 支持取消功能
}
