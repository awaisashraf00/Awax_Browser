#include "TLSConnection.h"

TLS::TLS(QObject *parent)
{
}

SSL_CTX *TLS::Set_Create_Context()
{
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *cntx = SSL_CTX_new(method);
    // SSL_set_fd(ssl, SSL_Socket);
    if(!context){
        qDebug()<<"Error while creating context for TLS";
    }
    
    
    return cntx;
    
}

bool TLS::Send_Request(int SSL_Socket, QString host, int port, QString path, QString query, QString &errorMessage, HttpResponse &response)
{
    SSL_set_fd(ssl,SSL_Socket);
    return false;
}

TLS::~TLS()
{
    SSL_shutdown(ssl);
}
