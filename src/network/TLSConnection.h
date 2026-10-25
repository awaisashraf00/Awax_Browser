#pragma once

#include <QObject>
#include <QDebug>
#include "HttpResponse.h"
#include "HttpClient.h"
#include <openssl/ssl.h>   
#include <openssl/err.h> 

class TLS : public QObject
{
    Q_OBJECT

public:
    explicit TLS(QObject *parent = nullptr);
    ~TLS() override;

    // Runs the complete HTTPS exchange on a socket that TcpConnection has
    // already connected: TLS handshake -> certificate verification ->
    // write request -> read response -> parse. Same contract as
    // HttpClient::sendRequest so the caller can pick one or the other
    // based on the URL scheme without changing its own logic.
    // Returns false with a human-readable reason in errorMessage on any
    // failure (context creation, handshake, verification, I/O, parse).
    bool Send_Request(int socketFd,
                      const QString &host,
                      quint16 port,
                      const QString &path,
                      const QString &query,
                      QString &errorMessage,
                      HttpResponse &response);

signals:
    void responseReceived(const HttpResponse &response);
    void Is_failed(const QString &reason);
    void Is_closed();

private:

    SSL_CTX *ensureContext(QString &errorMessage);

    // The TLS "factory": holds connection-independent policy. Created
    // lazily on the first https request, freed in the destructor.
    SSL_CTX *context{nullptr};

    // One TLS session per request: negotiated cipher, session keys, buffers.
    // Null whenever no request is in flight (initialized and re-nulled so
    // the destructor can never free a stale pointer).
    SSL *ssl{nullptr};

    // Reused unchanged: GET request bytes and response parsing are
    // scheme-independent -- only the pipe they travel through differs.
    HttpClient httpClient;
};
