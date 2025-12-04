#include "StoryViewModel.h"
#include <QDebug>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QFile>
#include <QJsonArray>
#include <QRandomGenerator>

// *定义常量，避免魔法字符串
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

//
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
    m_currentTaskId.clear(); // *确保清理旧 ID

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

// 辅助函数：简单的随机特效分配 (让画面动起来)
QString getRandomEffect() {
    const QString effects[] = {"zoom_in", "zoom_out", "pan_left", "pan_right"};
    int index = QRandomGenerator::global()->bounded(4);
    return effects[index];
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

QJsonObject StoryViewModel::buildRenderConfig(const QVariantMap &projectData)
{
    QJsonObject root;

    // 1. 项目信息
    QJsonObject project;
    project["name"] = projectData["name"].toString();
    root["project"] = project;

    // 2. 场景序列
    QJsonArray scenes;
    QVariantList storyboards = projectData["storyboards"].toList();
    int count = storyboards.size();

    for (int i = 0; i < count; ++i) {
        QVariantMap shot = storyboards[i].toMap();
        QJsonObject sceneItem;

        QString imagePath = shot["localImagePath"].toString();
        // 假设你已经在 createStory 时把音频路径存进去了
        QString audioPath = shot["audioUrl"].toString();

        // 音频驱动时长
        sceneItem["type"] = "image_scene";
        QJsonObject res;

        // 1. 设置图片
        QJsonObject img; img["path"] = imagePath;
        res["image"] = img;

        // 2. 设置音频与时长策略
        if (!audioPath.isEmpty()) {
            // Case A: 有音频 -> 引擎读取音频长度决定时长
            QJsonObject aud; aud["path"] = audioPath;
            res["audio"] = aud;
        } else {
            // Case B: 无音频 -> 强制指定时长
            // 否则引擎不知道这张图要播多久
            sceneItem["duration"] = 5000; // 默认 5秒
        }

        sceneItem["resources"] = res;

        // 3. 特效配置 (Ken Burns)
        QJsonObject effects;
        QJsonObject kb;
        kb["enabled"] = true;
        QString myEffect = shot["effect"].toString();
        kb["preset"] = myEffect.isEmpty() ? "zoom_in" : myEffect;
        effects["ken_burns"] = kb;
        sceneItem["effects"] = effects;

        scenes.append(sceneItem);

        // 4. 插入转场 (逻辑不变)
        if (i < count - 1) {
            QJsonObject transItem;
            transItem["type"] = "transition";
            transItem["transition_type"] = "crossfade";
            transItem["duration"] = 1.0;
            scenes.append(transItem);
        }
    }

    root["scenes"] = scenes;
    return root;
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

void StoryViewModel::exportVideo(const QString &projectId)
{
    qDebug() << "ViewModel: 请求导出视频 (服务端/AI生成模式) ->" << projectId;

    // 留空，仅打印日志
    generateVideo(projectId);
}
