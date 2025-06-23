#include "server.h"
#include <QDebug>
#include <QThread>

Server::Server(QObject* parent) : QTcpServer(parent) {
    if (listen(QHostAddress::Any, 1234)) {
        qDebug() << "Сервер запущен на порту 1234";
    } else {
        qDebug() << "Ошибка запуска сервера:" << errorString();
    }
}

void Server::incomingConnection(qintptr socketDescriptor) {
    if (clients.size() >= 9) {
        QTcpSocket* temp = new QTcpSocket(this);
        temp->setSocketDescriptor(socketDescriptor);

        temp->write("Сервер занят. Попробуйте позже. Нажмите Enter для выхода.\r\n");
        temp->flush();

        // Подключаем обработчик чтения: ждём, пока клиент нажмёт Enter
        connect(temp, &QTcpSocket::readyRead, temp, &QTcpSocket::disconnectFromHost);

        return;
    }

    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    clients.insert(socket, "Гость");

    socket->write("Привет! Установи ник: /nick <твой_ник>\r\n");

    connect(socket, &QTcpSocket::readyRead, this, [=]() {
        while (socket->canReadLine()) {
            QString message = QString::fromUtf8(socket->readLine()).trimmed();
            handleCommand(socket, message);
        }
    });

    connect(socket, &QTcpSocket::disconnected, this, [=]() {
        clients.remove(socket);
        socket->deleteLater();
        broadcastClientCount();
    });

    broadcastClientCount();
}

void Server::handleCommand(QTcpSocket* socket, const QString& message) {
    if (message.startsWith("/nick ")) {
        QString nick = message.section(' ', 1).trimmed();

        if (nick.isEmpty()) {
            socket->write("Ник не может быть пустым.\r\n");
        } else {
            clients[socket] = nick;
            socket->write(("Ник установлен: " + nick + "\r\n").toUtf8());
        }

    } else if (message.startsWith("/pm ")) {
        QString targetNick = message.section(' ', 1, 1).trimmed();
        QString msg = message.section(' ', 2).trimmed();
        sendPrivateMessage(socket, targetNick, msg);

    } else {
        socket->write("Неизвестная команда. Используй /nick или /pm.\r\n");
    }
}

void Server::sendPrivateMessage(QTcpSocket* sender, const QString& targetNick, const QString& message) {
    QString senderNick = clients.value(sender, "Безымянный");

    for (QTcpSocket* client : clients.keys()) {
        if (clients[client] == targetNick) {
            client->write(("Личное сообщение от " + senderNick + ": " + message + "\r\n").toUtf8());
            return;
        }
    }

    sender->write("Пользователь с таким ником не найден.\r\n");
}

void Server::broadcastClientCount() {
    QString msg = "Подключено клиентов: " + QString::number(clients.size()) + "\r\n";
    for (QTcpSocket* client : clients.keys()) {
        client->write(msg.toUtf8());
    }
}
