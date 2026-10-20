#include "HttpResponseParser.h"

// Parses one complete HTTP response out of raw socket bytes.
//
// Wire format we are decoding (from the real Vercel response):
//
//   HTTP/1.0 308 Permanent Redirect\r\n     <- status line
//   Content-Type: text/plain\r\n            \
//   Location: https://uppbeat.io/\r\n        > header lines
//   server: Vercel\r\n                       |
//   \r\n                                    / <- empty line = boundary
//   Redirecting...                          <- body
//
// std::optional<HttpResponse> means "maybe nothing": if any step proves
// the bytes are not a valid response we return std::nullopt and set
// errorMessage instead of throwing or crashing.
std::optional<HttpResponse> HttpResponseParser::parse(
    const QByteArray &raw,
    QString &errorMessage) const
{
    // QByteArray::indexOf finds the first occurrence of a byte pattern and
    // returns its position (qsizetype, Qt's 64-bit signed index type) or -1
    // when absent. The empty line "\r\n\r\n" is the only reliable boundary
    // between headers and body, so without it the response is incomplete.
    static const QByteArray headerBodySeparator = "\r\n\r\n";
    const qsizetype separatorIndex = raw.indexOf(headerBodySeparator);
    if (separatorIndex < 0) {
        errorMessage = QStringLiteral("response has no header/body boundary");
        return std::nullopt;
    }

    // left(n) copies the first n bytes (everything before the boundary),
    // mid(n) copies from offset n onward. mid() starts after the separator
    // itself, so the body contains no leading "\r\n\r\n".
    const QByteArray head = raw.left(separatorIndex);
    const QByteArray body = raw.mid(separatorIndex + headerBodySeparator.size());

    // The status line is head's first line. If head contains no "\r\n" at
    // all it consists of only the status line, and the header block is empty.
    const qsizetype statusLineEnd = head.indexOf("\r\n");
    const QByteArray statusLine = (statusLineEnd < 0)
        ? head
        : head.left(statusLineEnd);
    const QByteArray headerBlock = (statusLineEnd < 0)
        ? QByteArray()
        : head.mid(statusLineEnd + 2); // +2 skips the line's own "\r\n"

    HttpResponse response;

    // ---- Status line: "HTTP/1.0 308 Permanent Redirect" -----------------
    // We cannot just split(' ') and take three tokens, because the reason
    // phrase may contain spaces ("Permanent Redirect") or be missing
    // entirely ("HTTP/1.1 204"). So we locate the first two spaces and
    // treat everything after the second one as the reason.
    const qsizetype firstSpace = statusLine.indexOf(' ');
    if (firstSpace < 0) {
        errorMessage = QStringLiteral("malformed status line: %1")
                           .arg(QString::fromLatin1(statusLine));
        return std::nullopt;
    }
    const qsizetype secondSpace = statusLine.indexOf(' ', firstSpace + 1);

    // Version token: first word. Real responses start with "HTTP/";
    // anything else means these bytes are not an HTTP response.
    response.version = QString::fromLatin1(statusLine.left(firstSpace));
    if (!response.version.startsWith(QStringLiteral("HTTP/"))) {
        errorMessage = QStringLiteral("not an HTTP response: %1")
                           .arg(response.version);
        return std::nullopt;
    }

    // Status code token: between the first and second space. toInt(&ok)
    // writes true/false into `ok` - the pointer-to-flag pattern lets us
    // detect "not a number" instead of silently getting 0.
    const QByteArray codeToken = (secondSpace < 0)
        ? statusLine.mid(firstSpace + 1)
        : statusLine.mid(firstSpace + 1, secondSpace - firstSpace - 1);
    bool codeOk = false;
    const int statusCode = codeToken.toInt(&codeOk);
    if (!codeOk || statusCode < 100 || statusCode > 599) {
        errorMessage = QStringLiteral("invalid status code: %1")
                           .arg(QString::fromLatin1(codeToken));
        return std::nullopt;
    }
    response.statusCode = statusCode;

    // Reason phrase: everything after the second space (empty when absent).
    if (secondSpace >= 0) {
        response.reasonPhrase = QString::fromLatin1(statusLine.mid(secondSpace + 1));
    }

    // ---- Header lines: "Name: value" ------------------------------------
    // QByteArray::split only accepts a single-character separator, so we
    // split on '\n' and strip each line's trailing '\r' ourselves. A side
    // benefit: responses that (incorrectly) use bare Unix '\n' line
    // endings still parse instead of failing.
    const QList<QByteArray> headerLines = headerBlock.split('\n');
    for (QByteArray line : headerLines) {
        if (!line.isEmpty() && line.endsWith('\r')) {
            line.chop(1); // remove the '\r' of a "\r\n" line ending
        }
        if (line.isEmpty()) {
            continue;
        }
        const qsizetype colonIndex = line.indexOf(':');
        if (colonIndex <= 0) {
            // No colon, or a colon at position 0 (empty name). We fail
            // strictly on purpose: silently skipping malformed headers
            // hides bugs in our own receive loop. Real browsers are more
            // lenient because the web is messy; we can relax later.
            errorMessage = QStringLiteral("malformed header line: %1")
                               .arg(QString::fromLatin1(line));
            return std::nullopt;
        }

        // Headers are historically Latin-1 (ISO-8859-1) text per the HTTP
        // spec; fromLatin1 maps every byte to a character and never fails.
        // trim() strips optional surrounding whitespace: RFC 9110 allows
        // "Name:value" and "Name : value" to mean the same thing.
        const QString name = QString::fromLatin1(line.left(colonIndex)).trimmed();
        const QString value = QString::fromLatin1(line.mid(colonIndex + 1)).trimmed();

        response.headers.append({name, value});
    }

    // The body stays raw: no chunked decoding, no gunzip, no charset
    // conversion yet. Those are Phase 6 follow-ups, each applied only when
    // the matching header (Transfer-Encoding / Content-Encoding) says so.
    response.body = body;

    return response;
}
