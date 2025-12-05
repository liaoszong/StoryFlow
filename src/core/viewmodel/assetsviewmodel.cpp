#include "AssetsViewModel.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QRandomGenerator>

// API 端点常量
namespace AssetsApi {
    const QString ENDPOINT_PROJECT_LIST = "/api/client/project/list";
    const QString ENDPOINT_PROJECT_DELETE = "/api/client/project/delete";
}

AssetsViewModel::AssetsViewModel(ApiService *apiService, QObject *parent)
    : QObject(parent), m_apiService(apiService), m_isLoading(false)
{
    m_fullList = QVariantList();
    m_projectList = QVariantList();
    // 生成一个临时的 user_id (实际应该从登录状态获取)
    m_userId = "user_" + QString::number(QRandomGenerator::global()->generate());
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
    payload["project_id"] = projectId;
    payload["user_id"] = m_userId;

    m_apiService->post(AssetsApi::ENDPOINT_PROJECT_DELETE, payload,
                       [this](bool success, const QJsonObject &resp) {
                           if (!success) {
                               emit errorOccurred("网络错误，删除失败");
                               return;
                           }

                           // 检查业务返回 code == 0 表示成功
                           if (resp["code"].toInt() == 0) {
                               // 删除成功后，重新拉取列表
                               loadAssets();
                           } else {
                               QString msg = resp["msg"].toString();
                               emit errorOccurred(msg.isEmpty() ? "删除失败" : msg);
                           }
                       });
}

void AssetsViewModel::loadAssets()
{
    if (m_isLoading) return;
    setIsLoading(true);

    // 构建查询参数
    QJsonObject params;
    params["user_id"] = m_userId;

    m_apiService->get(AssetsApi::ENDPOINT_PROJECT_LIST, params,
                      [this](bool success, const QJsonObject &resp) {
                          setIsLoading(false);

                          if (!success) {
                              emit errorOccurred("加载资产列表失败");
                              return;
                          }

                          // 检查业务返回 code == 0 表示成功
                          if (resp["code"].toInt() != 0) {
                              QString msg = resp["msg"].toString();
                              emit errorOccurred(msg.isEmpty() ? "加载失败" : msg);
                              return;
                          }

                          QJsonArray arr;
                          QJsonObject data = resp["data"].toObject();

                          // 从 data.projects 或 data 直接获取项目列表
                          if (data.contains("projects") && data["projects"].isArray()) {
                              arr = data["projects"].toArray();
                          } else if (resp["data"].isArray()) {
                              arr = resp["data"].toArray();
                          }

                          QVariantList newList;
                          for (auto it = arr.constBegin(); it != arr.constEnd(); ++it) {
                              if (it->isObject()) {
                                  newList.append(it->toObject().toVariantMap());
                              }
                          }

                          // 同时更新"仓库"和"视图"
                          m_fullList = newList;
                          setProjectList(newList);

                          qDebug() << "Assets loaded:" << newList.size();
                      });
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
    qDebug() << "Local filter:" << query << "Found:" << filteredList.size();
}

void AssetsViewModel::setUserId(const QString &userId)
{
    m_userId = userId;
}
