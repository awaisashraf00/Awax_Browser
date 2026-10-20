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
std::optional<struct HttpResponse> HttpResponseParser::parse(
    const QByteArray &raw,
    QString &errorMessage) const
{
    static const QByteArray headerBodySeparator = "\r\n\r\n";
    const qsizetype separatorIndex = raw.indexOf(headerBodySeparator);
    if (separatorIndex < 0) {
        errorMessage = QStringLiteral("response has no header/body boundary");
        return std::nullopt;
    }

    const QByteArray head = raw.left(separatorIndex);
    const QByteArray body = raw.mid(separatorIndex + headerBodySeparator.size());


    const qsizetype statusLineEnd = head.indexOf("\r\n");
    const QByteArray statusLine = (statusLineEnd < 0)
        ? head
        : head.left(statusLineEnd);
    const QByteArray headerBlock = (statusLineEnd < 0)
        ? QByteArray()
        : head.mid(statusLineEnd + 2);

    HttpResponse response;

    const qsizetype firstSpace = statusLine.indexOf(' ');
    if (firstSpace < 0) {
        errorMessage = QStringLiteral("malformed status line: %1")
                           .arg(QString::fromLatin1(statusLine));
        return std::nullopt;
    }
    const qsizetype secondSpace = statusLine.indexOf(' ', firstSpace + 1);

    response.version = QString::fromLatin1(statusLine.left(firstSpace));
    if (!response.version.startsWith(QStringLiteral("HTTP/"))) {
        errorMessage = QStringLiteral("not an HTTP response: %1")
                           .arg(response.version);
        return std::nullopt;
    }

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

    if (secondSpace >= 0) {
        response.reasonPhrase = QString::fromLatin1(statusLine.mid(secondSpace + 1));
    }

    const QList<QByteArray> headerLines = headerBlock.split('\n');
    for (QByteArray line : headerLines) {
        if (!line.isEmpty() && line.endsWith('\r')) {
            line.chop(1);
        }
        if (line.isEmpty()) {
            continue;
        }
        const qsizetype colonIndex = line.indexOf(':');
        if (colonIndex <= 0) {
            errorMessage = QStringLiteral("malformed header line: %1")
                               .arg(QString::fromLatin1(line));
            return std::nullopt;
        }
        const QString name = QString::fromLatin1(line.left(colonIndex)).trimmed();
        const QString value = QString::fromLatin1(line.mid(colonIndex + 1)).trimmed();

        response.headers.append({name, value});
    }

    response.body = body;

    return response;
}
