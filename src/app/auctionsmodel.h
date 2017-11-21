#pragma once

#include "config.h"
#include "utils.h"

#include <QtWebKitWidgets/QWebPage>

#include <QtNetwork/QNetworkAccessManager>

#include <QtCore/QAbstractTableModel>
#include <QtCore/QLoggingCategory>
#include <QtCore/QTimer>

#include <memory>
#include <deque>

class AuctionsModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(bool autoUpdateEnabled READ autoUpdateEnabled WRITE setAutoUpdateEnabled NOTIFY autoUpdateEnabledChanged)
    Q_PROPERTY(int autoUpdateInterval READ autoUpdateInterval WRITE setAutoUpdateInterval NOTIFY autoUpdateIntervalChanged)
    Q_PROPERTY(int autoUpdateDispersion READ autoUpdateDispersion WRITE setAutoUpdateDispersion NOTIFY autoUpdateDispersionChanged)
public:
    enum Columns
    {
        Lot = 0,
        Seller,
        Shipping,
        Duration,
        Bid,
        ColumnCount
    };

    struct Data
    {
        int id {0};
        QUrl url;
        QString lot;
        QString seller;
        QString shipping;
        QString shippingFull;
        qint64 duration;
        QDateTime start;
        QDateTime end;
        int startBid {0};
        int bid {0};
        int bidCount {0};
    };

    explicit AuctionsModel(const std::shared_ptr<Config> &config, QObject *parent = nullptr);

    std::shared_ptr<QNetworkAccessManager> networkAccessManager() const;
    void setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager> &manager);

    bool autoUpdateEnabled() const;
    void setAutoUpdateEnabled(bool autoUpdateEnabled);

    int autoUpdateInterval() const;
    void setAutoUpdateInterval(int interval);

    int autoUpdateDispersion() const;
    void setAutoUpdateDispersion(int autoUpdateDispersion);

    Data auctionData(const QModelIndex &index) const;

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex& index, int role) const override;

public slots:
    void update();

signals:
    void networkAccessManagerChanged();
    void autoUpdateEnabledChanged(bool enabled);
    void autoUpdateIntervalChanged(int interval);
    void autoUpdateDispersionChanged(int autoUpdateDispersion);

private slots:
    void loadFinished();
    void onUdpateTimeout();
    void onUdpateDurationTimeout();

private:
    void restartUpdateTimer();
    void initUpdateDurationTimer();

private:
    Utils _utils;
    std::deque<Data> _data;
    std::shared_ptr<QNetworkAccessManager> _manager;
    std::unique_ptr<QWebPage> _page;
    std::unique_ptr<QTimer> _updateTimer;
    std::unique_ptr<QTimer> _updateDurationTimer;
    bool _autoUpdateEnabled {true};
    int _autoUpdateInterval {20 * 60 * 1000};
    int _autoUpdateDispersion {10 * 60 * 1000};
};

Q_DECLARE_LOGGING_CATEGORY(auctionsModel);
