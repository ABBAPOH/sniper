#include "application.h"
#include "loginmanager.h"
#include "logindialog.h"
#include "mainwindow.h"

#include <QtNetwork/QNetworkReply>

#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QSystemTrayIcon>

#include <QtGui/QIcon>

#include <QtCore/QUrlQuery>

static std::unique_ptr<QProgressDialog> createProgressDialog()
{
    std::unique_ptr<QProgressDialog> dialog(new QProgressDialog());
    dialog->setWindowTitle(Application::tr("Logging in"));
    dialog->setMinimum(0);
    dialog->setMaximum(0);
    dialog->show();
    QObject::connect(dialog.get(), &QProgressDialog::canceled, qApp, &QCoreApplication::quit);
    return dialog;
}

static Application *_instance = nullptr;

Application::Application(int& argc, char** argv, const std::shared_ptr<Config> &config) :
    QApplication(argc, argv),
    _config(config),
    _nam(new QNetworkAccessManager()),
    _loginManager(std::make_shared<LoginManager>(config)),
    _auctionsModel(std::make_shared<AuctionsModel>()),
    _bidsModel(std::make_shared<BidsModel>()),
    _systemTray(new QSystemTrayIcon())
{
    _instance = this;

    _auctionsModel->setNetworkAccessManager(_nam);
    _bidsModel->infoLoader().setNetworkAccessManager(_nam);
    _loginManager->setNetworkAccessManager(_nam);

    _systemTray->setIcon(QIcon(":/sniper.png"));
    _systemTray->setToolTip(tr("Sniper"));

    connect(_systemTray.get(), &QSystemTrayIcon::activated,
            this, &Application::onTrayActivated);
}

Application::~Application()
{
    _instance = nullptr;
}

const std::shared_ptr<LoginManager> &Application::loginManager() const
{
    return _loginManager;
}

Application *Application::instance()
{
    if (!_instance)
        qFatal("Must construct Application object before calling instance() method");

    return _instance;
}

int Application::exec()
{
    _progressDialog = createProgressDialog();

    QObject::connect(_loginManager.get(), &LoginManager::loginChecked,
                     this, &Application::onLoginChecked);

    _loginManager->checkLogin();
    _systemTray->show();

    return QApplication::exec();
}

void Application::makeBid(int auctionId, int bid)
{
    const auto urls = _config->data()["urls"].toMap();
    const auto keys = _config->data()["keys"].toMap().value("makebid").toMap();

    QUrlQuery postData;
    postData.addQueryItem(keys["auc_id"].toString(), QString::number(auctionId));
    postData.addQueryItem(keys["bid"].toString(), QString::number(bid));

    QNetworkRequest request(QUrl(urls["makebid"].toString()));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
        "application/x-www-form-urlencoded");

    auto reply = _nam->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, &Application::onFinished);
}

bool Application::event(QEvent *e)
{
    if (e->type() == QEvent::ApplicationStateChange) {
        if (applicationState() == Qt::ApplicationActive) {
            if (_mainWindow)
                _mainWindow->show();
        }
    }
    return QApplication::event(e);
}

void Application::onFinished()
{
    auto reply = qobject_cast<QNetworkReply *>(sender());
    Q_ASSERT(reply);

    qDebug() << reply->error();
    qDebug() << reply->errorString();
    qDebug() << QString::fromUtf8(reply->readAll());
}

void Application::onLoginDialogAccepted()
{
    _loginManager->login(_loginDialog->login(), _loginDialog->password());
    _progressDialog->show();
    _loginDialog.reset();
}

void Application::showLoginDialog()
{
    if (!_loginDialog) {
        _loginDialog = std::unique_ptr<LoginDialog>(new LoginDialog());
        QObject::connect(_loginDialog.get(), &QDialog::rejected,
                         this, &QCoreApplication::quit);
        QObject::connect(_loginDialog.get(), &QDialog::accepted,
                         this, &Application::onLoginDialogAccepted);
    }
    _loginDialog->setLogin(_loginManager->lastUsedLogin());
    _loginDialog->show();
}

void Application::onLoginChecked(bool logined)
{
    _progressDialog->hide();
    if (logined) {
        _progressDialog.reset();

        _auctionsModel->update();
        _mainWindow = std::unique_ptr<MainWindow>(new MainWindow());
        _mainWindow->setAuctionsModel(_auctionsModel);
        _mainWindow->setBidsModel(_bidsModel);
        _mainWindow->show();
        setQuitOnLastWindowClosed(false);
    } else {
        showLoginDialog();
    }
}

void Application::onTrayActivated()
{
    if (_mainWindow)
        _mainWindow->show();
}
