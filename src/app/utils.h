#pragma once

#include <QtCore/QString>

namespace Utils {

qint64 parseDuration(const QString &duration);
QString durationToString(qint64 msecs);

QString logPath();

} // namespace Utils
