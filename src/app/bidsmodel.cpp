#include "bidsmodel.h"

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
                    data.duration = line.mid(QString(constLines[2]).length());
                }
            }

            break;
        }
    }

    data.timer = std::make_shared<QTimer>();
    data.timer->setSingleShot(true);
    data.timer->setProperty("id", rowCount());
    connect(data.timer.get(), &QTimer::timeout, this, &BidsModel::onTimeout);

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    _data.push_back(data);
    endInsertRows();

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
    const auto &data = _data.at(id);

}
