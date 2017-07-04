#include "application.h"
#include "loghandler.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    LogHandler logger;

    Application app(argc, argv);

    MainWindow w;

    w.setAuctionsModel(app.auctionsModel());
    w.setBidsModel(app.bidsModel());
    w.show();

    return app.exec();
}
