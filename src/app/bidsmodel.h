#pragma once

#include "aucinfoloader.h"
#include "auctionsmodel.h"

#include <QtCore/QAbstractTableModel>
#include <QtCore/QLoggingCategory>
#include <QtCore/QUrl>

#include <deque>
#include <memory>

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
        NextUpdate,
        ColumnCount
    };

    BidsModel();

    AucInfoLoader &infoLoader() { return *_loader.get(); }
    const AucInfoLoader &infoLoader() const { return *_loader.get(); }

    void addBid(const AuctionsModel::Data &data, int bid);

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    struct Data : public AuctionsModel::Data
    {
        int aucId {0};
        int myBid {0};
        int step {0};
        QDateTime nextUpdate;
        std::shared_ptr<QTimer> timer;
    };

private slots:
    void onInfoLoaded(const AucInfoLoader::Info &info);
    void onTimeout();
    void processDuration(Data &data);
    void makeBid(const Data &data);

private:
    std::vector<Data> _data;
    std::unique_ptr<AucInfoLoader> _loader;
};

Q_DECLARE_LOGGING_CATEGORY(bidsModel);
