#pragma once

#include <QtWebKitWidgets/QWebPage>

#include <QtNetwork/QNetworkAccessManager>

#include <QtCore/QObject>
#include <QtCore/QUrl>

#include <deque>

class AucInfoLoader : public QObject
{
    Q_OBJECT
public:
    struct Info
    {
        QUrl url;
        int bid {0};
        int step {0};
        int aucId {0};
        qint64 duration {0};
    };

    explicit AucInfoLoader(QObject *parent = nullptr);
    ~AucInfoLoader();

    std::shared_ptr<QNetworkAccessManager> networkAccessManager() const;
    void setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager> &manager);

public slots:
    void load(const QUrl &url);

signals:
    void loaded(const Info &info);

private:
    enum class Status { Idle, Adding };

    void processNextUrl();

private slots:
    void loadFinished(bool ok);

private:
    std::shared_ptr<QNetworkAccessManager> _manager;
    std::unique_ptr<QWebPage> _page;
    Status _status = Status::Idle;
    std::deque<QUrl> _queue;
};
