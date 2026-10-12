#include "TcpConnection.h"
#include <QDebug>
#include <arpa/inet.h>   // inet_pton: text "93.184.216.34" -> binary address
#include <netinet/in.h>  // sockaddr_in: the IPv4 address struct
#include <sys/socket.h>  // socket(), connect(), AF_INET, SOCK_STREAM
#include <unistd.h>      // close()
#include <cerrno>        // errno: per-thread error code set by failed syscalls
#include <cstring>       // strerror: errno number -> human-readable text

TcpConnection::TcpConnection(QObject *parent)
    : QObject(parent)
{
}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::connectToHost(const QHostAddress &ip, quint16 port)
{
    if (m_running) {
        return;  
    }

    m_worker = std::thread([this, ip, port]() {
        m_running = true;

        // ---- 1. Create the socket ----------------------------------
        // socket(domain, type, protocol) returns a "file descriptor"
        // (fd): a small integer the OS uses as a handle for this
        // communication endpoint. Every later call refers to the
        // socket through that number. < 0 means failure.
        //   AF_INET     -> IPv4 addressing
        //   SOCK_STREAM -> reliable byte stream = TCP
        //                  (SOCK_DGRAM would be connectionless UDP)
        //   0           -> default protocol for this type (TCP)
        const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            // The leading "::" means "the GLOBAL function". Qt and
            // the C socket API both define something called connect;
            // "::" disambiguates. strerror() converts the errno that
            // the failed syscall set into readable text.
            emit Is_failed(QString("socket: %1").arg(strerror(errno)));
            m_running = false;
            return;
        }

        // ---- 2. Fill in the destination address --------------------
        // sockaddr_in is the IPv4 flavor of "an address". connect()
        // accepts the generic sockaddr*, so we fill this struct and
        // cast a pointer to it.
        //   sin_family -> must match the socket domain (AF_INET)
        //   sin_port   -> port in NETWORK byte order (big-endian);
        //                 htons = "host to network short" converts
        //   sin_addr   -> the 32-bit IPv4 address, also network order
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        // inet_pton ("presentation to network") parses the dotted
        // text form into the binary 4-byte field. Returns 1 on
        // success, 0 on parse failure. You already met it in
        // DnsResolver -- same tool, opposite direction of inet_ntop.
        if (inet_pton(AF_INET, ip.toString().toUtf8().constData(),
                      &addr.sin_addr) != 1) {
            emit Is_failed(QString("invalid IPv4 address: %1").arg(ip.toString()));
            ::close(fd);
            m_running = false;
            return;
        }

        // ---- 3. Dial the server -------------------------------------
        // connect() performs the TCP three-way handshake (SYN,
        // SYN-ACK, ACK). It BLOCKS until the handshake completes or
        // fails; on failure errno tells you why (ECONNREFUSED,
        // ETIMEDOUT, ENETUNREACH...).
        // TODO (yours):
        //   - call ::connect(fd, reinterpret_cast<const sockaddr*>(&addr),
        //                    sizeof(addr))
        //   - if it fails: emit failed(QString("connect: %1").arg(strerror(errno))),
        //     ::close(fd), m_running = false, return
        //   - if it succeeds: store fd in m_fd and emit connected()

        // ---- 4. Exchange bytes --------------------------------------
        // (Phase 4) ::send() writes request bytes; ::recv() reads the
        // response into a buffer. NOTE for later: recv returning 0
        // means the server closed the connection (EOF); -1 means
        // error; otherwise the return value is how many bytes you
        // got -- it can be FEWER than you asked for, so callers must
        // loop and accumulate. A TCP stream has no "messages".
    });
}
