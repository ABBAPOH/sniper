#include "application.h"

#include "loginmanager.h"

#include <QNetworkReply>
#include <QUrlQuery>

static Application *_instance = nullptr;

Application::Application(int& argc, char** argv, const std::shared_ptr<Config> &config) :
    QApplication(argc, argv),
    _config(config),
    _nam(new QNetworkAccessManager()),
    _loginManager(std::make_shared<LoginManager>(config)),
    _auctionsModel(new AuctionsModel()),
    _bidsModel(new BidsModel())
{
    _instance = this;

    _auctionsModel->setNetworkAccessManager(_nam);
    _bidsModel->infoLoader().setNetworkAccessManager(_nam);
    _loginManager->setNetworkAccessManager(_nam);
}

Application::~Application()
{
    _instance = nullptr;
}

const std::shared_ptr<LoginManager> &Application::loginManager() const
{
    return _loginManager;
}

Application *Application::instance()
{
    if (!_instance)
        qFatal("Must construct Application object before calling instance() method");

    return _instance;
}

void Application::makeBid(int auctionId, int bid)
{
    const auto urls = _config->data()["urls"].toMap();
    const auto keys = _config->data()["keys"].toMap().value("login").toMap();

    QUrlQuery postData;
    postData.addQueryItem(keys["auc_id"].toString(), QString::number(auctionId));
    postData.addQueryItem(keys["bid"].toString(), QString::number(bid));

    QNetworkRequest request(QUrl(urls["makebid"].toString()));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
        "application/x-www-form-urlencoded");

    auto reply = _nam->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, &Application::onFinished);
}

void Application::onFinished()
{
    auto reply = qobject_cast<QNetworkReply *>(sender());
    Q_ASSERT(reply);

    qDebug() << reply->error();
    qDebug() << reply->errorString();
    qDebug() << QString::fromUtf8(reply->readAll());
}

