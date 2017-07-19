#pragma once

#include "auctionsmodel.h"
#include "config.h"
#include "bidsmodel.h"

#include <QtWidgets/QApplication>
#include <QNetworkAccessManager>

#include <memory>

class LoginManager;
class LoginDialog;
class MainWindow;
class QProgressDialog;

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
    void makeBid(int auctionId, int bid);

private slots:
    void onFinished();

private:
    std::shared_ptr<Config> _config;
    std::shared_ptr<QNetworkAccessManager> _nam;
    std::shared_ptr<LoginManager> _loginManager;
    std::shared_ptr<AuctionsModel> _auctionsModel;
    std::shared_ptr<BidsModel> _bidsModel;
    std::unique_ptr<QProgressDialog> _progressDialog;
    std::unique_ptr<LoginDialog> _loginDialog;
    std::unique_ptr<MainWindow> _mainWindow;
};
