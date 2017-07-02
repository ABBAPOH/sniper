#include "aucinfoloader.h"
#include "utils.h"

#include <QtWebKit/QWebElement>
#include <QtWebKitWidgets/QWebFrame>

#include <QtCore/QUrlQuery>

static const char aboutBlankUrl[] = "about:blank";

AucInfoLoader::AucInfoLoader(QObject *parent) :
    QObject(parent),
    _page(new QWebPage)
{
    connect(_page.get(), &QWebPage::loadFinished, this, &AucInfoLoader::loadFinished);
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

    qCDebug(aucInfoLoader) << "Processing next url" << url;

    _status = Status::Adding;
    _page->mainFrame()->load(url);
}

void AucInfoLoader::loadFinished(bool ok)
{
    if (!_page)
        return;

    if (_page->mainFrame()->url() == QUrl(aboutBlankUrl))
        return;

    auto url = _queue.front();
    _status = Status::Idle;
    _queue.pop_front();

    if (!ok) {
        qCWarning(aucInfoLoader) << "Failed to load page" << url;
        processNextUrl();
        return;
    }

    qCInfo(aucInfoLoader) << "Load finished";

    Info info;
    info.url = url;

    for (auto frame : _page->mainFrame()->childFrames()) {
        const auto url = frame->baseUrl();
        if (url.path() == "/auc/auc.php") {
            info.aucId = QUrlQuery(url).queryItemValue("id").toInt();

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
                    info.bid = line.mid(QString(constLines[0]).length()).toInt();
                } else if (line.startsWith(constLines[1])) {
                    info.step = line.mid(QString(constLines[1]).length()).toInt();
                } else if (line.startsWith(constLines[2])) {
                    info.duration = Utils::parseDuration(line.mid(QString(constLines[2]).length()));
                }
            }

            break;
        }
    }

    _page->mainFrame()->setUrl(QUrl(aboutBlankUrl));

    emit loaded(info);

    processNextUrl();
}

Q_LOGGING_CATEGORY(aucInfoLoader, "sniper.aucInfoLoader");
