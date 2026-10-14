#include "TcpConnection.h"
#include <QDebug>
#include <arpa/inet.h>   // inet_pton: text "93.184.216.34" -> binary address
#include <netinet/in.h>  // sockaddr_in: the IPv4 address struct
#include <sys/socket.h>  // socket(), connect(), AF_INET, SOCK_STREAM
#include <unistd.h>      // close()
#include <cerrno>        // errno: per-thread error code set by failed syscalls
#include <cstring>       // strerror: errno number -> human-readable text

TcpConnection::TcpConnection(QObject *parent)
    : QObject(parent)
{
}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::connectToHost(const QHostAddress &ip, quint16 port)
{
    if (m_running) {
        return;  `
    }

    m_worker = std::thread([this, ip, port]() {
        m_running = true;

        const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {

            emit Is_failed(QString("socket: %1").arg(strerror(errno)));
            m_running = false;
            return;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, ip.toString().toUtf8().constData(),
                      &addr.sin_addr) != 1) {

            emit Is_failed(QString("invalid IPv4 address: %1").arg(ip.toString()));
            ::close(fd);
            m_running = false;
            return;
        }

        if(::connect(m_fd,reinterpret_cast<const sockaddr*>(&addr),sizeof(addr))
        {
        }

        // ---- 4. Exchange bytes --------------------------------------
        // (Phase 4) ::send() writes request bytes; ::recv() reads the
        // response into a buffer. NOTE for later: recv returning 0
        // means the server closed the connection (EOF); -1 means
        // error; otherwise the return value is how many bytes you
        // got -- it can be FEWER than you asked for, so callers must
        // loop and accumulate. A TCP stream has no "messages".
    });
}
