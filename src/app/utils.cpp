#include "utils.h"

#include <QRegularExpression>

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
}

} // namespace Utils

