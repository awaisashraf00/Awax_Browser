#pragma once
#include<QObject>
#include<QDebug>
#include<HttpResponse.h>
#include<openssl/ssl.h>
#include<openssl/err.h>

class TLS : private QObject{
    Q_OBJECT
    private:
        SSL *ssl;
        SSL_CTX * context;
    public:
        explicit TLS(QObject *parent = nullptr);
        char* m_ip_address;
        SSL_CTX* Set_Create_Context();

        bool Send_Request(int SSL_Socket,
            QString host,
            int port, 
            QString path,
            QString query,
            QString &errorMessage,
            HttpResponse &response);

        ~TLS();
    signals:
        void responseReceived(const HttpResponse &response); 
        void Is_failed(const QString &reason);
        void Is_closed();

};
