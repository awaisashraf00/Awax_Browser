#include "TLSConnection.h"

#include "HttpResponseParser.h"

#include <QDebug>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>

namespace {
constexpr int TLS_IO_TIMEOUT_MILLISECONDS = 30000;
constexpr qsizetype MAX_HTTP_RESPONSE_BYTES = 64 * 1024 * 1024;
constexpr qsizetype TLS_READ_BUFFER_BYTES = 16 * 1024;

// Reads a useful OpenSSL error from its thread-local error queue after an API
// call has failed, instead of reporting an unrelated stale error.
QString lastOpenSslError()
{
    const unsigned long code = ERR_get_error();
    if (code == 0) {
        return QStringLiteral("no OpenSSL error recorded");
    }

    char buffer[256];
    ERR_error_string_n(code, buffer, sizeof(buffer));
    return QString::fromLocal8Bit(buffer);
}

// Makes the socket non-blocking so TLS operations can be retried only when
// poll reports readiness, with a finite timeout instead of hanging forever.
class NonBlockingSocket
{
public:
    explicit NonBlockingSocket(int socketFd)
        : m_socketFd(socketFd)
    {
    }

    // Restores the socket's original mode when the TLS exchange exits.
    ~NonBlockingSocket()
    {
        if (m_enabled) {
            if (fcntl(m_socketFd, F_SETFL, m_originalFlags) < 0) {
                qWarning() << "Could not restore socket flags after TLS:"
                           << std::strerror(errno);
            }
        }
    }

    // Enables non-blocking mode while retaining the flags needed for restore().
    bool enable(QString &errorMessage)
    {
        m_originalFlags = fcntl(m_socketFd, F_GETFL);
        if (m_originalFlags < 0) {
            errorMessage = QStringLiteral("fcntl(F_GETFL): %1")
                               .arg(QString::fromLocal8Bit(std::strerror(errno)));
            return false;
        }

        if (fcntl(m_socketFd, F_SETFL, m_originalFlags | O_NONBLOCK) < 0) {
            errorMessage = QStringLiteral("fcntl(F_SETFL): %1")
                               .arg(QString::fromLocal8Bit(std::strerror(errno)));
            return false;
        }

        m_enabled = true;
        return true;
    }

    // Restores the socket before its caller continues using or closes it.
    bool restore(QString &errorMessage)
    {
        if (!m_enabled) {
            return true;
        }

        if (fcntl(m_socketFd, F_SETFL, m_originalFlags) < 0) {
            errorMessage = QStringLiteral("could not restore socket flags: %1")
                               .arg(QString::fromLocal8Bit(std::strerror(errno)));
            return false;
        }

        m_enabled = false;
        return true;
    }

private:
    int m_socketFd;
    int m_originalFlags{-1};
    bool m_enabled{false};
};

// Waits for the readiness event requested by OpenSSL and turns timeout or
// operating-system failures into a diagnostic for the browser's error path.
bool waitForSocket(int socketFd, short events, QString &errorMessage)
{
    pollfd descriptor{};
    descriptor.fd = socketFd;
    descriptor.events = events;

    int result = 0;
    do {
        result = poll(&descriptor, 1, TLS_IO_TIMEOUT_MILLISECONDS);
    } while (result < 0 && errno == EINTR);

    if (result == 0) {
        errorMessage = QStringLiteral("TLS network operation timed out");
        return false;
    }
    if (result < 0) {
        errorMessage = QStringLiteral("poll: %1")
                           .arg(QString::fromLocal8Bit(std::strerror(errno)));
        return false;
    }
    if (descriptor.revents & POLLNVAL) {
        errorMessage = QStringLiteral("TLS socket became invalid during network operation");
        return false;
    }

    return true;
}

// Handles OpenSSL's retry states for non-blocking sockets and formats all
// other handshake/read/write failures without hiding a TLS protocol error.
bool retryTlsOperation(int socketFd,
                       SSL *ssl,
                       int result,
                       int systemError,
                       const QString &operation,
                       QString &errorMessage)
{
    const int sslError = SSL_get_error(ssl, result);
    if (sslError == SSL_ERROR_WANT_READ) {
        return waitForSocket(socketFd, POLLIN, errorMessage);
    }
    if (sslError == SSL_ERROR_WANT_WRITE) {
        return waitForSocket(socketFd, POLLOUT, errorMessage);
    }

    QString detail = lastOpenSslError();
    if (sslError == SSL_ERROR_SYSCALL && systemError != 0) {
        detail = QString::fromLocal8Bit(std::strerror(systemError));
    }
    errorMessage = QStringLiteral("%1 failed (OpenSSL error %2): %3")
                       .arg(operation)
                       .arg(sslError)
                       .arg(detail);
    return false;
}
}

// Gives the TLS helper QObject ownership so it can be used by the browser's
// network worker while keeping its reusable SSL context in this instance.
TLS::TLS(QObject *parent)
    : QObject(parent)
{
}

