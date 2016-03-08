#ifndef JENKINSDATAFETCHER_H
#define JENKINSDATAFETCHER_H

#include <QObject>

#include "jenkinssettings.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QTimer>

namespace JenkinsPlugin
{
namespace Internal
{

class BuildInfo
{
public:
    enum class Result
    {
        Aborted,
        Failure,
        NotBuilt,
        Success,
        Unstable,
        Unknown,
    };

    BuildInfo() {}

    QString url() const;
    void setUrl(const QString &url);

    int number() const;
    void setNumber(int number);

    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &timestamp);
    void setTimestamp(const qint64 timestamp);

    Result result() const;
    QString getResultIcon();

    void setResult(const Result &result);
    void setResult(const QString &result);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    QString fullDisplayName() const;
    void setFullDisplayName(const QString &fullDisplayName);

    QString description() const;
    void setDescription(const QString &description);

    int duration() const;
    void setDuration(int duration);

private:
    QString _url;
    int _number;
    QDateTime _timestamp;
    Result _result{Result::Unknown};
    QString _displayName;
    QString _fullDisplayName;
    QString _description;
    int _duration; // in miliseconds
};

class JenkinsJob
{
public:
    JenkinsJob() {}

    QString jobUrl() const;
    void setJobUrl(const QString &jobUrl);

    QString name() const;
    void setName(const QString &name);

    void setBuildStatus(const QString &colorEntry);

    bool isValid() { return !_jobUrl.isEmpty() && !_name.isEmpty(); }

    BuildInfo buildInfo() const;
    void setBuildInfo(const BuildInfo &buildInfo);

    bool isRunning() const;
    QString colorIcon() const;

private:
    QString _jobUrl;
    QString _name;
    QString _color;
    BuildInfo _buildInfo;

    bool _isRunning{false};
    QString _colorIcon;
};

class JenkinsDataFetcher : public QObject
{
    Q_OBJECT
public:
    explicit JenkinsDataFetcher(QObject *parent = 0);

    JenkinsSettings jenkinsSettings() const;
    void setJenkinsSettings(const JenkinsSettings &jenkinsSettings);

    void getAvaliableJobs();

    static QString urlToRestApiUrl(const QString &url);

signals:
    void jobsUpdated(QList< JenkinsJob >);
    void jobUpdated(JenkinsJob);

public slots:

private slots:
    void readReply(QNetworkReply *reply);
    void switchToNextFetchStep();
//    void authentificateUser();

private:
    enum class State
    {
        Ready,
        FetchingJobs,
        FetchingJobDetails,
    };

    QList< JenkinsJob > fetchJobList(QNetworkReply *reply);
    JenkinsJob fillBuildDetails(QNetworkReply *reply);
    static QString cutRestApiUrlPart(const QString url);
    static QString buildUrlToJobUrl(QString buildUrl);

    QNetworkRequest createRequest(const QString urlString) const;

    void sendDetailsRequestForFirstJob();

    QList< JenkinsJob > _jobsForDetalization;

    JenkinsSettings _jenkinsSettings;
    QNetworkAccessManager *_manager;
    State _state{State::Ready};
    QTimer *_timer;

//    bool _isAuthentificated{false};

    static const QString REST_API_URL_SUFFIX;
};
}
}

#endif  // JENKINSDATAFETCHER_H
