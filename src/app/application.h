#pragma once

#include "auctionsmodel.h"
#include "config.h"
#include "bidsmodel.h"

#include <QtWidgets/QApplication>
#include <QNetworkAccessManager>

#include <memory>

class Application: public QApplication
{
    Q_OBJECT
public:
    explicit Application(int &argc, char **argv, const std::shared_ptr<Config> &config);
    ~Application();

    AuctionsModel *auctionsModel() const { return _auctionsModel.get(); }
    BidsModel *bidsModel() const { return _bidsModel.get(); }

    static Application *instance();

public slots:
    void makeBid(int auctionId, int bid);

private slots:
    void login();
    void loginFinished();
    void onFinished();

private:
    std::shared_ptr<Config> _config;
    std::shared_ptr<QNetworkAccessManager> _nam;
    std::unique_ptr<AuctionsModel> _auctionsModel;
    std::unique_ptr<BidsModel> _bidsModel;


};
