#include "auctionsmodel.h"

#include <QtCore/QDebug>

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtWebKit/QWebElement>
#include <QtWebKitWidgets/QWebFrame>

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

QVariant AuctionsModel::data(const QModelIndex& index, int role) const
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
    beginResetModel();
    auto table = _page->mainFrame()->findFirstElement("table[class=reftable]");
    auto body = table.findFirst("tbody");
    for (auto tr = body.firstChild().nextSibling(); tr != body.lastChild(); tr = tr.nextSibling()) {
        QStringList rawData;
        for (auto td = tr.firstChild(); td != tr.lastChild(); td = td.nextSibling()) {
            rawData.append(td.toPlainText());
        }
        Data d;
        d.lot = rawData[0];
        d.seller = rawData[1];
        d.shipping = rawData[2];
        d.duration = rawData[3];
        d.bid = rawData[4];
        _data.push_back(d);
    }
    endResetModel();
}
