#include "utils.h"
#include "application.h"

#include <QtCore/QStandardPaths>
#include <QtCore/QTime>
#include <QtCore/QUrlQuery>

Utils::Utils(const std::shared_ptr<Config>& config, QObject* parent) :
    QObject(parent)
{
    const char * const requiredKeys[] = {"bid", "mybid1", "mybid2", "bids_count", "step", "duration", "ended"};
    for (const auto &key: requiredKeys) {
        _configData.templates.insert(
            {key, config->value(QStringLiteral("templates/") + key).toString()});
    }

    _configData.timeExpression = QRegularExpression(config->value("timeExpression").toString());
}

QString Utils::durationToString(qint64 msecs)
{
    if (msecs < 0)
        return QString();

    const auto days = msecs / 1000 / 60 / 60 / 24;
    const auto hours = int((msecs / 1000 / 60 / 60) - days * 24);
    const auto minutes = int((msecs / 1000 / 60) - (days * 24 + hours) * 60);
    const auto secs = int((msecs / 1000) - ((days * 24 + hours) * 60 + minutes) * 60);
    if (days > 0)
        return tr("%1 d %2 h").arg(days).arg(hours + 1);
    else if (hours > 0)
        return tr("%1 h %2 m").arg(hours).arg(minutes + 1);
    else
        return tr("%1 m %2 s").arg(minutes).arg(secs + 1);
}

QString Utils::logPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
            + "/sniper.log";
}

QString Utils::configPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
            + "/config.json";
}

QString Utils::getAucDuration(const QDateTime& current, const QDateTime& end)
{
    const auto msecs = current.msecsTo(end);
    return durationToString(msecs);
}

qint64 Utils::parseDuration(const QString& duration) const
{
    auto match = _configData.timeExpression.match(duration);
    if (match.isValid()) {
        const qint64 days = match.captured(2).toInt();
        const qint64 hours = match.captured(4).toInt();
        const qint64 mins = match.captured(6).toInt();
        const qint64 secs = match.captured(8).toInt();

        return (((days * 24 + hours) * 60 + mins) * 60 + secs) * 1000;
    }
    return -1;
}

Q_LOGGING_CATEGORY(utils, "sniper.utils");
