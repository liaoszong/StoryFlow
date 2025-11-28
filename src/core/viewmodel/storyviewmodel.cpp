#include "StoryViewModel.h"
#include <QDebug>
#include <QJsonDocument>
#include <QUrlQuery>

// 定义常量，避免魔法字符串
namespace ApiConstants {
const QString ENDPOINT_CREATE = "/api/story/generate";
const QString ENDPOINT_CHECK_STATUS = "/api/check_status";
const QString ENDPOINT_REGEN_IMAGE = "/api/regenerate_image";
const QString ENDPOINT_GEN_VIDEO = "/api/generate_video";

const QString ACTION_SUCCESS = "ACTION_TYPE_SUCCESS";
const QString ACTION_PENDING = "ACTION_TYPE_PENDING";
const QString ACTION_PROCESSING = "ACTION_TYPE_PROCESSING";
const QString ACTION_FAILED = "ACTION_TYPE_FAILED";
}

StoryViewModel::StoryViewModel(ApiService *apiService, QObject *parent)
    : QObject(parent), m_apiService(apiService),
    m_isGenerating(false), m_progress(0), m_statusMessage("")
{
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(2000);
    // 确保 Timer 也是纯净的，只负责触发检查
    connect(m_pollTimer, &QTimer::timeout, this, &StoryViewModel::checkTaskStatus);
}

// ... Setter 实现保持不变 ...
void StoryViewModel::setIsGenerating(bool status) {
    if (m_isGenerating == status) return;
    m_isGenerating = status;
    emit isGeneratingChanged();
}
void StoryViewModel::setProgress(int value) {
    if (m_progress == value) return;
    m_progress = value;
    emit progressChanged();
}
void StoryViewModel::setStatusMessage(const QString &message) {
    if (m_statusMessage == message) return;
    m_statusMessage = message;
    emit statusMessageChanged();
}

// 1. 生成故事
void StoryViewModel::createStory(const QString &prompt, const QString &style)
{
    if (m_isGenerating) {
        emit errorOccurred("当前有任务正在进行中");
        return;
    }

    // 重置状态
    setIsGenerating(true);
    setProgress(0);
    setStatusMessage("正在提交任务...");
    m_currentTaskId.clear(); // 确保清理旧 ID

    QJsonObject payload;
    payload["rawStory"] = prompt;
    payload["style"] = style;

    m_apiService->post(ApiConstants::ENDPOINT_CREATE, payload,
                       [this](bool success, const QJsonObject &resp) {
                           // Lambda 内部先检查对象是否存活（防御性编程）
                           // 这里的 this 捕获在 QObject 体系下通常安全，但有时会用 QPointer 检查

                           if (!success || resp["type"].toString() == ApiConstants::ACTION_FAILED) {
                               setIsGenerating(false);
                               QString errorMsg = resp["payload"].toObject()["msg"].toString();
                               if (errorMsg.isEmpty()) errorMsg = "任务提交失败";
                               emit errorOccurred(errorMsg);
                               return;
                           }

                           // 任务提交成功
                           QString taskId = resp["payload"].toObject()["taskId"].toString();
                           if (taskId.isEmpty()) {
                               setIsGenerating(false);
                               emit errorOccurred("服务器返回了无效的任务ID");
                               return;
                           }

                           m_currentTaskId = taskId;
                           setStatusMessage("任务已提交，等待 AI 调度...");
                           m_pollTimer->start();
                       });
}

// 2. 重绘图片
void StoryViewModel::regenerateImage(const QString &projectId, const QString &shotId, const QString &newPrompt, const QString &style)
{
    // 这里其实也建议加一个 isRegenerating 的状态，防止用户狂点
    // 但为了简化，我们只发请求

    QJsonObject payload;
    payload["id"] = projectId;
    payload["shotId"] = shotId;
    payload["prompt"] = newPrompt;
    payload["style"] = style;

    m_apiService->post(ApiConstants::ENDPOINT_REGEN_IMAGE, payload,
                       [this](bool success, const QJsonObject &resp) {
                           if (success && resp["type"].toString() == ApiConstants::ACTION_SUCCESS) {
                               emit imageRegenerated(resp["payload"].toObject().toVariantMap());
                           } else {
                               QString errMsg = success ? resp["payload"].toObject()["message"].toString() : "网络请求失败";
                               emit errorOccurred("重绘失败: " + errMsg);
                           }
                       });
}

// 3. 生成视频
void StoryViewModel::generateVideo(const QString &projectId)
{
    QJsonObject payload;
    payload["projectId"] = projectId;

    setStatusMessage("正在请求合成视频...");

    m_apiService->post(ApiConstants::ENDPOINT_GEN_VIDEO, payload,
                       [this](bool success, const QJsonObject &resp) {
                           if (success && resp["type"].toString() == ApiConstants::ACTION_SUCCESS) {
                               emit videoGenerated(resp["payload"].toObject().toVariantMap());
                           } else {
                               QString errMsg = success ? resp["payload"].toObject()["message"].toString() : "网络请求失败";
                               emit errorOccurred("视频生成失败: " + errMsg);
                           }
                       });
}

// 轮询逻辑
void StoryViewModel::checkTaskStatus()
{
    if (m_currentTaskId.isEmpty()) {
        m_pollTimer->stop();
        return;
    }

    QJsonObject params;
    params["id"] = m_currentTaskId;

    m_apiService->get(ApiConstants::ENDPOINT_CHECK_STATUS, params,
                      [this](bool success, const QJsonObject &resp) {

                          if (!success) {
                              // 网络波动不应该直接杀掉任务，可以增加重试机制
                              // 这里简单处理：直接报错停止
                              m_pollTimer->stop();
                              setIsGenerating(false);
                              emit errorOccurred("连接服务器超时");
                              return;
                          }

                          QString type = resp["type"].toString();
                          QJsonObject payload = resp["payload"].toObject();

                          if (type == ApiConstants::ACTION_SUCCESS) {
                              m_pollTimer->stop();
                              m_currentTaskId.clear();
                              setProgress(100);
                              setStatusMessage("生成完成！");
                              setIsGenerating(false); // 结束 Loading 状态
                              emit storyCreated(payload.toVariantMap());

                          } else if (type == ApiConstants::ACTION_PROCESSING || type == ApiConstants::ACTION_PENDING) {
                              int newProgress = payload["progress"].toInt();
                              // 保证进度条不倒退
                              if (newProgress > m_progress) setProgress(newProgress);

                              QString msg = (type == ApiConstants::ACTION_PENDING) ? "排队中..." : "正在绘制分镜...";
                              setStatusMessage(msg);

                          } else {
                              m_pollTimer->stop();
                              setIsGenerating(false);
                              emit errorOccurred("任务失败: " + payload["message"].toString());
                          }
                      });
}
