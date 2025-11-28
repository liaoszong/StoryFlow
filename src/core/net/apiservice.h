#ifndef APISERVICE_H
#define APISERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <functional> // 用于回调

class ApiService : public QObject
{
    Q_OBJECT
public:
    explicit ApiService(QObject *parent = nullptr);

    // 设置 Token
    void setToken(const QString &token);

    // 通用的 Post 请求，使用回调函数返回结果，而不是信号
    // callback 定义：接收 bool (是否成功) 和 QJsonObject (数据)
    using ApiResponseCallback = std::function<void(bool success, const QJsonObject &data)>;

    void post(const QString &endpoint, const QJsonObject &payload, ApiResponseCallback callback);
    void get(const QString &endpoint, const QJsonObject &params, ApiResponseCallback callback);

private:
    QNetworkAccessManager *manager;
    QString jwtToken;
    const QString BASE_URL = "http://39.105.112.239";
};

#endif
