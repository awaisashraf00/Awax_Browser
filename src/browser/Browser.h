#pragma once
#include <TcpConnection.h>
#include <URL.h>
#include <QThread>
#include <DnsResolver.h>
#include <QObject>

class BROWSER{

    private:
        URL url;

        QThread m_dns_thread;
        QThread m_Tcp_thread;

        DnsResolver *dns = nullptr;
        ParsedUrl m_parsed_Browser;
        TcpConnection *open_connection = nullptr;
        uint16_t m_port_num{0};

    public:
        explicit BROWSER();
        ~BROWSER();
        void RUN();
        URL& Give_Url_address() { return url;}
};
