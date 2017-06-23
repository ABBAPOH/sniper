#pragma once

#include <QNetworkAccessManager>

#include <memory>

class Class: public QObject
{
    Q_OBJECT
public:
    Class();

private slots:
    void login();
    void loginFinished();
    void makeBid();
    void onFinished();

private:
    std::unique_ptr<QNetworkAccessManager> _nam;

};
