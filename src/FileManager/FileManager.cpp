#include "FileManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QImage>
#include <QVariant> 
#include <QVariantMap>
#include <QDebug> // 日志系统

// --- 内部辅助常量 ---
const QString APP_DIR_NAME = "StoryToVideo-PC-LocalDocs";
const QString USERS_DIR_NAME = "Users";
const QString PROJECTS_DIR_NAME = "projects";
const QString SHOTS_DIR_NAME = "shots";
const QString STAGING_DIR_NAME = "staging";
const QString EXPORT_DIR_NAME = "export";
const QString CACHE_DIR_NAME = "cache";
const QString THUMBNAILS_CACHE_DIR_NAME = "thumbnails";
const QString APP_CACHE_DIR_NAME = "app";
const QString MODELS_DIR_NAME = "models";       // 新增：应用缓存-模型
const QString RESOURCES_DIR_NAME = "resources"; // 新增：应用缓存-资源
const QString TEMP_DIR_NAME = "temp";

const QString PROJECT_HEALTH_FILE_NAME = "projectHealth.json";
const QString PROJECT_THUMBNAIL_FILE_NAME = "thumbnail.jpg";
const QString SHOT_IMAGE_FILE_NAME = "image.png";
const QString SHOT_AUDIO_FILE_NAME = "audio.mp3";
const QString SHOT_HEALTH_FILE_NAME = "shotHealth.json";

const QString STAGING_IMAGE_SUFFIX = "_image_draft.png";
const QString STAGING_AUDIO_SUFFIX = "_audio_draft.mp3";

const int HASH_PREFIX_LENGTH = 2; // 缓存子目录名长度

// --- 构造/析构 ---

FileManager::FileManager(QObject* parent, const QString& customBasePath, const QString& userId)
    : QObject(parent)
{
    // 初始化存储目录的基础路径，支持自定义路径
    if (!customBasePath.isEmpty()) {
        m_baseAppPath = customBasePath;
    }
    else {
        QString standardAppDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        m_baseAppPath = QDir(standardAppDataPath).filePath(APP_DIR_NAME);
    }

    // 确保主应用目录及其子目录存在
    ensureDir(m_baseAppPath);
    setCurrentUserId(userId);

    ensureDir(getUserBasePath()); // 假设用户目录在此层级下，或需要动态获取当前用户 hash
    ensureDir(getProjectsBasePath());
    ensureDir(getCacheBasePath());
    ensureDir(getCacheThumbnailBasePath());
    ensureDir(getAppCacheBasePath());
    ensureDir(getTempBasePath());


    qDebug() << "FileManager initialized with base path:" << m_baseAppPath;
}

bool FileManager::initUserDirectories()
{
    // 假设 getUserBasePath() 已经存在
    QString userBase = getUserBasePath();

    // 检查基础路径是否已设置（如果 m_currentUserHash 为空，则可能返回空路径，但通常在构造函数中已处理）
    if (userBase.isEmpty()) {
        qWarning() << "FileManager: User base path is empty. Cannot initialize directories.";
        return false;
    }

    // --- 1. 用户基础目录 ---
    bool ok = ensureDir(userBase);

    // --- 2. 项目和缓存根目录 ---
    ok &= ensureDir(userBase + QDir::separator() + PROJECTS_DIR_NAME);
    ok &= ensureDir(getCacheBasePath()); // 缓存根目录 (cache/)

    // --- 3. 缩略图缓存根目录 ---
    ok &= ensureDir(getCacheThumbnailBasePath()); // cache/thumbnails/

    // --- 4. 应用级缓存目录 ---
    ok &= ensureDir(getAppCachePath());         // cache/app/
    ok &= ensureDir(getAppCacheModelsPath());   // cache/app/models/
    ok &= ensureDir(getAppCacheResourcesPath()); // cache/app/resources/

    if (!ok) {
        qWarning() << "FileManager: Failed to create one or more critical user directories.";
    }

    return ok;
}

// 自定义用户 ID 的设置和切换逻辑
void FileManager::setCurrentUserId(const QString& userId) {
    // 确保哈希值至少有默认值
    m_currentUserHash = hashString(userId.trimmed());

    // 1. 尝试初始化用户目录结构
        // 假设 initUserDirectories() 会调用 ensureDir 创建所有必要目录
    bool success = initUserDirectories();

    QVariantMap context;
    context["userId"] = userId;
    context["userHash"] = m_currentUserHash;

    if (success) {
        qDebug() << "FileManager: User switched. Hash:" << m_currentUserHash;
        // 信号触发：操作完成
        emit operationCompleted("setCurrentUserId", QVariant(m_currentUserHash), context);
    }
    else {
        qWarning() << "FileManager: Failed to set up directories for user:" << userId;
        // 信号触发：操作失败
        emit operationFailed("setCurrentUserId", "Failed to create user directory structure.", context);
    }
}

// 获取项目临时文件路径 (projectRoot/staging/temp)
QString FileManager::getProjectStagingTempPath(const QString& projectId) const
{
    return getProjectStagingBasePath(projectId) + QDir::separator() + TEMP_DIR_NAME;
}

// 获取缩略图缓存路径 (cacheRoot/thumbnails/{hash_prefix})
QString FileManager::getCacheThumbnailPath(const QString& id) const
{
    QString idHash = hashString(id);
    // 使用哈希值的前两位作为子目录，分散文件
    QString hashPrefix = idHash.left(2);
    return getCacheBasePath() + QDir::separator() + THUMBNAILS_CACHE_DIR_NAME + QDir::separator() + hashPrefix;
}

// 获取应用缓存路径 (cacheRoot/app)
QString FileManager::getAppCachePath() const
{
    return getCacheBasePath() + QDir::separator() + APP_CACHE_DIR_NAME;
}

// 获取应用模型缓存路径 (cacheRoot/app/models)
QString FileManager::getAppCacheModelsPath() const
{
    return getAppCachePath() + QDir::separator() + MODELS_DIR_NAME;
}

// 获取应用资源缓存路径 (cacheRoot/app/resources)
QString FileManager::getAppCacheResourcesPath() const
{
    return getAppCachePath() + QDir::separator() + RESOURCES_DIR_NAME;
}


// 程序运行时可切换基础路径
//void FileManager::setBaseAppPath(const QString& customBasePath)
//{
//    if (!customBasePath.isEmpty()) {
//        m_baseAppPath = customBasePath;
//        // 重新 ensureDir 各子目录
//        ensureDir(m_baseAppPath);
//        ensureDir(getUserBasePath());
//        ensureDir(getProjectsBasePath());
//        ensureDir(getCacheBasePath());
//        ensureDir(getCacheThumbnailBasePath());
//        ensureDir(getAppCacheBasePath());
//        ensureDir(getTempBasePath());
//    }
//}

