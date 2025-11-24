#include "Backend.h"
#include <QDebug>

Backend::Backend(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
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
    QUrl url("http://127.0.0.1:5000/api/create_story");
   // QUrl url("http://115.190.232.13");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    //超时限制
    request.setTransferTimeout(30000);

    // 3. 发送请求（POST）
    QNetworkReply *reply = manager->post(request, data);

    // 4. 等待回应
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            // --- 成功接收数据 ---

            // 1. 先把数据全部读出来存到一个变量里
            // 数据读一次就没了，不能读第二次，所以必须先存起来
            QByteArray responseData = reply->readAll();

            // 在这里打印
            // 在 Qt Creator 下方的 "Application Output" 窗口就能看到完整的 JSON 字符串
            qDebug() << "========================================";
            qDebug() << "【后端返回的原始数据】:" << responseData;
            qDebug() << "========================================";

            // 2. 再用刚才存好的 responseData 去解析
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObj = jsonDoc.object();
            // 假设后端返回的结构里有一个 "payload" 字段装着数据
            // 这里转换成 QVariantMap 方便 QML 使用
            QVariantMap resultMap = jsonObj["payload"].toObject().toVariantMap();

            // 触发信号
            emit storyCreated(resultMap);
        } else {
            // --- 出错了 ---
            emit errorOccurred(reply->errorString());
        }
        reply->deleteLater(); // 清理内存
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
    QUrl url("http://127.0.0.1:5000/api/regenerate_image");
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

            // 预期 payload 返回的是更新后的 shotData (文档格式 4)
            if (jsonObj.contains("payload")) {
                QVariantMap resultMap = jsonObj["payload"].toObject().toVariantMap();
                // 发射信号通知 QML
                emit imageRegenerated(resultMap);
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
// 4. 生成视频 API (对应 JSON 结构 5 & 6)
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
