#pragma once

#include <QObject>
#include <QDebug>
#include "HttpResponse.h"
#include "HttpClient.h"
#include <memory>
#include <openssl/ssl.h>
#include <openssl/err.h>

class TLS : public QObject
{
    Q_OBJECT

public:
    explicit TLS(QObject *parent = nullptr);
    ~TLS() override = default;

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

    std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)> context{nullptr, SSL_CTX_free};

    HttpClient httpClient;
};
// https://uppbeat.io
// https://uppbeat.io
// https://uppbeat.io
// https://uppbeat.io