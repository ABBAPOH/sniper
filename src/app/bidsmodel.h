#pragma once

#include "aucinfoloader.h"
#include "auctionsmodel.h"
#include "fastaucinfoloader.h"
#include "config.h"

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

    explicit BidsModel(const std::shared_ptr<Config> &config);

    AucInfoLoader &infoLoader() { return *_loader.get(); }
    const AucInfoLoader &infoLoader() const { return *_loader.get(); }

    FastAucInfoLoader &fastInfoLoader() { return *_fastLoader.get(); }

    void addBid(const AuctionsModel::Data &data, int bid);
    void update(const QModelIndex &index);

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    void makeBid(const QModelIndex &index);

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
    void onInfoLoaded(const QUrl &url, const AucInfoLoader::AucInfo &info);
    void onFastInfoLoaded(int aucId, const Utils::AucInfo &info);
    void updateInfo(const std::vector<Data>::iterator &it, const AucInfoLoader::AucInfo &info);
    void onTimeout();
    void processDuration(Data &data);
    void makeBid(const Data &data);
    void onUdpateDurationTimeout();

private:
    std::vector<Data>::iterator findAuc(const QUrl& url);
    void initUpdateDurationTimer();

private:
    QTime _maxDuration;
    QTime _minDuration;
    double _expectedValue;
    double _dispersion;
    std::vector<Data> _data;
    std::unique_ptr<AucInfoLoader> _loader;
    std::unique_ptr<FastAucInfoLoader> _fastLoader;
    std::unique_ptr<QTimer> _updateDurationTimer;
};

Q_DECLARE_LOGGING_CATEGORY(bidsModel);
