#include "DnsResolver.h"

DnsResolver::DnsResolver(QObject *parent)
    : QObject(parent)
{
}

void DnsResolver::resolve(const QString &host)
{
    m_host = host;

    addrinfo hints{0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    

    addrinfo *results = nullptr;
    const int response = ::getaddrinfo(m_host.toUtf8().constData(), nullptr, &hints, &results);

    if (response != 0) {
        qDebug() << "DNS: lookup failed for" << m_host << "-" << gai_strerror(response);
        m_address.clear();
        return;
    }
    const addrinfo *chosen = nullptr;   
    const addrinfo *fallback = nullptr; 
    for (const addrinfo *node = results; node != nullptr; node = node->ai_next) {
        if (node->ai_family == AF_INET) {  
            chosen = node;                 
            break;
        }
        if (!fallback) {
            fallback = node;               
        }
    }
    if (!chosen) {
        chosen = fallback;                 
    }

    if (chosen && chosen->ai_addr) {
        const sockaddr *sa = chosen->ai_addr;
        char ip[INET6_ADDRSTRLEN] = {0};

        if (sa->sa_family == AF_INET6) {
            const auto *sin6 = reinterpret_cast<const sockaddr_in6 *>(sa);
            inet_ntop(AF_INET6, &(sin6->sin6_addr), ip, sizeof(ip));
        } else if (sa->sa_family == AF_INET) {
            const auto *sin = reinterpret_cast<const sockaddr_in *>(sa);
            inet_ntop(AF_INET, &(sin->sin_addr), ip, sizeof(ip));
        }

        m_address = QString::fromLatin1(ip);
    }

        
    if (results) {
        ::freeaddrinfo(results);
    }

    QHostAddress addr(m_address);

    emit resolved(addr);
    qDebug() << "DNS: ---- >" << m_host << "address-----> " << m_address;
}
