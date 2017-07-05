#include "application.h"
#include "config.h"
#include "loghandler.h"
#include "mainwindow.h"

#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

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

    MainWindow w;

    w.setAuctionsModel(app.auctionsModel());
    w.setBidsModel(app.bidsModel());
    w.show();

    return app.exec();
}
