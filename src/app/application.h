#pragma once

#include "auctionsmodel.h"

#include <QtWidgets/QApplication>
#include <QNetworkAccessManager>

#include <memory>

class Application: public QApplication
{
    Q_OBJECT
public:
    explicit Application(int &argc, char **argv);

    AuctionsModel *model() const { return _model.get(); }

private slots:
    void login();
    void loginFinished();
    void makeBid();
    void onFinished();

private:
    std::shared_ptr<QNetworkAccessManager> _nam;
    std::unique_ptr<AuctionsModel> _model;

};
