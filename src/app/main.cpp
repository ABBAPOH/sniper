#include "application.h"
#include "config.h"
#include "loghandler.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("Sniper");
    QCoreApplication::setOrganizationDomain("sniper.abbapohr.org");

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

    return app.exec();
}