// 哈希工具函数，用于生成用户ID、项目ID等的SHA256哈希值
QString FileManager::hashString(const QString& input) const {
    if (input.isEmpty()) {
        // "anonymous" 的 SHA256 哈希
        return QString(QCryptographicHash::hash("anonymous", QCryptographicHash::Sha256).toHex());
    }
    QByteArray data = input.toUtf8();
    return QString(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}




//QString FileManager::getUserBasePath() const {
//    QString currentUserIdHash = QCryptographicHash::hash(getCurrentUserId().toUtf8(), QCryptographicHash::Md5).toHex().left(HASH_PREFIX_LENGTH * 2); // 使用MD5并取前N位字符作为哈希值
//    return QDir(m_baseAppPath).filePath(USERS_DIR_NAME + "/" + "user_" + currentUserIdHash);
//}

// 获取用户目录的基础路径 (appRoot/Users/user_{hash})
QString FileManager::getUserBasePath() const
{
    // 确保使用 m_currentUserHash 成员变量
    return m_baseAppPath + QDir::separator() + USERS_DIR_NAME + QDir::separator() + "user_" + m_currentUserHash;
}

QString FileManager::getProjectsBasePath() const {
    return QDir(getUserBasePath()).filePath(PROJECTS_DIR_NAME);
}

QString FileManager::getCacheThumbnailFilePath(const QString& id) const
{
    // 文件名格式: {id_hash}_thumbnail.jpg
    QString fileName = hashString(id) + "_thumbnail.jpg";
    return getCacheThumbnailPath(id) + QDir::separator() + fileName;
}


// 修改 saveCacheThumbnail
bool FileManager::saveCacheThumbnail(const QString& id, const QImage& thumbnail) const
{
    QString dirPath = getCacheThumbnailPath(id);
    QString filePath = getCacheThumbnailFilePath(id);

    // 确保哈希前缀的子目录存在
    if (!ensureDir(dirPath)) {
        emit operationFailed("saveCacheThumbnail", "Failed to create directory: " + dirPath, QVariantMap());
        return false;
    }

    // 后续保存逻辑：将 QImage 写入文件
    // 缩略图使用较低质量的 JPG 格式 (85) 存储以节省空间。
    if (!thumbnail.save(filePath, "JPG", 85)) {
        // 保存失败，触发 operationFailed
        emit operationFailed("saveCacheThumbnail", "Failed to save image to: " + filePath, QVariantMap());
        return false;
    }

    // 保存成功，触发 operationCompleted
    QVariantMap context;
    context["id"] = id;

    // operationCompleted 信号需要 3 个参数：操作名, 结果, 上下文
    // 结果 (result) 可以是保存的文件路径
    emit operationCompleted("saveCacheThumbnail", filePath, context);

    return true;
}

// --- 私有辅助函数 ---

QString FileManager::getCacheBasePath() const {
    return QDir(m_baseAppPath).filePath(CACHE_DIR_NAME);
}

//QString FileManager::ensureDir(const QString& dirPath) {
//    QDir dir(dirPath);
//    if (!dir.exists()) {
//        if (!dir.mkpath(".")) {
//            qWarning() << "Failed to create directory:" << dirPath;
//            // 需要抛出异常或设置错误状态
//        }
//    }
//    return dirPath;
//}

bool FileManager::ensureDir(const QString& dirPath) const
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        // 将 dirPath 本身作为参数传递给 mkpath，或者直接在 QDir 对象上调用 mkpath(dirPath)
        if (!dir.mkpath(dirPath)) {
            qWarning() << "FileManager: Failed to create directory:" << dirPath;
            return false;
        }
    }
    return true;
}



QString FileManager::getProjectRootPath(const QString& projectId) const {
    //return QDir(getProjectsBasePath()).filePath(projectId);
    return getUserBasePath()
        + QDir::separator() + PROJECTS_DIR_NAME
        + QDir::separator() + "proj_" + projectId;
}

QString FileManager::getShotRootPath(const QString& projectId, const QString& shotId) const {
    //return QDir(getProjectRootPath(projectId)).filePath(SHOTS_DIR_NAME + "/" + shotId);
    return getProjectRootPath(projectId)
        + QDir::separator() + SHOTS_DIR_NAME
        + QDir::separator() + "shot_" + shotId; // <-- 严格控制命名
}

QString FileManager::getShotImagePath(const QString& projectId, const QString& shotId) const {
    return QDir(getShotRootPath(projectId, shotId)).filePath(SHOT_IMAGE_FILE_NAME);
}

QString FileManager::getShotAudioPath(const QString& projectId, const QString& shotId) const {
    return QDir(getShotRootPath(projectId, shotId)).filePath(SHOT_AUDIO_FILE_NAME);
}

QString FileManager::getShotHealthSnapshotPath(const QString& projectId, const QString& shotId) const {
    return QDir(getShotRootPath(projectId, shotId)).filePath(SHOT_HEALTH_FILE_NAME);
}

QString FileManager::getProjectHealthSnapshotPath(const QString& projectId) const {
    return QDir(getProjectRootPath(projectId)).filePath(PROJECT_HEALTH_FILE_NAME);
}

QString FileManager::getProjectThumbnailPath(const QString& projectId) const {
    return QDir(getProjectRootPath(projectId)).filePath(PROJECT_THUMBNAIL_FILE_NAME);
}

QString FileManager::getStagingImagePath(const QString& projectId, const QString& shotId) const {
    return QDir(getProjectRootPath(projectId)).filePath(STAGING_DIR_NAME + "/" + shotId + STAGING_IMAGE_SUFFIX);
}

QString FileManager::getStagingAudioPath(const QString& projectId, const QString& shotId) const {
    return QDir(getProjectRootPath(projectId)).filePath(STAGING_DIR_NAME + "/" + shotId + STAGING_AUDIO_SUFFIX);
}

QString FileManager::getExportVideoPath(const QString& projectId) const {
    return QDir(getProjectRootPath(projectId)).filePath(EXPORT_DIR_NAME + "/final_video.mp4");
}

QString FileManager::getCacheThumbnailBasePath() const {
    return QDir(getCacheBasePath()).filePath(THUMBNAILS_CACHE_DIR_NAME);
}

QString FileManager::getAppCacheBasePath() const {
    return QDir(getCacheBasePath()).filePath(APP_CACHE_DIR_NAME);
}

QString FileManager::getTempBasePath() const {
    return QDir(getCacheBasePath()).filePath(TEMP_DIR_NAME);
}

QString FileManager::getProjectStagingBasePath(const QString& projectId) const {
    return QDir(getProjectRootPath(projectId)).filePath(STAGING_DIR_NAME);
}

QString FileManager::getProjectExportBasePath(const QString& projectId) const {
    return QDir(getProjectRootPath(projectId)).filePath(EXPORT_DIR_NAME);
}

