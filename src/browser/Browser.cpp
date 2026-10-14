#include "Browser.h"
BROWSER::BROWSER()
{

    dns = new DnsResolver();

    dns->moveToThread(&m_dns_thread);

    QObject::connect(&m_dns_thread, &QThread::finished,
                     dns, &QObject::deleteLater);

    m_dns_thread.start();

    QObject::connect(&url, &URL::navigationRequested,
                     &open_connection, [p = &m_port_num](const ParsedUrl &parsed) {
        *p = parsed.port;
    });
    
    QObject::connect(&url, &URL::navigationRequested,
                     dns, [srv = dns](const ParsedUrl &parsed) {
        srv->resolve(parsed.host);
    });

    QObject::connect(dns, &DnsResolver::resolved,
                     &open_connection, [this](const QHostAddress &address_of_server) {
        open_connection.connectToHost(address_of_server, m_port_num);
    });
}
BROWSER::~BROWSER()
{
    m_dns_thread.quit();
    m_dns_thread.wait();
}
void BROWSER::RUN()
{
}
