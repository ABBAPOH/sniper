#pragma once

#include "utils.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtWebKitWidgets/QWebView>

#include <deque>
#include <memory>

class AucInfoLoader : public QObject
{
    Q_OBJECT
public:
    struct AucInfo
    {
        int id {0};
        int bid {0};
        QString csrfName;
        QString csrfValue;
    };

    explicit AucInfoLoader(const std::shared_ptr<Config> &config, QObject *parent = nullptr);
    ~AucInfoLoader();

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
    QString _urlTemplate;
    std::shared_ptr<QNetworkAccessManager> _manager;
    std::unique_ptr<QWebPage> _page;
    Status _status = Status::Idle;
    std::deque<QUrl> _queue;
};


Q_DECLARE_LOGGING_CATEGORY(fastAucInfoLoader);
