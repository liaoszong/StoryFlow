// FileManager.h
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QUrl>
#include <QJsonObject>
#include <QVariantMap>
#include <QStringList>

class FileManager : public QObject
{
    Q_OBJECT

public:

    // 构造例子：FileManager* mgr = new FileManager(nullptr, "D:/MyDocs/StoryToVideo");
    // parent父对象，customBasePath为可选参数，用于设置自定义的基础路径，默认为用户文档下的特定子目录
    // 默认目录比如C:\Users\LIU\AppData\Local\Temp\StoryToVideo_TestEnv\Users\user_default\projects
    explicit FileManager(QObject* parent = nullptr, const QString& customBasePath = QString(), const QString& userId = "");



    //在运行时切换基础路径，目前仅用于测试环境，暂不支持实际操作
    //void setBaseAppPath(const QString& customBasePath); 
    QString getUserBasePath() const; // 获取用户级基础路径(测试用
    QString getProjectsBasePath() const; // 获取所有项目基础路径(测试用


    // ——————————
    //【用户级】
    //———————————
    // 设置当前用户ID，并创建相应的用户目录结构
    void setCurrentUserId(const QString& userId);
    bool initUserDirectories();



    // ─────────────────────
    // 【项目级】
    // ─────────────────────

    // 保存项目封面缩略图，返回 file:// URI
    // thumbnailData 应为 JPEG/JPG等 格式的二进制数据
    QUrl saveProjectThumbnail(
        const QString& projectId,
        const QByteArray& thumbnailData
    );

    // 读取封面图
    // 如果失败返回空 QByteArray
    QByteArray loadProjectThumbnail(const QString& projectId);

    // 保存项目健康状态快照，返回 file:// URI
    // 如果 metadata 包含完整的 ProjectState 子集则覆盖保存，否则仅更新指定字段
    QUrl saveProjectHealthSnapshot(
        const QString& projectId,
        const QJsonObject& metadata // 对应 ProjectState 子集
    );

    // 读取项目快照
    QJsonObject loadProjectHealthSnapshot(const QString& projectId);

    QString getProjectStagingTempPath(const QString& projectId) const; // 项目临时文件目录

    // 删除整个项目（异步安全，含 shots/staging/export）
    // 如果成功删除就返回 true
    bool deleteProject(const QString& projectId);

    // 更新项目状态快照中的完整性校验数据
    // 成功就返回 true
    bool updateIntegrityData(
        const QString& projectId,
        const QString& filePath,
        const QString& checksum
    );

    // 根据预期文件清单验证项目完整性，未作测试
    QVariantMap verifyProjectFilesAgainstManifest(const QString& projectId);


    // ───────────────────────────────
    // 【分镜级】（正式区：shots/）
    // ───────────────────────────────

    // 保存图像，返回 file:// URI,以下所有保存函数均类似返回
    // 失败时会返回无效的 QUrl，即 url.isValid() == false
    QUrl saveShotImage(
        const QString& projectId,
        const QString& shotId,
        const QByteArray& imageData
    );

    // 读取分镜图像
    QByteArray loadShotImage(const QString& projectId, const QString& shotId);

    // 保存音频
    // audioData 应为 MP3 等格式的二进制数据
    QUrl saveShotAudio(
        const QString& projectId,
        const QString& shotId,
        const QByteArray& audioData
    );

    // 保存分镜状态快照
    QUrl saveShotHealthSnapshot(
        const QString& projectId,
        const QString& shotId,
        const QJsonObject& statusInfo
    );

    // 加载分镜快照
    QJsonObject loadShotHealthSnapshot(
        const QString& projectId,
        const QString& shotId
    );


    // ───────────────────────────────────────
    // 【临时生成区】staging/ 管理（比如AI 生成时）
    // ───────────────────────────────────────

