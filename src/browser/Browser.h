#pragma once
#include <URL.h>
#include <Server.h>
#include <DnsResolver.h>
#include <QObject>

class BROWSER{

    private:
        URL url;
        DnsResolver dns;
        TcpConnection open_connection;
        uint16_t m_port_num{0}; 
    public:
        explicit BROWSER();
        const void RUN();
        URL& Give_Url_address() { return url;}
};