QUrl FileManager::toFileUrl(const QString& localPath) const {
    QFileInfo info(localPath);
    if (info.exists()) {
        return QUrl::fromLocalFile(info.absoluteFilePath());
    }
    return QUrl(); // Return invalid URL if file doesn't exist or path is empty
}

bool FileManager::fileExists(const QString& filePathOrUrl) const {
    if (filePathOrUrl.startsWith("file://")) {
        return QFile(QUrl(filePathOrUrl).toLocalFile()).exists();
    }
    else {
        return QFile(filePathOrUrl).exists();
    }
}


QString FileManager::calculateChecksum(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file for checksum calculation:" << filePath;
        return QString();
    }

    QCryptographicHash hash(QCryptographicHash::Md5); // Or Sha256 etc.
    if (hash.addData(&file)) {
        return hash.result().toHex();
    }
    else {
        qWarning() << "Failed to read data for checksum of file:" << filePath;
        return QString();
    }
}

// Simplified move function - might need more robustness (overwrite handling, permissions)
bool FileManager::moveFile(const QString& sourcePath, const QString& destinationPath) {
    QFileInfo destInfo(destinationPath);
    QDir destDir = destInfo.dir();
    if (!destDir.exists()) {
        ensureDir(destDir.absolutePath());
    }

    if (QFile::rename(sourcePath, destinationPath)) {
        return true;
    }
    else {
        qWarning() << "Failed to move file from" << sourcePath << "to" << destinationPath;
        return false;
    }
}


// --- 项目级接口实现 ---

QUrl FileManager::saveProjectThumbnail(const QString& projectId, const QByteArray& thumbnailData) {
    QString projectThumbPath = getProjectThumbnailPath(projectId);

    // 确保项目缩略图文件所在的目录存在
    QFileInfo fileInfo(projectThumbPath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) { // mkpath(".") 会创建 dir 所代表的路径
            QString errorStr = QString("Failed to create directory for thumbnail: %1").arg(dir.path());
            qWarning() << errorStr; // 添加日志记录
            emit operationFailed("saveProjectThumbnail", errorStr, { {"projectId", projectId}, {"targetDirectory", dir.path()} });
            return QUrl();
        }
    }

    QFile file(projectThumbPath);
    if (file.open(QIODevice::WriteOnly)) {
        qint64 written = file.write(thumbnailData);
        file.close();
        if (written == thumbnailData.size()) {
            emit operationCompleted("saveProjectThumbnail", QVariant(toFileUrl(projectThumbPath)), { {"projectId", projectId} });
            return toFileUrl(projectThumbPath);
        }
        else {
            QString errorStr = QString("Failed to write complete thumbnail data. Wrote %1/%2 bytes.").arg(written).arg(thumbnailData.size());
            emit operationFailed("saveProjectThumbnail", errorStr, { {"projectId", projectId} });
        }
    }
    else {
        QString errorStr = QString("Cannot open file for writing: %1").arg(projectThumbPath);
        emit operationFailed("saveProjectThumbnail", errorStr, { {"projectId", projectId} });
    }
    return QUrl(); // Return invalid URL on failure
}

QByteArray FileManager::loadProjectThumbnail(const QString& projectId) {
    QString projectThumbPath = getProjectThumbnailPath(projectId);
    QFile file(projectThumbPath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        emit operationCompleted("loadProjectThumbnail", QVariant(data), { {"projectId", projectId} });
        return data;
    }
    else {
        QString errorStr = QString("Cannot open file for reading: %1").arg(projectThumbPath);
        emit operationFailed("loadProjectThumbnail", errorStr, { {"projectId", projectId} });
    }
    return QByteArray();
}

QUrl FileManager::saveProjectHealthSnapshot(const QString& projectId, const QJsonObject& metadata) {
    QString snapshotPath = getProjectHealthSnapshotPath(projectId);

    // 确保项目健康快照文件所在的目录存在
    QFileInfo fileInfo(snapshotPath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            QString errorStr = QString("Failed to create directory for health snapshot: %1").arg(dir.path());
            qWarning() << errorStr;
            emit operationFailed("saveProjectHealthSnapshot", errorStr, { {"projectId", projectId}, {"targetDirectory", dir.path()} });
            return QUrl();
        }
    }

    QFile file(snapshotPath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(metadata);
        file.write(doc.toJson());
        file.close();
        emit operationCompleted("saveProjectHealthSnapshot", QVariant(toFileUrl(snapshotPath)), { {"projectId", projectId} });
        return toFileUrl(snapshotPath);
    }
    else {
        QString errorStr = QString("Cannot open file for writing: %1").arg(snapshotPath);
        emit operationFailed("saveProjectHealthSnapshot", errorStr, { {"projectId", projectId} });
    }
    return QUrl();
}

QJsonObject FileManager::loadProjectHealthSnapshot(const QString& projectId) {
    QString snapshotPath = getProjectHealthSnapshotPath(projectId);
    QFile file(snapshotPath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            emit operationCompleted("loadProjectHealthSnapshot", QVariant(doc.object()), { {"projectId", projectId} });
            return doc.object();
        }
        else {
            QString errorStr = QString("Failed to parse JSON in %1: %2").arg(snapshotPath).arg(parseError.errorString());
            emit operationFailed("loadProjectHealthSnapshot", errorStr, { {"projectId", projectId} });
        }
    }
    else {
        QString errorStr = QString("Cannot open file for reading: %1").arg(snapshotPath);
        emit operationFailed("loadProjectHealthSnapshot", errorStr, { {"projectId", projectId} });
    }
    return QJsonObject(); // Return empty object on failure
}

//bool FileManager::deleteProject(const QString& projectId) {
//    QString projectPath = getProjectRootPath(projectId);
//    if (QDir(projectPath).removeRecursively()) {
//        emit operationCompleted("deleteProject", QVariant(true), { {"projectId", projectId} });
//        return true;
//    }
//    else {
//        QString errorStr = QString("Failed to remove project directory: %1").arg(projectPath);
//        emit operationFailed("deleteProject", errorStr, { {"projectId", projectId} });
//        return false;
//    }
//}

// 假设 getProjectRootPath() 存在

// 删除整个项目（异步安全，含 shots/staging/export）
bool FileManager::deleteProject(const QString& projectId)
{
    const QString operation = "deleteProject";
    QString projectPath = getProjectRootPath(projectId);
    QDir projectDir(projectPath);

    QVariantMap context;
    context["projectId"] = projectId;

    if (!projectDir.exists()) {
        // 如果目录不存在，视为成功
        emit operationCompleted(operation, QVariant(true), context);
        return true;
    }

    // 递归删除目录内容及本身
    if (projectDir.removeRecursively()) {
        context["path"] = projectPath;
        // 信号触发：操作完成
        emit operationCompleted(operation, QVariant(true), context);
        return true;
    }

    // 信号触发：操作失败
    emit operationFailed(operation, "Failed to recursively delete project directory.", context);
    return false;
}

