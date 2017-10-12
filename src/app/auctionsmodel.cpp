#include "auctionsmodel.h"
#include "utils.h"

#include <QtCore/QDebug>

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtWebKit/QWebElement>
#include <QtWebKitWidgets/QWebFrame>

AuctionsModel::AuctionsModel(QObject* parent) :
    QAbstractTableModel(parent),
    _page(new QWebPage),
    _updateTimer(new QTimer),
    _updateDurationTimer(new QTimer)
{
    _updateTimer->setSingleShot(true);
    connect(_updateTimer.get(), &QTimer::timeout, this, &AuctionsModel::onUdpateTimeout);
    connect(_page.get(), &QWebPage::loadFinished, this, &AuctionsModel::loadFinished);

    initUpdateDurationTimer();
    restartUpdateTimer();
}

std::shared_ptr<QNetworkAccessManager> AuctionsModel::networkAccessManager() const
{
    return _manager;
}

void AuctionsModel::setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager>& manager)
{
    if (_manager == manager)
        return;

    _manager = manager;
    _page->setNetworkAccessManager(_manager.get());
    emit networkAccessManagerChanged();
    qCDebug(auctionsModel) << "Network manager changed";
}

bool AuctionsModel::autoUpdateEnabled() const
{
    return _autoUpdateEnabled;
}

void AuctionsModel::setAutoUpdateEnabled(bool enabled)
{
    if (_autoUpdateEnabled == enabled)
        return;

    _autoUpdateEnabled = enabled;
    qCDebug(auctionsModel) << "Auto update enabled changed to" << enabled;

    restartUpdateTimer();
    emit autoUpdateEnabledChanged(_autoUpdateEnabled);
}

int AuctionsModel::autoUpdateInterval() const
{
    return _autoUpdateInterval;
}

void AuctionsModel::setAutoUpdateInterval(int interval)
{
    if (_autoUpdateInterval == interval)
        return;

    _autoUpdateInterval = interval;
    qCDebug(auctionsModel) << "Auto update interval changed to" << _autoUpdateInterval;
    restartUpdateTimer();
    emit autoUpdateIntervalChanged(_autoUpdateInterval);
}

int AuctionsModel::autoUpdateDispersion() const
{
    return _autoUpdateDispersion;
}

void AuctionsModel::setAutoUpdateDispersion(int dispersion)
{
    if (_autoUpdateDispersion == dispersion)
        return;

    _autoUpdateDispersion = dispersion;
    qCDebug(auctionsModel) << "Auto update dispersion changed to" << _autoUpdateDispersion;
    restartUpdateTimer();
    emit autoUpdateDispersionChanged(_autoUpdateDispersion);
}

AuctionsModel::Data AuctionsModel::auctionData(const QModelIndex &index) const
{
    if (!index.isValid())
        return Data();

    return _data.at(index.row());
}

int AuctionsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return int(_data.size());
}

int AuctionsModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return Columns::ColumnCount;
}

QVariant AuctionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case Columns::Lot: return tr("Lot");
            case Columns::Seller: return tr("Seller");
            case Columns::Shipping: return tr("Shipping");
            case Columns::Duration: return tr("Duration");
            case Columns::Bid: return tr("Bid");
            default:
                break;
            }
        }
    }
    return QVariant();
}

QVariant AuctionsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const auto currentDateTime = QDateTime::currentDateTimeUtc();
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
            return Utils::getAucDuration(currentDateTime, data.end);
        } else if (column == Bid) {
            return data.bid;
        }
    }

    return QVariant();
}

void AuctionsModel::update()
{
    if (!_manager)
        return;

    qCInfo(auctionsModel) << "Requested update";
    _page->mainFrame()->setUrl(QUrl("http://topdeck.ru/auc/aucs.php"));
}

void AuctionsModel::loadFinished()
{
    beginResetModel();
    auto table = _page->mainFrame()->findFirstElement("table[class=reftable]");
    auto body = table.findFirst("tbody");
    for (auto tr = body.firstChild().nextSibling(); tr != body.lastChild(); tr = tr.nextSibling()) {
        QStringList rawData;
        QUrl url;
        for (auto td = tr.firstChild(); td != tr.lastChild(); td = td.nextSibling()) {
            if (rawData.empty()) {
                auto a = td.findFirst("a");
                url = a.attribute("href");
            }
            rawData.append(td.toPlainText());
        }
        Data d;
        d.url = url;
        d.lot = rawData[0];
        d.seller = rawData[1];
        d.shipping = rawData[2];
        const auto duration = rawData[3];
        if (duration == "Завершен")
            continue;

        auto msecs = Utils::parseDuration(duration);

        QDateTime dateTime = QDateTime::currentDateTimeUtc();
        dateTime = dateTime.addMSecs(msecs);
        d.end = dateTime;

        d.bid = rawData[4].toInt();
        _data.push_back(d);
    }
    endResetModel();
    qCInfo(auctionsModel) << "Updated";
}

void AuctionsModel::onUdpateTimeout()
{
    update();
    restartUpdateTimer();
}

void AuctionsModel::onUdpateDurationTimeout()
{
    if (!rowCount())
        return;

    const auto top = index(0, Columns::Duration);
    const auto bottom = index(rowCount() - 1, Columns::Duration);
    emit dataChanged(top, bottom);
}

void AuctionsModel::restartUpdateTimer()
{
    _updateTimer->stop();
    if (_autoUpdateEnabled) {
        const auto dispersion = qrand() % _autoUpdateDispersion - _autoUpdateDispersion / 2;
        const auto delay = _autoUpdateInterval + dispersion;
        qCInfo(auctionsModel) << "Next update in" << QDateTime::currentDateTimeUtc().addMSecs(delay);
        _updateTimer->start(delay);
    }
}

void AuctionsModel::initUpdateDurationTimer()
{
    connect(_updateDurationTimer.get(), &QTimer::timeout,
            this, &AuctionsModel::onUdpateDurationTimeout);
    _updateDurationTimer->start(1000);
}

Q_LOGGING_CATEGORY(auctionsModel, "sniper.auctionsModel");
