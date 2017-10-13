#pragma once

#include "config.h"
#include "utils.h"

#include <QtWebKitWidgets/QWebPage>

#include <QtNetwork/QNetworkAccessManager>

#include <QtCore/QObject>
#include <QtCore/QLoggingCategory>
#include <QtCore/QUrl>

#include <deque>
#include <memory>

class AucInfoLoader : public QObject
{
    Q_OBJECT
public:
    using AucInfo = Utils::AucInfo;

    explicit AucInfoLoader(const std::shared_ptr<Config> &config, QObject *parent = nullptr);
    ~AucInfoLoader();

    std::shared_ptr<QNetworkAccessManager> networkAccessManager() const;
    void setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager> &manager);

public slots:
    void load(const QUrl &url);

signals:
    void loaded(const QUrl &url, const AucInfo &info);

private:
    enum class Status { Idle, Adding };

    void processNextUrl();

private slots:
    void onFrameLoadFinished(bool ok);

private:
    Utils _utils;
    std::shared_ptr<QNetworkAccessManager> _manager;
    std::unique_ptr<QWebPage> _page;
    Status _status = Status::Idle;
    std::deque<QUrl> _queue;
};

Q_DECLARE_LOGGING_CATEGORY(aucInfoLoader);