bool FileManager::updateIntegrityData(const QString& projectId, const QString& filePath, const QString& checksum) {
    QJsonObject projectHealth = loadProjectHealthSnapshot(projectId);
    if (projectHealth.isEmpty()) {
        QString errorStr = QString("Cannot load project health snapshot to update integrity for %1").arg(projectId);
        emit operationFailed("updateIntegrityData", errorStr, { {"projectId", projectId}, {"filePath", filePath} });
        return false;
    }

    QJsonObject integrityObj = projectHealth["integrity"].toObject();
    QJsonObject fileInfo;
    fileInfo["checksum"] = checksum;
    fileInfo["lastVerified"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    integrityObj[filePath] = fileInfo;
    projectHealth["integrity"] = integrityObj;

    // Update lastChecked timestamp
    projectHealth["lastChecked"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    if (!saveProjectHealthSnapshot(projectId, projectHealth).isValid()) {
        // saveProjectHealthSnapshot already emitted operationFailed
        return false;
    }
    emit operationCompleted("updateIntegrityData", QVariant(true), { {"projectId", projectId}, {"filePath", filePath} });
    return true;
}


QVariantMap FileManager::verifyProjectFilesAgainstManifest(const QString& projectId) {
    QJsonObject projectHealth = loadProjectHealthSnapshot(projectId);
    if (projectHealth.isEmpty()) {
        QString errorStr = QString("Cannot load project health snapshot to verify files for %1").arg(projectId);
        emit operationFailed("verifyProjectFilesAgainstManifest", errorStr, { {"projectId", projectId} });
        return QVariantMap({ {"error", errorStr} });
    }

    QVariantMap report;
    QVariantList missingFiles;
    QVariantList corruptedFiles;
    QJsonObject expectedFiles = projectHealth["expectedFiles"].toObject();
    QJsonObject integrityData = projectHealth["integrity"].toObject();

    // Check Thumbnail
    QString thumbRelPath = expectedFiles["thumbnail"].toString();
    if (!thumbRelPath.isEmpty()) {
        QString absPath = QDir(getProjectRootPath(projectId)).filePath(thumbRelPath);
        if (!fileExists(absPath)) {
            missingFiles.append(thumbRelPath);
        }
        else {
            QJsonObject thumbIntegrity = integrityData[thumbRelPath].toObject();
            QString expectedChecksum = thumbIntegrity["checksum"].toString();
            if (!expectedChecksum.isEmpty() && !validateFileIntegrity(QUrl::fromLocalFile(absPath), expectedChecksum)) {
                corruptedFiles.append(thumbRelPath);
            }
        }
    }


    // Check Shots
    QJsonArray shotsArray = expectedFiles["shots"].toArray();
    for (const auto& shotValue : shotsArray) {
        QJsonObject shotEntry = shotValue.toObject();
        QString shotId = shotEntry["shotId"].toString();
        QString imageRelPath = shotEntry["image"].toString();
        QString audioRelPath = shotEntry["audio"].toString(); // Can be null/empty

        if (!imageRelPath.isEmpty()) {
            QString absImagePath = QDir(getProjectRootPath(projectId)).filePath(imageRelPath);
            if (!fileExists(absImagePath)) {
                missingFiles.append(imageRelPath);
            }
            else {
                QJsonObject imgIntegrity = integrityData[imageRelPath].toObject();
                QString expectedImgChecksum = imgIntegrity["checksum"].toString();
                if (!expectedImgChecksum.isEmpty() && !validateFileIntegrity(QUrl::fromLocalFile(absImagePath), expectedImgChecksum)) {
                    corruptedFiles.append(imageRelPath);
                }
            }
        }

        if (!audioRelPath.isEmpty()) {
            QString absAudioPath = QDir(getProjectRootPath(projectId)).filePath(audioRelPath);
            if (!fileExists(absAudioPath)) {
                missingFiles.append(audioRelPath);
            }
            else {
                QJsonObject audioIntegrity = integrityData[audioRelPath].toObject();
                QString expectedAudioChecksum = audioIntegrity["checksum"].toString();
                if (!expectedAudioChecksum.isEmpty() && !validateFileIntegrity(QUrl::fromLocalFile(absAudioPath), expectedAudioChecksum)) {
                    corruptedFiles.append(audioRelPath);
                }
            }
        }
    }

    // Check Export (Optional)
    QString exportRelPath = expectedFiles["export"].toString();
    if (!exportRelPath.isEmpty()) {
        QString absExportPath = QDir(getProjectRootPath(projectId)).filePath(exportRelPath);
        if (!fileExists(absExportPath)) {
            // Note: Export might not exist yet, maybe don't treat as missing?
            // For now, let's include it if it's listed but missing.
            missingFiles.append(exportRelPath);
        } // No integrity check for export file typically
    }


    report["missingFiles"] = missingFiles;
    report["corruptedFiles"] = corruptedFiles;
    report["verifiedAt"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    emit operationCompleted("verifyProjectFilesAgainstManifest", QVariant(report), { {"projectId", projectId} });
    return report;
}



// --- 分镜级接口实现 (正式区: shots/) ---

QUrl FileManager::saveShotImage(const QString& projectId, const QString& shotId, const QByteArray& imageData) {
    QString imagePath = getShotImagePath(projectId, shotId);
    ensureDir(QFileInfo(imagePath).absolutePath()); // 确保分镜目录存在

    QFile file(imagePath);
    if (file.open(QIODevice::WriteOnly)) {
        qint64 written = file.write(imageData);
        file.close();
        if (written == imageData.size()) {
            emit operationCompleted("saveShotImage", QVariant(toFileUrl(imagePath)), { {"projectId", projectId}, {"shotId", shotId} });
            return toFileUrl(imagePath);
        }
        else {
            QString errorStr = QString("Failed to write complete image data. Wrote %1/%2 bytes.").arg(written).arg(imageData.size());
            emit operationFailed("saveShotImage", errorStr, { {"projectId", projectId}, {"shotId", shotId} });
        }
    }
    else {
        QString errorStr = QString("Cannot open file for writing: %1").arg(imagePath);
        emit operationFailed("saveShotImage", errorStr, { {"projectId", projectId}, {"shotId", shotId} });
    }
    return QUrl(); //失败时返回无效URL
}

QByteArray FileManager::loadShotImage(const QString& projectId, const QString& shotId) {
    QString imagePath = getShotImagePath(projectId, shotId);
    QFile file(imagePath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        emit operationCompleted("loadShotImage", QVariant(data), { {"projectId", projectId}, {"shotId", shotId} });
        return data;
    }
    else {
        QString errorStr = QString("Cannot open file for reading: %1").arg(imagePath);
        emit operationFailed("loadShotImage", errorStr, { {"projectId", projectId}, {"shotId", shotId} });
    }
    return QByteArray();
}

QUrl FileManager::saveShotAudio(const QString& projectId, const QString& shotId, const QByteArray& audioData) {
    QString audioPath = getShotAudioPath(projectId, shotId);
    ensureDir(QFileInfo(audioPath).absolutePath()); // Ensure shot directory exists

    QFile file(audioPath);
    if (file.open(QIODevice::WriteOnly)) {
        qint64 written = file.write(audioData);
        file.close();
        if (written == audioData.size()) {
            emit operationCompleted("saveShotAudio", QVariant(toFileUrl(audioPath)), { {"projectId", projectId}, {"shotId", shotId} });
            return toFileUrl(audioPath);
        }
        else {
            QString errorStr = QString("Failed to write complete audio data. Wrote %1/%2 bytes.").arg(written).arg(audioData.size());
            emit operationFailed("saveShotAudio", errorStr, { {"projectId", projectId}, {"shotId", shotId} });
        }
    }
    else {
        QString errorStr = QString("Cannot open file for writing: %1").arg(audioPath);
        emit operationFailed("saveShotAudio", errorStr, { {"projectId", projectId}, {"shotId", shotId} });
    }
    return QUrl();
}

QUrl FileManager::saveShotHealthSnapshot(const QString& projectId, const QString& shotId, const QJsonObject& statusInfo) {
    QString snapshotPath = getShotHealthSnapshotPath(projectId, shotId);
    ensureDir(QFileInfo(snapshotPath).absolutePath()); // Ensure shot directory exists

    QFile file(snapshotPath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(statusInfo);
        file.write(doc.toJson());
        file.close();
        emit operationCompleted("saveShotHealthSnapshot", QVariant(toFileUrl(snapshotPath)), { {"projectId", projectId}, {"shotId", shotId} });
        return toFileUrl(snapshotPath);
    }
    else {
        QString errorStr = QString("Cannot open file for writing: %1").arg(snapshotPath);
        emit operationFailed("saveShotHealthSnapshot", errorStr, { {"projectId", projectId}, {"shotId", shotId} });
    }
    return QUrl();
}

QJsonObject FileManager::loadShotHealthSnapshot(const QString& projectId, const QString& shotId) {
    QString snapshotPath = getShotHealthSnapshotPath(projectId, shotId);
    QFile file(snapshotPath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            emit operationCompleted("loadShotHealthSnapshot", QVariant(doc.object()), { {"projectId", projectId}, {"shotId", shotId} });
            return doc.object();
        }
        else {
            QString errorStr = QString("Failed to parse JSON in %1: %2").arg(snapshotPath).arg(parseError.errorString());
            emit operationFailed("loadShotHealthSnapshot", errorStr, { {"projectId", projectId}, {"shotId", shotId} });
        }
    }
    else {
        QString errorStr = QString("Cannot open file for reading: %1").arg(snapshotPath);
        emit operationFailed("loadShotHealthSnapshot", errorStr, { {"projectId", projectId}, {"shotId", shotId} });
    }
    return QJsonObject();
}


// --- 临时生成区接口实现 (staging/) ---

QUrl FileManager::stageImage(const QString& projectId, const QString& shotId, const QByteArray& imageData) {
    QString stagingPath = getStagingImagePath(projectId, shotId);
    ensureDir(QFileInfo(stagingPath).absolutePath()); // Ensure staging directory exists

    QFile file(stagingPath);
    if (file.open(QIODevice::WriteOnly)) {
        qint64 written = file.write(imageData);
        file.close();
        if (written == imageData.size()) {
            emit operationCompleted("stageImage", QVariant(toFileUrl(stagingPath)), { {"projectId", projectId}, {"shotId", shotId} });
            return toFileUrl(stagingPath);
        }
        else {
            QString errorStr = QString("Failed to write complete staged image data. Wrote %1/%2 bytes.").arg(written).arg(imageData.size());
            emit operationFailed("stageImage", errorStr, { {"projectId", projectId}, {"shotId", shotId} });
        }
    }
    else {
        QString errorStr = QString("Cannot open file for writing: %1").arg(stagingPath);
        emit operationFailed("stageImage", errorStr, { {"projectId", projectId}, {"shotId", shotId} });
    }
    return QUrl();
}

QUrl FileManager::stageAudio(const QString& projectId, const QString& shotId, const QByteArray& audioData) {
    QString stagingPath = getStagingAudioPath(projectId, shotId);
    ensureDir(QFileInfo(stagingPath).absolutePath()); // Ensure staging directory exists

    QFile file(stagingPath);
    if (file.open(QIODevice::WriteOnly)) {
        qint64 written = file.write(audioData);
        file.close();
        if (written == audioData.size()) {
            emit operationCompleted("stageAudio", QVariant(toFileUrl(stagingPath)), { {"projectId", projectId}, {"shotId", shotId} });
            return toFileUrl(stagingPath);
        }
        else {
            QString errorStr = QString("Failed to write complete staged audio data. Wrote %1/%2 bytes.").arg(written).arg(audioData.size());
            emit operationFailed("stageAudio", errorStr, { {"projectId", projectId}, {"shotId", shotId} });
        }
    }
    else {
        QString errorStr = QString("Cannot open file for writing: %1").arg(stagingPath);
        emit operationFailed("stageAudio", errorStr, { {"projectId", projectId}, {"shotId", shotId} });
    }
    return QUrl();
}

bool FileManager::commitStagedMedia(const QString& projectId, const QString& shotId, const QString& mediaType) {
    QString sourcePath, destinationPath;
    if (mediaType == "image") {
        sourcePath = getStagingImagePath(projectId, shotId);
        destinationPath = getShotImagePath(projectId, shotId);
    }
    else if (mediaType == "audio") {
        sourcePath = getStagingAudioPath(projectId, shotId);
        destinationPath = getShotAudioPath(projectId, shotId);
    }
    else {
        QString errorStr = QString("Invalid media type '%1' for commitStagedMedia").arg(mediaType);
        emit operationFailed("commitStagedMedia", errorStr, { {"projectId", projectId}, {"shotId", shotId}, {"mediaType", mediaType} });
        return false;
    }

    if (!fileExists(sourcePath)) {
        QString errorStr = QString("Staged file does not exist: %1").arg(sourcePath);
        emit operationFailed("commitStagedMedia", errorStr, { {"projectId", projectId}, {"shotId", shotId}, {"mediaType", mediaType} });
        return false;
    }

    if (moveFile(sourcePath, destinationPath)) {
        // Optionally, clean up the staging entry in shotHealth.json if needed
        emit operationCompleted("commitStagedMedia", QVariant(true), { {"projectId", projectId}, {"shotId", shotId}, {"mediaType", mediaType} });
        return true;
    }
    else {
        // moveFile should have logged the error
        QString errorStr = QString("Failed to move staged %1 to final location.").arg(mediaType);
        emit operationFailed("commitStagedMedia", errorStr, { {"projectId", projectId}, {"shotId", shotId}, {"mediaType", mediaType} });
        return false;
    }
}

bool FileManager::cleanupStagingForProject(const QString& projectId) {
    QString stagingPath = getProjectStagingBasePath(projectId);
    if (QDir(stagingPath).removeRecursively()) {
        emit operationCompleted("cleanupStagingForProject", QVariant(true), { {"projectId", projectId} });
        return true;
    }
    else {
        QString errorStr = QString("Failed to remove staging directory: %1").arg(stagingPath);
        emit operationFailed("cleanupStagingForProject", errorStr, { {"projectId", projectId} });
        return false;
    }
}


// --- 导出区接口实现 (export/) ---

QUrl FileManager::saveExportedVideoAsync(const QString& projectId, const QByteArray& videoData) {
    // This is the synchronous part, actual async would be in startExportVideoAsync
    QString exportPath = getExportVideoPath(projectId);
    ensureDir(QFileInfo(exportPath).absolutePath()); // Ensure export directory exists

    QFile file(exportPath);
    if (file.open(QIODevice::WriteOnly)) {
        // Simulate progress (in real async, this would be chunked)
        emit exportProgress(50, projectId);
        qint64 written = file.write(videoData);
        file.close();
        emit exportProgress(100, projectId);
        if (written == videoData.size()) {
            QUrl url = toFileUrl(exportPath);
            emit exportFinished(url, projectId);
            emit operationCompleted("saveExportedVideoAsync", QVariant(url), { {"projectId", projectId} });
            return url;
        }
        else {
            QString errorStr = QString("Failed to write complete exported video data. Wrote %1/%2 bytes.").arg(written).arg(videoData.size());
            emit exportError(errorStr, projectId);
            emit operationFailed("saveExportedVideoAsync", errorStr, { {"projectId", projectId} });
        }
    }
    else {
        QString errorStr = QString("Cannot open file for writing: %1").arg(exportPath);
        emit exportError(errorStr, projectId);
        emit operationFailed("saveExportedVideoAsync", errorStr, { {"projectId", projectId} });
    }
    return QUrl();
}

QUrl FileManager::saveExportedVideoToCustomPath(const QString& customAbsolutePath, const QByteArray& videoData) {
    // Validate custom path? Ensure parent dir exists?
    ensureDir(QFileInfo(customAbsolutePath).absolutePath());

    QFile file(customAbsolutePath);
    if (file.open(QIODevice::WriteOnly)) {
        qint64 written = file.write(videoData);
        file.close();
        if (written == videoData.size()) {
            QUrl url = QUrl::fromLocalFile(customAbsolutePath);
            emit operationCompleted("saveExportedVideoToCustomPath", QVariant(url), { {"customPath", customAbsolutePath} });
            return url;
        }
        else {
            QString errorStr = QString("Failed to write complete exported video data to custom path. Wrote %1/%2 bytes.").arg(written).arg(videoData.size());
            emit operationFailed("saveExportedVideoToCustomPath", errorStr, { {"customPath", customAbsolutePath} });
        }
    }
    else {
        QString errorStr = QString("Cannot open custom file for writing: %1").arg(customAbsolutePath);
        emit operationFailed("saveExportedVideoToCustomPath", errorStr, { {"customPath", customAbsolutePath} });
    }
    return QUrl();
}


// --- 缓存区接口实现 (cache/) ---

bool FileManager::cleanupThumbnailCache(const QString& /* projectId */) { // TODO: Implement per-project cleanup if needed
    // This example cleans all thumbnails. Modify logic if projectId-specific cleaning is required.
    QString cachePath = getCacheThumbnailBasePath();
    if (QDir(cachePath).removeRecursively()) {
        // Recreate the base cache dirs
        ensureDir(getCacheBasePath());
        ensureDir(cachePath);
        emit operationCompleted("cleanupThumbnailCache", QVariant(true), {});
        return true;
    }
    else {
        QString errorStr = QString("Failed to remove thumbnail cache directory: %1").arg(cachePath);
        emit operationFailed("cleanupThumbnailCache", errorStr, {});
        return false;
    }
}

void FileManager::cleanupExpiredThumbnails(int expirationDays) {
    QString cachePath = getCacheThumbnailBasePath();
    QDir cacheDir(cachePath);
    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-expirationDays);

    QStringList subDirs = cacheDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& subDirName : subDirs) {
        QDir subDir(cacheDir.filePath(subDirName));
        QStringList files = subDir.entryList(QDir::Files);
        for (const QString& fileName : files) {
            QFileInfo fileInfo(subDir.filePath(fileName));
            if (fileInfo.lastModified() < cutoffDate) {
                if (QFile::remove(fileInfo.absoluteFilePath())) {
                    qDebug() << "Removed expired thumbnail:" << fileInfo.absoluteFilePath();
                }
                else {
                    qWarning() << "Failed to remove expired thumbnail:" << fileInfo.absoluteFilePath();
                }
            }
        }
    }
    emit operationCompleted("cleanupExpiredThumbnails", QVariant(), { {"expirationDays", expirationDays} });
}

