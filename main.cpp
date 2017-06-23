#include <QCoreApplication>
#include <QDebug>

#include "class.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "123";

    auto c = std::make_unique<Class>();

    return a.exec();
}
