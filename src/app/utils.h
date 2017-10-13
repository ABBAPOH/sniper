#pragma once

#include "config.h"

#include <QtCore/QDateTime>
#include <QtCore/QObject>
#include <QtCore/QRegularExpression>
#include <QtCore/QString>

#include <memory>

class QWebFrame;

class Utils : public QObject
{
    Q_OBJECT
public:
    explicit Utils(const std::shared_ptr<Config> &config, QObject *parent = nullptr);

    struct AucInfo
    {
        bool ended {false};
        int bid {0};
        int step {0};
        int aucId {0};
        qint64 duration {0};
    };

    static QString durationToString(qint64 msecs);

    static QString getAucDuration(const QDateTime &current, const QDateTime &end);

    static QString logPath();
    static QString configPath();

    qint64 parseDuration(const QString &duration) const;
    bool parseAucInfo(const QWebFrame* frame, AucInfo &result) const;

private:
    struct ConfigData
    {
        std::map<QString, QString> templates;
        QRegularExpression timeExpression;
    };

    ConfigData _configData;
};
