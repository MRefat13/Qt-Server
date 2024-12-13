#include <QCoreApplication>
#include <QHostAddress>
#include "server.h"
#include <QThread>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server myServer;

    QThread worker_thread;
    myServer.moveToThread(&worker_thread);

    QObject::connect(&worker_thread, &QThread::started, [&]() {
        myServer.Connect(1234);
        qInfo() << "------------------------------------------";
    });

    worker_thread.start();

    return a.exec();
}
