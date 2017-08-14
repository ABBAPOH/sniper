#include "application.h"
#include "bidsmodel.h"
#include "utils.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>

BidsModel::BidsModel(const std::shared_ptr<Config>& config) :
    _loader(new AucInfoLoader)
{
    _maxDuration = QTime::fromString(config->value("delays/max").toString(), "h:m:s");
    _minDuration = QTime::fromString(config->value("delays/min").toString(), "h:m:s");
    _expectedValue = config->value("delays/expeted").toDouble();
    _dispersion = config->value("delays/dispersion").toDouble();
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
    qint64 delay = 0;

    const auto maxDuration = _maxDuration.msecsSinceStartOfDay();
    const auto minDuration = _minDuration.msecsSinceStartOfDay();
    const auto duration = data.duration < maxDuration ? data.duration : maxDuration;
    if (duration <= minDuration) {
        makeBid(data);
        delay = data.duration + 10 * 1000;
    } else {
        const auto dispersion = qint64(duration * _dispersion);
        delay = qint64(duration * _expectedValue)
                + qint64(qrand()) % dispersion - dispersion / 2;
    }

    data.timer->start(int(delay));
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
