#ifndef JENKINSPLUGIN_H
#define JENKINSPLUGIN_H

#include "jenkinsplugin_global.h"

#include <extensionsystem/iplugin.h>

#include "jenkinsdatafetcher.h"
#include "optionspage.h"
#include "jenkinspane.h"
#include "restrequestbuilder.h"

namespace JenkinsPlugin
{
namespace Internal
{

class JenkinsPluginPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "JenkinsPlugin.json")

public:
    JenkinsPluginPlugin();
    ~JenkinsPluginPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();

private slots:
    void triggerAction();
    void updateJobs(QList< JenkinsJob > jobs);
    void updateJob(JenkinsJob job);

    void onSettingsChanged(const JenkinsSettings &settings);

private:
    void createOptionsPage();

    std::shared_ptr<RestRequestBuilder> _restRequestBuilder;

    JenkinsDataFetcher *_fetcher;
    OptionsPage *_optionsPage;
    JenkinsSettings _settings;
    JenkinsPane *_pane;
};

}  // namespace Internal
}  // namespace JenkinsPlugin

#endif  // JENKINSPLUGIN_H
