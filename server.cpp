#include "server.h"
#include <QDebug>
#include <QString>
#include <QHostAddress>
#include <QTcpSocket>
#include <QThread>
#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <QUrlQuery>

Server::Server(QObject *parent)
    : QObject{parent} {
    setObjectName("Server");
    pServer_ = new QTcpServer(this);
}

bool Server::Connect(int port) {
    if (pServer_->listen(QHostAddress::Any, port) == true) {
        qInfo() << "Server listening on port : " << port;

        connect(pServer_, &QTcpServer::newConnection, this, &Server::OnNewConnection);

        return true;
    }
    qInfo() << "Server Cannot listening at port : " << port;
    return false;
}

void Server::OnNewConnection() {
    pSocket_ = pServer_->nextPendingConnection();

    if (!pSocket_->waitForConnected(300)) {
        qInfo() << "[Server] Cannot connect to the server socket";
    }

    if (pSocket_ != nullptr && pSocket_->isOpen()) {

        connect(pSocket_, &QTcpSocket::readyRead, this, [this]() {

            QByteArray data = pSocket_->readAll();

            QString str_data = data;
            int message_idx = str_data.indexOf("message");

            if(message_idx != -1) {
                QString extracted_message = str_data.mid(message_idx + QString("message").length() + 1);
                qDebug() << "Received from client:" << extracted_message;
                if (extracted_message == "exit") {
                    QThread::currentThread()->quit();
                    pSocket_->deleteLater();
                    QThread::currentThread()->deleteLater();
                    QCoreApplication::quit();
                }
            }else {
                qDebug() << "Received from client:" << data;
            }
        });

        connect(pSocket_, &QTcpSocket::disconnected, this, [this]() {
            qInfo() << "Clinet is disconnected";
        });


        QDir working_dir = QDir::current();
        working_dir.cd("../../");

        QString html_file_path =  working_dir.absolutePath() + QDir::separator() + "index.html";

        QFile file(html_file_path);
        if (file.open(QFile::ReadOnly)) {
            QByteArray data = file.readAll();
            file.close();

            QString httpResponse =
                "HTTP/1.1 2 00 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " + QByteArray::number(data.size()) + "\r\n"
                                                    "Connection: close\r\n\r\n" +
                data;

            pSocket_->write(httpResponse.toUtf8());
            pSocket_->flush();
        }else{
            qInfo() << "Cannot Open Html file. Error messasge: " << file.errorString();
        }

        return;
    }
    qInfo() << "There is no pending connection";
}

bool Server::SendMessageToClient(QString msg) {
    if(pSocket_ != nullptr && pSocket_->isOpen()) {
        pSocket_->write(msg.toUtf8());
        pSocket_->flush(); // Send data immediately
        return true;
    }
    qInfo() << "Socket is not Oppend";
    return false;
}

Server::~Server() {
    pServer_->close();
    qInfo() << "Connection is closed";
    delete pServer_;
}

