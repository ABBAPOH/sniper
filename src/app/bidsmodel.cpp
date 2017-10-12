#include "application.h"
#include "bidsmodel.h"
#include "utils.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>

BidsModel::BidsModel(const std::shared_ptr<Config>& config) :
    _loader(new AucInfoLoader),
    _fastLoader(new FastAucInfoLoader),
    _updateDurationTimer(new QTimer)
{
    _maxDuration = QTime::fromString(config->value("delays/max").toString(), "h:m:s");
    _minDuration = QTime::fromString(config->value("delays/min").toString(), "h:m:s");
    _expectedValue = config->value("delays/expected").toDouble();
    _dispersion = config->value("delays/dispersion").toDouble();

    connect(_loader.get(), &AucInfoLoader::loaded, this, &BidsModel::onInfoLoaded);
    connect(_fastLoader.get(), &FastAucInfoLoader::loaded, this, &BidsModel::onFastInfoLoaded);

    initUpdateDurationTimer();
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
    d.timer->setProperty("id", d.url);
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
            return Utils::getAucDuration(QDateTime::currentDateTime(), data.end);
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

void BidsModel::onInfoLoaded(const QUrl& url, const AucInfoLoader::AucInfo &info)
{
    updateInfo(findAuc(url), info);
}

void BidsModel::onFastInfoLoaded(int aucId, const Utils::AucInfo& info)
{
    const auto predicate = [&aucId](const Data &d)
    {
        return aucId == d.aucId;
    };
    const auto it = std::find_if(_data.begin(), _data.end(), predicate);
    if (it == _data.end()) {
        qCCritical(bidsModel) << "Can't find auc with auc_id" << info.aucId;
        return;
    }

    updateInfo(it, info);
}

void BidsModel::updateInfo(const std::vector<Data>::iterator& it, const Utils::AucInfo &info)
{
    if (it == _data.end()) {
        qCCritical(bidsModel) << "updateInfo called with invalid iterator";
        return;
    }

    auto &data = *it;
    const auto row = int(it - _data.begin());
    const auto begin = index(row, 0);
    const auto end = index(row, Columns::ColumnCount - 1);

    qCDebug(bidsModel) << "Update data for auc" << data.lot;

    if (info.ended) {
        qCDebug(bidsModel) << "rowCount before = " << rowCount();
        beginRemoveRows(QModelIndex(), row, row);
        _data.erase(it);
        endRemoveRows();
        qCDebug(bidsModel) << "rowCount after = " << rowCount();
        return;
    }

    data.aucId = info.aucId;
    data.bid = info.bid;
    data.step = info.step;
    data.duration = info.duration;
    data.end = QDateTime::currentDateTime().addMSecs(data.duration);

    qCDebug(bidsModel) << "Current bid = " << data.bid;
    if (data.duration < 60 * 60 * 1000)
        qCDebug(bidsModel) << "Duration = " << QTime::fromMSecsSinceStartOfDay(int(data.duration));

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

    auto url = timer->property("id").toUrl();
    const auto it = findAuc(url);
    if (it != _data.end()) {
        const auto &data = *it;

        qCInfo(bidsModel) << "Timeout for auc" << data.lot;
        _fastLoader->load(data.aucId);
    }
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
        Q_ASSERT(delay > 0);
        Q_ASSERT(delay < duration);
    }

    data.timer->start(int(delay));
    data.nextUpdate = QDateTime::currentDateTimeUtc().addMSecs(delay);

    qCInfo(bidsModel) << "Next update for auc" << data.lot << "will be at" << data.nextUpdate;
}

void BidsModel::makeBid(const BidsModel::Data &data)
{
    const auto myBid = data.myBid + 1 + (qrand() % 9); // add random 1-10 roubles
    if (data.bid + data.step > myBid) {
        qCInfo(bidsModel) << "Current bid is higher than you bid, skipping auc"
                          << data.lot;
    } else {
        qCInfo(bidsModel) << "Making bid" << myBid << "for" << data.lot << tr("( id = %1)").arg(data.aucId);
        Application::instance()->makeBid(data.aucId, myBid);
    }
}

void BidsModel::onUdpateDurationTimeout()
{
    if (!rowCount())
        return;

    const auto top = index(0, Columns::Duration);
    const auto bottom = index(rowCount() - 1, Columns::Duration);
    emit dataChanged(top, bottom);
}

std::vector<BidsModel::Data>::iterator BidsModel::findAuc(const QUrl& url)
{
    const auto predicate = [url](const Data &d)
    {
        return !url.isEmpty() && url == d.url;
    };
    const auto it = std::find_if(_data.begin(), _data.end(), predicate);
    if (it == _data.end())
        qCCritical(bidsModel) << "Can't find auc with url" << url;

    return it;
}

void BidsModel::initUpdateDurationTimer()
{
    connect(_updateDurationTimer.get(), &QTimer::timeout,
            this, &BidsModel::onUdpateDurationTimeout);
    _updateDurationTimer->start(1000);
}

Q_LOGGING_CATEGORY(bidsModel, "sniper.bidsModel");
