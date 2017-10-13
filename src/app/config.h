#pragma once

#include <QtCore/QObject>
#include <QtCore/QLoggingCategory>

class Config : public QObject
{
    Q_OBJECT
public:
    Config();

    bool load();

    inline const QVariantMap &data() const { return _data; }

    inline QVariant value(const QString &key)
    {
        auto parts = key.split("/");
        auto data = _data;
        for (auto part : parts.mid(0, parts.length() - 1)) {
            data = data.value(part).toMap();
        }
        const auto it = data.find(parts.last());
        if (it == data.end())
            qFatal("Key %s is not found in config", qPrintable(key));
        return it.value();
    }

private:
    QVariantMap _data;
};

Q_DECLARE_LOGGING_CATEGORY(config);
