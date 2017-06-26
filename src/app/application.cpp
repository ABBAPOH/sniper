#include "application.h"

#include <QNetworkReply>
#include <QUrlQuery>

Application::Application(int& argc, char** argv) :
    QApplication(argc, argv),
    _nam(new QNetworkAccessManager())
{
    login();
}

void Application::login()
{
    QUrlQuery postData;
    postData.addQueryItem("referer", "http://topdeck.ru/forum/index.php?");
//    postData.addQueryItem("username", "ஐWingS_OF_ButterFlyஐ");
//    postData.addQueryItem("password", "f5a-tJF-qQZ-raR");
    postData.addQueryItem("username", "ABBAPOH");
    postData.addQueryItem("password", "idkfa123");
    postData.addQueryItem("rememberMe", "1");

    QNetworkRequest request(QUrl("http://topdeck.ru/forum/index.php?app=core&module=global&section=login&do=process"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
        "application/x-www-form-urlencoded");

    auto reply = _nam->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, &Application::loginFinished);
}

void Application::loginFinished()
{
    auto reply = qobject_cast<QNetworkReply *>(sender());
    Q_ASSERT(reply);

    qDebug() << reply->error();
    qDebug() << reply->errorString();
    qDebug() << QString::fromUtf8(reply->readAll());

//    makeBid();
}

void Application::makeBid()
{
    QUrlQuery postData;
    postData.addQueryItem("auc_id", "44030");
    postData.addQueryItem("bid", "2750");

    QNetworkRequest request(QUrl("http://topdeck.ru/auc/makebid.php"));
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
