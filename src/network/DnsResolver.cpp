#include "DnsResolver.h"

#include <QtDebug>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

DnsResolver::DnsResolver(QObject *parent)
    : QObject(parent)
{
}

void DnsResolver::resolve(const QString &host)
{
    m_host = host;

    // "hints" tells the resolver what WE want before the lookup runs:
    //   ai_family   = which address families to accept.
    //                 AF_UNSPEC = "IPv4 or IPv6, whichever exists"
    //                 (AF_INET would mean IPv4-only, AF_INET6 IPv6-only)
    //   ai_socktype = which transport; SOCK_STREAM = TCP-capable results
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
    const addrinfo *chosen = nullptr;   // best candidate found so far
    const addrinfo *fallback = nullptr; // first IPv6 node, used only if no IPv4 exists
    for (const addrinfo *node = results; node != nullptr; node = node->ai_next) {
        if (node->ai_family == AF_INET) {  // AF_INET = IPv4
            chosen = node;                 // first IPv4 wins
            break;
        }
        if (!fallback) {
            fallback = node;               // remember the first IPv6 node
        }
    }
    if (!chosen) {
        chosen = fallback;                 // IPv6-only host: still connectable
    }

    // Each node's ai_addr is a generic `sockaddr` pointer. A sockaddr is
    // just {address family + raw bytes}; it is intentionally opaque so
    // one type can represent every family. To read the actual address
    // you reinterpret_cast it to the family-specific struct:
    //   AF_INET  -> sockaddr_in,  address in ->sin_addr  (4 bytes, IPv4)
    //   AF_INET6 -> sockaddr_in6, address in ->sin6_addr (16 bytes, IPv6)
    // This cast-on-family is the classic BSD sockets pattern; the TCP
    // stage later passes this same pointer straight into connect().
    if (chosen && chosen->ai_addr) {
        const sockaddr *sa = chosen->ai_addr;
        // INET6_ADDRSTRLEN = buffer size for the LONGEST possible text
        // address (an IPv4-mapped IPv6 string is up to 45 chars).
        char ip[INET6_ADDRSTRLEN] = {0};

        if (sa->sa_family == AF_INET6) {
            const auto *sin6 = reinterpret_cast<const sockaddr_in6 *>(sa);
            // inet_ntop = "network (binary) TO presentation (text)".
            inet_ntop(AF_INET6, &(sin6->sin6_addr), ip, sizeof(ip));
        } else if (sa->sa_family == AF_INET) {
            const auto *sin = reinterpret_cast<const sockaddr_in *>(sa);
            inet_ntop(AF_INET, &(sin->sin_addr), ip, sizeof(ip));
        }

        m_address = QString::fromLatin1(ip);
    }

    // getaddrinfo allocated the whole linked list; freeaddrinfo releases
    // every node in one call.
    if (results) {
        ::freeaddrinfo(results);
    }

    qDebug() << "DNS: ---- >" << m_host << "address->" << m_address;
    qDebug() << "next stage: TCP connect to" << m_address << "(" << host << ")";
}
