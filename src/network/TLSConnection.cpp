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

} // namespace

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

// The complete HTTPS exchange. Ownership rule (documented in the header):
// socketFd is BORROWED -- TcpConnection still owns and closes it. Everything
// SSL_* we create here is ours and is freed before every return.
bool TLS::Send_Request(int socketFd,
                       const QString &host,
                       quint16 port,
                       const QString &path,
                       const QString &query,
                       QString &errorMessage,
                       HttpResponse &response)
{
    // Local failure helper: frees the per-connection SSL object, stores the
    // reason, and returns false -- one place instead of repeating cleanup at
    // every error return. (No SSL_shutdown on failure: the tunnel was never
    // fully up or is already broken; SSL_free alone is correct here.)
    auto fail = [&](const QString &reason) {
        errorMessage = reason;
        if (ssl) {
            SSL_free(ssl);
            ssl = nullptr;
        }
        return false;
    };

    // --- Step 1: policy context (created once, cached).
    SSL_CTX *ctx = ensureContext(errorMessage);
    if (!ctx) {
        return false;
    }

    // --- Step 2: the per-connection TLS session.
    // SSL_new stamps one instance out of the context "class": it will own
    // the negotiated cipher, session keys, and read/write buffers.
    ssl = SSL_new(ctx);
    if (!ssl) {
        return fail(QStringLiteral("SSL_new failed: %1").arg(lastOpenSslError()));
    }

    // --- Step 3: glue the session onto OUR socket.
    // SSL_set_fd creates nothing and owns nothing -- it only tells OpenSSL
    // "when you need bytes, read/write this fd". The socket stays owned by
    // TcpConnection, which closes it after this function returns.
    SSL_set_fd(ssl, socketFd);

    // Step 4: SNI (Server Name Indication) -- the hostname sent INSIDE the
    // ClientHello, in plaintext. Servers hosting many sites on one IP use it
    // to pick which certificate to present. No SNI => wrong cert => failure.
    const QByteArray hostUtf8 = host.toUtf8(); // kept alive: both calls copy later,
                                               // but a named buffer documents intent
    // SSL_set_tlsext_host_name is a MACRO wrapping SSL_ctrl(...) -- that is
    // why its failure check is "!= 1" and not "< 0".
    if (SSL_set_tlsext_host_name(ssl, hostUtf8.constData()) != 1) {
        return fail(QStringLiteral("failed to set SNI: %1").arg(lastOpenSslError()));
    }

    // --- Step 5: hostname verification -- what the certificate MUST match.
    // Distinct job from SNI: SNI tells the SERVER who we came for, this tells
    // OPENSSL which name to demand inside the presented certificate (SAN/CN).
    if (SSL_set1_host(ssl, hostUtf8.constData()) != 1) {
        return fail(QStringLiteral("failed to set verification hostname: %1").arg(lastOpenSslError()));
    }

    // --- Step 6: the handshake. Blocking, because our socket is blocking.
    // SSL_connect runs: ClientHello -> ServerHello -> certificate chain ->
    // chain validation (because SSL_VERIFY_PEER) -> ECDHE key exchange ->
    // Finished. Returns 1 only when the encrypted tunnel is fully up.
    if (SSL_connect(ssl) != 1) {
        return fail(QStringLiteral("TLS handshake with %1 failed: %2")
                        .arg(host, lastOpenSslError()));
    }

    // --- Step 7: explicit verdict of the certificate verifier.
    // Belt-and-braces on top of SSL_VERIFY_PEER: SSL_get_verify_result returns
    // the verifier's decision code; X509_V_OK is the only acceptable value.
    // (X509 is the ITU-T standard format of public-key certificates.)
    const long verifyResult = SSL_get_verify_result(ssl);
    if (verifyResult != X509_V_OK) {
        // X509_verify_cert_error_string turns the numeric code into text
        // like "certificate has expired" -- far more useful in the UI.
        return fail(QStringLiteral("certificate verification failed: %1")
                        .arg(QString::fromLocal8Bit(
                            X509_verify_cert_error_string(verifyResult))));
    }

    // Free, static strings describing the negotiated tunnel -- great for
    // confirming you really got TLS 1.3 and not some ancient fallback.
    qDebug() << "TLS: tunnel up with" << host << "|"
             << SSL_get_version(ssl) << "|" << SSL_get_cipher_name(ssl);

    // --- Step 8: request bytes -- byte-for-byte identical to plain HTTP.
    // This line is the whole thesis of HTTPS: HTTP does not change, the
    // transport underneath it does.
    const QByteArray request = httpClient.buildRequest(host, port, path, query);

    // --- Step 9: encrypted write. Like ::send, SSL_write MAY accept fewer
    // bytes than requested (partial write), so we loop and advance the
    // pointer by however much it actually consumed. Return > 0 = bytes
    // encrypted and handed to the socket; <= 0 = trouble, disambiguated
    // below via SSL_get_error.
    qsizetype bytesSent = 0;
    while (bytesSent < request.size()) {
        const int written = SSL_write(ssl,
                                      request.constData() + bytesSent,
                                      static_cast<int>(request.size() - bytesSent));
        if (written <= 0) {
            // SSL_get_error(ssl, ret) inspects the session state to say WHY
            // the call returned <= 0. On blocking sockets, every result here
            // is a hard failure (WANT_READ/WRITE only happen when the socket
            // is non-blocking -- that lesson comes later).
            return fail(QStringLiteral("SSL_write failed (reason %1): %2")
                            .arg(SSL_get_error(ssl, written))
                            .arg(lastOpenSslError()));
        }
        bytesSent += written;
    }

    // --- Step 10: encrypted read until the tunnel says EOF.
    // TLS has its own EOF: the close_notify alert. When the peer sends it,
    // SSL_read returns <= 0 with SSL_ERROR_ZERO_RETURN -- that is the clean
    // end-of-response, equivalent to recv() returning 0 on a plain socket.
    QByteArray rawResponse;
    QByteArray chunk;
    chunk.resize(4096);
    while (true) {
        const int received = SSL_read(ssl, chunk.data(), chunk.size());
        if (received > 0) {
            // Bytes arrive DECRYPTED: from here down this is ordinary HTTP.
            rawResponse.append(chunk.constData(), received);
            continue;
        }
        const int reason = SSL_get_error(ssl, received);
        if (reason == SSL_ERROR_ZERO_RETURN) {
            break;   // clean close_notify -- normal end of an HTTP/1.1 exchange
        }
        return fail(QStringLiteral("SSL_read failed (reason %1): %2")
                        .arg(reason)
                        .arg(lastOpenSslError()));
    }

    if (rawResponse.isEmpty()) {
        return fail(QStringLiteral("server closed the TLS connection without sending a response"));
    }

    // --- Step 11: parse with the SAME parser plain HTTP uses. If this works
    // unchanged (it does), it proves the layering: TLS ends where HTTP begins.
    HttpResponseParser parser;
    const std::optional<HttpResponse> parsed = parser.parse(rawResponse, errorMessage);
    if (!parsed.has_value()) {
        return fail(errorMessage);
    }
    response = std::move(*parsed);

    // --- Step 12: polite close inside the tunnel BEFORE the fd dies.
    // SSL_shutdown sends close_notify so the server knows the truncation was
    // intentional (and so TLS session resumption becomes possible later).
    // The fd itself is NOT touched here -- TcpConnection owns it.
    SSL_shutdown(ssl);
    SSL_free(ssl);
    ssl = nullptr;   // destructor must not free it a second time
    return true;
}
