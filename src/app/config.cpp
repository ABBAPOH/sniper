#include "config.h"
#include "utils.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

Config::Config()
{

}

bool Config::load()
{
    const auto path = Utils::configPath();
    QFile file(path);

#ifdef QT_DEBUG
    if (!QFile::remove(path)) {
        qCCritical(config) << "Can't remove config from" << path;
        return false;
    }
#endif

    if (!file.exists()) {
        if (!QFile::copy(":/config.json", path)) {
            qCCritical(config) << "Can't copy config to" << path;
            return false;
        }
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qCCritical(config) << "Can't open config" << path;
        return false;
    }

    const auto data = file.readAll();
    if (data.isEmpty()) {
        qCCritical(config) << "Empty config" << path;
        return false;
    }

    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(data, &error);;
    if (!doc.isObject()) {
        qCCritical(config) << "Empty config" << path;
        return false;
    }

    const auto map = doc.object().toVariantMap();
    if (map.isEmpty()) {
        qCCritical(config) << "Empty config" << path;
        return false;
    }

    _data = map;

    return true;
}

Q_LOGGING_CATEGORY(config, "sniper.config");