    // 保存图像草稿 返回 file:// URI
    // 把AI生成的图像数据先存到 staging 区，待确认后再移到正式区
    QUrl stageImage(
        const QString& projectId,
        const QString& shotId,
        const QByteArray& imageData
    );

    // 保存音频草稿
    QUrl stageAudio(
        const QString& projectId,
        const QString& shotId,
        const QByteArray& audioData
    );

    // 将 staging 草稿提交为正式资产（移动文件 + 清理 staging）
    /// mediaType: "image" | "audio"
    /// 如果成功移动并清理原来staging区对应文件，就返回 true
    bool commitStagedMedia(
        const QString& projectId,
        const QString& shotId,
        const QString& mediaType
    );

    // 清理整个单个项目 staging 区（如用户取消生成）
    bool cleanupStagingForProject(const QString& projectId);


    // ──────────────────────────
    // 【导出区】export/ 管理
    // ──────────────────────────

    // 保存导出视频到项目默认路径 export目录中，返回 file:// URI
    QUrl saveExportedVideoAsync(
        const QString& projectId,
        const QByteArray& videoData
    );

    // 保存导出视频到自定义路径（注：此路径可能不在项目目录内）
    // 要提供一个完整的文件路径，包括文件名和扩展名
    QUrl saveExportedVideoToCustomPath(
        const QString& customAbsolutePath,
        const QByteArray& videoData
    );


    // ──────────────────────────
    // 【缓存区】cache/ 管理
    // ──────────────────────────

    bool saveCacheThumbnail(const QString& id, const QImage& thumbnail) const;

    // 清理指定项目的缩略图缓存
    bool cleanupThumbnailCache(const QString& projectId);

    // 清理所有过期的缩略图缓存（根据时间戳）
    void cleanupExpiredThumbnails(int expirationDays);

    // 清理应用级缓存（例如AI模型、公共资源等）
    void cleanupAppCache();


    // ──────────────────────────
    // 【错误检测与恢复】
    // ──────────────────────────
    QVariantMap validateProjectIntegrity(const QString& projectId);         // 检查项目完整性
    bool attemptProjectRecovery(const QString& projectId);                  // 尝试自动修复项目
    QVariantMap getProjectHealthReport(const QString& projectId);           // 获取项目健康状态报告
    bool validateFileIntegrity(const QUrl& fileUrl, const QString& expectedChecksum = ""); // 验证单个文件完整性
    QVariantMap validateShotStatus(const QString& projectId, const QString& shotId);       // 检测单个分镜的状态一致性
    bool repairShotStatus(const QString& projectId, const QString& shotId);         // 修复单个分镜的状态


    // ─────────────────
    // 【通用工具】
    // ─────────────────

    // 检查 file:// URI 对应的本地文件是否存在
    bool fileExists(const QUrl& fileUrl) const;

    // 列出某项目下所有分镜 ID（扫描 shots/ 目录）
    QStringList listShotIds(const QString& projectId) const;

    // 列出所有项目 ID（扫描 projects/ 目录）
    QStringList listProjIds() const;

    // 清理临时文件（>24h 的 cache/temp/ 和 projects/*/staging/）
    void cleanupTempFiles();


signals:

    // 通用操作结果
    void operationCompleted(
        const QString& operation,   // 操作名，如 "saveShotImage"
        const QVariant& result,     // 操作结果，如 保存成功后的QUrl
        const QVariantMap& context  // 操作上下文，如 {"projectId": "proj_123", "shotId": "shot_1"}
    )const;

    void operationFailed(
        const QString& operation,
        const QString& error,
        const QVariantMap& context) const;

    // 文件操作进度（用于大文件导出）
    void exportProgress(int percent, const QString& projectId) const;
    void exportFinished(const QUrl& videoUrl, const QString& projectId) const;
    void exportError(const QString& error, const QString& projectId) const;

