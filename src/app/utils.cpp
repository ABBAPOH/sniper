#include "utils.h"

#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKit/QWebElement>
#include <QtCore/QStandardPaths>
#include <QtCore/QRegularExpression>
#include <QtCore/QTime>

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
    return QStringLiteral("%1:%2").arg(days).arg(QTime(hours, minutes, secs).toString("hh:mm:ss"));
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
    auto body = frame->findFirstElement("body");

    if (body.isNull()) {
        qWarning() << "Can't find <body> element in frame" << frame->baseUrl();
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

} // namespace Utils

