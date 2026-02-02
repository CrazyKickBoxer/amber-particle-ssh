#pragma once

#include <QObject>
#include <QString>
#include <libssh2.h>
#include <memory>
#include <netinet/in.h>

class SshClient : public QObject
{
    Q_OBJECT

public:
    explicit SshClient(QObject* parent = nullptr);
    ~SshClient();

    struct PortForwardRule {
        enum Type { Local, Remote, Dynamic };
        Type type;
        int bindPort;
        QString targetHost;
        int targetPort;
    };

    void connectToHost(const QString& host, int port, const QString& user, const QString& password, const QString& keyPath = "");
    void addLocalForward(int localPort, const QString& targetHost, int targetPort); // Setup tunnel
    void disconnectFromHost();
    void sendData(const QByteArray& data);
    void setPtySize(int cols, int rows); // Update remote PTY size
    void poll(); // Call periodically
    bool isConnected() const { return m_isConnected; }

signals:
    void connected();
    void disconnected();
    void dataReceived(QByteArray data);
    void errorOccurred(QString message);
    void debugMessage(QString message); // New signal

private:
    void logError(const QString& ctx);
    int m_socket;
    LIBSSH2_SESSION* m_session;
    LIBSSH2_CHANNEL* m_channel;
    bool m_isConnected;
    
    QList<class PortForwarder*> m_activeForwarders;
    QList<class QTcpServer*> m_forwardServers;
};
