#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

QString LoginDialog::login() const
{
    return ui->loginEdit->text();
}

void LoginDialog::setLogin(const QString &login)
{
    ui->loginEdit->setText(login);
}

QString LoginDialog::password() const
{
    return ui->passwordEdit->text();
}

void LoginDialog::setPassword(const QString &password)
{
    ui->passwordEdit->setText(password);
}
