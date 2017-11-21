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

    static QString durationToString(qint64 msecs);

    static QString getAucDuration(const QDateTime &current, const QDateTime &end);

    static QString logPath();
    static QString configPath();

    qint64 parseDuration(const QString &duration) const;

private:
    struct ConfigData
    {
        std::map<QString, QString> templates;
        QRegularExpression timeExpression;
    };

    ConfigData _configData;
};

Q_DECLARE_LOGGING_CATEGORY(utils);
