#ifndef ASSETSVIEWMODEL_H
#define ASSETSVIEWMODEL_H

#include <QObject>
#include <QVariantList>

class FileManager;

class AssetsViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(QVariantList projectList READ projectList NOTIFY projectListChanged)
    Q_PROPERTY(QString scanPath READ scanPath WRITE setScanPath NOTIFY scanPathChanged)

public:
    explicit AssetsViewModel(FileManager *fileManager, QObject *parent = nullptr);

    // 加载本地项目列表（使用当前 scanPath）
    Q_INVOKABLE void loadAssets();

    // 从指定路径扫描项目
    Q_INVOKABLE void scanProjects(const QString &directoryPath);

    // 本地过滤
    Q_INVOKABLE void filterAssets(const QString &keyword);

    // 删除本地项目
    Q_INVOKABLE void deleteProject(const QString &projectPath);

    // 加载单个项目详情
    Q_INVOKABLE QVariantMap loadProjectDetail(const QString &projectJsonPath);

    bool isLoading() const { return m_isLoading; }
    QVariantList projectList() const { return m_projectList; }
    QString scanPath() const { return m_scanPath; }
    void setScanPath(const QString &path);

signals:
    void isLoadingChanged();
    void projectListChanged();
    void scanPathChanged();
    void errorOccurred(QString message);
    void projectDeleted(QString projectPath);

private:
    FileManager *m_fileManager;
    bool m_isLoading;
    QString m_scanPath;  // 扫描路径

    // 数据源分离
    QVariantList m_fullList;     // 数据仓库：保存扫描到的所有项目
    QVariantList m_projectList;  // 视图数据：UI 真正显示的数据（可能是过滤过的）

    void setIsLoading(bool status);
    void setProjectList(const QVariantList &list);
};

#endif
