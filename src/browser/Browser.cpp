#include "Browser.h"
BROWSER::BROWSER()
{

    dns = new DnsResolver();
    open_connection = new TcpConnection();
    
    dns->moveToThread(&m_dns_thread);
    open_connection->moveToThread(&m_Tcp_thread);

    QObject::connect(&m_dns_thread, &QThread::finished,
                     dns, &QObject::deleteLater);
                     
    QObject::connect(&m_Tcp_thread, &QThread::finished,
                    open_connection, &QObject::deleteLater);
    
    m_dns_thread.start();
    m_Tcp_thread.start();

    QObject::connect(&url, &URL::navigationRequested,
                     open_connection, [p = &m_port_num](const ParsedUrl &parsed) {
        *p = parsed.port;
    });
    
    QObject::connect(&url, &URL::navigationRequested,
                     dns, [srv = dns](const ParsedUrl &parsed) {
        srv->resolve(parsed.host);
    });

    QObject::connect(dns, &DnsResolver::resolved,
                     open_connection, [srv = open_connection,pp = m_port_num](const QHostAddress &address_of_server) {
        srv->connectToHost(address_of_server, pp);
    });
}
BROWSER::~BROWSER()
{
    m_dns_thread.quit();
    m_dns_thread.wait();
    m_Tcp_thread.quit();
    m_Tcp_thread.wait();
}
void BROWSER::RUN()
{
}
