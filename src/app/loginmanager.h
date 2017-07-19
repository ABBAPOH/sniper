#pragma once

#include "config.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>

#include <memory>

class QNetworkAccessManager;
class QWebPage;

class LoginManager : public QObject
{
    Q_OBJECT
public:
    explicit LoginManager(const std::shared_ptr<Config> &config, QObject *parent = nullptr);
    ~LoginManager();

    std::shared_ptr<QNetworkAccessManager> networkAccessManager() const;
    void setNetworkAccessManager(const std::shared_ptr<QNetworkAccessManager> &manager);

    QString lastUsedLogin() const;

public slots:
    void checkLogin();
    void login(const QString &login, const QString &password);

signals:
    void loginChecked(bool logined);

private:
    void pageLoaded(bool ok);
    void onLoginFinished();

private:
    std::shared_ptr<Config> _config;
    std::shared_ptr<QNetworkAccessManager> _manager;
    std::unique_ptr<QWebPage> _page;
};

Q_DECLARE_LOGGING_CATEGORY(loginManager);
