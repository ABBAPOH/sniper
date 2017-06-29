#include "auctionsmodel.h"

#include <QtCore/QDebug>

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtWebKit/QWebElement>
#include <QtWebKitWidgets/QWebFrame>

#include <QRegularExpression>

AuctionsModel::AuctionsModel(QObject* parent) :
    QAbstractTableModel(parent),
    _page(new QWebPage)
{
    connect(_page.get(), &QWebPage::loadFinished, this, &AuctionsModel::loadFinished);
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

QString getAucDuration(const QDateTime &current, const QDateTime &end)
{
    const auto msecs = current.msecsTo(end);
    const auto days = msecs / 1000 / 60 / 60 / 24;
    const auto hours = int((msecs / 1000 / 60 / 60) - days * 24);
    const auto minutes = int((msecs / 1000 / 60) - (days * 24 + hours) * 60);
    const auto secs = int((msecs / 1000) - ((days * 24 + hours) * 60 + minutes) * 60);
    return AuctionsModel::tr("%1:%2").arg(days).arg(QTime(hours, minutes, secs).toString("hh:mm:ss"));
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
            return getAucDuration(currentDateTime, data.end);
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

    _page->mainFrame()->setUrl(QUrl("http://topdeck.ru/auc/aucs.php"));
}

void AuctionsModel::loadFinished()
{
    QRegularExpression expr("((\\d+) д. ?)?((\\d+) ч. ?)?((\\d+) мин. ?)?((\\d+) с.)?$");

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
        d.duration = rawData[3];
        if (d.duration == "Завершен")
            continue;
        auto match = expr.match(d.duration);
        if (match.isValid()) {
            const auto days = match.captured(2);
            const auto hours = match.captured(4);
            const auto mins = match.captured(6);
            const auto secs = match.captured(8);
            QTime time(hours.toInt(), mins.toInt(), secs.toInt());
            QDateTime dateTime = QDateTime::currentDateTimeUtc();
            dateTime = dateTime.addDays(days.toInt());
            dateTime = dateTime.addMSecs(time.msecsSinceStartOfDay());
            d.end = dateTime;
        }
        d.bid = rawData[4].toInt();
        _data.push_back(d);
    }
    endResetModel();
}
