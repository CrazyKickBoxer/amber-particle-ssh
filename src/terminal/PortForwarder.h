#pragma once

#include <QObject>
#include <QTcpSocket>
#include <libssh2.h>
#include <QByteArray>

class PortForwarder : public QObject
{
    Q_OBJECT
public:
    PortForwarder(QTcpSocket* socket, LIBSSH2_CHANNEL* channel, QObject* parent = nullptr);
    ~PortForwarder();

    void poll();
    bool isActive() const { return m_active; }

signals:
    void finished();

private slots:
    void onSocketReadyRead();
    void onSocketDisconnected();

private:
    QTcpSocket* m_socket;
    LIBSSH2_CHANNEL* m_channel;
    bool m_active;
    
    QByteArray m_sshWriteBuffer;
    
    void writeToSsh();
};
