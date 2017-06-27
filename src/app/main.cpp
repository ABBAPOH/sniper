#include "application.h"

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    Application app(argc, argv);

    MainWindow w;

    w.setModel(app.model());
    w.show();

    return app.exec();
}
