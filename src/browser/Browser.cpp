#include "Browser.h"

// BROWSER is the wiring layer: it owns the pipeline objects and connects
// their signals. It contains NO thread management of its own -- threading
// is an implementation detail of each component (DNS uses a QThread event
// loop, TcpConnection uses its own std::thread worker).
BROWSER::BROWSER()
{
    // ---- Worker-object pattern (the canonical QThread architecture) ----

    // 1. Create the worker with no parent (see header comment).
    dns = new DnsResolver();

    // 2. Move the worker onto the DNS thread. From this moment, any slot
    // invoked on `dns` through a QUEUED connection executes on
    // m_dns_thread instead of the GUI thread.
    dns->moveToThread(&m_dns_thread);

    // 3. When the thread's event loop stops, delete the worker.
    // deleteLater() posts a "delete this" event that the thread processes
    // during its shutdown sequence. Never `delete` a worker by hand while
    // its thread may still be running -- this connection does it safely.
    QObject::connect(&m_dns_thread, &QThread::finished,
                     dns, &QObject::deleteLater);

    // 4. Start the thread's event loop. It now sits waiting for queued
    // work (i.e. resolve() calls) without burning CPU.
    m_dns_thread.start();

    // (a) Runs on the MAIN thread: the context object open_connection
    // lives there, and the signal is emitted from the main thread, so Qt
    // calls this lambda directly. It only touches main-thread state
    // (m_port_num), which is why no mutex is needed.
    QObject::connect(&url, &URL::navigationRequested,
                     &open_connection, [this](const ParsedUrl &parsed) {
        m_port_num = parsed.port;
    });

    // (b) Runs on the DNS thread: the context object `dns` was moved to
    // m_dns_thread, so Qt delivers this connection QUEUED -- the lambda
    // (and the blocking getaddrinfo() inside resolve) executes on the
    // worker. The GUI thread never blocks on DNS.
    // Init-capture `srv = dns`: copies the POINTER VALUE at connect time.
    // A class member cannot be captured by name ([dns] is ill-formed --
    // members are not variables), and [this] would drag the whole browser
    // in when the lambda only needs the worker.
    QObject::connect(&url, &URL::navigationRequested,
                     dns, [srv = dns](const ParsedUrl &parsed) {
        srv->resolve(parsed.host);
    });

    // (c) resolved() is emitted FROM the DNS thread; the receiver context
    // lives on the main thread, so Qt automatically queues the delivery
    // back. This signal hop is the thread boundary of the pipeline:
    // DNS worker  --queued-->  main thread  --own thread-->  TCP worker
    QObject::connect(dns, &DnsResolver::resolved,
                     &open_connection, [this](const QHostAddress &address_of_server) {
        open_connection.connectToHost(address_of_server, m_port_num);
    });
}

// Shutdown order matters: first ask the event loop to exit, then WAIT
// until the thread has fully finished. After wait() returns, no queued
// resolve() can still be running and the worker has already been deleted
// via deleteLater -- so the QThread destructor finds no live thread.
BROWSER::~BROWSER()
{
    m_dns_thread.quit();
    m_dns_thread.wait();
}

// Navigation is driven by the URL object's signals; RUN() is the
// application entry point hook and currently has nothing to do.
void BROWSER::RUN()
{
}
