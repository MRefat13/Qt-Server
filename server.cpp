#include "server.h"
#include <QDebug>
#include <QString>
#include <QHostAddress>
#include <QTcpSocket>
#include <QThread>
#include <QFile>
#include <QCoreApplication>
#include <QDir>
Server::Server(QObject *parent)
    : QObject{parent} {
    setObjectName("Server");
    pServer_ = new QTcpServer(this);
    client_connected_ = false;
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
            qInfo() << "Server has recieved : ";
            QByteArray data = pSocket_->readAll();
            qInfo() << "Server has recieved : " << data.trimmed() << ", From the client";
        });

        connect(pSocket_, &QTcpSocket::disconnected, this, [this]() {
            qInfo() << "Clinet is disconnected";
            pSocket_->deleteLater();
        });

        QString current_path = QCoreApplication::applicationDirPath();
        QDir working_dir = QDir::current();
        working_dir.cd("../../");


        qInfo() << " QDir : "<< working_dir.absolutePath();
        QString index_path =  working_dir.absolutePath() + QDir::separator() + "index.html";
        qInfo() << "index path location: " << index_path;
        QFile file(index_path);
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
            qInfo() << "html has beed sent successfully";
            pSocket_->flush();
        }else{
            qInfo() << "Cannot Open Html file. Error messasge: " << file.errorString();
        }

        client_connected_ = true;
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

bool Server::IsClientConnected() {
    return client_connected_;
}

Server::~Server() {
    pServer_->close();
    qInfo() << "Connection is closed";
    delete pServer_;
}

