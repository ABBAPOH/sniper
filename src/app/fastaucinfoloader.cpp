#include "fastaucinfoloader.h"

#include "utils.h"

#include <QtWebKit/QWebElement>
#include <QtWebKitWidgets/QWebFrame>

#include <QtCore/QUrlQuery>

FastAucInfoLoader::FastAucInfoLoader(QObject *parent) :
    QObject(parent),
    _page(new QWebPage)
{
    connect(_page.get(), &QWebPage::loadFinished, this, &FastAucInfoLoader::onLoadFinished);
}

FastAucInfoLoader::~FastAucInfoLoader()
{
    _page.reset();
}

std::shared_ptr<QNetworkAccessManager> FastAucInfoLoader::networkAccessManager() const
{
    return _manager;
}

void FastAucInfoLoader::setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager>& manager)
{
    if (_manager == manager)
        return;

    _manager = manager;
    _page->setNetworkAccessManager(_manager.get());
    qCDebug(fastAucInfoLoader) << "Network manager changed";
}

void FastAucInfoLoader::load(int auc_id)
{
    QUrl url(QString("http://topdeck.ru/auc/auc.php?id=%1").arg(auc_id));
    qCInfo(fastAucInfoLoader) << "Request to load" << url;

    _queue.push_back(url);
    processNextUrl();
}

void FastAucInfoLoader::processNextUrl()
{
    if (_status != Status::Idle)
        return;
    if (_queue.empty())
        return;

    auto url = _queue.front();
    _queue.pop_front();
    qCDebug(fastAucInfoLoader) << "Processing next url" << url;

    _status = Status::Adding;
    _page->mainFrame()->load(url);
}

void FastAucInfoLoader::onLoadFinished(bool ok)
{
    const auto frame = _page->mainFrame();
    const auto frameUrl = frame->baseUrl();

    if (!ok) {
        qCWarning(fastAucInfoLoader) << "Failed to load frame" << frameUrl;
        _status = Status::Idle;
        processNextUrl();
        return;
    }

    Info info;
    info.aucId = QUrlQuery(frameUrl).queryItemValue("id").toInt();

    auto body = frame->findFirstElement("body");

    const char *constLines[] = {
        "Текущая ставка, рубли: ",
        "Шаг: ",
        "До окончания аукциона: ",
        "Аукцион завершен."
    };

    auto lines = body.toPlainText().split("\n", QString::SkipEmptyParts);
    for (const auto &line : lines) {
        if (line.contains(constLines[3])) {
            info.ended = true;
            info.duration = -1;
            info.step = -1;
            info.bid = -1;
            break;
        }
        if (line.startsWith(constLines[0])) {
            const auto subLine = line.mid(QString(constLines[0]).length());
            info.bid = subLine.split(" ").at(0).toInt();
        } else if (line.startsWith(constLines[1])) {
            info.step = line.mid(QString(constLines[1]).length()).toInt();
        } else if (line.startsWith(constLines[2])) {
            info.duration = Utils::parseDuration(line.mid(QString(constLines[2]).length()));
        }
    }

    emit loaded(info.aucId, info);

    _status = Status::Idle;
    processNextUrl();
}


Q_LOGGING_CATEGORY(fastAucInfoLoader, "sniper.fastAucInfoLoader");
