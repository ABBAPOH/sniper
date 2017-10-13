#pragma once

#include "aucinfoloader.h"
#include "utils.h"

class FastAucInfoLoader : public QObject
{
    Q_OBJECT
public:
    using AucInfo = Utils::AucInfo;

    explicit FastAucInfoLoader(const std::shared_ptr<Config> &config, QObject *parent = nullptr);
    ~FastAucInfoLoader();

    std::shared_ptr<QNetworkAccessManager> networkAccessManager() const;
    void setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager> &manager);

public slots:
    void load(int auc_id);

signals:
    void loaded(int auc_id, const AucInfo &info);

private:
    enum class Status { Idle, Adding };

    void processNextUrl();

private slots:
    void onLoadFinished(bool ok);

private:
    Utils _utils;
    std::shared_ptr<QNetworkAccessManager> _manager;
    std::unique_ptr<QWebPage> _page;
    Status _status = Status::Idle;
    std::deque<QUrl> _queue;
};


Q_DECLARE_LOGGING_CATEGORY(fastAucInfoLoader);
