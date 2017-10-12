#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QString>

class QWebFrame;

namespace Utils {

struct AucInfo
{
    bool ended {false};
    int bid {0};
    int step {0};
    int aucId {0};
    qint64 duration {0};
};

qint64 parseDuration(const QString &duration);
QString durationToString(qint64 msecs);

QString getAucDuration(const QDateTime &current, const QDateTime &end);

bool parseAucInfo(const QWebFrame* frame, AucInfo &result);

QString logPath();
QString configPath();

} // namespace Utils
