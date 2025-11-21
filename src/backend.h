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

    // Q_INVOKABLE 让 QML 能直接调用这个函数
    Q_INVOKABLE void createStory(const QString &prompt, const QString &style);

signals:
    // 成功则带回来数据 (projectData)
    void storyCreated(QVariantMap projectData);
    // 失败了则带回来错误信息
    void errorOccurred(QString message);

private:
    QNetworkAccessManager *manager; // 专门用来发网络请求的工具
};

#endif