void FileManager::cleanupAppCache() {
    QString appCachePath = getAppCacheBasePath();
    if (QDir(appCachePath).removeRecursively()) {
        // Recreate the base cache dirs
        ensureDir(getCacheBasePath());
        ensureDir(appCachePath);
        emit operationCompleted("cleanupAppCache", QVariant(), {});
    }
    else {
        QString errorStr = QString("Failed to remove app cache directory: %1").arg(appCachePath);
        emit operationFailed("cleanupAppCache", errorStr, {});
    }
}


// --- 错误检测与恢复接口实现 ---

QVariantMap FileManager::validateProjectIntegrity(const QString& projectId) {
    // This could be a wrapper around verifyProjectFilesAgainstManifest or do more checks
    emit integrityCheckStarted(projectId);
    QVariantMap report = verifyProjectFilesAgainstManifest(projectId);
    emit integrityCheckFinished(projectId, report);
    return report;
}

bool FileManager::attemptProjectRecovery(const QString& projectId) {
    // Placeholder for complex recovery logic
    // 1. Check if project root exists
    // 2. Try to rebuild metadata from existing files
    // 3. Report findings via signals
    bool success = rebuildMetadataJson(projectId); // Example call
    if (success) {
        emit operationCompleted("attemptProjectRecovery", QVariant(true), { {"projectId", projectId} });
    }
    else {
        emit operationFailed("attemptProjectRecovery", "Recovery attempt failed or was incomplete.", { {"projectId", projectId} });
    }
    return success;
}

