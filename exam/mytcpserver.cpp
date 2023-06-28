#include "mytcpserver.h"
#include <QDebug>
#include <QCoreApplication>

MyTcpServer::~MyTcpServer()
{
    mTcpServer->close();
}

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if (!mTcpServer->listen(QHostAddress::Any, 33333)) {
        qDebug() << "Сервер не запущен";
    } else {
        qDebug() << "Сервер запущен";
    }
}

void MyTcpServer::slotNewConnection()
{
    QTcpSocket *clientSocket = mTcpServer->nextPendingConnection();
    mClientSockets.append(clientSocket);

    connect(clientSocket, &QTcpSocket::readyRead, this, &MyTcpServer::slotServerRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);

    clientSocket->write("Привет! Я сервер эхо!\r\n");
}

void MyTcpServer::slotServerRead()
{
    QTcpSocket *senderSocket = qobject_cast<QTcpSocket*>(sender());
    QByteArray data = senderSocket->readAll();

    // Отправляем сообщение всем клиентам без изменений
    for (QTcpSocket *clientSocket : mClientSockets) {
        if (clientSocket != senderSocket) {
            clientSocket->write(data);
        }
    }

    // Отправляем каждую третью букву клиентам в отдельном сообщении
    QByteArray modifiedData;
    for (int i = 2; i < data.size(); i += 3) {
        modifiedData.append(data[i]);
        if (i < data.size() - 1) {
            modifiedData.append(' ');
        }
    }

    for (QTcpSocket *clientSocket : mClientSockets) {
        clientSocket->write(modifiedData);
    }
}

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    mClientSockets.removeOne(clientSocket);
    clientSocket->deleteLater();
}
