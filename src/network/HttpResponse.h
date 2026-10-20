#pragma once
#include <QString>
#include <QByteArray>
#include <QList>
#include <QPair>
#include <optional>

// The parsed result of one HTTP response, produced by HttpResponseParser.
// This is the value type that sits at the Phase 4 -> Phase 6 boundary:
// raw network bytes go in (to the parser), and this structured view comes
// out so the rest of the browser never touches raw bytes again.
//
// Why a plain struct and not a class with logic: it is a dumb data holder
// (a "value type"). All the hard work lives in the parser, which keeps
// this type trivial to construct in tests.
struct HttpResponse
{
    // Protocol version the server answered with, e.g. "HTTP/1.0".
    // Note: this can differ from the version we requested (we send 1.1,
    // Vercel answered 1.0) - the server picks the response version.
    QString version;

    // The 3-digit status code, e.g. 200, 308, 404.
    int statusCode{0};

    // Human-readable reason phrase, e.g. "Permanent Redirect".
    // Purely informational; only the number is meaningful to machines.
    QString reasonPhrase;

    // Headers in original order, as (name, value) pairs.
    // QList + QPair instead of QHash because HTTP allows the same header
    // name to appear multiple times (Set-Cookie is the famous example),
    // and a hash map would silently keep only the last one.
    QList<QPair<QString, QString>> headers;

    // Payload after the empty line, still raw and undecoded.
    // Compression (gzip), chunked transfer decoding and character
    // encoding conversion are later phases - this is exactly the bytes
    // the server sent after "\r\n\r\n".
    QByteArray body;

    // True when the status code is one of the redirect codes a browser
    // follows: 301/308 (moved permanently), 302/307 (temporary).
    bool isRedirect() const;

    // Case-insensitive lookup of the first header named `name`.
    // Returns std::optional<QString>: either the header's value, or
    // "nothing" when the header is absent. Header names are
    // case-insensitive per RFC 9110, so "Location" and "location"
    // must both match.
    std::optional<QString> headerValue(const QString &name) const;
};
