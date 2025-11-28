#include "Backend.h"
#include <QDebug>
#include <QUrlQuery>

static const QString BASE_URL = "http://127.0.0.1:5000";

Backend::Backend(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);

    // 初始化定时器
    pollTimer = new QTimer(this);
    pollTimer->setInterval(2000); // 设置轮询间隔：2秒
    // 连接定时器超时信号到 checkTaskStatus 槽函数
    connect(pollTimer, &QTimer::timeout, this, &Backend::checkTaskStatus);
}

void Backend::createStory(const QString &prompt, const QString &style)
{
    // 1. 打包 JSON 数据
    QJsonObject json;
    json["rawStory"] = prompt; // 用户输入的文本
    json["style"] = style;   // 用户选择的风格

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    // 2. 设置 URL 和 Header
    // 服务器地址
    QUrl url(BASE_URL + "/api/create_story");
    // QUrl url("http://39.105.112.239/api/story/generate");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 3. 发送请求（POST）
    QNetworkReply *reply = manager->post(request, data);

    // 4. 处理初步响应（获取 TaskID）
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObj = jsonDoc.object();

            // 解析服务器返回的状态
            QString type = jsonObj["type"].toString();

            if (type == "ACTION_TYPE_PENDING") {
                // 获取 TaskID
                QJsonObject payload = jsonObj["payload"].toObject();
                currentTaskId = payload["taskId"].toString();

                qDebug() << "任务提交成功，TaskID:" << currentTaskId;

                // 启动轮询定时器
                if (!currentTaskId.isEmpty()) {
                    pollTimer->start();
                    emit storyProgress(0, "任务排队中...");
                }
            } else {
                emit errorOccurred("服务器返回了未知的任务类型: " + type);
            }
        } else {
            emit errorOccurred("网络请求失败: " + reply->errorString());
        }
        reply->deleteLater();
    });
}

// 轮询状态的核心逻辑
void Backend::checkTaskStatus()
{
    if (currentTaskId.isEmpty()) {
        pollTimer->stop();
        return;
    }

    // 1. 拼接查询 URL (GET)
    QUrl url(BASE_URL + "/api/check_status");
    QUrlQuery query;
    query.addQueryItem("id", currentTaskId);
    url.setQuery(query);

    QNetworkRequest request(url);

    // 2. 发送查询请求
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObj = jsonDoc.object();

            QString type = jsonObj["type"].toString();
            QJsonObject payload = jsonObj["payload"].toObject();

            if (type == "ACTION_TYPE_PENDING") {
                // 仍在排队
                int progress = payload["progress"].toInt();
                qDebug() << "排队中..." << progress << "%";
                emit storyProgress(progress, "排队中...");

            } else if (type == "ACTION_TYPE_PROCESSING") {
                // 正在生成
                int progress = payload["progress"].toInt();
                qDebug() << "生成中..." << progress << "%";
                emit storyProgress(progress, "正在生成故事分镜...");

            } else if (type == "ACTION_TYPE_SUCCESS") {
                // --- 成功完成 ---
                qDebug() << "任务完成！收到最终数据。";

                // 1. 停止轮询
                pollTimer->stop();
                currentTaskId = "";

                // 2. 发送成功信号，将 payload (包含 storyboards) 传给前端
                QVariantMap resultMap = payload.toVariantMap();
                emit storyCreated(resultMap);

                // 3. 发送 100% 进度让进度条消失
                emit storyProgress(100, "完成");

            } else {
                // 未知状态或错误
                pollTimer->stop();
                emit errorOccurred("任务状态异常: " + jsonObj["message"].toString());
            }
        } else {
            // 网络错误
            pollTimer->stop();
            emit errorOccurred("轮询请求失败: " + reply->errorString());
        }
        reply->deleteLater();
    });
}

void Backend::regenerateImage(const QString &projectId, const QString &shotId, const QString &newPrompt, const QString &style)
{
    // 1. 打包 JSON
    QJsonObject json;
    json["id"] = projectId;       // 项目ID
    json["shotId"] = shotId;      // 分镜ID
    json["prompt"] = newPrompt;   // 新提示词
    json["style"] = style;        // 风格

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    // 2. 设置 URL
    QUrl url(BASE_URL + "/api/regenerate_image");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(30000); // 30s timeout

    qDebug() << "Sending Regenerate Request:" << json;

    // 3. 发送请求
    QNetworkReply *reply = manager->post(request, data);

    // 4. 处理响应
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            qDebug() << "Regenerate Response:" << responseData;

            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObj = jsonDoc.object();

            if (jsonObj["type"].toString() == "ACTION_TYPE_SUCCESS") {
                if (jsonObj.contains("payload")) {
                    QVariantMap resultMap = jsonObj["payload"].toObject().toVariantMap();
                    emit imageRegenerated(resultMap);
                }
            } else {
                emit errorOccurred("图片生成失败");
            }
        } else {
            qDebug() << "Regenerate Error:" << reply->errorString();
            emit errorOccurred(reply->errorString());
        }
        reply->deleteLater();
    });
}

/*
// -----------------------------------------------------
// 4. 生成视频 API
// -----------------------------------------------------
void Backend::generateVideo(const QString &projectId)
{
    // 1. 打包 JSON
    QJsonObject json;
    json["id"] = projectId; [cite_start]// [cite: 92]
    json["type"] = "Video"; [cite_start]// [cite: 92]

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    // 2. 设置 URL (注意这里的接口名)
    QUrl url("http://127.0.0.1:5000/api/generate_video");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(60000); // 视频生成时间可能更长，设置 60s

    QNetworkReply *reply = manager->post(request, data);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObj = jsonDoc.object();

            [cite_start]// 预期 payload 返回最终视频信息 [cite: 95]
                QVariantMap resultMap = jsonObj["payload"].toObject().toVariantMap();

            // 发射新的信号
            emit videoGenerationFinished(resultMap);
        } else {
            emit errorOccurred(reply->errorString());
        }
        reply->deleteLater();
    });
}
*/
