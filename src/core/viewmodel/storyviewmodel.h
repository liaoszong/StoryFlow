#ifndef STORYVIEWMODEL_H
#define STORYVIEWMODEL_H

#include <QObject>
#include <QTimer>
#include "core/net/ApiService.h"

class StoryViewModel : public QObject
{
    Q_OBJECT

    // MVVM 核心：通过属性 (Property) 通知 UI 变化
    Q_PROPERTY(bool isGenerating READ isGenerating NOTIFY isGeneratingChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    // QML 可以绑定此属性来显示当前状态文本
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit StoryViewModel(ApiService *apiService, QObject *parent = nullptr);

    // QML 调用的方法 (Q_INVOKABLE)
    Q_INVOKABLE void createStory(const QString &prompt, const QString &style);
    Q_INVOKABLE void regenerateImage(const QString &projectId, const QString &shotId, const QString &newPrompt, const QString &style);
    Q_INVOKABLE void generateVideo(const QString &projectId);

    // Q_PROPERTY 的 Getter (保持不变)
    bool isGenerating() const { return m_isGenerating; }
    int progress() const { return m_progress; }
    QString statusMessage() const { return m_statusMessage; } // 【新增 Getter】

signals:
    void isGeneratingChanged();
    void progressChanged();
    void storyCreated(QVariantMap storyData);
    void errorOccurred(QString message);
    void statusMessageChanged();         // 用于 Q_PROPERTY(statusMessage)
    void imageRegenerated(QVariantMap shotData); // 用于 RegenerateImage 成功
    void videoGenerated(QVariantMap videoData);  // 用于 GenerateVideo 成功

private slots:
    void checkTaskStatus();

private:
    ApiService *m_apiService;
    QTimer *m_pollTimer;
    QString m_currentTaskId;

    // 状态数据
    bool m_isGenerating;
    int m_progress;
    QString m_statusMessage; // 存储变量

    // ------------------------------------------------------------------
    // 内部 Setter 声明
    // 否则 C++ 编译器认为 set... 方法不存在
    // ------------------------------------------------------------------
    void setIsGenerating(bool status);
    void setProgress(int value);
    void setStatusMessage(const QString &message); // Setter声明
    // ------------------------------------------------------------------
};

#endif
