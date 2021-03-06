#pragma once

#include "auctionsmodel.h"
#include "config.h"
#include "cookiejar.h"
#include "bidsmodel.h"

#include <QtWidgets/QApplication>
#include <QNetworkAccessManager>

#include <memory>

class LoginManager;
class LoginDialog;
class MainWindow;
class QProgressDialog;
class QSystemTrayIcon;

class Application: public QApplication
{
    Q_OBJECT
public:
    explicit Application(int &argc, char **argv, const std::shared_ptr<Config> &config);
    ~Application();

    const std::shared_ptr<LoginManager> &loginManager() const;
    const std::shared_ptr<AuctionsModel> &auctionsModel() const { return _auctionsModel; }
    const std::shared_ptr<BidsModel> bidsModel() const { return _bidsModel; }

    static Application *instance();

    int exec();

public slots:
    void makeBid(int auctionId, int bid, const QString& csrfName, const QString& csrfValue);

protected:
    bool event(QEvent *e);

private:
    struct Options
    {
        bool dryRun {false};
    };

    bool parseOptions(const QStringList& args);
    void onFinished();
    void onLoginDialogAccepted();
    void showLoginDialog();
    void onLoginChecked(bool logined);
    void onLoginError();
    void onTrayActivated();

private:
    std::shared_ptr<Config> _config;
    Options _options;
    std::shared_ptr<QNetworkAccessManager> _nam;
    std::shared_ptr<CookieJar> _cookieJar;
    std::shared_ptr<LoginManager> _loginManager;
    std::shared_ptr<AuctionsModel> _auctionsModel;
    std::shared_ptr<BidsModel> _bidsModel;
    std::unique_ptr<QProgressDialog> _progressDialog;
    std::unique_ptr<LoginDialog> _loginDialog;
    std::unique_ptr<MainWindow> _mainWindow;
    std::unique_ptr<QSystemTrayIcon> _systemTray;
};
