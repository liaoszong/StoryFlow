#include "AssetsViewModel.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>

AssetsViewModel::AssetsViewModel(ApiService *apiService, QObject *parent)
    : QObject(parent), m_apiService(apiService), m_isLoading(false)
{
    m_fullList = QVariantList();
    m_projectList = QVariantList();
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

void AssetsViewModel::deleteProject(const QString &projectId)
{
    QJsonObject payload;
    payload["id"] = projectId;

    // 乐观更新：先在本地 UI 删掉，让用户感觉很快，然后再发请求
    // 采用发请求成功后刷新列表的方式

    m_apiService->post("/api/assets/delete", payload,
                       [this](bool success, const QJsonObject &resp) {
                           if (success && resp["type"].toString() == "ACTION_TYPE_SUCCESS") {
                               // 删除成功后，重新拉取列表
                               loadAssets();
                           } else {
                               emit errorOccurred("删除失败");
                           }
                       });
}

void AssetsViewModel::loadAssets()
{
    if (m_isLoading) return;
    setIsLoading(true);

    m_apiService->get("/api/assets/list", QJsonObject(),
                      [this](bool success, const QJsonObject &resp) {
                          setIsLoading(false);

                          if (success) {
                              QJsonArray arr;
                              // 兼容多种后端格式
                              if (resp.contains("assets")) {
                                  arr = resp["assets"].toArray();
                              } else if (resp.contains("payload") && resp["payload"].isArray()) {
                                  arr = resp["payload"].toArray();
                              }

                              QVariantList newList;
                              // **使用迭代器优化遍历
                              for (auto it = arr.constBegin(); it != arr.constEnd(); ++it) {
                                  if (it->isObject()) {
                                      newList.append(it->toObject().toVariantMap());
                                  }
                              }

                              // *同时更新“仓库”和“视图”
                              m_fullList = newList;
                              setProjectList(newList); // 初始状态显示全部

                              qDebug() << "Assets loaded:" << newList.size();
                          } else {
                              // 如果失败，不要清空旧数据，除非你想显示空状态
                              emit errorOccurred("加载资产列表失败");
                          }
                      });
}

// *前端本地过滤，零延迟
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
    // 使用显式 const 迭代器，避免深拷贝/Detach
    // 遍历 m_fullList (数据仓库)
    for (QVariantList::const_iterator it = m_fullList.constBegin(); it != m_fullList.constEnd(); ++it) {
        // 解引用迭代器获取当前项
        QVariantMap map = it->toMap();
        QString name = map["name"].toString();
        // 不区分大小写搜索
        if (name.contains(query, Qt::CaseInsensitive)) {
            filteredList.append(*it); // 注意：这里直接 append *it (QVariant)
        }
    }

    // 3. 更新 UI
    setProjectList(filteredList);
    qDebug() << "Local filter:" << query << "Found:" << filteredList.size();
}
