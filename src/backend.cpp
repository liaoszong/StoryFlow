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
