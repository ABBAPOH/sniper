#include "aucinfoloader.h"
#include "utils.h"

#include <QtWebKit/QWebElement>
#include <QtWebKitWidgets/QWebFrame>

#include <QtCore/QUrlQuery>

AucInfoLoader::AucInfoLoader(QObject *parent) :
    QObject(parent),
    _page(new QWebPage)
{
    connect(_page.get(), &QWebPage::loadFinished, this, &AucInfoLoader::loadFinished);
}

AucInfoLoader::~AucInfoLoader()
{
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
}

void AucInfoLoader::load(const QUrl &url)
{
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
    _status = Status::Adding;
    _page->mainFrame()->setUrl(url);
}

void AucInfoLoader::loadFinished(bool ok)
{
    auto url = _queue.front();
    _status = Status::Idle;
    _queue.pop_front();
    qDebug() << "loadFinished()" << ok;

    Info info;
    info.url = url;

    qDebug() << _page->mainFrame()->url();
    for (auto frame : _page->mainFrame()->childFrames()) {
        const auto url = frame->baseUrl();
        if (url.path() == "/auc/auc.php") {
            info.aucId = QUrlQuery(url).queryItemValue("id").toInt();

            auto body = frame->findFirstElement("body");

            const char *constLines[] = {
                "Текущая ставка, рубли: ",
                "Шаг: ",
                "До окончания аукциона: "
            };

            auto lines = body.toPlainText().split("\n", QString::SkipEmptyParts);
            for (const auto &line : lines) {
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

    emit loaded(info);

    processNextUrl();
}
