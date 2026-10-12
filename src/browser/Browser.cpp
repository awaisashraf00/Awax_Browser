#include "Browser.h"

BROWSER::BROWSER():url()
{
    QObject::connect(&url, &URL::navigationRequested,
        &dns, [this](const ParsedUrl &parsed) {
            dns.resolve(parsed.host);
            m_port_num = parsed.port;
        });
    QObject::connect(&dns, &DnsResolver::resolved,
        &open_connection, [this](const QHostAddress &address_of_server) {
                open_connection.connectToHost(address_of_server,m_port_num);
        });
}

const void BROWSER::RUN()
{
    return void();
}
