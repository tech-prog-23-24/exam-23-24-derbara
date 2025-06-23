#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QObject>

class Server : public QTcpServer {
    Q_OBJECT

public:
    explicit Server(QObject* parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QMap<QTcpSocket*, QString> clients;

    void handleCommand(QTcpSocket* socket, const QString& message);
    void broadcastClientCount();
    void sendPrivateMessage(QTcpSocket* sender, const QString& targetNick, const QString& message);
};

#endif // SERVER_H
