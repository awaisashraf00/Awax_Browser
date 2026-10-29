// Standalone TLS test: drives the REAL TLS::Send_Request pipeline against a
// live host without launching the GUI, so handshake failures can be repeated
// and debugged from the console in seconds instead of through the browser UI.
//
// Pipeline position: this exercises everything from DNS down to the parsed
// response EXCEPT TcpConnection and the Browser wiring -- exactly the slice
// where HTTPS bugs hide.
#include <QCoreApplication>
#include <QDebug>
#include <QString>

#include "HttpResponse.h"
#include "TLSConnection.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Host from argv so the same binary can test any site:
    //   ./test_tls_handshake uppbeat.io
    const QString host = argc > 1
        ? QString::fromUtf8(argv[1])
        : QStringLiteral("uppbeat.io");
    constexpr quint16 port = 443;

    // --- DNS: same AF_INET restriction as DnsResolver so the test connects
    // to the same kind of address the browser would.
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *results = nullptr;
    if (::getaddrinfo(host.toUtf8().constData(), "443", &hints, &results) != 0
        || results == nullptr) {
        qWarning() << "DNS lookup failed for" << host;
        return 2;
    }
    char ip[INET_ADDRSTRLEN] = {0};
    ::inet_ntop(AF_INET,
                &reinterpret_cast<sockaddr_in *>(results->ai_addr)->sin_addr,
                ip, sizeof(ip));
    qDebug() << "DNS:" << host << "->" << ip;

    // --- TCP: plain blocking socket, the same thing TcpConnection creates.
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        qWarning() << "socket failed:" << std::strerror(errno);
        ::freeaddrinfo(results);
        return 3;
    }
    if (::connect(fd, results->ai_addr, results->ai_addrlen) < 0) {
        qWarning() << "TCP connect failed:" << std::strerror(errno);
        ::freeaddrinfo(results);
        ::close(fd);
        return 3;
    }
    ::freeaddrinfo(results);
    qDebug() << "TCP connected to" << ip << ":" << port;

    // --- THE UNIT UNDER TEST: the same Send_Request the browser calls.
    TLS tls;
    QString errorMessage;
    HttpResponse response;
    const bool ok = tls.Send_Request(fd, host, port,
                                     QStringLiteral("/"), QString(),
                                     errorMessage, response);
    ::close(fd);

    if (!ok) {
        qWarning() << "TLS EXCHANGE FAILED:" << errorMessage;
        return 1;
    }
    qDebug() << "TLS EXCHANGE OK:" << response.statusCode << response.reasonPhrase
             << "| headers:" << response.headers.size()
             << "| body bytes:" << response.body.size();
    return 0;
}
