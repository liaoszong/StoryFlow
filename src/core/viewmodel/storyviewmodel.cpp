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

namespace ApiConstants {
const QString ENDPOINT_CREATE = "/api/client/generate/story";
const QString ENDPOINT_SHOTS_LIST = "/api/client/shots/list";
const QString ENDPOINT_GENERATE_IMAGES = "/api/generate/images";
const QString ENDPOINT_GENERATE_AUDIOS = "/api/generate/audios";
const QString ENDPOINT_REGEN_IMAGE = "/api/client/image/regenerate";
const QString ENDPOINT_CHECK_IMAGE = "/api/client/image/result";
const QString STATUS_COMPLETED = "completed";
const QString STATUS_RUNNING = "running";
const QString STATUS_FAILED = "failed";
}

static QString getNetworkErrorMessage(int errorCode, const QString &errorString) {
    switch (errorCode) {
        case QNetworkReply::ConnectionRefusedError: return "Connection refused";
        case QNetworkReply::HostNotFoundError: return "Host not found";
        case QNetworkReply::TimeoutError: return "Request timeout";
        default: return "Network error: " + errorString;
    }
}

StoryViewModel::StoryViewModel(ApiService *apiService, FileManager *fileManager, QObject *parent)
    : QObject(parent), m_apiService(apiService), m_fileManager(fileManager),
    m_isGenerating(false), m_progress(0), m_statusMessage(""),
    m_totalImages(0), m_downloadedImages(0), m_totalAudios(0), m_downloadedAudios(0)
{
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(2000);
    connect(m_pollTimer, &QTimer::timeout, this, &StoryViewModel::checkTaskStatus);
    
    m_mediaPollTimer = new QTimer(this);
    m_mediaPollTimer->setInterval(3000);
    connect(m_mediaPollTimer, &QTimer::timeout, this, &StoryViewModel::checkMediaGenerationStatus);
}

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

void StoryViewModel::createStory(const QString &prompt, const QString &style, const QString &savePath)
{
    if (m_isGenerating) return;
    setIsGenerating(true);
    setProgress(0);
    setStatusMessage("正在提交任务...");

    m_savePath = savePath;
    m_currentStyle = style;
    emit savePathChanged();
    m_currentUserId = "user_" + QString::number(QRandomGenerator::global()->generate());

    QJsonObject payload;
    payload["project_id"] = "proj_" + QString::number(QRandomGenerator::global()->generate());
    payload["raw_story"] = prompt;
    payload["style"] = style;
    payload["user_id"] = m_currentUserId;

    m_apiService->post(ApiConstants::ENDPOINT_CREATE, payload, [this](bool success, const QJsonObject &resp) {
        if (!success) { setIsGenerating(false); emit errorOccurred(getNetworkErrorMessage(resp["code"].toInt(), resp["error"].toString())); return; }
        if (resp["code"].toInt() != 0) { setIsGenerating(false); emit errorOccurred(resp["msg"].toString().isEmpty() ? "Server error" : resp["msg"].toString()); return; }
        QString projectId = resp["data"].toObject()["project_id"].toString();
        if (projectId.isEmpty()) { setIsGenerating(false); emit errorOccurred("Invalid project ID"); return; }
        m_currentProjectId = projectId;
        emit currentProjectIdChanged();
        setStatusMessage("AI 正在创作分镜...");
        m_pollTimer->start();
    });
}

void StoryViewModel::checkTaskStatus()
{
    if (m_currentProjectId.isEmpty()) { m_pollTimer->stop(); return; }
    QJsonObject params; params["project_id"] = m_currentProjectId;

    m_apiService->get(ApiConstants::ENDPOINT_SHOTS_LIST, params, [this](bool success, const QJsonObject &resp) {
        if (!success || resp["code"].toInt() != 0) return;
        QJsonObject data = resp["data"].toObject();
        QString status = data["status"].toString();

        if (status == ApiConstants::STATUS_COMPLETED) {
            m_pollTimer->stop();
            setProgress(30);
            setStatusMessage("分镜已就绪，正在生成媒体...");
            m_pendingProjectData = data["project_data"].toObject().toVariantMap();
            qDebug() << "Shots count:" << m_pendingProjectData["storyboards"].toList().size();
            requestGenerateImages();
            requestGenerateAudios();
        } else if (status == ApiConstants::STATUS_RUNNING) {
            int newProgress = qMin(m_progress + 5, 30);
            setProgress(newProgress);
            setStatusMessage("正在生成分镜...");
        } else if (status == ApiConstants::STATUS_FAILED) {
            m_pollTimer->stop();
            setIsGenerating(false);
            emit errorOccurred("分镜生成失败");
        }
    });
}

