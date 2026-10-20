#pragma once

#include<QThread>
#include <QObject>
#include <QHostAddress>
#include <QString>
#include <HttpClient.h>
#include <QDebug>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


class TcpConnection : public QObject
{
    Q_OBJECT

public:
    explicit TcpConnection(QObject *parent = nullptr);
    ~TcpConnection() override;

public slots:

    void connectToHost(const QHostAddress &ip,
         quint16 port,
         QString host,
         QString scheme,
         QString path,
         QString query);

signals:
    void connected();
    void Is_failed(const QString &reason);
    void Is_closed();
    void Response_Resceived(const QString &res);

private:
    HttpClient httpClient;
    int fd{-1};
    // std::mutex mtx_lock;
    // std::thread m_worker;               
    // std::atomic<bool> m_running{false}; 
};
