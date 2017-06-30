#pragma once

#include "auctionsmodel.h"

#include <QtCore/QAbstractTableModel>
#include <QtCore/QUrl>

#include <deque>

class BidsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Columns
    {
        Lot = 0,
        Seller,
        Shipping,
        Duration,
        CurrentBid,
        MyBid,
        AucId,
        ColumnCount
    };

    BidsModel();

    std::shared_ptr<QNetworkAccessManager> networkAccessManager() const;
    void setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager> &manager);

    void addBid(const AuctionsModel::Data &data, int bid);

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    void processNextAddedBid();

private slots:
    void loadFinished(bool ok);
    void onTimeout();

private:
    struct Data : public AuctionsModel::Data
    {
        int aucId {0};
        int myBid {0};
        int step {0};
        std::shared_ptr<QTimer> timer;
    };

    enum class Status { Idle, Adding };

    std::shared_ptr<QNetworkAccessManager> _manager;
    std::deque<Data> _data;
    Status _status = Status::Idle;
    std::deque<Data> _queue;
    std::unique_ptr<QWebPage> _page;
};

