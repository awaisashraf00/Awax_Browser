#include "TcpConnection.h"

#include <QDebug>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

TcpConnection::TcpConnection(QObject *parent)
    : QObject(parent)
{
}

// Releases the socket descriptor owned by this connection worker, if one was
// successfully created, so the operating system can reclaim the TCP endpoint.
TcpConnection::~TcpConnection()
{
    if (fd >= 0) {
        ::close(fd);
    }
}

// Completes the transport connection and, for plain HTTP, sends the first
// request. HTTPS must wait for a TLS layer; raw HTTP bytes are not safe there.
void TcpConnection::connectToHost(const QHostAddress &ip,
                                  quint16 port,
                                  QString host,
                                  QString scheme,
                                  QString path,
                                  QString query)
{
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }

    if (scheme != QStringLiteral("http")) {
        emit Is_failed(QStringLiteral("HTTPS requires TLS, which is not implemented yet"));
        return;
    }

    fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        emit Is_failed(QString("socket: %1").arg(strerror(errno)));
        return;
    }

    // setsockopt configures SO_NOSIGPIPE so macOS reports a closed-peer write
    // as EPIPE instead of terminating the browser with SIGPIPE; socklen_t is
    // the POSIX type describing the option value's byte length.
    const int noSigPipe = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &noSigPipe,
                     static_cast<socklen_t>(sizeof(noSigPipe))) < 0) {
        emit Is_failed(QString("setsockopt: %1").arg(strerror(errno)));
        ::close(fd);
        fd = -1;
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.toString().toUtf8().constData(),
                  &addr.sin_addr) != 1) {
        emit Is_failed(QString("invalid IPv4 address: %1").arg(ip.toString()));
        ::close(fd);
        fd = -1;
        return;
    }

    if (::connect(fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0) {
        emit Is_failed(QString("connect: %1").arg(strerror(errno)));
        ::close(fd);
        fd = -1;
        return;
    }

    emit connected();

    QString errorMessage;
    if (!httpClient.sendRequest(fd, host, port, path, query, errorMessage)) {
        emit Is_failed(errorMessage);
        ::close(fd);
        fd = -1;
    }

}