// Creates the reusable TLS client context and requires OpenSSL's configured
// certificate authorities; without them the browser cannot authenticate sites.
SSL_CTX *TLS::ensureContext(QString &errorMessage)
{
    if (context) {
        return context.get();
    }

    ERR_clear_error();
    context.reset(SSL_CTX_new(TLS_client_method()));
    if (!context) {
        errorMessage = QStringLiteral("SSL_CTX_new failed: %1")
                           .arg(lastOpenSslError());
        return nullptr;
    }

    SSL_CTX_set_verify(context.get(), SSL_VERIFY_PEER, nullptr);
    if (SSL_CTX_set_min_proto_version(context.get(), TLS1_2_VERSION) != 1) {
        errorMessage = QStringLiteral("could not require TLS 1.2 or newer: %1")
                           .arg(lastOpenSslError());
        context.reset();
        return nullptr;
    }

    ERR_clear_error();
    if (SSL_CTX_set_default_verify_paths(context.get()) != 1) {
        errorMessage = QStringLiteral("could not load the system CA certificates: %1")
                           .arg(lastOpenSslError());
        context.reset();
        return nullptr;
    }

    return context.get();
}

// Performs the HTTPS exchange on an already-connected TCP socket: it
// authenticates the server, sends the HTTP request through TLS, reads the
// response, and passes the complete wire bytes to the HTTP response parser.
bool TLS::Send_Request(int socketFd,
                       const QString &host,
                       quint16 port,
                       const QString &path,
                       const QString &query,
                       QString &errorMessage,
                       HttpResponse &response)
{
    errorMessage.clear();
    if (socketFd < 0 || host.isEmpty() || port == 0 || !path.startsWith('/')) {
        errorMessage = QStringLiteral("invalid socket or HTTPS request target");
        return false;
    }
    if (host.contains(QLatin1Char('\r')) || host.contains(QLatin1Char('\n')) ||
        path.contains(QLatin1Char('\r')) || path.contains(QLatin1Char('\n')) ||
        query.contains(QLatin1Char('\r')) || query.contains(QLatin1Char('\n'))) {
        errorMessage = QStringLiteral("HTTPS request components contain a line break");
        return false;
    }

    SSL_CTX *ctx = ensureContext(errorMessage);
    if (!ctx) {
        return false;
    }

#ifdef SO_NOSIGPIPE
    // Prevent a failed TLS write from terminating the process with SIGPIPE.
    const int noSigPipe = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_NOSIGPIPE,
                   &noSigPipe, sizeof(noSigPipe)) < 0) {
        errorMessage = QStringLiteral("setsockopt(SO_NOSIGPIPE): %1")
                           .arg(QString::fromLocal8Bit(std::strerror(errno)));
        return false;
    }
