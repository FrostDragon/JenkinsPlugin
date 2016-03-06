#include "jenkinsjobsmodel.h"

using namespace JenkinsPlugin::Internal;

static JenkinsJobsModel *_instance = nullptr;

JenkinsJobsModel::JenkinsJobsModel() { updateHeader(); }

JenkinsJobsModel *JenkinsJobsModel::instance()
{
    if (_instance == nullptr)
        _instance = new JenkinsJobsModel(nullptr);
    return _instance;
}

void JenkinsJobsModel::updateHeader()
{
    _rootItem->setName(QString(QStringLiteral("Jenkins [%1]")).arg(_jenkinsSettings.jenkinsUrl()));
    emit dataChanged(QAbstractItemModel::createIndex(0, 0, _rootItem->parent()),
                     QAbstractItemModel::createIndex(0, 0, _rootItem->parent()),
                     QVector< int >() << Qt::DisplayRole);
}

JenkinsJobsModel::JenkinsJobsModel(QObject *parent)
    : TreeModel(parent), _rootItem(new JenkinsTreeItem(tr("Jenkins"), JenkinsTreeItem::Type::Root))
{
    rootItem()->appendChild(_rootItem);
}

void JenkinsJobsModel::resetJobs(QList< JenkinsJob > newJobs)
{
    _rootItem->removeChildren();
    foreach (JenkinsJob job, newJobs)
    {
        _rootItem->appendChild(new JenkinsTreeItem(job.name(), JenkinsTreeItem::Type::Job));
    }
    updateHeader();
}

JenkinsSettings JenkinsJobsModel::jenkinsSettings() const { return _jenkinsSettings; }

void JenkinsJobsModel::setJenkinsSettings(const JenkinsSettings &jenkinsSettings)
{
    _jenkinsSettings = jenkinsSettings;
    updateHeader();
}

void JenkinsJobsModel::setJenkinsJobs(QList< JenkinsJob > jobs) { resetJobs(jobs); }

void JenkinsJobsModel::setOrUpdateJob(JenkinsJob job)
{
    QVector< Utils::TreeItem * > items = _rootItem->children();
    QList<JenkinsTreeItem *> castedItems;
    foreach (Utils::TreeItem *treeItem, items)
        castedItems.append(dynamic_cast<JenkinsTreeItem *>(treeItem));
    for (int i = 0; i < castedItems.size(); ++i)
    {
        if (castedItems[i]->job().jobUrl() == job.jobUrl())
        {
            castedItems[i]->setJob(job);
            emit dataChanged(QAbstractItemModel::createIndex(i, 0, castedItems[i]->parent()),
                             QAbstractItemModel::createIndex(i, 0, castedItems[i]->parent()));
            return;
        }
    }

    // item for job not found. Add new one
    JenkinsTreeItem *newItem = new JenkinsTreeItem(JenkinsTreeItem::Type::Job, job);
    _rootItem->appendChild(newItem);
}