void StoryViewModel::requestGenerateImages()
{
    QVariantList storyboards = m_pendingProjectData["storyboards"].toList();
    if (storyboards.isEmpty()) return;

    QJsonObject payload;
    payload["project_id"] = m_currentProjectId;
    payload["user_id"] = m_currentUserId;

    for (int i = 0; i < storyboards.size(); ++i) {
        QVariantMap shot = storyboards[i].toMap();
        QJsonObject shotData;
        shotData["prompt"] = shot["prompt"].toString();
        shotData["image_url"] = "";
        payload["shot_" + QString::number(i + 1)] = shotData;
    }
    m_totalImages = storyboards.size();
    m_downloadedImages = 0;

    qDebug() << "Requesting images:" << QJsonDocument(payload).toJson(QJsonDocument::Compact);
    m_apiService->post(ApiConstants::ENDPOINT_GENERATE_IMAGES, payload, [this](bool success, const QJsonObject &resp) {
        if (!success || resp["code"].toInt() != 0) { qWarning() << "Image gen request failed"; return; }
        qDebug() << "Image generation submitted";
        if (!m_mediaPollTimer->isActive()) m_mediaPollTimer->start();
    });
}

void StoryViewModel::requestGenerateAudios()
{
    QVariantList storyboards = m_pendingProjectData["storyboards"].toList();
    if (storyboards.isEmpty()) return;

    QJsonObject payload;
    payload["project_id"] = m_currentProjectId;
    payload["user_id"] = m_currentUserId;

    for (int i = 0; i < storyboards.size(); ++i) {
        QVariantMap shot = storyboards[i].toMap();
        QJsonObject shotData;
        shotData["narration"] = shot["narration"].toString();
        shotData["audio_url"] = "";
        payload["shot_" + QString::number(i + 1)] = shotData;
    }
    m_totalAudios = storyboards.size();
    m_downloadedAudios = 0;

    qDebug() << "Requesting audios:" << QJsonDocument(payload).toJson(QJsonDocument::Compact);
    m_apiService->post(ApiConstants::ENDPOINT_GENERATE_AUDIOS, payload, [this](bool success, const QJsonObject &resp) {
        if (!success || resp["code"].toInt() != 0) { qWarning() << "Audio gen request failed"; return; }
        qDebug() << "Audio generation submitted";
        if (!m_mediaPollTimer->isActive()) m_mediaPollTimer->start();
    });
}

void StoryViewModel::checkMediaGenerationStatus()
{
    QJsonObject params; params["project_id"] = m_currentProjectId;
    m_apiService->get(ApiConstants::ENDPOINT_SHOTS_LIST, params, [this](bool success, const QJsonObject &resp) {
        if (!success || resp["code"].toInt() != 0) return;
        QJsonArray arr = resp["data"].toObject()["project_data"].toObject()["storyboards"].toArray();
        int imagesReady = 0, audiosReady = 0;
        QVariantList updated;
        for (int i = 0; i < arr.size(); ++i) {
            QJsonObject shot = arr[i].toObject();
            // 打印第一个 shot 的所有字段，用于调试
            if (i == 0) qDebug() << "Shot keys:" << shot.keys() << "image_url:" << shot["image_url"].toString() << "audio_url:" << shot["audio_url"].toString();
            // 检查 image_url 或 localImagePath
            QString imgUrl = shot["image_url"].toString();
            if (imgUrl.isEmpty()) imgUrl = shot["localImagePath"].toString();
            if (imgUrl.startsWith("http")) imagesReady++;
            // 检查 audio_url 或 audioPath
            QString audUrl = shot["audio_url"].toString();
            if (audUrl.isEmpty()) audUrl = shot["audioPath"].toString();
            if (audUrl.startsWith("http")) audiosReady++;
            // 保存 URL 到统一字段名
            shot["localImagePath"] = imgUrl;
            shot["audioPath"] = audUrl;
            updated.append(shot.toVariantMap());
        }
        qDebug() << "Media:" << imagesReady << "/" << m_totalImages << "img," << audiosReady << "/" << m_totalAudios << "aud";
        int total = m_totalImages + m_totalAudios;
        setProgress(30 + ((imagesReady + audiosReady) * 50 / qMax(total, 1)));
        setStatusMessage(QString("正在生成媒体 %1/%2...").arg(imagesReady + audiosReady).arg(total));
        if (imagesReady >= m_totalImages && audiosReady >= m_totalAudios) {
            m_mediaPollTimer->stop();
            m_pendingProjectData["storyboards"] = updated;
            setProgress(80);
            setStatusMessage("正在下载媒体...");
            downloadAllMedia();
        }
    });
}

