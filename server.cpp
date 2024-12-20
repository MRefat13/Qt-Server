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

Server::Server(QString html_page_path, QObject *parent)
    : QObject{parent} {
    setObjectName("Server");
    server_ = new QTcpServer(this);
    html_page_path_ = html_page_path;
}

bool Server::Connect(int port) {
    //-- 1] Listent to a specific post and for any IP
    if (server_->listen(QHostAddress::Any, port) == true) {
        qInfo() << "Server listening on port : " << port;
        //-- 2]  Connect the `newConnection` signal to the `OnNewConnection` slot of the `Server`. 
        connect(server_, &QTcpServer::newConnection, this, &Server::OnNewConnection);

        return true;
    }
    qInfo() << "Server Cannot listening at port : " << port;
    return false;
}

void Server::OnNewConnection() {
    //-- 1] Get the created socket & check if there is a socket has been created
    socket_ = server_->nextPendingConnection();
    if (socket_ != nullptr && socket_->isOpen()) {
        //-- 2] // Connect the `readyRead` signal to the `OnReadyRead` slot of the `Server`.  
        connect(socket_, &QTcpSocket::readyRead, this, &Server::OnReadyRead);

        //-- 3] Connect the `disconnected` signal of the socket to a lambda function. 
        connect(socket_, &QTcpSocket::disconnected, this, [this]() {
            qInfo() << "Client is disconnected";
        });

        return;
    }
    qInfo() << "There is no pending connection";
}

void Server::OnReadyRead()  {
    //-- 1] Read all the incoming data as a byte array
    QByteArray data = socket_->readAll();

    //-- 2] Convert data to a string
    QString str_data = data;

    //-- 3] Split data and display it line by line
    QStringList str_list = str_data.split("\r\n");
    qInfo() << "\n\n----------------------------------------------------";
    qInfo() << "Server has received: \n";
    foreach (QString str, str_list) {
        qInfo() << str;
    }

    //-- 4] Parsing the incoming HTTP request

    //-- 4.1] Case the Incoming HTTP request is GET...
    if ( str_list[0] == QString("GET / HTTP/1.1")) {
        SendHtmlFile();
    }

    //-- 4.2] Case the Incoming HTTP request is POST...
    if ( str_list[0].contains("POST")) {
        QString extracted_message = GetPostedMessage(str_data);
        //-- 1] Display the extracted message
        qDebug() << "Received message from client:" << extracted_message;

        //-- 2] Check if the received message is exit
        if (extracted_message == "exit") {
            //-- 2.1] Quit From the App
            socket_->deleteLater();
            QCoreApplication::quit();
        }

        //-- 3] Send acknowledge to the client
        QString http_response = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n";
        SendMessageToClient(http_response);
        socket_->disconnectFromHost();
    }
    qInfo() << "----------------------------------------------------\n\n";
}

void Server::SendHtmlFile() {
    //-- 1] Create a QFile to handle the html file
    QFile file(html_page_path_);

    //-- 2] Check if we can open the file (IF file exists)
    if (file.open(QFile::ReadOnly)) {
        //-- 3] Read the content of the file as bytes
        QByteArray data = file.readAll();

        //-- 4] Construct the response
        QString http_response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + QByteArray::number(data.size()) + "\r\n"
                                                "Connection: close\r\n\r\n" +
            data;

        //-- 5] Send HTTP response to the client
        SendMessageToClient(http_response);

        //-- 6] Close the file
        file.close();
    }else{
        qInfo() << "Cannot Open Html file. Error message: " << file.errorString();
    }
}

QString Server::GetPostedMessage(QString data) {
    QString extracted_message = "";
    //-- 1]  Get Index of the received message and check whether the received data contain message or not
    int message_idx = data.indexOf("message");
    if (message_idx > -1) {
        //-- 2] Extract the received message
        extracted_message = data.mid(message_idx + QString("message").length() + 1);
    }
    return extracted_message;
}

bool Server::SendMessageToClient(QString msg) {
    //-- Ensure that the socket is created and oppend
    if(socket_ != nullptr && socket_->isOpen()) {
        socket_->write(msg.toUtf8());
        //-- Flush(write) data immediately
        socket_->flush();
        return true;
    }
    qInfo() << "Socket is not Oppend";
    return false;
}

Server::~Server() {
    server_->close();
    qInfo() << "Connection is closed";
    delete server_;
}

