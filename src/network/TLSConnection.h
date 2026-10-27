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

    SSL_CTX *context{nullptr};

    SSL *ssl{nullptr};

    HttpClient httpClient;
};
// https://uppbeat.io
// https://uppbeat.io
// https://uppbeat.io
// https://uppbeat.io