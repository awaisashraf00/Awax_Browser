#include "HttpClient.h"

// Builds the HTTP request line and required Host header from parsed URL
// components; the fragment is intentionally absent because it is client-side.
QByteArray HttpClient::buildRequest(const QString &host,
                                    quint16 port,
                                    const QString &path,
                                    const QString &query) const
{
    QByteArray request = "GET ";
    request += path.toUtf8();
    if (!query.isEmpty()) {
        request += '?';
        request += query.toUtf8();
    }
    request += " HTTP/1.1\r\nHost: ";
    request += host.toUtf8();
    if (port != HTTP_PORT) {
        request += ':';
        request += QByteArray::number(port);
    }
    request += "\r\nConnection: close\r\n\r\n";
    return request;
}


bool HttpClient::sendRequest(int socketFd,
                             const QString &host,
                             quint16 port,
                             const QString &path,
                             const QString &query,
                             QString &errorMessage) const
{
    if (socketFd < 0 || host.isEmpty() || port == 0 || !path.startsWith('/')) {
        errorMessage = QStringLiteral("invalid socket or HTTP request target");
        return false;
    }
    if (host.contains(QLatin1Char('\r')) || host.contains(QLatin1Char('\n')) ||
        path.contains(QLatin1Char('\r')) || path.contains(QLatin1Char('\n')) ||
        query.contains(QLatin1Char('\r')) || query.contains(QLatin1Char('\n'))) {
        errorMessage = QStringLiteral("HTTP request components contain a line break");
        return false;
    }
    // port = 8000;
    const QByteArray request = buildRequest(host, port, path, query);
    
    qsizetype bytesSent = 0;
    while (bytesSent < request.size()) {
        const ssize_t result = ::send(socketFd,
            request.constData() + bytesSent,
            static_cast<size_t>(request.size() - bytesSent),
            0);
            if (result < 0) {
                if (errno == EINTR) {
                    continue;
                }
                errorMessage = QStringLiteral("send: %1")
                .arg(QString::fromLocal8Bit(std::strerror(errno)));
                return false;
            }
            if (result == 0) {
                errorMessage = QStringLiteral("send: connection closed before request was written");
                return false;
            }
            bytesSent += result;
    }
    QByteArray responseBuffer;
    responseBuffer.resize(4096); // Make space for recv() to write into.

    qsizetype bytesRecieved{0};
    
    while (true) {
        const ssize_t bytesReceived = ::recv(
                socketFd,
                responseBuffer.data(),   
                static_cast<size_t>(responseBuffer.size()),
                0);
        if (bytesReceived < 0) {
            if (errno == EINTR) {
                continue;
            }
            errorMessage = QStringLiteral("recieve: %1")
            .arg(QString::fromLocal8Bit(std::strerror(errno)));
            return false;
        }
        if (bytesReceived == 0) {
            break;
        }
        qDebug() << QByteArray(responseBuffer.constData(), bytesReceived);   
    }
    return true;
}
