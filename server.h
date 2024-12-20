#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>
class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QString html_page_path, QObject *parent = nullptr);
    bool Connect(int port);
    bool SendMessageToClient(QString msg);
    ~Server();
public slots:
    void OnNewConnection();
    void OnReadyRead();
private:
    QTcpServer *server_;
    QTcpSocket *socket_;
    QString html_page_path_;
    void SendHtmlFile();
    QString GetPostedMessage(QString data);
};

#endif // SERVER_H
