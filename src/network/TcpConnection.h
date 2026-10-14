#pragma once

#include <QObject>
#include <QThread>
#include <QHostAddress>
#include <atomic>
#include <thread>
#include <netinet/in.h>

class TcpConnection : public QObject
{
    Q_OBJECT

public:
    explicit TcpConnection(QObject *parent = nullptr);

    
    ~TcpConnection() override;

public slots:

    void connectToHost(const QHostAddress &ip, quint16 port);

signals:
    void connected();
    void Is_failed(const QString &reason);
    void Is_closed();

private:
    int m_fd = -1; 
    std::mutex mtx_lock;
    std::thread m_worker;               
    std::atomic<bool> m_running{false}; 
    struct sockaddr_in addr;
};
