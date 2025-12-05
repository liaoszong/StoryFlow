#ifndef ASSETSVIEWMODEL_H
#define ASSETSVIEWMODEL_H

#include <QObject>
#include <QVariantList>
#include "core/net/ApiService.h"

class AssetsViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(QVariantList projectList READ projectList NOTIFY projectListChanged)

public:
    explicit AssetsViewModel(ApiService *apiService, QObject *parent = nullptr);

    Q_INVOKABLE void loadAssets();
    Q_INVOKABLE void filterAssets(const QString &keyword);
    Q_INVOKABLE void deleteProject(const QString &projectId);
    Q_INVOKABLE void setUserId(const QString &userId);

    bool isLoading() const { return m_isLoading; }
    QVariantList projectList() const { return m_projectList; }
    QString userId() const { return m_userId; }

signals:
    void isLoadingChanged();
    void projectListChanged();
    void errorOccurred(QString message);

private:
    ApiService *m_apiService;
    bool m_isLoading;
    QString m_userId;  // 用户 ID

    // 数据源分离
    QVariantList m_fullList;     // 数据仓库：保存从服务器拉回来的所有数据
    QVariantList m_projectList;  // 视图数据：UI 真正显示的数据（可能是过滤过的）

    void setIsLoading(bool status);
    void setProjectList(const QVariantList &list); // 更新 UI
};

#endif
