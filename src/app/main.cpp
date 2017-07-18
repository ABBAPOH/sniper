#include "application.h"
#include "config.h"
#include "loghandler.h"
#include "loginmanager.h"
#include "logindialog.h"
#include "mainwindow.h"

#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

#include <QtWidgets/QProgressDialog>

QProgressDialog *createProgressDialog()
{
    const auto dialog = new QProgressDialog();
    dialog->setWindowTitle(Application::tr("Logging in"));
    dialog->setMinimum(0);
    dialog->setMaximum(0);
    dialog->show();
    QObject::connect(dialog, &QProgressDialog::canceled, qApp, &QCoreApplication::quit);
    return dialog;
}

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("Sniper");

    QFileInfo info(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!info.exists()) {
        if (!info.dir().mkpath(info.baseName()))
            qCritical() << "Can't create a data location path" << info.absoluteFilePath();
        return -1;
    }

    LogHandler logger;

    const auto cfg = std::make_shared<Config>();
    if (!cfg->load())
        return -1;

    Application app(argc, argv, cfg);

    const auto proggressDialog = createProgressDialog();

    MainWindow w;

    w.setAuctionsModel(app.auctionsModel());
    w.setBidsModel(app.bidsModel());

    auto onLoginChecked = [&app, &w, proggressDialog](bool logined)
    {
        proggressDialog->hide();
        if (logined) {
            delete proggressDialog;
            app.auctionsModel()->update();
            w.show();
        } else {
            const auto dialog = new LoginDialog();
            auto onAccepted = [&app, proggressDialog, dialog]()
            {
                app.loginManager()->login(dialog->login(), dialog->password());
                proggressDialog->show();
                delete dialog;
            };

            QObject::connect(dialog, &QDialog::rejected, &app, &QCoreApplication::quit);
            QObject::connect(dialog, &QDialog::accepted, onAccepted);
            dialog->show();
        }
    };
    QObject::connect(app.loginManager().get(), &LoginManager::loginChecked, onLoginChecked);

    app.loginManager()->checkLogin();

    return app.exec();
}
