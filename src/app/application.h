#pragma once

#include <QtWidgets/QApplication>
#include <QNetworkAccessManager>

#include <memory>

class Application: public QApplication
{
    Q_OBJECT
public:
    explicit Application(int &argc, char **argv);

private slots:
    void login();
    void loginFinished();
    void makeBid();
    void onFinished();

private:
    std::unique_ptr<QNetworkAccessManager> _nam;

};