QVariantMap FileManager::getProjectHealthReport(const QString& projectId) {
    QJsonObject snapshot = loadProjectHealthSnapshot(projectId);
    if (!snapshot.isEmpty()) {
        QVariantMap map = snapshot.toVariantMap(); // Simple conversion
        emit operationCompleted("getProjectHealthReport", QVariant(map), { {"projectId", projectId} });
        return map;
    }
    else {
        // loadProjectHealthSnapshot will emit operationFailed
        return QVariantMap({ {"error", "Could not load health snapshot"} });
    }
}

bool FileManager::validateFileIntegrity(const QUrl& fileUrl, const QString& expectedChecksum) {
    QString localPath = fileUrl.toLocalFile();
    if (localPath.isEmpty() || !fileExists(localPath)) {
        return false;
    }

    QString calculatedChecksum = calculateChecksum(localPath);
    if (calculatedChecksum.isEmpty()) {
        return false; // Error during checksum calc
    }

    if (expectedChecksum.isEmpty()) {
        // If no expected checksum, validation passes if file exists and checksum is calculable
        return true;
    }
    else {
        return (calculatedChecksum == expectedChecksum);
    }
}

QVariantMap FileManager::validateShotStatus(const QString& projectId, const QString& shotId) {
    QJsonObject shotHealth = loadShotHealthSnapshot(projectId, shotId);
    if (shotHealth.isEmpty()) {
        // loadShotHealthSnapshot emits operationFailed
        return QVariantMap({ {"error", "Could not load shot health snapshot"} });
    }

    QVariantMap report;
    report["shotId"] = shotId;
    QVariantList inconsistencies;

    // Check image consistency
    QJsonObject imageStatus = shotHealth["fileStatus"].toObject()["image"].toObject();
    bool expectedImageExists = imageStatus["exists"].toBool(false);
    QString imagePath = getShotImagePath(projectId, shotId);
    bool actualImageExists = fileExists(imagePath);

    if (expectedImageExists != actualImageExists) {
        inconsistencies.append(QString("Image existence mismatch: Expected %1, Found %2").arg(expectedImageExists).arg(actualImageExists));
    }

    // Check audio consistency (similarly)
    QJsonObject audioStatus = shotHealth["fileStatus"].toObject()["audio"].toObject();
    bool expectedAudioExists = audioStatus["exists"].toBool(false);
    QString audioPath = getShotAudioPath(projectId, shotId);
    bool actualAudioExists = fileExists(audioPath);

    if (expectedAudioExists != actualAudioExists) {
        inconsistencies.append(QString("Audio existence mismatch: Expected %1, Found %2").arg(expectedAudioExists).arg(actualAudioExists));
    }

    // Check active drafts vs actual staging files (if needed)

    report["isConsistent"] = inconsistencies.isEmpty();
    report["inconsistencies"] = inconsistencies;
    report["checkedAt"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    emit operationCompleted("validateShotStatus", QVariant(report), { {"projectId", projectId}, {"shotId", shotId} });
    return report;
}

bool FileManager::repairShotStatus(const QString& projectId, const QString& shotId) {
    // Load current snapshot
    QJsonObject shotHealth = loadShotHealthSnapshot(projectId, shotId);
    if (shotHealth.isEmpty()) {
        // loadShotHealthSnapshot emits operationFailed
        return false;
    }

    // Get actual file states
    QString imagePath = getShotImagePath(projectId, shotId);
    QString audioPath = getShotAudioPath(projectId, shotId);
    bool imageExists = fileExists(imagePath);
    bool audioExists = fileExists(audioPath);

    // Update the snapshot based on reality
    QJsonObject fileStatus = shotHealth["fileStatus"].toObject();
    QJsonObject imageStatus = fileStatus["image"].toObject();
    QJsonObject audioStatus = fileStatus["audio"].toObject();

    imageStatus["exists"] = imageExists;
    audioStatus["exists"] = audioExists;

    // Update timestamps/sizes/checksums if file exists
    if (imageExists) {
        QFileInfo imgInfo(imagePath);
        imageStatus["lastModified"] = imgInfo.lastModified().toString(Qt::ISODate);
        imageStatus["size"] = static_cast<qint64>(imgInfo.size());
        imageStatus["checksum"] = calculateChecksum(imagePath); // Could be slow
    }
    else {
        imageStatus["lastModified"] = QJsonValue(); // null
        imageStatus["size"] = QJsonValue(); // null
        imageStatus["checksum"] = QJsonValue(); // null
    }

    if (audioExists) {
        QFileInfo audioInfo(audioPath);
        audioStatus["lastModified"] = audioInfo.lastModified().toString(Qt::ISODate);
        audioStatus["size"] = static_cast<qint64>(audioInfo.size());
        audioStatus["checksum"] = calculateChecksum(audioPath); // Could be slow
    }
    else {
        audioStatus["lastModified"] = QJsonValue(); // null
        audioStatus["size"] = QJsonValue(); // null
        audioStatus["checksum"] = QJsonValue(); // null
    }

    fileStatus["image"] = imageStatus;
    fileStatus["audio"] = audioStatus;
    shotHealth["fileStatus"] = fileStatus;

    // Clear active drafts if they were committed (optional refinement)
    // For simplicity here, we just clear them if main files exist
    QJsonObject activeDrafts = shotHealth["activeDrafts"].toObject();
    if (imageExists) activeDrafts["image"] = QJsonValue(); // null
    if (audioExists) activeDrafts["audio"] = QJsonValue(); // null
    shotHealth["activeDrafts"] = activeDrafts;

    // Save updated snapshot
    if (!saveShotHealthSnapshot(projectId, shotId, shotHealth).isValid()) {
        // saveShotHealthSnapshot emits operationFailed
        return false;
    }

    emit operationCompleted("repairShotStatus", QVariant(true), { {"projectId", projectId}, {"shotId", shotId} });
    return true;
}


// --- 通用工具接口实现 ---

bool FileManager::fileExists(const QUrl& fileUrl) const {
    return fileExists(fileUrl.toString()); // Delegate to internal helper
}

QStringList FileManager::listShotIds(const QString& projectId) const {
    QString shotsDirPath = QDir(getProjectRootPath(projectId)).filePath(SHOTS_DIR_NAME);
    QDir shotsDir(shotsDirPath);
    if (shotsDir.exists()) {
        return shotsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    }
    return QStringList(); // Return empty list if shots dir doesn't exist
}

QStringList FileManager::listProjIds() const {
    QDir projectsDir(getProjectsBasePath());
    if (projectsDir.exists()) {
        return projectsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    }
    return QStringList(); // Return empty list if projects dir doesn't exist
}

void FileManager::cleanupTempFiles() {
    // Cleanup global temp
    QString globalTempPath = getTempBasePath();
    QDateTime cutoffDate = QDateTime::currentDateTime().addSecs(-24 * 3600); // 24 hours ago
    QDir globalTempDir(globalTempPath);
    if (globalTempDir.exists()) {
        QStringList tempFiles = globalTempDir.entryList(QDir::Files);
        for (const QString& fileName : tempFiles) {
            QFileInfo fileInfo(globalTempDir.filePath(fileName));
            if (fileInfo.lastModified() < cutoffDate) {
                if (QFile::remove(fileInfo.absoluteFilePath())) {
                    qDebug() << "Removed old global temp file:" << fileInfo.absoluteFilePath();
                }
                else {
                    qWarning() << "Failed to remove old global temp file:" << fileInfo.absoluteFilePath();
                }
            }
        }
    }

    // Cleanup per-project staging temp (assuming files older than 24h in any staging dir are temp)
    QStringList projectIds = listProjIds();
    for (const QString& projectId : projectIds) {
        QString projectStagingPath = getProjectStagingBasePath(projectId);
        QDir projectStagingDir(projectStagingPath);
        if (projectStagingDir.exists()) {
            QStringList stagingEntries = projectStagingDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QString& entry : stagingEntries) {
                QFileInfo entryInfo(projectStagingDir.filePath(entry));
                if (entryInfo.lastModified() < cutoffDate) {
                    if (entryInfo.isDir()) {
                        if (QDir(entryInfo.absoluteFilePath()).removeRecursively()) {
                            qDebug() << "Removed old project staging dir:" << entryInfo.absoluteFilePath();
                        }
                        else {
                            qWarning() << "Failed to remove old project staging dir:" << entryInfo.absoluteFilePath();
                        }
                    }
                    else { // File
                        if (QFile::remove(entryInfo.absoluteFilePath())) {
                            qDebug() << "Removed old project staging file:" << entryInfo.absoluteFilePath();
                        }
                        else {
                            qWarning() << "Failed to remove old project staging file:" << entryInfo.absoluteFilePath();
                        }
                    }
                }
            }
        }
    }
    emit operationCompleted("cleanupTempFiles", QVariant(), {});
}


