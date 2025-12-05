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
    Q_INVOKABLE void cancelGeneration();
    Q_INVOKABLE void regenerateImage(const QString &projectId, const QString &shotId, const QString &newPrompt, const QString &style);
    Q_INVOKABLE void generateVideo(const QString &projectId);

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
    void imageDownloadProgress(int current, int total);
    void allImagesDownloaded(QVariantMap projectData);

private slots:
    void checkTaskStatus();
    void checkMediaGenerationStatus();
    void checkSingleImageStatus(const QString &taskId, const QString &shotId);

private:
    ApiService *m_apiService;
    FileManager *m_fileManager;
    QTimer *m_pollTimer;
    QTimer *m_mediaPollTimer;
    QString m_currentTaskId;
    QString m_currentProjectId;
    QString m_currentUserId;
    QString m_currentStyle;
    QString m_savePath;

    bool m_isGenerating;
    int m_progress;
    QString m_statusMessage;

    QVariantMap m_pendingProjectData;
    int m_totalImages;
    int m_downloadedImages;
    int m_totalAudios;
    int m_downloadedAudios;

    void setIsGenerating(bool status);
    void setProgress(int value);
    void setStatusMessage(const QString &message);

    void requestGenerateImages();
    void requestGenerateAudios();
    void downloadAllMedia();
    void downloadAndSaveImage(const QString &shotId, const QString &imageUrl, int index);
    void downloadAndSaveAudio(const QString &shotId, const QString &audioUrl, int index);
    void checkDownloadComplete();
    void finishGeneration();
    void downloadRegeneratedImage(const QString &shotId, const QString &imageUrl);

    void processStoryboardImages(const QVariantMap &projectData);
    void fetchShotsList();
};

#endif
