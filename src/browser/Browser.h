#pragma once
#include <TcpConnection.h>
#include <URL.h>
#include <QThread>
#include <DnsResolver.h>
#include <QObject>

class BROWSER{

    private:
        URL url;

        // QThread is NOT the work itself -- it is just a thread running
        // an event loop. Jobs are handed to it by queuing signals into
        // that loop; the thread wakes up and executes them one by one.
        QThread m_dns_thread;
        QThread m_Tcp_thread;

        // DNS worker object. Heap-allocated with NO parent on purpose:
        // a QObject that has a parent cannot be moved to another thread,
        // and children would silently stay behind on the main thread.
        // Deleted automatically (deleteLater) when the thread finishes.
        DnsResolver *dns = nullptr;

        TcpConnection *open_connection = nullptr;
        uint16_t m_port_num{0};

    public:
        explicit BROWSER();
        ~BROWSER();
        void RUN();
        URL& Give_Url_address() { return url;}
};
