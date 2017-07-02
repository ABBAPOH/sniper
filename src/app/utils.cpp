#include "utils.h"

#include <QRegularExpression>
#include <QTime>

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

} // namespace Utils

