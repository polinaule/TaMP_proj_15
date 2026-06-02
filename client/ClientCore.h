#ifndef CLIENTCORE_H
#define CLIENTCORE_H

#include <QObject>
#include <QTcpSocket>
#include <QString>

class ClientCore : public QObject {
    Q_OBJECT
public:
    static ClientCore& getInstance();
    void sendCommand(const QString& cmd);
    QString getLastResponse() const { return lastResponse; }
    void setCurrentRole(const QString& role);
    void setCurrentLogin(const QString& login);
    QString getCurrentRole() const;
    QString getCurrentLogin() const;
    bool isAdmin() const;
    void setCurrentPassword(const QString& password);
    QString getCurrentPassword() const;

signals:
    void responseReceived(const QString& response);
    void errorOccurred(const QString& error);

private:
    ClientCore();
    ~ClientCore();
    ClientCore(const ClientCore&) = delete;
    ClientCore& operator=(const ClientCore&) = delete;

    QTcpSocket socket;
    QString lastResponse;
    void onConnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError);
    QString currentRole;
    QString currentLogin;
    QString currentPassword;
};

#endif
