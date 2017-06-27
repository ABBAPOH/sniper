#pragma once

#include <QtCore/QAbstractTableModel>
#include <QtNetwork/QNetworkAccessManager>
#include <QtWebKitWidgets/QWebPage>

#include <memory>
#include <deque>

class AuctionsModel : public QAbstractTableModel
{
    Q_OBJECT
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
        QString duration;
        int bid;
    };

    explicit AuctionsModel(QObject *parent = nullptr);

    std::shared_ptr<QNetworkAccessManager> networkAccessManager() const;
    void setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager> &manager);

    Data auctionData(const QModelIndex &index) const;

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

public slots:
    void update();

signals:
    void networkAccessManagerChanged();

private slots:
    void loadFinished();

private:
    std::deque<Data> _data;
    std::shared_ptr<QNetworkAccessManager> _manager;
    std::unique_ptr<QWebPage> _page;
};

