#ifndef STORYVIEWMODEL_H
#define STORYVIEWMODEL_H

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include "core/net/ApiService.h"

class FileManager;

class StoryViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isGenerating READ isGenerating NOTIFY isGeneratingChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString currentProjectId READ currentProjectId NOTIFY currentProjectIdChanged)
    Q_PROPERTY(QString savePath READ savePath NOTIFY savePathChanged)

public:
    explicit StoryViewModel(ApiService *apiService, FileManager *fileManager, QObject *parent = nullptr);

    Q_INVOKABLE void createStory(const QString &prompt, const QString &style, const QString &savePath);
    Q_INVOKABLE void regenerateImage(const QString &projectId, const QString &shotId, const QString &newPrompt, const QString &style);
    Q_INVOKABLE void generateVideo(const QString &projectId);

    // 导出相关
    Q_INVOKABLE QJsonObject buildRenderConfig(const QVariantMap &projectData);
    Q_INVOKABLE void exportVideo(const QString &projectId);

    bool isGenerating() const { return m_isGenerating; }
    int progress() const { return m_progress; }
    QString statusMessage() const { return m_statusMessage; }
    QString currentProjectId() const { return m_currentProjectId; }
    QString savePath() const { return m_savePath; }

signals:
    void isGeneratingChanged();
    void progressChanged();
    void statusMessageChanged();
    void currentProjectIdChanged();
    void savePathChanged();
    void storyCreated(QVariantMap storyData);
    void errorOccurred(QString message);
    void imageRegenerated(QVariantMap shotData);
    void videoGenerated(QVariantMap videoData);
    void imageDownloadProgress(int current, int total);  // 图片下载进度
    void allImagesDownloaded(QVariantMap projectData);   // 所有图片下载完成

private slots:
    void checkTaskStatus(); // 故事生成的主轮询
    // 【新增】单张图片生成的轮询槽函数
    void checkSingleImageStatus(const QString &taskId, const QString &shotId);

private:
    ApiService *m_apiService;
    FileManager *m_fileManager;
    QTimer *m_pollTimer;
    QString m_currentTaskId;
    QString m_currentProjectId;
    QString m_savePath;  // 用户选择的保存路径

    bool m_isGenerating;
    int m_progress;
    QString m_statusMessage;

    // 图片下载相关
    QVariantMap m_pendingProjectData;  // 待处理的项目数据
    int m_totalImages;                  // 总图片数
    int m_downloadedImages;             // 已下载图片数

    void setIsGenerating(bool status);
    void setProgress(int value);
    void setStatusMessage(const QString &message);

    // 图片下载保存
    void processStoryboardImages(const QVariantMap &projectData);
    void downloadAndSaveImage(const QString &shotId, const QString &imageUrl, int index);
    void downloadRegeneratedImage(const QString &shotId, const QString &imageUrl);

    // 获取分镜列表 (新 API)
    void fetchShotsList();
};

#endif
