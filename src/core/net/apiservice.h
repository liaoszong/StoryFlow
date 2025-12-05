#ifndef APISERVICE_H
#define APISERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest> // 【新增】确保包含此头文件
#include <QJsonObject>
#include <functional>

class ApiService : public QObject
{
    Q_OBJECT
public:
    explicit ApiService(QObject *parent = nullptr);

    // 设置 Token
    void setToken(const QString &token);

    // 回调定义
    using ApiResponseCallback = std::function<void(bool success, const QJsonObject &data)>;

    // 核心网络方法
    void post(const QString &endpoint, const QJsonObject &payload, ApiResponseCallback callback);
    void get(const QString &endpoint, const QJsonObject &params, ApiResponseCallback callback);

    // 获取网络管理器（用于图片下载等）
    QNetworkAccessManager* getManager() const { return manager; }

private:
    QNetworkAccessManager *manager;
    QString jwtToken;
    // 注意：BASE_URL 在 cpp 文件中定义为 static const，这里不需要再定义，或者定义为 const 成员变量均可
    // 如果你在 cpp 里用了 static const，这里就可以不写，或者写成:
    // const QString BASE_URL = "http://39.105.112.239";

    // -------------------------------------------------------------
    // 添加这个私有函数的声明
    // -------------------------------------------------------------
    void addAuthorizationHeader(QNetworkRequest &request);
};

#endif // APISERVICE_H
