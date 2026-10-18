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
                     open_connection, [pp = &m_parsed_Browser](const ParsedUrl &parsed) {
        *pp = parsed;
    });
    
    QObject::connect(&url, &URL::navigationRequested,
                     dns, [srv = dns](const ParsedUrl &parsed) {
        srv->resolve(parsed.host);
    });

    QObject::connect(dns, &DnsResolver::resolved,
                     open_connection, [this](const QHostAddress &address_of_server) {
        open_connection->connectToHost(address_of_server,
             m_parsed_Browser.port,
            m_parsed_Browser.host,
            m_parsed_Browser.scheme,
            m_parsed_Browser.path,
            m_parsed_Browser.query);
    });
}
BROWSER::~BROWSER()
{
    m_dns_thread.quit();
    m_dns_thread.wait();
    m_Tcp_thread.quit();
    m_Tcp_thread.wait();
}
