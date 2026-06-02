#include "ClientCore.h"
#include <QDataStream>

ClientCore::ClientCore() {
    connect(&socket, &QTcpSocket::connected, this, &ClientCore::onConnected);
    connect(&socket, &QTcpSocket::readyRead, this, &ClientCore::onReadyRead);
    connect(&socket, &QTcpSocket::errorOccurred, this, &ClientCore::onError);
}

ClientCore::~ClientCore() {
    if (socket.state() == QAbstractSocket::ConnectedState)
        socket.disconnectFromHost();
}

ClientCore& ClientCore::getInstance() {
    static ClientCore instance;
    return instance;
}

void ClientCore::sendCommand(const QString& cmd) {
    if (socket.state() != QAbstractSocket::ConnectedState) {
        socket.connectToHost("localhost", 12345);
        // Ждём подключения (можно добавить цикл ожидания или использовать события)
        if (!socket.waitForConnected(3000)) {
            emit errorOccurred("Connection failed");
            return;
        }
    }
    QByteArray data = cmd.toUtf8() + '\n';
    socket.write(data);
    socket.flush();
}

void ClientCore::onConnected() {
    // Можно ничего не делать
}

void ClientCore::onReadyRead() {
    QByteArray data = socket.readAll();
    lastResponse = QString::fromUtf8(data).trimmed();
    emit responseReceived(lastResponse);
    socket.disconnectFromHost(); // сервер закрывает соединение после ответа
}

void ClientCore::onError(QAbstractSocket::SocketError) {
    emit errorOccurred(socket.errorString());
}
