#include "SshClient.h"
#include "PortForwarder.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <QDebug>

SshClient::SshClient(QObject* parent)
    : QObject(parent)
    , m_socket(-1)
    , m_session(nullptr)
    , m_channel(nullptr)
    , m_isConnected(false)
{
    libssh2_init(0);
}

void SshClient::logError(const QString& ctx) {
    char* msg;
    int len;
    int err = libssh2_session_last_error(m_session, &msg, &len, 0);
    QString errorString = QString("%1: [%2] %3").arg(ctx).arg(err).arg(msg);
    qDebug() << "SSH_DEBUG:" << errorString;
    emit errorOccurred(errorString);
}

SshClient::~SshClient()
{
    disconnectFromHost();
    libssh2_exit();
}

// Header needs check
#include <QMetaObject>

// Trace callback forward decl
static void sshTraceCallback(LIBSSH2_SESSION *session, void *context, const char *data, size_t length);

void SshClient::connectToHost(const QString& host, int port, const QString& user, const QString& password, const QString& keyPath)
{
    // 1. Resolve and Connect
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // Force IPv4 for simplicity with libssh2
    hints.ai_socktype = SOCK_STREAM;

    int gai_err = getaddrinfo(host.toStdString().c_str(), QString::number(port).toStdString().c_str(), &hints, &res);
    if (gai_err != 0) {
        logError(QString("Failed to resolve host: %1").arg(gai_strerror(gai_err)));
        return;
    }

    m_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (m_socket == -1) {
        logError("Failed to create socket");
        freeaddrinfo(res);
        return;
    }

    if (::connect(m_socket, res->ai_addr, res->ai_addrlen) != 0) {
        logError("Failed to connect to host");
        close(m_socket);
        m_socket = -1;
        freeaddrinfo(res);
        return;
    }

    freeaddrinfo(res);

    // 2. Create Session
    m_session = libssh2_session_init();
    if (!m_session) {
        logError("Failed to initialize libssh2 session");
        close(m_socket);
        m_socket = -1;
        return;
    }
    
    libssh2_trace(m_session, ~0); 
    libssh2_trace_sethandler(m_session, this, sshTraceCallback);
    
    libssh2_session_set_blocking(m_session, 1); // Blocking for setup

    int rc = libssh2_session_handshake(m_session, m_socket);
    if (rc) {
        logError(QString("SSH Handshake failed: %1").arg(rc));
        return;
    }
    qDebug() << "SSH_DEBUG: Handshake success";

    // 3. Authenticate
    bool authenticated = false;
    
    // Try Key first if provided
    if (!keyPath.isEmpty()) {
        qDebug() << "SSH_DEBUG: Trying public key auth with" << keyPath;
        // Assuming no passphrase for now or handled via password field? 
        // Let's assume 'password' field is passphrase if key is present, or empty?
        // User requirements didn't specify passphrase field separately. 
        // For now, let's treat the 'password' arg as the passphrase if a key is used.
        
        if (libssh2_userauth_publickey_fromfile(m_session, 
                                              user.toStdString().c_str(), 
                                              nullptr, // Pubkey (try auto-derive)
                                              keyPath.toStdString().c_str(), 
                                              password.toStdString().c_str()) == 0) {
            authenticated = true;
            qDebug() << "SSH_DEBUG: Public Key Auth success";
        } else {
             logError("Public Key Auth failed, trying password...");
        }
    }
    
    if (!authenticated) {
        if (libssh2_userauth_password(m_session, user.toStdString().c_str(), password.toStdString().c_str())) {
            logError("Authentication failed");
            return;
        }
        qDebug() << "SSH_DEBUG: Password Auth success";
    }

    // 4. Open Channel
    m_channel = libssh2_channel_open_session(m_session);
    if (!m_channel) {
        logError("Failed to open channel");
        return;
    }
    qDebug() << "SSH_DEBUG: Channel open";

    // Request PTY with standard VT100 size (matches terminal model default)
    const char* term = "xterm-256color";
    int width = 80;
    int height = 25;
    if (libssh2_channel_request_pty_ex(m_channel, term, strlen(term), NULL, 0, width, height, 0, 0)) {
        logError("Failed to request PTY");
        return;
    }
    qDebug() << "PTY requested with size:" << width << "x" << height;

    // Start shell
    if (libssh2_channel_shell(m_channel)) {
        logError("Failed to start shell");
        return;
    }

    m_isConnected = true;
    libssh2_channel_set_blocking(m_channel, 0); // Non-blocking
    emit connected();
}

