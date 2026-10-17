#include "TcpConnection.h"
#include <QDebug>
#include <arpa/inet.h>   // inet_pton: text "93.184.216.34" -> binary address
#include <netinet/in.h>  // sockaddr_in: the IPv4 address struct
#include <sys/socket.h>  // socket(), connect(), AF_INET, SOCK_STREAM
#include <unistd.h>      // close()
#include <cerrno>        // errno: per-thread error code set by failed syscalls
#include <cstring>       // strerror: errno number -> human-readable text

TcpConnection::TcpConnection(QObject *parent)
    : QObject(parent)
{
}

TcpConnection::~TcpConnection(){}

void TcpConnection::connectToHost(const QHostAddress &ip, quint16 port)
{

    fd  = {::socket(AF_INET, SOCK_STREAM, 0)};
    if (fd < 0) {

        emit Is_failed(QString("socket: %1").arg(strerror(errno)));
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip.toString().toUtf8().constData(),
                    &addr.sin_addr) != 1) 
    {
        emit Is_failed(QString("invalid IPv4 address: %1").arg(ip.toString()));
        qDebug()<<QString("invalid IPv4 address: %1").arg(ip.toString());
        ::close(fd);
        return;
    }
    
    if (::connect(fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0) {

        emit Is_failed(QString("Connection failed with address: %1").arg(ip.toString()));
        qDebug() << QString("Connection failed with address: %1").arg(ip.toString());
        ::close(fd);
        return;
        
    }else{
        qDebug() << QString("Connection succeeded with address: %1").arg(ip.toString());
    }

}
