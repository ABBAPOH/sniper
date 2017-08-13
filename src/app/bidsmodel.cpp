#include "application.h"
#include "bidsmodel.h"
#include "utils.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>

BidsModel::BidsModel() :
    _loader(new AucInfoLoader)
{
    connect(_loader.get(), &AucInfoLoader::loaded, this, &BidsModel::onInfoLoaded);
}

void BidsModel::addBid(const AuctionsModel::Data &data, int bid)
{
    qCInfo(bidsModel) << "Request to add bid for auc" << data.lot << bid;

    const auto newRow = rowCount();

    Data d;
    d.AuctionsModel::Data::operator=(data);
    d.myBid = bid;

    d.timer = std::make_shared<QTimer>();
    d.timer->setSingleShot(true);
    d.timer->setProperty("id", newRow);
    connect(d.timer.get(), &QTimer::timeout, this, &BidsModel::onTimeout);

    _loader->load(d.url);

    beginInsertRows(QModelIndex(), newRow, newRow);
    _data.push_back(d);
    endInsertRows();
}

void BidsModel::update(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const auto data = _data.at(size_t(index.row()));
    _loader->load(data.url);
}

int BidsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return int(_data.size());
}

int BidsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return Columns::ColumnCount;
}

QVariant BidsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int column = index.column();
    const auto &data = _data.at(size_t(index.row()));
    if (role == Qt::DisplayRole) {
        if (column == Lot) {
            return data.lot;
        } else if (column == Seller) {
            return data.seller;
        } else if (column == Shipping) {
            return data.shipping;
        } else if (column == Duration) {
            return Utils::durationToString(data.duration);
        } else if (column == CurrentBid) {
            return data.bid;
        } else if (column == MyBid) {
            return data.myBid;
        } else if (column == AucId) {
            return data.aucId;
        } else if (column == NextUpdate) {
            return data.nextUpdate.toLocalTime();
        }
    }

    return QVariant();
}

QVariant BidsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case Columns::Lot: return tr("Lot");
            case Columns::Seller: return tr("Seller");
            case Columns::Shipping: return tr("Shipping");
            case Columns::Duration: return tr("Duration");
            case Columns::CurrentBid: return tr("Current bid");
            case Columns::MyBid: return tr("My bid");
            case Columns::AucId: return tr("Auc id");
            case Columns::NextUpdate: return tr("Next update");
            default:
                break;
            }
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

void BidsModel::makeBid(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    makeBid(_data.at(size_t(index.row())));
}

void BidsModel::onInfoLoaded(const AucInfoLoader::Info &info)
{
    const auto predicate = [&info](const Data &d)
    {
        return info.url == d.url;
    };
    const auto it = std::find_if(_data.begin(), _data.end(), predicate);
    if (it == _data.end()) {
        qCCritical(bidsModel) << "Can't find auc with url" << info.url;
        return;
    }

    auto &data = *it;
    const auto begin = index(int(it - _data.begin()), 0);
    const auto end = index(int(it - _data.begin()), Columns::ColumnCount);

    qCDebug(bidsModel) << "Update data for auc" << data.lot;

    if (info.ended) {
        beginRemoveRows(QModelIndex(), begin.row(), end.row());
        _data.erase(it);
        endRemoveRows();
        return;
    }

    data.aucId = info.aucId;
    data.bid = info.bid;
    data.step = info.step;
    data.duration = info.duration;

    processDuration(data);

    emit dataChanged(begin, end, {Qt::DisplayRole});
}

void BidsModel::onTimeout()
{
    auto timer = qobject_cast<QTimer *>(sender());
    if (!timer) {
        qCCritical(bidsModel) << "Sender object is not a QTimer";
        return;
    }

    auto id = timer->property("id").toInt();
    const auto &data = _data.at(size_t(id));

    qCInfo(bidsModel) << "Timeout for auc" << data.lot;
    _loader->load(data.url);
}

void BidsModel::processDuration(BidsModel::Data &data)
{
    int delay = 0;
    int msecsInHour = 60 * 60 * 1000;
    int msecsInMinute = 60 * 1000;
    int msecsInSecond = 1000;
    if (data.duration > 12 * msecsInHour)
        delay = 10 * msecsInHour + qrand() % msecsInHour; // reload in next 10-11 hours
    else if (data.duration > 6 * msecsInHour)
        delay = 5 * msecsInHour + qrand() % (msecsInHour / 2); // reload in next 5-5.5 hours
    else if (data.duration > 2 * msecsInHour)
        delay = 1 * msecsInHour + qrand() % (msecsInHour / 2); // reload in next 1-1.5 hours
    else if (data.duration > 1 * msecsInHour)
        delay = 30 * msecsInMinute + qrand() % (15 * msecsInMinute); // reload in next 30-45 minutes
    else if (data.duration > 30 * msecsInMinute)
        delay = 15 * msecsInMinute + qrand() % (10 * msecsInMinute); // reload in next 15-25 minutes
    else if (data.duration > 15 * msecsInMinute)
        delay = 10 * msecsInMinute + qrand() % (3 * msecsInMinute); // reload in next 10-13 minutes
    else if (data.duration > 10 * msecsInMinute)
        delay = 5 * msecsInMinute + qrand() % (3 * msecsInMinute); // reload in next 5-8 minutes
    else if (data.duration > 5 * msecsInMinute)
        delay = 2 * msecsInMinute + qrand() % (1 * msecsInMinute); // reload in next 2-3 minutes
    else if (data.duration > 2 * msecsInMinute)
        delay = 60 * msecsInSecond + qrand() % (30 * msecsInSecond); // reload in next 60-90 seconds
    else if (data.duration > 1 * msecsInMinute)
        delay = int(data.duration - 15 * msecsInSecond - qrand() % (15 * msecsInSecond)); // reload in 15-30 seconds before end
    else if (data.duration > 15 * msecsInSecond)
        delay = int(data.duration - 5 * msecsInSecond - qrand() % (7 * msecsInSecond)); // reload in 5-7 seconds before end
    else { // less than a 15 seconds
        makeBid(data);
        delay = 30 * msecsInSecond;
    }

    data.timer->start(delay);
    data.nextUpdate = QDateTime::currentDateTimeUtc().addMSecs(delay);

    qCInfo(bidsModel) << "Next update for auc" << data.lot << "will be at" << data.nextUpdate;
}

void BidsModel::makeBid(const BidsModel::Data &data)
{
    if (data.bid + data.step > data.myBid) {
        qCInfo(bidsModel) << "Current bid is higher than you bid, skipping auc"
                          << data.lot;
    } else {
        qCInfo(bidsModel) << "Making bid" << data.myBid << "for" << data.lot << tr("( id = %1)").arg(data.aucId);
        Application::instance()->makeBid(data.aucId, data.myBid);
    }
}

Q_LOGGING_CATEGORY(bidsModel, "sniper.bidsModel");
