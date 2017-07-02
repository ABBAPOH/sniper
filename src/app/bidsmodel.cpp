#include "bidsmodel.h"
#include "utils.h"

#include <QtWebKit/QWebElement>
#include <QtWebKitWidgets/QWebPage>
#include <QtWebKitWidgets/QWebFrame>

#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtCore/QUrlQuery>

BidsModel::BidsModel() :
    _page(new QWebPage)
{
    connect(_page.get(), &QWebPage::loadFinished, this, &BidsModel::loadFinished);
}

std::shared_ptr<QNetworkAccessManager> BidsModel::networkAccessManager() const
{
    return _manager;
}

void BidsModel::setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager> &manager)
{
    if (_manager == manager)
        return;

    _manager = manager;
    _page->setNetworkAccessManager(_manager.get());
}

void BidsModel::addBid(const AuctionsModel::Data &data, int bid)
{
    Data d;
    d.AuctionsModel::Data::operator=(data);
    d.myBid = bid;

    _queue.push_back(d);

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    _data.push_back(d);
    endInsertRows();

    processNextAddedBid();
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
            return data.duration;
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

void BidsModel::processNextAddedBid()
{
    if (_status != Status::Idle)
        return;
    if (_queue.empty())
        return;

    auto d = _queue.front();
    _status = Status::Adding;
    _page->mainFrame()->setUrl(d.url);
}

void BidsModel::loadFinished(bool ok)
{
    auto data = _queue.front();
    _status = Status::Idle;
    _queue.pop_front();
    qDebug() << "loadFinished()" << ok;

    const auto predicate = [&data](const Data &d)
    {
        return data.url == d.url;
    };
    const auto it = std::find_if(_data.begin(), _data.end(), predicate);
    if (it == _data.end()) {
        qWarning() << "Can't find auc with url" << data.url;
        processNextAddedBid();
        return;
    }

    qDebug() << _page->mainFrame()->url();
    for (auto frame : _page->mainFrame()->childFrames()) {
        const auto url = frame->baseUrl();
        if (url.path() == "/auc/auc.php") {
            data.aucId = QUrlQuery(url).queryItemValue("id").toInt();

            auto body = frame->findFirstElement("body");

            const char *constLines[] = {
                "Текущая ставка, рубли: ",
                "Шаг: ",
                "До окончания аукциона: "
            };

            auto lines = body.toPlainText().split("\n", QString::SkipEmptyParts);
            for (const auto &line : lines) {
                if (line.startsWith(constLines[0])) {
                    data.bid = line.mid(QString(constLines[0]).length()).toInt();
                } else if (line.startsWith(constLines[1])) {
                    data.step = line.mid(QString(constLines[1]).length()).toInt();
                } else if (line.startsWith(constLines[2])) {
                    data.duration = Utils::parseDuration(line.mid(QString(constLines[2]).length()));
                }
            }

            break;
        }
    }

    const auto begin = index(int(it - _data.begin()), 0);
    const auto end = index(int(it - _data.begin()), Columns::ColumnCount);

    if (!data.timer) {
        data.timer = std::make_shared<QTimer>();
        data.timer->setSingleShot(true);
        data.timer->setProperty("id", begin.row());
        connect(data.timer.get(), &QTimer::timeout, this, &BidsModel::onTimeout);
        processTimeout(data);
    }
    *it = data;
    emit dataChanged(begin, end, {Qt::DisplayRole});

    processNextAddedBid();
}

void BidsModel::onTimeout()
{
    auto timer = qobject_cast<QTimer *>(sender());
    if (!timer) {
        qWarning() << "No timer object";
        return;
    }

    auto id = timer->property("id").toInt();
    qDebug() << "Timout for row" << id;
    _queue.push_back(_data.at(size_t(id)));
    processNextAddedBid();
}

void BidsModel::processTimeout(BidsModel::Data &data)
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
        delay = 30;
    }

    data.timer->start(delay);
    data.nextUpdate = QDateTime::currentDateTimeUtc().addMSecs(delay);
}

void BidsModel::makeBid(const BidsModel::Data &data)
{
    qDebug() << "Making bid for " << data.lot << data.myBid;
}
