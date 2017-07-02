#pragma once

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
        QUrl url;
        QString lot;
        QString seller;
        QString shipping;
        qint64 duration;
        QDateTime end;
        int bid;
    };

    explicit AuctionsModel(QObject *parent = nullptr);

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
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
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

private:
    void restartUpdateTimer();

private:
    std::deque<Data> _data;
    std::shared_ptr<QNetworkAccessManager> _manager;
    std::unique_ptr<QWebPage> _page;
    std::unique_ptr<QTimer> _updateTimer;
    bool _autoUpdateEnabled {true};
    int _autoUpdateInterval {10 * 60 * 1000};
    int _autoUpdateDispersion {5 * 60 * 1000};
};

Q_DECLARE_LOGGING_CATEGORY(auctionsModel);
