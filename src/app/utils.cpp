#include "utils.h"
#include "application.h"

#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKit/QWebElement>
#include <QtCore/QStandardPaths>
#include <QtCore/QRegularExpression>
#include <QtCore/QTime>
#include <QtCore/QUrlQuery>

namespace Utils {

qint64 parseDuration(const QString& duration)
{
    QRegularExpression expr("((\\d+) д. ?)?((\\d+) ч. ?)?((\\d+) мин. ?)?((\\d+) с.)?$");

    auto match = expr.match(duration);
    if (match.isValid()) {
        const qint64 days = match.captured(2).toInt();
        const qint64 hours = match.captured(4).toInt();
        const qint64 mins = match.captured(6).toInt();
        const qint64 secs = match.captured(8).toInt();

        return (((days * 24 + hours) * 60 + mins) * 60 + secs) * 1000;
    }
    return -1;
}

QString durationToString(qint64 msecs)
{
    if (msecs < 0)
        return QString();

    const auto days = msecs / 1000 / 60 / 60 / 24;
    const auto hours = int((msecs / 1000 / 60 / 60) - days * 24);
    const auto minutes = int((msecs / 1000 / 60) - (days * 24 + hours) * 60);
    const auto secs = int((msecs / 1000) - ((days * 24 + hours) * 60 + minutes) * 60);
    if (days > 0)
        return Application::tr("%1 d %2 h").arg(days).arg(hours + 1);
    else if (hours > 0)
        return Application::tr("%1 h %2 m").arg(hours).arg(minutes + 1);
    else
        return Application::tr("%1 m %2 s").arg(minutes).arg(secs + 1);
}

QString logPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
            + "/sniper.log";
}

QString configPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
            + "/config.json";
}

bool parseAucInfo(const QWebFrame* frame, AucInfo& info)
{
    const auto url = frame->baseUrl();
    const auto query = QUrlQuery(url);

    if (!query.hasQueryItem("id")) {
        qWarning() << "Frame url" << url << "doesn't have id query item";
        return false;
    }
    info.aucId = query.queryItemValue("id").toInt();

    auto body = frame->findFirstElement("body");

    if (body.isNull()) {
        qWarning() << "Can't find <body> element in frame" << url;
        return false;
    }

    const char *constLines[] = {
        "Текущая ставка, рубли: ",
        "Шаг: ",
        "До окончания аукциона: ",
        "Аукцион завершен."
    };

    auto lines = body.toPlainText().split("\n", QString::SkipEmptyParts);
    for (const auto &line : lines) {
        if (line.contains(constLines[3])) {
            info.ended = true;
            info.duration = -1;
            info.step = -1;
            info.bid = -1;
            break;
        }
        if (line.startsWith(constLines[0])) {
            const auto subLine = line.mid(QString(constLines[0]).length());
            info.bid = subLine.split(" ").at(0).toInt();
        } else if (line.startsWith(constLines[1])) {
            info.step = line.mid(QString(constLines[1]).length()).toInt();
        } else if (line.startsWith(constLines[2])) {
            info.duration = Utils::parseDuration(line.mid(QString(constLines[2]).length()));
        }
    }

    return true;
}

QString getAucDuration(const QDateTime& current, const QDateTime& end)
{
    const auto msecs = current.msecsTo(end);
    return durationToString(msecs);
}

} // namespace Utils

