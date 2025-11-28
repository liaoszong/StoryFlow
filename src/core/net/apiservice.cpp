#include "apiservice.h"
#include <QDebug>
#include <QJsonDocument>
#include <QUrlQuery>

// 定义远程服务器地址
static const QString BASE_URL = "http://127.0.0.1:5000";

ApiService::ApiService(QObject *parent)
    : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
}

void ApiService::setToken(const QString &token)
{
    // 存储 access Token
    jwtToken = token;
    qDebug() << "ApiService: JWT Token 已设置。";
}

// -------------------------------------------------------------
// 辅助函数：添加认证头
// -------------------------------------------------------------
void ApiService::addAuthorizationHeader(QNetworkRequest &request)
{
    if (!jwtToken.isEmpty()) {
        QString authValue = QString("Bearer %1").arg(jwtToken);
        request.setRawHeader("Authorization", authValue.toUtf8());
    }
}

// -------------------------------------------------------------
// 核心方法 1: POST 请求 (用于生成故事, 图片, 登录)
// -------------------------------------------------------------
void ApiService::post(const QString &endpoint, const QJsonObject &payload, ApiResponseCallback callback)
{
    QUrl url(BASE_URL + endpoint);
    QNetworkRequest request(url);

    // 设置通用头部
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    addAuthorizationHeader(request);

    QJsonDocument doc(payload);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    QNetworkReply *reply = manager->post(request, data);

    // 使用 Lambda 处理响应
    connect(reply, &QNetworkReply::finished, [reply, callback]() {
        bool success = (reply->error() == QNetworkReply::NoError);
        QJsonObject responseData;

        if (success) {
            QByteArray rawData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(rawData);

            if (jsonDoc.isObject()) {
                responseData = jsonDoc.object();
            } else {
                // 如果不是合法的 JSON (例如 Nginx 404 HTML)，也视为失败
                success = false;
                responseData["error"] = "Invalid JSON response or HTML content.";
            }
        } else {
            // 网络错误
            responseData["error"] = reply->errorString();
            responseData["code"] = reply->error();
        }

        // 执行回调函数，将结果传给 ViewModel
        callback(success, responseData);
        reply->deleteLater();
    });
}

// -------------------------------------------------------------
// 核心方法 2: GET 请求 (用于检查状态, 加载资产列表)
// -------------------------------------------------------------
void ApiService::get(const QString &endpoint, const QJsonObject &params, ApiResponseCallback callback)
{
    QUrl url(BASE_URL + endpoint);

    // 1. 处理 GET 请求的查询参数 (将 QJsonObject 转换为 URLQuery)
    if (!params.isEmpty()) {
        QUrlQuery query;
        for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
            // key() 返回参数名
            // value().toVariant().toString() 能更安全地处理数字、布尔值等类型转字符串
            query.addQueryItem(it.key(), it.value().toVariant().toString());
        }
        url.setQuery(query);
    }

    QNetworkRequest request(url);

    // 添加认证头
    addAuthorizationHeader(request);

    QNetworkReply *reply = manager->get(request);

    // 使用 Lambda 处理响应
    connect(reply, &QNetworkReply::finished, [reply, callback]() {
        bool success = (reply->error() == QNetworkReply::NoError);
        QJsonObject responseData;

        if (success) {
            QByteArray rawData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(rawData);

            if (jsonDoc.isObject()) {
                responseData = jsonDoc.object();
            } else {
                success = false;
                responseData["error"] = "Invalid JSON response or HTML content.";
            }
        } else {
            responseData["error"] = reply->errorString();
            responseData["code"] = reply->error();
        }

        // 执行回调函数，将结果传给 ViewModel
        callback(success, responseData);
        reply->deleteLater();
    });
}
