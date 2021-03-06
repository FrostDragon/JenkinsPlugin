#include "jenkinspane.h"

#include "jenkinstreeitemdelegate.h"
#include "jenkinspluginconstants.h"

#include <QHeaderView>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>

using namespace JenkinsCI::Internal;

JenkinsPane::JenkinsPane(const std::shared_ptr< RestRequestBuilder > builder, QObject *parent)
    : Core::IOutputPane(parent), _restRequestBuilder(builder)
{
    _view = new QTreeView();
    _view->setHeaderHidden(false);
    _model = JenkinsJobsModel::instance();
    _view->setModel(_model);

    _view->header()->setStretchLastSection(true);
    _view->setContextMenuPolicy(Qt::CustomContextMenu);
    _delegate = new JenkinsTreeItemDelegate(this);
    _view->setItemDelegateForColumn(0, _delegate);

    connect(_view, &QTreeView::customContextMenuRequested, this,
            &JenkinsPane::onCustomContextMenuRequested);
    connect(_model, &JenkinsJobsModel::rootItemUpdated, [=](){
        QFontMetrics metrics(_view->font());
        int width = metrics.width(_model->getRootItemContent());
        _view->header()->resizeSection(0, width);
    });

    _viewLabel = new QLabel(tr("View: "));
    _jenkinsViewComboBox = new JenkinsViewComboBox(builder->jenkinsSettings().selectedViewUrl());
    connect(_jenkinsViewComboBox, &JenkinsViewComboBox::currentViewChanged, this, [=]()
            {
                _model->resetJobs({});
            });

    connect(_jenkinsViewComboBox,
            static_cast< void (QComboBox::*)(int) >(&QComboBox::currentIndexChanged), [=](int)
            {
                emit currentViewChanged();
            });
}

JenkinsPane::~JenkinsPane()
{
    delete _view;
    delete _viewLabel;
    delete _jenkinsViewComboBox;
}

QList< QWidget * > JenkinsPane::toolBarWidgets() const
{
    return QList< QWidget * >{_viewLabel, _jenkinsViewComboBox};
}

QString JenkinsPane::displayName() const { return tr("Jenkins"); }

int JenkinsPane::priorityInStatusBar() const { return 0; }

void JenkinsPane::visibilityChanged(bool visible) { _view->setVisible(visible); }

void JenkinsPane::setFocus() { _view->setFocus(); }

bool JenkinsPane::hasFocus() const { return _view->hasFocus(); }

bool JenkinsPane::canFocus() const { return true; }

bool JenkinsPane::canNavigate() const { return false; }

bool JenkinsPane::canNext() const { return false; }

bool JenkinsPane::canPrevious() const { return false; }

ViewInfo JenkinsPane::getSelectedView() const { return _jenkinsViewComboBox->getSelectedView(); }

void JenkinsPane::clearViews()
{
    _jenkinsViewComboBox->updateViews({});
}

void JenkinsPane::updateViews(const QSet< ViewInfo > &views)
{
    _jenkinsViewComboBox->updateViews(views);
}

void JenkinsPane::onCustomContextMenuRequested(const QPoint &point)
{
    _contextMenuIndex = _view->indexAt(point);
    if (!_contextMenuIndex.isValid())
        return;
    JenkinsTreeItem *item = static_cast< JenkinsTreeItem * >(_contextMenuIndex.internalPointer());
    if (item == nullptr)
        return;

    QMenu *contextMenu = new QMenu(_view);

    QAction *openInBrowserEntry = new QAction(tr("open in browser"), contextMenu);
    openInBrowserEntry->setIcon(
        QIcon(QLatin1String(JenkinsCI::Constants::OPEN_IN_BROWSER_ICON)));
    contextMenu->addAction(openInBrowserEntry);
    connect(openInBrowserEntry, &QAction::triggered, this, &JenkinsPane::openInBrowser);

    if (item->itemType() == JenkinsTreeItem::Type::Job)
    {
        QAction *buildHistoryEntry = new QAction(tr("show build history"), contextMenu);
        buildHistoryEntry->setIcon(
            QIcon(QLatin1String(JenkinsCI::Constants::BUILD_HISTORY_ICON)));
        contextMenu->addAction(buildHistoryEntry);
        connect(buildHistoryEntry, &QAction::triggered, this, &JenkinsPane::requestBuildHistory);
    }
    contextMenu->popup(_view->viewport()->mapToGlobal(point));
}

void JenkinsPane::requestBuildHistory()
{
    JenkinsTreeItem *item = static_cast< JenkinsTreeItem * >(_contextMenuIndex.internalPointer());
    if (item == nullptr)
        return;
    if (item->itemType() != JenkinsTreeItem::Type::Job)
        return;
    emit buildHistoryRequested(item->job());
}

void JenkinsPane::openInBrowser()
{
    JenkinsTreeItem *item = static_cast< JenkinsTreeItem * >(_contextMenuIndex.internalPointer());
    if (item == nullptr)
        return;
    QUrl itemUrl = _restRequestBuilder->buildUrl(item->itemUrl());
    QDesktopServices::openUrl(itemUrl);
}