#endif

    NonBlockingSocket socketMode(socketFd);
    if (!socketMode.enable(errorMessage)) {
        return false;
    }

    std::unique_ptr<SSL, decltype(&SSL_free)> ssl(SSL_new(ctx), SSL_free);
    if (!ssl) {
        errorMessage = QStringLiteral("SSL_new failed: %1").arg(lastOpenSslError());
        return false;
    }
    if (SSL_set_fd(ssl.get(), socketFd) != 1) {
        errorMessage = QStringLiteral("SSL_set_fd failed: %1")
                           .arg(lastOpenSslError());
        return false;
    }

    const QByteArray hostUtf8 = host.toUtf8();
    unsigned char ipAddressBytes[16]{};
    // inet_pton recognizes numeric IPv4/IPv6 addresses without adding Qt
    // Network as a dependency to this TLS-only test target.
    const bool isIpAddress =
        inet_pton(AF_INET, hostUtf8.constData(), ipAddressBytes) == 1 ||
        inet_pton(AF_INET6, hostUtf8.constData(), ipAddressBytes) == 1;
    if (isIpAddress) {
        // IP literals are checked against certificate IP subjectAltNames,
        // rather than being incorrectly treated as DNS hostnames.
        X509_VERIFY_PARAM *verifyParameters = SSL_get0_param(ssl.get());
        if (X509_VERIFY_PARAM_set1_ip_asc(verifyParameters,
                                          hostUtf8.constData()) != 1) {
            errorMessage = QStringLiteral("failed to set certificate IP verification: %1")
                               .arg(lastOpenSslError());
            return false;
        }
    } else {
        if (SSL_set_tlsext_host_name(ssl.get(), hostUtf8.constData()) != 1) {
            errorMessage = QStringLiteral("failed to set TLS SNI: %1")
                               .arg(lastOpenSslError());
            return false;
        }
        if (SSL_set1_host(ssl.get(), hostUtf8.constData()) != 1) {
            errorMessage = QStringLiteral("failed to set certificate hostname verification: %1")
                               .arg(lastOpenSslError());
            return false;
        }
    }

    while (true) {
        ERR_clear_error();
        errno = 0;
        const int result = SSL_connect(ssl.get());
        const int systemError = errno;
        if (result == 1) {
            break;
        }
        if (!retryTlsOperation(socketFd, ssl.get(), result, systemError,
                               QStringLiteral("TLS handshake"), errorMessage)) {
            return false;
        }
    }

    const long verifyResult = SSL_get_verify_result(ssl.get());
    if (verifyResult != X509_V_OK) {
        errorMessage = QStringLiteral("certificate verification failed: %1")
                           .arg(QString::fromLocal8Bit(
                               X509_verify_cert_error_string(verifyResult)));
        return false;
    }

    qDebug() << "TLS: authenticated" << host
             << "| version:" << SSL_get_version(ssl.get())
             << "| cipher:" << SSL_get_cipher_name(ssl.get());

    const QByteArray request = httpClient.buildRequest(host, port, path, query);
    qsizetype bytesSent = 0;
    while (bytesSent < request.size()) {
        std::size_t written = 0;
        ERR_clear_error();
        errno = 0;
        const int result = SSL_write_ex(
            ssl.get(),
            request.constData() + bytesSent,
            static_cast<std::size_t>(request.size() - bytesSent),
            &written);
        const int systemError = errno;

        if (result == 1) {
            if (written == 0) {
                errorMessage = QStringLiteral("TLS write made no progress");
                return false;
            }
            bytesSent += static_cast<qsizetype>(written);
            continue;
        }
        if (!retryTlsOperation(socketFd, ssl.get(), result, systemError,
                               QStringLiteral("TLS request write"), errorMessage)) {
            return false;
        }
    }

    QByteArray rawResponse;
    QByteArray readBuffer;
    readBuffer.resize(TLS_READ_BUFFER_BYTES);
    bool receivedCloseNotify = false;
    bool receivedUnexpectedEof = false;
    while (true) {
        std::size_t received = 0;
        ERR_clear_error();
        errno = 0;
        const int result = SSL_read_ex(
            ssl.get(),
            readBuffer.data(),
            static_cast<std::size_t>(readBuffer.size()),
            &received);
        const int systemError = errno;

        if (result == 1) {
            if (received == 0) {
                errorMessage = QStringLiteral("TLS read made no progress");
                return false;
            }
            const qsizetype remainingCapacity =
                MAX_HTTP_RESPONSE_BYTES - rawResponse.size();
            if (received > static_cast<std::size_t>(remainingCapacity)) {
                errorMessage = QStringLiteral("HTTP response exceeds the 64 MiB limit");
                return false;
            }
            rawResponse.append(readBuffer.constData(),
                              static_cast<qsizetype>(received));
            continue;
        }

        const int sslError = SSL_get_error(ssl.get(), result);
        if (sslError == SSL_ERROR_ZERO_RETURN) {
            receivedCloseNotify = true;
            break;
        }
        if (sslError == SSL_ERROR_SSL &&
            ERR_GET_REASON(ERR_peek_last_error()) == SSL_R_UNEXPECTED_EOF_WHILE_READING) {
            receivedUnexpectedEof = true;
            break;
        }
        if (sslError == SSL_ERROR_SYSCALL && result == 0 && systemError == 0) {
            receivedUnexpectedEof = true;
            break;
        }
        if (!retryTlsOperation(socketFd, ssl.get(), result, systemError,
                               QStringLiteral("TLS response read"), errorMessage)) {
            return false;
        }
    }

    if (rawResponse.isEmpty()) {
        errorMessage = QStringLiteral("server closed the TLS connection without an HTTP response");
        return false;
    }

    HttpResponseParser parser;
    const std::optional<HttpResponse> parsed = parser.parse(rawResponse, errorMessage);
    if (!parsed.has_value()) {
        return false;
    }

    if (receivedUnexpectedEof) {
        const std::optional<QString> transferEncoding =
            parsed->headerValue(QStringLiteral("Transfer-Encoding"));
        const std::optional<QString> contentLength =
            parsed->headerValue(QStringLiteral("Content-Length"));
        const bool chunkedBodyIsDelimited =
            transferEncoding &&
            transferEncoding->compare(QStringLiteral("chunked"),
                                      Qt::CaseInsensitive) == 0;
        const bool statusHasNoBody =
            (parsed->statusCode >= 100 && parsed->statusCode < 200) ||
            parsed->statusCode == 204 || parsed->statusCode == 304;
        if (!chunkedBodyIsDelimited && !contentLength && !statusHasNoBody) {
            errorMessage = QStringLiteral(
                "TLS peer closed without close_notify and the HTTP body has no length boundary");
            return false;
        }
        qWarning() << "TLS peer closed without close_notify; accepted the complete "
                      "length-delimited HTTP response";
    }

    if (receivedCloseNotify) {
        while (true) {
            ERR_clear_error();
            errno = 0;
            const int shutdownResult = SSL_shutdown(ssl.get());
            const int systemError = errno;
            if (shutdownResult >= 0) {
                break;
            }
            QString shutdownError;
            if (!retryTlsOperation(socketFd, ssl.get(), shutdownResult, systemError,
                                   QStringLiteral("TLS close notification"), shutdownError)) {
                qWarning() << shutdownError;
                break;
            }
        }
    }

    if (!socketMode.restore(errorMessage)) {
        return false;
    }
    response = std::move(*parsed);

    qDebug() << "HTTP response:" << response.statusCode << response.reasonPhrase
             << "| body bytes:" << response.body.size();
    return true;
}