void StoryViewModel::downloadAllMedia()
{
    QVariantList storyboards = m_pendingProjectData["storyboards"].toList();
    m_totalImages = m_downloadedImages = m_totalAudios = m_downloadedAudios = 0;
    for (const QVariant &v : storyboards) {
        QVariantMap shot = v.toMap();
        if (shot["localImagePath"].toString().startsWith("http")) m_totalImages++;
        if (shot["audioPath"].toString().startsWith("http")) m_totalAudios++;
    }
    if (m_totalImages == 0 && m_totalAudios == 0) { finishGeneration(); return; }
    for (int i = 0; i < storyboards.size(); ++i) {
        QVariantMap shot = storyboards[i].toMap();
        QString shotId = shot["shotId"].toString();
        if (shot["localImagePath"].toString().startsWith("http")) downloadAndSaveImage(shotId, shot["localImagePath"].toString(), i);
        if (shot["audioPath"].toString().startsWith("http")) downloadAndSaveAudio(shotId, shot["audioPath"].toString(), i);
    }
}

void StoryViewModel::downloadAndSaveImage(const QString &shotId, const QString &imageUrl, int index)
{
    QUrl url(imageUrl); QNetworkRequest req(url); req.setTransferTimeout(60000);
    QNetworkReply *reply = m_apiService->getManager()->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, shotId, index]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError && m_fileManager) {
            QString basePath = m_savePath;
            if (basePath.startsWith("file:///")) basePath = QUrl(basePath).toLocalFile();
            m_fileManager->setBaseAppPath(basePath);
            QUrl localUrl = m_fileManager->saveShotImage(m_currentProjectId, shotId, reply->readAll());
            if (localUrl.isValid()) {
                QVariantList sb = m_pendingProjectData["storyboards"].toList();
                if (index < sb.size()) { QVariantMap s = sb[index].toMap(); s["localImagePath"] = localUrl.toString(); s["status"] = "generated"; sb[index] = s; m_pendingProjectData["storyboards"] = sb; }
            }
        }
        m_downloadedImages++;
        checkDownloadComplete();
    });
}

void StoryViewModel::downloadAndSaveAudio(const QString &shotId, const QString &audioUrl, int index)
{
    QUrl url(audioUrl); QNetworkRequest req(url); req.setTransferTimeout(60000);
    QNetworkReply *reply = m_apiService->getManager()->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, shotId, index]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError && m_fileManager) {
            QString basePath = m_savePath;
            if (basePath.startsWith("file:///")) basePath = QUrl(basePath).toLocalFile();
            m_fileManager->setBaseAppPath(basePath);
            QUrl localUrl = m_fileManager->saveShotAudio(m_currentProjectId, shotId, reply->readAll());
            if (localUrl.isValid()) {
                QVariantList sb = m_pendingProjectData["storyboards"].toList();
                if (index < sb.size()) { QVariantMap s = sb[index].toMap(); s["audioPath"] = localUrl.toString(); sb[index] = s; m_pendingProjectData["storyboards"] = sb; }
            }
        }
        m_downloadedAudios++;
        checkDownloadComplete();
    });
}

void StoryViewModel::checkDownloadComplete()
{
    int total = m_totalImages + m_totalAudios;
    int downloaded = m_downloadedImages + m_downloadedAudios;
    setProgress(80 + (downloaded * 20 / qMax(total, 1)));
    setStatusMessage(QString("正在下载 %1/%2...").arg(downloaded).arg(total));
    emit imageDownloadProgress(downloaded, total);
    if (m_downloadedImages >= m_totalImages && m_downloadedAudios >= m_totalAudios) finishGeneration();
}

void StoryViewModel::cancelGeneration()
{
    if (!m_isGenerating) return;
    
    // 停止所有轮询定时器
    if (m_pollTimer) m_pollTimer->stop();
    if (m_mediaPollTimer) m_mediaPollTimer->stop();
    
    // 重置状态
    setIsGenerating(false);
    setProgress(0);
    setStatusMessage("已取消生成");
    
    // 清理数据
    m_currentProjectId.clear();
    m_currentTaskId.clear();
    m_pendingProjectData.clear();
    m_totalImages = 0;
    m_downloadedImages = 0;
    m_totalAudios = 0;
    m_downloadedAudios = 0;
    
    qDebug() << "Generation cancelled by user";
}

