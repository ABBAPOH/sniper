#include "aucinfoloader.h"
#include "utils.h"

#include <QtWebKit/QWebElement>
#include <QtWebKitWidgets/QWebFrame>

AucInfoLoader::AucInfoLoader(QObject *parent) :
    QObject(parent),
    _page(new QWebPage)
{
    auto onFrameLoadFinished = [this](QWebFrame *frame) {
        frame->setProperty("isLoaded", false);
        connect(frame, &QWebFrame::loadFinished,
                this, &AucInfoLoader::onFrameLoadFinished);
    };
    connect(_page.get(), &QWebPage::frameCreated, this, onFrameLoadFinished);
}

AucInfoLoader::~AucInfoLoader()
{
    _page.reset(); // to prevent crash when page emits finished() signal on deleting
}

std::shared_ptr<QNetworkAccessManager> AucInfoLoader::networkAccessManager() const
{
    return _manager;
}

void AucInfoLoader::setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager> &manager)
{
    if (_manager == manager)
        return;

    _manager = manager;
    _page->setNetworkAccessManager(_manager.get());
    qCDebug(aucInfoLoader) << "Network manager changed";
}

void AucInfoLoader::load(const QUrl &url)
{
    qCInfo(aucInfoLoader) << "Request to load" << url;

    _queue.push_back(url);
    processNextUrl();
}

void AucInfoLoader::processNextUrl()
{
    if (_status != Status::Idle)
        return;
    if (_queue.empty())
        return;

    auto url = _queue.front();
    _queue.pop_front();
    qCDebug(aucInfoLoader) << "Processing next url" << url;

    _status = Status::Adding;
    _page->mainFrame()->setProperty("isLoaded", false);
    _page->mainFrame()->load(url);
}

void AucInfoLoader::onFrameLoadFinished(bool ok)
{
    const auto frame = qobject_cast<QWebFrame *>(sender());
    const auto frameUrl = frame->baseUrl();

    if (!ok) {
        qCWarning(aucInfoLoader) << "Failed to load frame" << frameUrl;
        _status = Status::Idle;
        processNextUrl();
        return;
    }

    frame->setProperty("isLoaded", true);

    qCDebug(aucInfoLoader) << "Loaded frame" << frameUrl;

    if (frameUrl.path() == "/auc/auc.php") {
        const auto url = _page->mainFrame()->url();

        AucInfo info;
        if (!Utils::parseAucInfo(frame, info)) {
             qCWarning(aucInfoLoader) << "Failed to parse auc info";
             _status = Status::Idle;
             return processNextUrl();
        }

        emit loaded(url, info);
    }

    auto frames = _page->findChildren<QWebFrame *>();
    for (const auto frame : frames) {
        if (!frame->property("isLoaded").toBool())
            return; // still have loading frames
    }

    qCDebug(aucInfoLoader) << "All frames are loaded";

    _status = Status::Idle;
    processNextUrl();
}

Q_LOGGING_CATEGORY(aucInfoLoader, "sniper.aucInfoLoader");
