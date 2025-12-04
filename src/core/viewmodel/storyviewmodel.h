#ifndef STORYVIEWMODEL_H
#define STORYVIEWMODEL_H

#include <QObject>
#include <QTimer>
#include <QJsonObject> // 【新增】确保包含 QJsonObject
#include <QVariantMap> // 【新增】确保包含 QVariantMap
#include "core/net/ApiService.h"

class StoryViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isGenerating READ isGenerating NOTIFY isGeneratingChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit StoryViewModel(ApiService *apiService, QObject *parent = nullptr);

    // QML 调用的方法 (Q_INVOKABLE)
    Q_INVOKABLE void createStory(const QString &prompt, const QString &style);
    Q_INVOKABLE void regenerateImage(const QString &projectId, const QString &shotId, const QString &newPrompt, const QString &style);
    Q_INVOKABLE void generateVideo(const QString &projectId);

    // ---------------------------------------------------------
    // 【✅ 关键修复】添加这两个新函数的声明
    // ---------------------------------------------------------
    // 构建播放器配置 (纯逻辑)
    Q_INVOKABLE QJsonObject buildRenderConfig(const QVariantMap &projectData);

    // 导出视频 (调用本地或远程接口)
    Q_INVOKABLE void exportVideo(const QString &projectId);
    // ---------------------------------------------------------

    // Getters
    bool isGenerating() const { return m_isGenerating; }
    int progress() const { return m_progress; }
    QString statusMessage() const { return m_statusMessage; }

signals:
    void isGeneratingChanged();
    void progressChanged();
    void statusMessageChanged();
    void storyCreated(QVariantMap storyData);
    void errorOccurred(QString message);
    void imageRegenerated(QVariantMap shotData);
    void videoGenerated(QVariantMap videoData);

private slots:
    void checkTaskStatus();

private:
    ApiService *m_apiService;
    QTimer *m_pollTimer;
    QString m_currentTaskId;

    bool m_isGenerating;
    int m_progress;
    QString m_statusMessage;

    void setIsGenerating(bool status);
    void setProgress(int value);
    void setStatusMessage(const QString &message);
};

#endif
