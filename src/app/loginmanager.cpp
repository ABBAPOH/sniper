#include "loginmanager.h"

#include <QtWebKitWidgets/QWebPage>
#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKit/QWebElement>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>

LoginManager::LoginManager(const std::shared_ptr<Config> &config, QObject *parent) :
    QObject(parent),
    _config(config),
    _page(new QWebPage)
{
    connect(_page.get(), &QWebPage::loadFinished, this, &LoginManager::pageLoaded);
}

LoginManager::~LoginManager() = default;

std::shared_ptr<QNetworkAccessManager> LoginManager::networkAccessManager() const
{
    return _manager;
}

void LoginManager::setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager> &manager)
{
    if (_manager == manager)
        return;

    _manager = manager;
    _page->setNetworkAccessManager(_manager.get());
}

QString LoginManager::lastUsedLogin() const
{
    QSettings settings;
    return settings.value("lastUsedLogin").toString();
}

void LoginManager::checkLogin()
{
    _page->mainFrame()->load(QUrl("about:blank"));
    _page->mainFrame()->load(QUrl(_config->value("urls/index.php").toString()));
}

void LoginManager::login(const QString &login, const QString &password)
{
    QSettings settings;
    settings.setValue("lastUsedLogin", login);

    QUrlQuery postData;

    const auto urls = _config->data()["urls"].toMap();
    const auto keys = _config->data()["keys"].toMap().value("login").toMap();

    postData.addQueryItem(keys["referer"].toString(), urls["urls/index.php"].toString());
//    postData.addQueryItem("username", "ஐWingS_OF_ButterFlyஐ");
//    postData.addQueryItem("password", "f5a-tJF-qQZ-raR");
    postData.addQueryItem(keys["loginSubmitted"].toString(), "1");
    postData.addQueryItem(keys["csrfKey"].toString(), _key);
    postData.addQueryItem(keys["username"].toString(), login);
    postData.addQueryItem(keys["password"].toString(), password);
    postData.addQueryItem(keys["rememberMe"].toString(), "0");
    postData.addQueryItem(keys["rememberMeCheckbox"].toString(), "1");

    QNetworkRequest request(QUrl(urls["login"].toString()));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
        "application/x-www-form-urlencoded");

    auto reply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, &LoginManager::onLoginFinished);
}

void LoginManager::pageLoaded(bool ok)
{
    const auto url =_page->mainFrame()->url();
    if (!ok) {
        qCWarning(loginManager) << "Can't load" << url;
        emit error();
        return;
    }

    if (url == QUrl("about:blank"))
        return;

    const auto loginForm = _page->mainFrame()->findFirstElement("[data-role=loginForm]");
    const auto userLink = _page->mainFrame()->findFirstElement("[id=elUserLink]");
    if (!loginForm.isNull()) {
        const auto key = _page->mainFrame()->findFirstElement(QString("[name=csrfKey]"));
        if (key.isNull()) {
            qWarning() << "Can't find csrfKey";
            emit error();
            return;
        }
        _key = key.attribute("value");
        emit loginChecked(false);
    } else if (!userLink.isNull()) {
        emit loginChecked(true);
    } else {
        qCWarning(loginManager) << "Can't determine if user logined";
        emit error();
    }
}

void LoginManager::onLoginFinished()
{
    checkLogin();
}

Q_LOGGING_CATEGORY(loginManager, "sniper.loginManager");
