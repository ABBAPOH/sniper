#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString login READ login WRITE setLogin)
    Q_PROPERTY(QString password READ password WRITE setPassword)

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();

    QString login() const;
    void setLogin(const QString &login);

    QString password() const;
    void setPassword(const QString &password);

private:
    Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
