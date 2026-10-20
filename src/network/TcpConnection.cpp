#include "TcpConnection.h"

TcpConnection::TcpConnection(QObject *parent)
    : QObject(parent)
{
        
    QThread::currentThread()->setObjectName("TCP THREAD");
}

TcpConnection::~TcpConnection()
{
    if (fd >= 0) {
        ::close(fd);
    }
}

void TcpConnection::connectToHost(const QHostAddress &ip,
                                  quint16 port,
                                  QString host,
                                  QString scheme,
                                  QString path,
                                  QString query)
{
    // port = 8000;

    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }

    // if (scheme == QStringLiteral("https")) {
    //     emit Is_failed(QStringLiteral("HTTPS requires TLS, which is not implemented yet"));
    //     return;
    // }
    // if (scheme != QStringLiteral("http")) {
    //     emit Is_failed(QStringLiteral("unsupported URL scheme"));
    //     return;
    // }

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
    qDebug()<< " Connected to --->" << host;
    
    emit connected();
    
    QString errorMessage;
    HttpResponse response;

    if (!httpClient.sendRequest(fd, host, port, path, query, errorMessage, response)) {
        emit Is_failed(errorMessage);
        qDebug()<< errorMessage;
        ::close(fd);
        fd = -1;
        return;
    }

    qDebug()<< " request sent to --->" << host;
    qDebug()<< "  <----   RESPOMSE     ---->";
    qDebug()<< "  <-------->";
    qDebug()<< response.body;
    qDebug()<< "  <-------->";

    emit responseReceived(response);

    ::close(fd);
    fd = -1;
    emit Is_closed();
}