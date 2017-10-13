#include "fastaucinfoloader.h"

#include "utils.h"

#include <QtWebKit/QWebElement>
#include <QtWebKitWidgets/QWebFrame>

#include <QtCore/QUrlQuery>

FastAucInfoLoader::FastAucInfoLoader(const std::shared_ptr<Config>& config, QObject *parent) :
    QObject(parent),
    _utils(config),
    _page(new QWebPage)
{
    _urlTemplate = config->value("urls/aucTemplate").toString();
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
    QUrl url(_urlTemplate.arg(auc_id));
    qCInfo(fastAucInfoLoader) << "Request to load" << url;

    _queue.push_back(url);
    if (_status == Status::Idle)
        processNextUrl();
}

void FastAucInfoLoader::processNextUrl()
{
    _status = Status::Idle;

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
        return processNextUrl();
    }

    AucInfo info;
    if (!_utils.parseAucInfo(frame, info)) {
         qCWarning(fastAucInfoLoader) << "Failed to parse auc info";
         return processNextUrl();
    }

    emit loaded(info.aucId, info);

    return processNextUrl();
}


Q_LOGGING_CATEGORY(fastAucInfoLoader, "sniper.fastAucInfoLoader");
