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
        unsigned int m_redirectCount{0};
        bool m_redirectNavigation{false};

        // Handles a completed HTTP response on the UI thread; redirects are
        // fed back through URL parsing and DNS instead of reusing a TCP peer.
        void handleResponse(const HttpResponse &response);

    public:
        explicit BROWSER();
        ~BROWSER();
        URL& Give_Url_address() { return url;}
};
