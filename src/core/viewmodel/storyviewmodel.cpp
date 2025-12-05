#include "StoryViewModel.h"
#include "FileManager/FileManager.h"
#include <QDebug>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDir>
#include <QUrl>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QFile>
#include <QJsonArray>
#include <QRandomGenerator>

// 定义符合后端文档的常量
namespace ApiConstants {
const QString ENDPOINT_CREATE = "/api/client/story/generate"; // 故事生成提交
const QString ENDPOINT_CHECK_STATUS = "/api/client/story/result"; // 故事状态查询
const QString ENDPOINT_SHOTS_LIST = "/api/client/shots/list"; // 分镜列表查询 (新)
const QString ENDPOINT_REGEN_IMAGE = "/api/client/image/regenerate"; // 图片重绘提交
const QString ENDPOINT_CHECK_IMAGE = "/api/client/image/result"; // 图片状态查询

// 状态枚举 (根据文档)
const QString STATUS_COMPLETED = "completed";
const QString STATUS_RUNNING = "running";
const QString STATUS_FAILED = "failed";
}

// 辅助函数：将网络错误码转换为用户友好的中文提示
static QString getNetworkErrorMessage(int errorCode, const QString &errorString) {
    switch (errorCode) {
        case QNetworkReply::ConnectionRefusedError:
            return "连接被拒绝，请检查服务器是否正常运行";
        case QNetworkReply::RemoteHostClosedError:
            return "服务器断开连接，请稍后重试";
        case QNetworkReply::HostNotFoundError:
            return "无法找到服务器，请检查网络连接";
        case QNetworkReply::TimeoutError:
            return "请求超时，请检查网络连接后重试";
        case QNetworkReply::OperationCanceledError:
            return "请求已取消";
        case QNetworkReply::SslHandshakeFailedError:
            return "安全连接失败，请检查网络设置";
        case QNetworkReply::NetworkSessionFailedError:
            return "网络会话失败，请检查网络连接";
        case QNetworkReply::UnknownNetworkError:
            return "网络错误，请检查网络连接";
        case QNetworkReply::UnknownServerError:
            return "服务器错误，请稍后重试";
        default:
            if (errorString.contains("timeout", Qt::CaseInsensitive)) {
                return "请求超时，请检查网络连接后重试";
            }
            return "网络请求失败: " + errorString;
    }
}

StoryViewModel::StoryViewModel(ApiService *apiService, FileManager *fileManager, QObject *parent)
    : QObject(parent), m_apiService(apiService), m_fileManager(fileManager),
    m_isGenerating(false), m_progress(0), m_statusMessage(""),
    m_totalImages(0), m_downloadedImages(0)
{
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(2000);
    connect(m_pollTimer, &QTimer::timeout, this, &StoryViewModel::checkTaskStatus);
}

// ... Setters (保持不变) ...
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

// --------------------------------------------------------
// 1. 生成故事 (适配新接口)
// --------------------------------------------------------
void StoryViewModel::createStory(const QString &prompt, const QString &style, const QString &savePath)
{
    if (m_isGenerating) return;

    setIsGenerating(true);
    setProgress(0);
    setStatusMessage("正在提交任务...");
    

    // 保存用户选择的路径
    m_savePath = savePath;
    emit savePathChanged();

    QJsonObject payload;
    // 构造文档要求的字段
    payload["project_id"] = "proj_" + QString::number(QRandomGenerator::global()->generate());
    payload["raw_story"] = prompt;
    payload["style"] = style;
    payload["user_id"] = "user_" + QString::number(QRandomGenerator::global()->generate());

    m_apiService->post(ApiConstants::ENDPOINT_CREATE, payload,
                       [this](bool success, const QJsonObject &resp) {

                           // 检查网络错误
                           if (!success) {
                               setIsGenerating(false);
                               int errCode = resp["code"].toInt();
                               QString errStr = resp["error"].toString();
                               emit errorOccurred(getNetworkErrorMessage(errCode, errStr));
                               return;
                           }

                           // 检查业务错误 code == 0
                           if (resp["code"].toInt() != 0) {
                               setIsGenerating(false);
                               QString msg = resp["msg"].toString();
                               emit errorOccurred(msg.isEmpty() ? "服务器返回错误" : msg);
                               return;
                           }

                           // 拿到 project_id
                           QJsonObject data = resp["data"].toObject();
                           QString projectId = data["project_id"].toString();

                           if (projectId.isEmpty()) {
                               setIsGenerating(false);
                               emit errorOccurred("无效的项目ID");
                               return;
                           }

                           m_currentProjectId = projectId;
                           emit currentProjectIdChanged();
                           setStatusMessage("任务已提交，AI正在创作中...");
                           m_pollTimer->start(); // 启动轮询
                       });
}