// --- 公共槽函数实现 ---

void FileManager::startExportVideoAsync(const QString& projectId, const QByteArray& videoData) {
    // In a real async implementation, you'd launch this in a separate thread
    // For now, just call the sync version and pretend it's async
    qDebug() << "Starting async export for project:" << projectId; // Log or signal start
    saveExportedVideoAsync(projectId, videoData); // Synchronous call for demo
    // Real async would connect signals/slots or use futures/promises
}

void FileManager::cancelExportOperation(const QString& /*projectId*/) {
    // Implementation depends on how async operations are managed
    // For now, just log
    qWarning() << "Cancel export operation requested. Not implemented in this draft.";
}

void FileManager::startIntegrityScan(const QString& projectId) {
    // Wrapper for async scan
    qDebug() << "Starting integrity scan for project:" << projectId;
    validateProjectIntegrity(projectId); // Synchronous call for demo
    // Real async would run verifyProjectFilesAgainstManifest in background
}


// --- 内部恢复策略实现 (示例) ---

bool FileManager::rebuildMetadataJson(const QString& projectId) {
    // Very basic rebuild example - only checks for existing shot dirs and thumbnail
    QString projectPath = getProjectRootPath(projectId);
    if (!QDir(projectPath).exists()) {
        qWarning() << "Project directory does not exist for rebuild:" << projectPath;
        return false;
    }

    QJsonObject newHealth;
    newHealth["version"] = "1.0";
    newHealth["id"] = projectId;
    newHealth["name"] = "Recovered Project (" + projectId + ")";
    newHealth["createTime"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    newHealth["lastChecked"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    newHealth["updateTime"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    newHealth["rawStory"] = "";
    newHealth["style"] = "";

    QJsonObject expectedFiles;
    expectedFiles["thumbnail"] = PROJECT_THUMBNAIL_FILE_NAME;

    QJsonArray shotsArray;
    QStringList shotIds = listShotIds(projectId);
    for (const QString& shotId : shotIds) {
        QJsonObject shotEntry;
        shotEntry["shotId"] = shotId;
        QString imagePath = QDir(SHOTS_DIR_NAME + "/" + shotId).filePath(SHOT_IMAGE_FILE_NAME);
        QString audioPath = QDir(SHOTS_DIR_NAME + "/" + shotId).filePath(SHOT_AUDIO_FILE_NAME);

        // Check if files actually exist before listing them
        if (fileExists(QDir(projectPath).filePath(imagePath))) {
            shotEntry["image"] = imagePath;
        }
        else {
            shotEntry["image"] = QJsonValue(); // null
        }

        if (fileExists(QDir(projectPath).filePath(audioPath))) {
            shotEntry["audio"] = audioPath;
        }
        else {
            shotEntry["audio"] = QJsonValue(); // null
        }
        shotsArray.append(shotEntry);
    }
    expectedFiles["shots"] = shotsArray;

    QString exportPath = QDir(EXPORT_DIR_NAME).filePath("final_video.mp4");
    if (fileExists(QDir(projectPath).filePath(exportPath))) {
        expectedFiles["export"] = exportPath;
    }
    else {
        expectedFiles["export"] = QJsonValue(); // null
    }

    newHealth["expectedFiles"] = expectedFiles;

    // Leave integrity mostly empty or try to populate checksums?
    newHealth["integrity"] = QJsonObject();
    newHealth["healthStatus"] = QJsonObject(); // Leave empty or populate?

    QUrl resultUrl = saveProjectHealthSnapshot(projectId, newHealth);
    return resultUrl.isValid();
}

QString FileManager::findDraftForMissingFile(const QString& /*projectId*/, const QString& /*missingFileType*/, const QString& /*shotId*/) {
    // Placeholder - logic would search staging area
    return QString();
}

QVariantMap FileManager::findOrphanedFiles(const QString& /*projectId*/) {
    // Placeholder - logic would compare filesystem with expectedFiles manifest
    return QVariantMap();
}

bool FileManager::cleanupOrphanedFiles(const QString& /*projectId*/) {
    // Placeholder - logic would remove files found by findOrphanedFiles
    return false;
}

QVariantList FileManager::getMissingFilesList(const QJsonObject& /*manifest*/) {
    // Placeholder - logic would extract missing files from manifest/integrity check
    return QVariantList();
}

bool FileManager::canCancelCurrentOperation(const QString& /*projectId*/) {
    // Placeholder - depends on async op management
    return false;
}

void FileManager::performCancelOperation(const QString& /*projectId*/) {
    // Placeholder - depends on async op management
}