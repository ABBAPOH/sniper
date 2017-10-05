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

    AucInfo info;
    info.aucId = QUrlQuery(frameUrl).queryItemValue("id").toInt();

    if (!Utils::parseAucInfo(frame, info)) {
         qCWarning(fastAucInfoLoader) << "Failed to parse auc info";
         _status = Status::Idle;
         return processNextUrl();
    }

    emit loaded(info.aucId, info);

    _status = Status::Idle;
    processNextUrl();
}


Q_LOGGING_CATEGORY(fastAucInfoLoader, "sniper.fastAucInfoLoader");
