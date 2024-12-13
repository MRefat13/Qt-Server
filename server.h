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
    explicit Server(QObject *parent = nullptr);
    bool Connect(int port);
    bool SendMessageToClient(QString msg);
    bool IsClientConnected();
    ~Server();
public slots:
    void OnNewConnection();
private:
    QTcpServer *pServer_;
    QTcpSocket *pSocket_;
    bool client_connected_;
};

#endif // SERVER_H
