#include "Browser.h"
#include <QDebug>

namespace {
constexpr unsigned int MAX_REDIRECTS = 10;
}

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
                     &url, [this](const ParsedUrl &) {
        if (!m_redirectNavigation) {
            m_redirectCount = 0;
        }
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

    QObject::connect(open_connection, &TcpConnection::responseReceived,
                     &url, [this](const HttpResponse &response) {
        handleResponse(response);
    });
}

// Follows redirects by resolving Location against the current URL and
// restarting navigation, which naturally performs DNS and opens a new socket.
void BROWSER::handleResponse(const HttpResponse &response)
{
    if (!response.isRedirect()) {
        m_redirectCount = 0;
        return;
    }

    const std::optional<QString> location =
        response.headerValue(QStringLiteral("Location"));
    if (!location.has_value() || location->isEmpty()) {
        qWarning() << "HTTP redirect response is missing a Location header";
        m_redirectCount = 0;
        return;
    }

    if (m_redirectCount >= MAX_REDIRECTS) {
        qWarning() << "HTTP redirect limit exceeded";
        m_redirectCount = 0;
        return;
    }

    QString errorMessage;
    const std::optional<QString> destination =
        url.resolveReference(*location, errorMessage);
    if (!destination.has_value()) {
        qWarning() << "Cannot follow HTTP redirect:" << errorMessage;
        m_redirectCount = 0;
        return;
    }

    ++m_redirectCount;
    m_redirectNavigation = true;
    url.url_write(*destination);
    m_redirectNavigation = false;
}

BROWSER::~BROWSER()
{
    m_dns_thread.quit();
    m_dns_thread.wait();
    m_Tcp_thread.quit();
    m_Tcp_thread.wait();
}
