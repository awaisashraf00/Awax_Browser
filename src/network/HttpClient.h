#pragma once
#include "ProtocolPorts.h"
#include "HttpResponse.h"
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <QDebug>
#include <QByteArray>
#include <QString>
#include <QtGlobal>

class HttpClient
{
public:
    // Serializes the HTTP/1.1 request bytes from parsed URL components;
    // the fragment is intentionally absent because it is client-side.
    QByteArray buildRequest(const QString &host,
                            quint16 port,
                            const QString &path,
                            const QString &query) const;

    // Performs the full HTTP exchange on an already-connected socket:
    // writes the request bytes, reads the response until the server
    // closes the connection (EOF), then parses the accumulated bytes
    // into `response` via HttpResponseParser. Returns false with a
    // description in `errorMessage` on any failure - send error,
    // receive error, or bytes that are not a parsable HTTP response.
    bool sendRequest(int socketFd,
                     const QString &host,
                     quint16 port,
                     const QString &path,
                     const QString &query,
                     QString &errorMessage,
                     HttpResponse &response) const;

private:
    // Receive half of the exchange: reads chunks until EOF, accumulates
    // them, and parses. Kept separate from the send loop so each half
    // stays small and readable; sendRequest just chains the two.
    bool receiveAndParse(int socketFd,
                         HttpResponse &response,
                         QString &errorMessage) const;
};