// 轮询分镜状态 (使用 /api/client/shots/list)
void StoryViewModel::checkTaskStatus()
{
    if (m_currentProjectId.isEmpty()) {
        m_pollTimer->stop();
        return;
    }

    QJsonObject params;
    params["project_id"] = m_currentProjectId;

    m_apiService->get(ApiConstants::ENDPOINT_SHOTS_LIST, params,
                      [this](bool success, const QJsonObject &resp) {

                          if (!success) return; // 网络错误忽略，等下次轮询

                          // 检查业务返回
                          if (resp["code"].toInt() != 0) {
                              // 可能还在处理中，继续等待
                              return;
                          }

                          // 解析 data
                          QJsonObject data = resp["data"].toObject();
                          QString status = data["status"].toString();

                          if (status == ApiConstants::STATUS_COMPLETED) {
                              // --- 成功 ---
                              m_pollTimer->stop();
                              setProgress(90);
                              setStatusMessage("分镜生成完成，正在下载图片...");

                              // 获取 project_data
                              QJsonObject projectDataObj = data["project_data"].toObject();
                              QVariantMap projectData = projectDataObj.toVariantMap();

                              qDebug() << "========== 分镜列表返回 ==========";
                              qDebug() << "原始 JSON:" << QJsonDocument(data).toJson(QJsonDocument::Compact);
                              qDebug() << "项目名称:" << projectData["name"].toString();
                              qDebug() << "项目ID:" << projectData["id"].toString();
                              
                              QVariantList storyboards = projectData["storyboards"].toList();
                              qDebug() << "分镜数量:" << storyboards.size();
                              
                              for (int i = 0; i < storyboards.size(); i++) {
                                  QVariantMap shot = storyboards[i].toMap();
                                  qDebug() << "--- 分镜" << i+1 << "---";
                                  qDebug() << "  shotId:" << shot["shotId"].toString();
                                  qDebug() << "  sceneTitle:" << shot["sceneTitle"].toString();
                                  qDebug() << "  localImagePath:" << shot["localImagePath"].toString();
                                  qDebug() << "  status:" << shot["status"].toString();
                              }
                              qDebug() << "====================================";

                              // 继续处理分镜图片
                              processStoryboardImages(projectData);

                          } else if (status == ApiConstants::STATUS_RUNNING) {
                              // --- 进行中 ---
                              int serverProgress = data["progress"].toInt();
                              int newProgress = (serverProgress > 0) ? serverProgress : (m_progress + 5);
                              if (newProgress > 90) newProgress = 90;

                              setProgress(newProgress);
                              setStatusMessage("正在生成分镜...");

                          } else if (status == ApiConstants::STATUS_FAILED) {
                              // --- 失败 ---
                              m_pollTimer->stop();
                              setIsGenerating(false);
                              emit errorOccurred("分镜生成失败");
                          }
                      });
}

