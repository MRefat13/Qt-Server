#include <QCoreApplication>
#include <QHostAddress>
#include "server.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server myServer("/home/mohamedrefat/Desktop/myService/index.html", &a);
    myServer.Connect(1234);
    qInfo() << "------------------------------------------";


    return a.exec();
}
