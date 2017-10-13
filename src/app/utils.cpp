#include "utils.h"
#include "application.h"

#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKit/QWebElement>
#include <QtCore/QStandardPaths>
#include <QtCore/QTime>
#include <QtCore/QUrlQuery>

Utils::Utils(const std::shared_ptr<Config>& config, QObject* parent) :
    QObject(parent)
{
    const char * const requiredKeys[] = {"bid", "bids_count", "step", "duration", "ended"};
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

bool Utils::parseAucInfo(const QWebFrame* frame, AucInfo& info) const
{
    info = AucInfo();

    const auto url = frame->baseUrl();
    const auto query = QUrlQuery(url);

    if (!query.hasQueryItem("id")) {
        qCWarning(utils) << "Frame url" << url << "doesn't have id query item";
        return false;
    }
    info.aucId = query.queryItemValue("id").toInt();

    auto body = frame->findFirstElement("body");

    if (body.isNull()) {
        qCWarning(utils) << "Can't find <body> element in frame" << url;
        return false;
    }

    using Parser = std::function<QVariant(QString)>;
    const auto parseInt = [](const QString &text) -> QVariant
    {
        return text.toInt();
    };
    const auto parseTime = [this](const QString &text) -> QVariant
    {
        return parseDuration(text);
    };

    std::map<QString, Parser> parsers = {
        {"%int%", parseInt},
        {"%time%", parseTime}
    };

    const auto split = [](const QString &text) -> std::pair<QString, QString> {
        const auto kv = text.split(':');
        if (kv.size() == 2) {
            return {kv[0].trimmed(), kv.at(1).trimmed()};
        } else if (kv.size() == 1) {
            return {kv.at(0), QString()};
        }
        qCWarning(utils) << "Line" << text << "has too many parts";
        return {};
    };

    std::map<QString, std::pair<QString, QString>> parsedTemplates;
    for (const auto &item: _configData.templates) {
        const auto key = item.first;
        const auto line = item.second;
        parsedTemplates.insert({key, split(line)});
    }

    const auto plainBody = body.toPlainText();
    std::map<QString, QString> map1;
    for (const auto &line: plainBody.split('\n', QString::SkipEmptyParts)) {
        map1.insert(split(line));
    }

    const auto it = map1.find(parsedTemplates.at("ended").first);
    if (it != map1.end()) {
        info.ended = true;
        return true;
    }

    std::map<QString, QVariant> parsedValues;

    for (const auto &item: parsedTemplates) {
        const auto &key = item.first;
        const auto &value = item.second.first;
        const auto &type = item.second.second;
        const auto it = map1.find(value);
        if (key == "ended")
            continue;
        if (it == map1.end()) {
            qCWarning(utils) << "Can't find line" << key;
            return false;
        }
        const auto it2 = parsers.find(type);
        if (it2 == parsers.end()) {
            qCWarning(utils) << "Can't find parser for" << type;
            return false;
        }
        const auto &parser = it2->second;
        const auto realValue = parser(it->second);
        parsedValues.insert({key, realValue});
    }

    try {
        info.ended = false;
        info.bid = parsedValues.at("bid").toInt();
        info.step = parsedValues.at("step").toInt();
        info.duration = parsedValues.at("duration").toLongLong();
    } catch (const std::out_of_range &ex) {
        qCCritical(utils) << "Can't find required key in" << plainBody;
        return false;
    }

    return true;
}

Q_LOGGING_CATEGORY(utils, "sniper.utils");