// --------------------------------------------------------
// 2. 重绘图片 (完全重写为异步轮询)
// --------------------------------------------------------
void StoryViewModel::regenerateImage(const QString &projectId, const QString &shotId, const QString &newPrompt, const QString &style)
{
    QJsonObject payload;
    payload["project_id"] = projectId;
    payload["image_task_id"] = "image_task_" + shotId;  // 文档要求 image_task_id
    payload["image_id"] = shotId;
    payload["style"] = style;
    payload["prompt"] = newPrompt;
    payload["frame_num"] = 5;  // 默认帧数

    // 1. 提交重绘请求
    m_apiService->post(ApiConstants::ENDPOINT_REGEN_IMAGE, payload,
                       [this, shotId](bool success, const QJsonObject &resp) {

                           if (success && resp["code"].toInt() == 0) {
                               QJsonObject data = resp["data"].toObject();
                               QString taskId = data["regenerate_task_id"].toString();

                               qDebug() << "图片重绘任务已提交，ID:" << taskId;

                               // 2. 启动单次定时器开始轮询 (延迟1秒)
                               QTimer::singleShot(1000, this, [=](){
                                   checkSingleImageStatus(taskId, shotId);
                               });

                           } else {
                               // 网络错误或业务错误
                               if (resp.contains("error")) {
                                   int errCode = resp["code"].toInt();
                                   QString errStr = resp["error"].toString();
                                   emit errorOccurred(getNetworkErrorMessage(errCode, errStr));
                               } else {
                                   QString msg = resp["msg"].toString();
                                   emit errorOccurred(msg.isEmpty() ? "重绘请求失败" : "重绘请求失败: " + msg);
                               }
                           }
                       });
}

// 图片轮询逻辑 (递归调用)
void StoryViewModel::checkSingleImageStatus(const QString &taskId, const QString &shotId)
{
    QJsonObject params;
    params["image_task_id"] = taskId;  // 文档要求参数名为 image_task_id

    m_apiService->get(ApiConstants::ENDPOINT_CHECK_IMAGE, params,
                      [this, taskId, shotId](bool success, const QJsonObject &resp) {

                          if (!success) return; // 失败暂不处理

                          QJsonObject data = resp["data"].toObject();
                          QString status = data["status"].toString();

                          if (status == ApiConstants::STATUS_COMPLETED) {
                              // --- 成功 ---
                              QString newUrl = data["image_url"].toString();
                              qDebug() << "图片生成完成，开始下载:" << newUrl;

                              // 下载图片并保存到本地
                              downloadRegeneratedImage(shotId, newUrl);

                          } else if (status == ApiConstants::STATUS_FAILED) {
                              emit errorOccurred("图片生成失败");
                          } else {
                              // --- 仍在生成中，继续轮询 ---
                              // 1.5秒后再次检查
                              QTimer::singleShot(1500, this, [=](){
                                  checkSingleImageStatus(taskId, shotId);
                              });
                          }
                      });
}

// --------------------------------------------------------
// 3. 生成视频 / 导出 (保持不变或微调)
// --------------------------------------------------------
// 辅助函数
QString getRandomEffect() {
    const QString effects[] = {"zoom_in", "zoom_out", "pan_left", "pan_right"};
    int index = QRandomGenerator::global()->bounded(4);
    return effects[index];
}

void StoryViewModel::generateVideo(const QString &projectId) {
    // 略，因为我们主要用本地渲染
}

void StoryViewModel::exportVideo(const QString &projectId) {
    generateVideo(projectId);
}