void StoryViewModel::finishGeneration()
{
    setProgress(100);
    setStatusMessage("生成完成！");
    setIsGenerating(false);
    emit storyCreated(m_pendingProjectData);
    emit allImagesDownloaded(m_pendingProjectData);
}

void StoryViewModel::regenerateImage(const QString &projectId, const QString &shotId, const QString &newPrompt, const QString &style)
{
    QJsonObject payload;
    payload["project_id"] = projectId;
    payload["image_task_id"] = "image_task_" + shotId;
    payload["image_id"] = shotId;
    payload["style"] = style;
    payload["prompt"] = newPrompt;
    m_apiService->post(ApiConstants::ENDPOINT_REGEN_IMAGE, payload, [this, shotId](bool success, const QJsonObject &resp) {
        if (success && resp["code"].toInt() == 0) {
            QString taskId = resp["data"].toObject()["regenerate_task_id"].toString();
            QTimer::singleShot(1000, this, [=](){ checkSingleImageStatus(taskId, shotId); });
        } else { emit errorOccurred(resp["msg"].toString().isEmpty() ? "Regen failed" : resp["msg"].toString()); }
    });
}

void StoryViewModel::checkSingleImageStatus(const QString &taskId, const QString &shotId)
{
    QJsonObject params; params["image_task_id"] = taskId;
    m_apiService->get(ApiConstants::ENDPOINT_CHECK_IMAGE, params, [this, taskId, shotId](bool success, const QJsonObject &resp) {
        if (!success) return;
        QString status = resp["data"].toObject()["status"].toString();
        if (status == ApiConstants::STATUS_COMPLETED) downloadRegeneratedImage(shotId, resp["data"].toObject()["image_url"].toString());
        else if (status == ApiConstants::STATUS_FAILED) emit errorOccurred("Image gen failed");
        else QTimer::singleShot(1500, this, [=](){ checkSingleImageStatus(taskId, shotId); });
    });
}

void StoryViewModel::downloadRegeneratedImage(const QString &shotId, const QString &imageUrl)
{
    QUrl url(imageUrl); QNetworkRequest req(url); req.setTransferTimeout(60000);
    QNetworkReply *reply = m_apiService->getManager()->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, shotId, imageUrl]() {
        reply->deleteLater();
        QString localPath = imageUrl;
        if (reply->error() == QNetworkReply::NoError && m_fileManager && !m_savePath.isEmpty()) {
            QString basePath = m_savePath;
            if (basePath.startsWith("file:///")) basePath = QUrl(basePath).toLocalFile();
            m_fileManager->setBaseAppPath(basePath);
            QUrl localUrl = m_fileManager->saveShotImage(m_currentProjectId, shotId, reply->readAll());
            if (localUrl.isValid()) localPath = localUrl.toString();
        }
        QVariantMap result; result["shotId"] = shotId; result["localImagePath"] = localPath; result["status"] = "generated";
        emit imageRegenerated(result);
    });
}

QString getRandomEffect() { const QString e[] = {"zoom_in", "zoom_out", "pan_left", "pan_right"}; return e[QRandomGenerator::global()->bounded(4)]; }
void StoryViewModel::generateVideo(const QString &projectId) { Q_UNUSED(projectId) }
void StoryViewModel::exportVideo(const QString &projectId) { generateVideo(projectId); }

QJsonObject StoryViewModel::buildRenderConfig(const QVariantMap &projectData)
{
    QJsonObject root, project; project["name"] = projectData["name"].toString(); root["project"] = project;
    QJsonArray scenes; QVariantList sb = projectData["storyboards"].toList();
    for (int i = 0; i < sb.size(); ++i) {
        QVariantMap shot = sb[i].toMap();
        QJsonObject sc; sc["type"] = "image_scene"; sc["duration"] = 5000;
        QJsonObject res, img; img["path"] = shot["localImagePath"].toString(); res["image"] = img; sc["resources"] = res;
        QJsonObject eff, kb; kb["enabled"] = true; kb["preset"] = shot["effect"].toString().isEmpty() ? getRandomEffect() : shot["effect"].toString(); eff["ken_burns"] = kb; sc["effects"] = eff;
        scenes.append(sc);
        if (i < sb.size() - 1) { QJsonObject tr; tr["type"] = "transition"; tr["transition_type"] = "crossfade"; tr["duration"] = 1.0; scenes.append(tr); }
    }
    root["scenes"] = scenes; return root;
}

void StoryViewModel::processStoryboardImages(const QVariantMap &projectData) { m_pendingProjectData = projectData; downloadAllMedia(); }
void StoryViewModel::fetchShotsList() {}
