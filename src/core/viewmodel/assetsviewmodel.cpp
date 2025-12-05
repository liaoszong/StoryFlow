#include "AssetsViewModel.h"
#include "FileManager/FileManager.h"
#include <QDebug>
#include <QUrl>

AssetsViewModel::AssetsViewModel(FileManager *fileManager, QObject *parent)
    : QObject(parent), m_fileManager(fileManager), m_isLoading(false)
{
    m_fullList = QVariantList();
    m_projectList = QVariantList();
    m_scanPath = QString();
}

void AssetsViewModel::setIsLoading(bool status) {
    if (m_isLoading == status) return;
    m_isLoading = status;
    emit isLoadingChanged();
}

void AssetsViewModel::setProjectList(const QVariantList &list) {
    m_projectList = list;
    emit projectListChanged();
}

void AssetsViewModel::setScanPath(const QString &path) {
    QString localPath = path;
    // 处理 file:// URL
    if (localPath.startsWith("file:///")) {
        localPath = QUrl(localPath).toLocalFile();
    }
    
    if (m_scanPath == localPath) return;
    m_scanPath = localPath;
    emit scanPathChanged();
    
    qDebug() << "AssetsViewModel: scanPath changed to:" << m_scanPath;
}

void AssetsViewModel::loadAssets()
{
    if (m_scanPath.isEmpty()) {
        qDebug() << "AssetsViewModel: scanPath is empty, cannot load assets";
        return;
    }
    
    scanProjects(m_scanPath);
}

void AssetsViewModel::scanProjects(const QString &directoryPath)
{
    if (m_isLoading) return;
    
    QString localPath = directoryPath;
    // 处理 file:// URL
    if (localPath.startsWith("file:///")) {
        localPath = QUrl(localPath).toLocalFile();
    }
    
    if (localPath.isEmpty()) {
        emit errorOccurred("请选择有效的文件夹路径");
        return;
    }
    
    setIsLoading(true);
    
    // 更新扫描路径
    if (m_scanPath != localPath) {
        m_scanPath = localPath;
        emit scanPathChanged();
    }
    
    qDebug() << "AssetsViewModel: Scanning projects in:" << localPath;
    
    // 使用 FileManager 扫描本地项目
    QVariantList projects = m_fileManager->scanLocalProjects(localPath);
    
    // 按更新时间排序（最新的在前）
    std::sort(projects.begin(), projects.end(), [](const QVariant &a, const QVariant &b) {
        QVariantMap mapA = a.toMap();
        QVariantMap mapB = b.toMap();
        QString timeA = mapA["updated_at"].toString();
        QString timeB = mapB["updated_at"].toString();
        return timeA > timeB;  // 降序
    });
    
    // 更新数据
    m_fullList = projects;
    setProjectList(projects);
    
    setIsLoading(false);
    
    qDebug() << "AssetsViewModel: Found" << projects.size() << "projects";
}

void AssetsViewModel::deleteProject(const QString &projectPath)
{
    QString localPath = projectPath;
    // 处理 file:// URL
    if (localPath.startsWith("file:///")) {
        localPath = QUrl(localPath).toLocalFile();
    }
    
    if (localPath.isEmpty()) {
        emit errorOccurred("无效的项目路径");
        return;
    }
    
    qDebug() << "AssetsViewModel: Deleting project at:" << localPath;
    
    bool success = m_fileManager->deleteLocalProject(localPath);
    
    if (success) {
        emit projectDeleted(localPath);
        // 重新加载列表
        loadAssets();
    } else {
        emit errorOccurred("删除项目失败");
    }
}

QVariantMap AssetsViewModel::loadProjectDetail(const QString &projectJsonPath)
{
    QString localPath = projectJsonPath;
    // 处理 file:// URL
    if (localPath.startsWith("file:///")) {
        localPath = QUrl(localPath).toLocalFile();
    }
    
    if (localPath.isEmpty()) {
        qWarning() << "AssetsViewModel: Invalid project JSON path";
        return QVariantMap();
    }
    
    return m_fileManager->loadProjectFromLocal(localPath);
}

// 前端本地过滤，零延迟
void AssetsViewModel::filterAssets(const QString &keyword)
{
    QString query = keyword.trimmed();

    // 1. 如果关键字为空，显示全量数据
    if (query.isEmpty()) {
        if (m_projectList.size() != m_fullList.size()) {
            setProjectList(m_fullList);
        }
        return;
    }

    // 2. 本地遍历筛选
    QVariantList filteredList;
    for (QVariantList::const_iterator it = m_fullList.constBegin(); it != m_fullList.constEnd(); ++it) {
        QVariantMap map = it->toMap();
        QString name = map["name"].toString();
        // 不区分大小写搜索
        if (name.contains(query, Qt::CaseInsensitive)) {
            filteredList.append(*it);
        }
    }

    // 3. 更新 UI
    setProjectList(filteredList);
    qDebug() << "AssetsViewModel: Local filter:" << query << "Found:" << filteredList.size();
}