// --------------------------------------------------------
// 4. 构建渲染配置 (适配本地逻辑)
// --------------------------------------------------------
QJsonObject StoryViewModel::buildRenderConfig(const QVariantMap &projectData)
{
    QJsonObject root;
    QJsonObject project;
    project["name"] = projectData["name"].toString();
    root["project"] = project;

    QJsonArray scenes;
    QVariantList storyboards = projectData["storyboards"].toList();
    int count = storyboards.size();

    for (int i = 0; i < count; ++i) {
        QVariantMap shot = storyboards[i].toMap();
        QJsonObject sceneItem;

        QString imagePath = shot["localImagePath"].toString();
        // 简单处理：如果是网络URL，TimelineRenderer可能需要支持，或者我们要先下载
        // 这里直接透传

        sceneItem["type"] = "image_scene";
        QJsonObject res;
        QJsonObject img; img["path"] = imagePath;
        res["image"] = img;

        // 默认时长
        sceneItem["duration"] = 5000;
        sceneItem["resources"] = res;

        QJsonObject effects;
        QJsonObject kb;
        kb["enabled"] = true;
        QString myEffect = shot["effect"].toString();
        kb["preset"] = myEffect.isEmpty() ? getRandomEffect() : myEffect;
        effects["ken_burns"] = kb;
        sceneItem["effects"] = effects;

        scenes.append(sceneItem);

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

// --------------------------------------------------------
// 5. 图片下载保存功能
// --------------------------------------------------------

void StoryViewModel::processStoryboardImages(const QVariantMap &projectData)
{
    // 保存项目数据，后续更新路径后使用
    m_pendingProjectData = projectData;

    QVariantList storyboards = projectData["storyboards"].toList();
    m_totalImages = storyboards.size();
    m_downloadedImages = 0;

    if (m_totalImages == 0) {
        // 没有图片需要下载
        setProgress(100);
        setStatusMessage("生成完成！");
        setIsGenerating(false);
        emit storyCreated(projectData);
        emit allImagesDownloaded(projectData);
        return;
    }

    // 遍历所有分镜，下载图片
    for (int i = 0; i < storyboards.size(); ++i) {
        QVariantMap shot = storyboards[i].toMap();
        QString shotId = shot["shotId"].toString();
        QString imageUrl = shot["localImagePath"].toString();

        if (!imageUrl.isEmpty() && imageUrl.startsWith("http")) {
            downloadAndSaveImage(shotId, imageUrl, i);
        } else {
            // 图片URL为空或已经是本地路径，跳过
            m_downloadedImages++;
            emit imageDownloadProgress(m_downloadedImages, m_totalImages);

            if (m_downloadedImages >= m_totalImages) {
                setProgress(100);
                setStatusMessage("生成完成！");
                setIsGenerating(false);
                emit storyCreated(m_pendingProjectData);
                emit allImagesDownloaded(m_pendingProjectData);
            }
        }
    }
}

void StoryViewModel::downloadAndSaveImage(const QString &shotId, const QString &imageUrl, int index)
{
    QUrl url(imageUrl);
    QNetworkRequest request(url);
    request.setTransferTimeout(60000); // 60秒超时

    // 使用 ApiService 内部的 manager 进行下载
    QNetworkReply *reply = m_apiService->getManager()->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, shotId, index]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray imageData = reply->readAll();

            // 使用 FileManager 保存图片
            if (m_fileManager) {
                // 设置 FileManager 的基础路径为用户选择的路径
                QString basePath = m_savePath;
                if (basePath.startsWith("file:///")) {
                    basePath = QUrl(basePath).toLocalFile();
                }

                // 使用自定义路径初始化（如果需要）
                m_fileManager->setBaseAppPath(basePath);

                // 保存图片
                QUrl localUrl = m_fileManager->saveShotImage(m_currentProjectId, shotId, imageData);

                if (localUrl.isValid()) {
                    // 更新项目数据中的路径
                    QVariantList storyboards = m_pendingProjectData["storyboards"].toList();
                    if (index < storyboards.size()) {
                        QVariantMap shot = storyboards[index].toMap();
                        shot["localImagePath"] = localUrl.toString();
                        shot["status"] = "generated";
                        storyboards[index] = shot;
                        m_pendingProjectData["storyboards"] = storyboards;
                    }
                    qDebug() << "图片已保存到:" << localUrl.toString();
                } else {
                    qWarning() << "保存图片失败: shotId =" << shotId;
                }
            }
        } else {
            qWarning() << "下载图片失败:" << reply->errorString() << "shotId =" << shotId;
            emit errorOccurred("下载图片失败: " + shotId);
        }

        // 更新进度
        m_downloadedImages++;
        emit imageDownloadProgress(m_downloadedImages, m_totalImages);

        int downloadProgress = 90 + (m_downloadedImages * 10 / m_totalImages);
        setProgress(downloadProgress);
        setStatusMessage(QString("正在下载图片 %1/%2...").arg(m_downloadedImages).arg(m_totalImages));

        // 检查是否全部下载完成
        if (m_downloadedImages >= m_totalImages) {
            setProgress(100);
            setStatusMessage("生成完成！");
            setIsGenerating(false);
            emit storyCreated(m_pendingProjectData);
            emit allImagesDownloaded(m_pendingProjectData);
        }
    });
}