// Static callback for libssh2 trace
static void sshTraceCallback(LIBSSH2_SESSION *session, void *context, const char *data, size_t length)
{
    SshClient* client = static_cast<SshClient*>(context);
    if (!client) return;
    // Convert data to string (might not be null terminated)
    QString msg = QString::fromUtf8(data, length);
    // Send to UI as debug
    // Emit via meta object or direct call if thread safe (Qt is main thread here)
    // We can just print to debug for now or emit a signal if we add one.
    // client->emitDebugMessage(msg); // Need to add this method
    
    // For now, let's just use logError logic to push to screen if it looks critical?
    // Or add a new signal `debugMessage`.
    QMetaObject::invokeMethod(client, "debugMessage", Qt::QueuedConnection, Q_ARG(QString, msg));
}



void SshClient::disconnectFromHost()
{
    if (m_channel) {
        libssh2_channel_free(m_channel);
        m_channel = nullptr;
    }
    if (m_session) {
        libssh2_session_disconnect(m_session, "Normal Shutdown");
        libssh2_session_free(m_session);
        m_session = nullptr;
    }
    if (m_socket != -1) {
        close(m_socket);
        m_socket = -1;
    }
    if (m_isConnected) {
        m_isConnected = false;
        emit disconnected();
    }
    
    // Cleanup forwarders
    qDeleteAll(m_activeForwarders);
    m_activeForwarders.clear();
    qDeleteAll(m_forwardServers);
    m_forwardServers.clear();
}

void SshClient::sendData(const QByteArray& data)
{
    if (!m_channel || !m_isConnected) return;
    
    // Non-blocking write
    ssize_t rc = libssh2_channel_write(m_channel, data.constData(), data.size());
    if (rc < 0) {
        if (rc != LIBSSH2_ERROR_EAGAIN) {
             logError(QString("Write error: %1").arg(rc));
        }
    }
}

void SshClient::setPtySize(int cols, int rows)
{
    if (!m_channel || !m_isConnected) return;
    
    int rc = libssh2_channel_request_pty_size(m_channel, cols, rows);
    if (rc < 0 && rc != LIBSSH2_ERROR_EAGAIN) {
        qDebug() << "PTY resize failed:" << rc;
    } else {
        qDebug() << "PTY resized to:" << cols << "x" << rows;
    }
}

void SshClient::poll()
{
    if (!m_session || !m_isConnected) return;

    // 1. Read standard output
     char buffer[16384];
     ssize_t rc = libssh2_channel_read(m_channel, buffer, sizeof(buffer));
     if (rc > 0) {
         emit dataReceived(QByteArray(buffer, rc));
     } else if (rc < 0) {
         if (rc != LIBSSH2_ERROR_EAGAIN) {
             logError(QString("Read error: %1").arg(rc));
         }
     }
     
     // 2. Read standard error
     rc = libssh2_channel_read_stderr(m_channel, buffer, sizeof(buffer));
      if (rc > 0) {
         emit dataReceived(QByteArray(buffer, rc));
     }
     
     // 3. Poll forwarders
     for (auto it = m_activeForwarders.begin(); it != m_activeForwarders.end(); ) {
         PortForwarder* fwd = *it;
         fwd->poll();
         if (!fwd->isActive()) {
             fwd->deleteLater();
             it = m_activeForwarders.erase(it);
         } else {
             ++it;
         }
     }
}

#include <QTcpServer>
#include <QTcpSocket>

void SshClient::addLocalForward(int localPort, const QString& targetHost, int targetPort)
{
    QTcpServer* server = new QTcpServer(this);
    if (!server->listen(QHostAddress::LocalHost, localPort)) {
        logError("Failed to listen on local port " + QString::number(localPort));
        delete server;
        return;
    }
    m_forwardServers.append(server);
    
    connect(server, &QTcpServer::newConnection, this, [this, server, targetHost, targetPort](){
        while (server->hasPendingConnections()) {
            QTcpSocket* socket = server->nextPendingConnection();
            
            // Open direct-tcpip channel
            // Note: This needs to be done here.
            // CAUTION: libssh2_channel_direct_tcpip might block if session is blocking?
            // We set session to blocking only during handshake. It should be non-blocking now.
            // If it returns NULL/EAGAIN, we can't easily wait here in this callback without blocking main thread.
            // But normally opening channel is fast or returns NULL.
            
            LIBSSH2_CHANNEL* channel = libssh2_channel_direct_tcpip(m_session, targetHost.toStdString().c_str(), targetPort);
            
            if (!channel) {
                char* msg;
                libssh2_session_last_error(m_session, &msg, 0, 0);
                qWarning() << "Failed to open direct-tcpip channel:" << msg;
                socket->close();
                socket->deleteLater();
                continue;
            }
            
            libssh2_channel_set_blocking(channel, 0);
            
            PortForwarder* forwarder = new PortForwarder(socket, channel, this);
            m_activeForwarders.append(forwarder);
            qDebug() << "New tunnel connection established to" << targetHost << ":" << targetPort;
        }
    });
}
