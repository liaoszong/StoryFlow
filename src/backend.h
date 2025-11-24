#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr);

    // 生成分镜
    Q_INVOKABLE void createStory(const QString &prompt, const QString &style);
    // 重新生成图像：发送请求到服务器
    Q_INVOKABLE void regenerateImage(const QString &projectId, const QString &shotId, const QString &newPrompt, const QString &style);
    // // 启动视频合成：发送请求到服务器
    // Q_INVOKABLE void generateVideo(const QString &projectId);

signals:
    void storyCreated(QVariantMap projectData);
    void errorOccurred(QString message);
    // // 单张图片生成成功后通知 QML
    void imageRegenerated(QVariantMap shotData);
    // // 视频生成完成并返回视频 URL
    // void videoGenerationFinished(QVariantMap projectData);

private:
    QNetworkAccessManager *manager; // 专门用来发网络请求的工具
};

#endif