// --------------------------------------------------------
// 6. 下载重新生成的图片
// --------------------------------------------------------
void StoryViewModel::downloadRegeneratedImage(const QString &shotId, const QString &imageUrl)
{
    QUrl url(imageUrl);
    QNetworkRequest request(url);
    request.setTransferTimeout(60000); // 60秒超时

    QNetworkReply *reply = m_apiService->getManager()->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, shotId, imageUrl]() {
        reply->deleteLater();

        QString localPath = imageUrl; // 默认使用原URL

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray imageData = reply->readAll();

            // 使用 FileManager 保存图片
            if (m_fileManager && !m_savePath.isEmpty()) {
                QString basePath = m_savePath;
                if (basePath.startsWith("file:///")) {
                    basePath = QUrl(basePath).toLocalFile();
                }

                m_fileManager->setBaseAppPath(basePath);
                QUrl localUrl = m_fileManager->saveShotImage(m_currentProjectId, shotId, imageData);

                if (localUrl.isValid()) {
                    localPath = localUrl.toString();
                    qDebug() << "重新生成的图片已保存到:" << localPath;
                } else {
                    qWarning() << "保存重新生成的图片失败: shotId =" << shotId;
                }
            }
        } else {
            qWarning() << "下载重新生成的图片失败:" << reply->errorString();
            emit errorOccurred("下载图片失败");
        }

        // 发送图片重新生成完成信号
        QVariantMap result;
        result["shotId"] = shotId;
        result["localImagePath"] = localPath;
        result["status"] = "generated";

        emit imageRegenerated(result);
    });
}

// --------------------------------------------------------
// 7. 获取分镜列表 (新 API: /api/client/shots/list)
// --------------------------------------------------------
void StoryViewModel::fetchShotsList()
{
    if (m_currentProjectId.isEmpty()) {
        emit errorOccurred("项目ID为空，无法获取分镜列表");
        setIsGenerating(false);
        return;
    }

    QJsonObject params;
    params["project_id"] = m_currentProjectId;

    qDebug() << "Fetching shots list for project:" << m_currentProjectId;

    m_apiService->get(ApiConstants::ENDPOINT_SHOTS_LIST, params,
                      [this](bool success, const QJsonObject &resp) {

                          if (!success) {
                              emit errorOccurred("获取分镜列表失败，请检查网络连接");
                              setIsGenerating(false);
                              return;
                          }

                          // 检查业务返回 code == 0
                          if (resp["code"].toInt() != 0) {
                              QString msg = resp["msg"].toString();
                              emit errorOccurred(msg.isEmpty() ? "获取分镜列表失败" : msg);
                              setIsGenerating(false);
                              return;
                          }

                          // 解析返回数据
                          QJsonObject data = resp["data"].toObject();
                          QString status = data["status"].toString();

                          if (status != ApiConstants::STATUS_COMPLETED) {
                              emit errorOccurred("分镜数据尚未准备好");
                              setIsGenerating(false);
                              return;
                          }

                          // 获取 project_data
                          QJsonObject projectDataObj = data["project_data"].toObject();
                          QVariantMap projectData = projectDataObj.toVariantMap();

                          // 更新 project_id (使用服务器返回的 id)
                          if (projectDataObj.contains("id")) {
                              m_currentProjectId = projectDataObj["id"].toString();
                              emit currentProjectIdChanged();
                          }

                          qDebug() << "Shots list received, project:" << projectData["name"].toString();
                          qDebug() << "Storyboards count:" << projectData["storyboards"].toList().size();

                          setProgress(90);
                          setStatusMessage("分镜列表获取成功，正在处理图片...");

                          // 继续处理分镜图片
                          processStoryboardImages(projectData);
                      });
}