    // 健康监控信号
    void projectHealthChanged(const QString& projectId, const QString& status, const QVariantMap& report) const;
    void repairNeeded(const QString& projectId, const QVariantList& issues) const;
    void integrityCheckStarted(const QString& projectId) const;
    void integrityCheckFinished(const QString& projectId, const QVariantMap& report) const;


public slots:



    // 异步导出（避免阻塞 UI）
    void startExportVideoAsync(const QString& projectId, const QByteArray& videoData);
    // 取消正在进行的导出操作
    void cancelExportOperation(const QString& projectId);
    // 弥补文档中遗漏的异步完整性扫描槽函数
    void startIntegrityScan(const QString& projectId);


private:

    QString m_baseAppPath; // 应用基础路径（可定制）
    QString m_currentUserHash; // 存储当前用户的哈希值

    // 辅助函数和内部实现
    bool fileExists(const QString& filePathOrUrl) const; // 重载，内部调用


    bool ensureDir(const QString& dirPath) const; // 确保目录存在，否则创建，使用 QDir/QFileInfo
    //QString getUserBasePath() const; // 获取用户目录基础路径
    QString hashString(const QString& input) const; // 哈希工具函数 (SHA256)
    QString getCacheBasePath() const; // 获取缓存基路径
    QString getCacheThumbnailPath(const QString& id) const; // 修改：支持哈希前缀子目录
    QString getCacheThumbnailFilePath(const QString& id) const;
    QString getAppCachePath() const;
    QString getAppCacheModelsPath() const; // 新增：应用模型缓存路径
    QString getAppCacheResourcesPath() const; // 新增：应用资源缓存路径

    QString getProjectRootPath(const QString& projectId) const; // 获取项目根目录路径（本地绝对路径，仅用于内部计算）
    QString getShotRootPath(const QString& projectId, const QString& shotId) const; // 获取分镜根目录路径
    QString getShotImagePath(const QString& projectId, const QString& shotId) const; // 获取图像文件路径（本地绝对路径）
    QString getShotAudioPath(const QString& projectId, const QString& shotId) const; // 获取音频文件路径
    QString getShotHealthSnapshotPath(const QString& projectId, const QString& shotId) const; // 获取分镜快照路径
    QString getProjectHealthSnapshotPath(const QString& projectId) const; // 获取项目快照路径
    QString getProjectThumbnailPath(const QString& projectId) const; // 获取项目缩略图路径
    QString getStagingImagePath(const QString& projectId, const QString& shotId) const; // 获取图像草稿路径
    QString getStagingAudioPath(const QString& projectId, const QString& shotId) const; // 获取音频草稿路径
    QString getExportVideoPath(const QString& projectId) const; // 获取默认导出视频路径
    QString getCacheThumbnailBasePath() const; // 获取缩略图缓存基路径
    QString getAppCacheBasePath() const; // 获取应用缓存基路径
    QString getTempBasePath() const; // 获取全局临时文件基路径
    QString getProjectStagingBasePath(const QString& projectId) const; // 获取项目staging基路径
    QString getProjectExportBasePath(const QString& projectId) const; // 获取项目export基路径
    QUrl toFileUrl(const QString& localPath) const; // 将本地绝对路径字符串转为 file://

    // 恢复策略内部实现
    bool rebuildMetadataJson(const QString& projectId);
    QString findDraftForMissingFile(const QString& projectId, const QString& missingFileType, const QString& shotId = "");
    QVariantMap findOrphanedFiles(const QString& projectId); // 查找孤儿文件
    bool cleanupOrphanedFiles(const QString& projectId); // 清理孤儿文件
    QVariantList getMissingFilesList(const QJsonObject& manifest);
    QString calculateChecksum(const QString& filePath);

    // 异步操作控制
    bool canCancelCurrentOperation(const QString& projectId);
    void performCancelOperation(const QString& projectId);

    // 移动文件的通用函数
    bool moveFile(const QString& sourcePath, const QString& destinationPath);
};

#endif // FILEMANAGER_H
