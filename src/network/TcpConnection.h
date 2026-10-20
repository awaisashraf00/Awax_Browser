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

    // Carries the fully received and parsed HTTP response up to the
    // browser layer. This signal crosses a thread boundary (this object
    // lives on the TCP worker thread), and queued connections copy their
    // arguments into an event - which is why HttpResponse must be known
    // to Qt's meta-type system (see Q_DECLARE_METATYPE in HttpResponse.h).
    void responseReceived(const HttpResponse &response);
    void Is_failed(const QString &reason);
    void Is_closed();

private:
    HttpClient httpClient;
    int fd{-1};
    // std::mutex mtx_lock;
    // std::thread m_worker;               
    // std::atomic<bool> m_running{false}; 
};
