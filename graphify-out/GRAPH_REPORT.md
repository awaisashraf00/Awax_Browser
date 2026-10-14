# Graph Report - Awax_browser  (2026-10-13)

## Corpus Check
- Corpus is ~2,248 words - fits in a single context window. You may not need a graph.

## Summary
- 146 nodes · 171 edges · 21 communities (13 shown, 8 thin omitted)
- Extraction: 98% EXTRACTED · 2% INFERRED · 0% AMBIGUOUS · INFERRED: 4 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- URL Parsing
- DNS and POSIX Sockets
- TCP Connection
- Browser Controller
- HTTP Server
- DNS Resolver API
- Network Recommendations
- Parsed URL Model
- Graphify Instructions
- CMake Application Setup
- Chromium Networking
- URL Parser Tests
- DNS Resolution
- HTTP Response Framing
- Blocking GUI I/O
- Qt Socket Notifications
- Web Origin Security
- DNS over HTTPS
- HTTP Strict Transport Security

## God Nodes (most connected - your core abstractions)
1. `TcpConnection` - 18 edges
2. `DnsResolver` - 12 edges
3. `ParsedUrl` - 11 edges
4. `BROWSER` - 10 edges
5. `checkParsed()` - 5 edges
6. `main()` - 4 edges
7. `Graphify` - 4 edges
8. `POSIX Socket API` - 4 edges
9. `main()` - 3 edges
10. `Server_State` - 3 edges

## Surprising Connections (you probably didn't know these)
- `main()` --references--> `URL`  [INFERRED]
  tests/test_url.cpp → src/url/URL.h
- `checkParsed()` --references--> `URL`  [EXTRACTED]
  tests/test_url.cpp → src/url/URL.h
- `BROWSER` --references--> `DnsResolver`  [EXTRACTED]
  src/browser/Browser.h → src/network/DnsResolver.h
- `BROWSER` --references--> `TcpConnection`  [EXTRACTED]
  src/browser/Browser.h → src/network/TcpConnection.h
- `BROWSER` --references--> `URL`  [EXTRACTED]
  src/browser/Browser.h → src/url/URL.h

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **TCP, TLS, and HTTP Layering** — recommendation_tcp_connection, recommendation_tls, recommendation_http11 [EXTRACTED 1.00]

## Communities (21 total, 8 thin omitted)

### Community 0 - "URL Parsing"
Cohesion: 0.15
Nodes (5): URL, m_parsed, m_url, checkParsed(), main()

### Community 1 - "DNS and POSIX Sockets"
Cohesion: 0.11
Nodes (3): DnsResolver::DnsResolver(), HttpClient, TcpConnection::TcpConnection()

### Community 2 - "TCP Connection"
Cohesion: 0.13
Nodes (6): TcpConnection, m_fd, m_running, m_worker, mtx_lock, public

### Community 3 - "Browser Controller"
Cohesion: 0.15
Nodes (6): BROWSER, dns, m_port_num, open_connection, url, main()

### Community 4 - "HTTP Server"
Cohesion: 0.18
Nodes (5): REQUEST_SERVER, Prepare_Request, Server_State, Created, NotCreated

### Community 5 - "DNS Resolver API"
Cohesion: 0.24
Nodes (4): DnsResolver, m_address, m_host, public

### Community 6 - "Network Recommendations"
Cohesion: 0.22
Nodes (6): HTTP/1.1, Network Byte Order, POSIX Socket API, sockaddr_in, TCP Connection, TLS

### Community 7 - "Parsed URL Model"
Cohesion: 0.22
Nodes (8): ParsedUrl, fragment, host, isValid, path, port, query, scheme

### Community 8 - "Graphify Instructions"
Cohesion: 0.40
Nodes (4): Existing Knowledge Graph, Graphify, Graphify Report, Graphify Wiki Index

### Community 9 - "CMake Application Setup"
Cohesion: 0.50
Nodes (4): Awax_browser Executable, Browser Core Components, C++17, Qt Quick and QML UI

### Community 10 - "Chromium Networking"
Cohesion: 0.50
Nodes (4): Chromium Network Stack, Connection Reuse, HostResolver, Chromium Socket Pools

### Community 12 - "DNS Resolution"
Cohesion: 0.67
Nodes (3): Chromium Asynchronous DNS Resolver, DNS Resolution, getaddrinfo

### Community 13 - "HTTP Response Framing"
Cohesion: 0.67
Nodes (3): Content-Length, Chunked Transfer Encoding, HTTP Response Framing

## Knowledge Gaps
- **48 isolated node(s):** `url`, `dns`, `open_connection`, `m_port_num`, `public` (+43 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 90 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **8 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `URL` connect `URL Parsing` to `Browser Controller`, `DNS Resolver API`, `Parsed URL Model`?**
  _High betweenness centrality (0.164) - this node is a cross-community bridge._
- **What connects `url`, `dns`, `open_connection` to the rest of the system?**
  _48 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `URL Parsing` be split into smaller, more focused modules?**
  _Cohesion score 0.14761904761904762 - nodes in this community are weakly interconnected._
- **Why does `TcpConnection` connect `TCP Connection` to `Browser Controller`, `HTTP Server`?**
  _High betweenness centrality (0.153) - this node is a cross-community bridge._
- **Should `DNS and POSIX Sockets` be split into smaller, more focused modules?**
  _Cohesion score 0.10526315789473684 - nodes in this community are weakly interconnected._
- **Why does `BROWSER` connect `Browser Controller` to `URL Parsing`, `TCP Connection`, `DNS Resolver API`?**
  _High betweenness centrality (0.134) - this node is a cross-community bridge._
- **Should `TCP Connection` be split into smaller, more focused modules?**
  _Cohesion score 0.13071895424836602 - nodes in this community are weakly interconnected._