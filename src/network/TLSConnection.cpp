#include "TLSConnection.h"
#include "HttpResponseParser.h"

namespace {
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



TLS::TLS(QObject *parent)
    : QObject(parent)
{
}

TLS::~TLS()
{
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (context) {
        SSL_CTX_free(context);
    }
}

SSL_CTX *TLS::ensureContext(QString &errorMessage)
{
    if (context) {
        return context;   
    }

    context = SSL_CTX_new(TLS_client_method());
    if (!context) {
        errorMessage = QStringLiteral("SSL_CTX_new failed: %1").arg(lastOpenSslError());
        return nullptr;
    }

    SSL_CTX_set_verify(context, SSL_VERIFY_PEER, nullptr);

    if (SSL_CTX_set_default_verify_paths(context) != 1) {
        qDebug() << "TLS: no default CA paths found, relying on explicit bundle";
    }

    if (SSL_CTX_load_verify_locations(context, "/etc/ssl/cert.pem", nullptr) != 1) {
        qWarning() << "TLS: could not load /etc/ssl/cert.pem:" << lastOpenSslError();
    }

    return context;
}

bool TLS::Send_Request(int socketFd,
                       const QString &host,
                       quint16 port,
                       const QString &path,
                       const QString &query,
                       QString &errorMessage,
                       HttpResponse &response)
{
    auto fail = [&](const QString &reason) {
        errorMessage = reason;
        if (ssl) {
            SSL_free(ssl);
            ssl = nullptr;
        }
        return false;
    };

    SSL_CTX *ctx = ensureContext(errorMessage);
    if (!ctx) {
        return false;
    }

    ssl = SSL_new(ctx);
    if (!ssl) {
        return fail(QStringLiteral("SSL_new failed: %1").arg(lastOpenSslError()));
    }

    SSL_set_fd(ssl, socketFd);

    const QByteArray hostUtf8 = host.toUtf8();
                                            
    
    if (SSL_set_tlsext_host_name(ssl, hostUtf8.constData()) != 1) {
        return fail(QStringLiteral("failed to set SNI: %1").arg(lastOpenSslError()));
    }

    if (SSL_set1_host(ssl, hostUtf8.constData()) != 1) {
        return fail(QStringLiteral("failed to set verification hostname: %1").arg(lastOpenSslError()));
    }
    if (SSL_connect(ssl) != 1) {
        return fail(QStringLiteral("TLS handshake with %1 failed: %2")
                        .arg(host, lastOpenSslError()));
    }
    const long verifyResult = SSL_get_verify_result(ssl);
    if (verifyResult != X509_V_OK) {
        return fail(QStringLiteral("certificate verification failed: %1")
                        .arg(QString::fromLocal8Bit(
                            X509_verify_cert_error_string(verifyResult))));
    }
    qDebug() << "TLS: tunnel up with" << host << "|"
             << SSL_get_version(ssl) << "|" << SSL_get_cipher_name(ssl);
    const QByteArray request = httpClient.buildRequest(host, port, path, query);
    qsizetype bytesSent = 0;
    while (bytesSent < request.size()) {
        const int written = SSL_write(ssl,
                                      request.constData() + bytesSent,
                                      static_cast<int>(request.size() - bytesSent));
        if (written <= 0) {
            return fail(QStringLiteral("SSL_write failed (reason %1): %2")
                            .arg(SSL_get_error(ssl, written))
                            .arg(lastOpenSslError()));
        }
        bytesSent += written;
    }
    QByteArray rawResponse;
    QByteArray chunk;
    chunk.resize(4096);
    while (true) {
        const int received = SSL_read(ssl, chunk.data(), chunk.size());
        if (received > 0) {

            rawResponse.append(chunk.constData(), received);
            continue;
        }
        const int reason = SSL_get_error(ssl, received);
        if (reason == SSL_ERROR_ZERO_RETURN) {
            break; 
        }
        if (reason == SSL_ERROR_SSL &&
            ERR_GET_REASON(ERR_peek_last_error()) == SSL_R_UNEXPECTED_EOF_WHILE_READING) {
    
    
    
    
    
            qDebug() << "TLS: server closed without close_notify -- treating as end of response";
            break;
        }
        if (reason == SSL_ERROR_SYSCALL) {
    
    
    
            break;
        }
        return fail(QStringLiteral("SSL_read failed (reason %1): %2")
                        .arg(reason)
                        .arg(lastOpenSslError()));
    }

    if (rawResponse.isEmpty()) {
        return fail(QStringLiteral("server closed the TLS connection without sending a response"));
    }
    HttpResponseParser parser;
    const std::optional<HttpResponse> parsed = parser.parse(rawResponse, errorMessage);
    if (!parsed.has_value()) {
        return fail(errorMessage);
    }
    response = std::move(*parsed);
    
    
    
    SSL_shutdown(ssl);
    SSL_free(ssl);
    ssl = nullptr;   
    return true;
}
