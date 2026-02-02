#include "PortForwarder.h"
#include <QDebug>

PortForwarder::PortForwarder(QTcpSocket* socket, LIBSSH2_CHANNEL* channel, QObject* parent)
    : QObject(parent)
    , m_socket(socket)
    , m_channel(channel)
    , m_active(true)
{
    m_socket->setParent(this); // Take ownership
    connect(m_socket, &QTcpSocket::readyRead, this, &PortForwarder::onSocketReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &PortForwarder::onSocketDisconnected);
}

PortForwarder::~PortForwarder()
{
    if (m_channel) {
        libssh2_channel_free(m_channel);
        m_channel = nullptr;
    }
    if (m_socket) {
        // socket is child, will be deleted automatically or we can ensure close
        if (m_socket->isOpen()) m_socket->close();
    }
}

void PortForwarder::onSocketReadyRead()
{
    if (!m_active) return;
    m_sshWriteBuffer.append(m_socket->readAll());
    writeToSsh();
}

void PortForwarder::writeToSsh()
{
    if (m_sshWriteBuffer.isEmpty()) return;
    
    ssize_t written = libssh2_channel_write(m_channel, m_sshWriteBuffer.constData(), m_sshWriteBuffer.size());
    if (written > 0) {
        m_sshWriteBuffer.remove(0, written);
    } else if (written == LIBSSH2_ERROR_EAGAIN) {
        // Will try again next poll
    } else {
        qWarning() << "PortForwarder: Write error to SSH channel" << written;
        m_active = false;
        emit finished();
    }
}

void PortForwarder::onSocketDisconnected()
{
    m_active = false;
    emit finished();
}

void PortForwarder::poll()
{
    if (!m_active || !m_channel) return;

    // 1. Try to flush any pending write to SSH
    if (!m_sshWriteBuffer.isEmpty()) {
        writeToSsh();
    }

    // 2. Read from SSH and write to Socket
    char buffer[16384];
    ssize_t nread = libssh2_channel_read(m_channel, buffer, sizeof(buffer));
    
    if (nread > 0) {
        m_socket->write(buffer, nread);
    } else if (nread == LIBSSH2_ERROR_EAGAIN) {
        // No data
    } else {
        // EOF or Error
        if (nread != 0) { // 0 might mean EOF
            qDebug() << "PortForwarder: SSH channel read closed or error" << nread;
        }
        m_active = false;
        m_socket->disconnectFromHost();
        emit finished();
    }
    
    // Check if channel is closed EOF
    if (libssh2_channel_eof(m_channel)) {
         m_active = false;
         m_socket->disconnectFromHost();
         emit finished();
    }
}
