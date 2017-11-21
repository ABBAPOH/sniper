#include "aucinfoloader.h"

#include "utils.h"

#include <QtWebKit/QWebElement>
#include <QtWebKitWidgets/QWebFrame>

#include <QtCore/QFileInfo>
#include <QtCore/QUrlQuery>

AucInfoLoader::AucInfoLoader(const std::shared_ptr<Config>& config, QObject *parent) :
    QObject(parent),
    _page(new QWebPage)
{
    _urlTemplate = config->value("urls/aucTemplate").toString();
    connect(_page.get(), &QWebPage::loadFinished, this, &AucInfoLoader::onLoadFinished);
}

AucInfoLoader::~AucInfoLoader()
{
    _page.reset();
}

std::shared_ptr<QNetworkAccessManager> AucInfoLoader::networkAccessManager() const
{
    return _manager;
}

void AucInfoLoader::setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager>& manager)
{
    if (_manager == manager)
        return;

    _manager = manager;
    _page->setNetworkAccessManager(_manager.get());
    qCDebug(fastAucInfoLoader) << "Network manager changed";
}

void AucInfoLoader::load(int auc_id)
{
    QUrl url(_urlTemplate.arg(auc_id));
    qCInfo(fastAucInfoLoader) << "Request to load" << url;

    _queue.push_back(url);
    if (_status == Status::Idle)
        processNextUrl();
}

void AucInfoLoader::processNextUrl()
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

void AucInfoLoader::onLoadFinished(bool ok)
{
    const auto frame = _page->mainFrame();
    const auto frameUrl = frame->baseUrl();

    if (!ok) {
        qCWarning(fastAucInfoLoader) << "Failed to load frame" << frameUrl;
        return processNextUrl();
    }

    const auto name = frame->findFirstElement("[name=csrf_name]");
    if (name.isNull()) {
        qCWarning(fastAucInfoLoader) << "Can't find csrf_name element";
        return processNextUrl();
    }
    const auto value = frame->findFirstElement("[name=csrf_value]");
    if (value.isNull()) {
        qCWarning(fastAucInfoLoader) << "Can't find csrf_value element";
        return processNextUrl();
    }

    const auto label = frame->findFirstElement("[class=\"label label-default\"]");
    if (label.isNull()) {
        qCWarning(fastAucInfoLoader) << "Can't find label-default element";
    }

    AucInfo info;
    info.id = QFileInfo(frameUrl.path()).fileName().toInt();
    info.bid = label.toPlainText().toInt();
    info.csrfName = name.attribute("value");
    info.csrfValue = value.attribute("value");

    emit loaded(info.id, info);

    return processNextUrl();
}


Q_LOGGING_CATEGORY(fastAucInfoLoader, "sniper.fastAucInfoLoader");
