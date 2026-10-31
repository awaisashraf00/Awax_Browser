#include "HttpResponseParser.h"

#include <QStringList>

namespace {

// Decodes HTTP's byte-counted chunk framing so HttpResponse::body contains
// document bytes rather than the wire-level size lines and delimiters.
bool decodeChunkedBody(const QByteArray &encoded,
                       QByteArray &decoded,
                       QString &errorMessage)
{
    decoded.clear();
    qsizetype position = 0;

    while (true) {
        const qsizetype sizeLineEnd = encoded.indexOf("\r\n", position);
        if (sizeLineEnd < 0) {
            errorMessage = QStringLiteral("chunked body ended before a chunk-size line");
            return false;
        }

        const QByteArray sizeLine = encoded.mid(position, sizeLineEnd - position);
        const qsizetype extensionStart = sizeLine.indexOf(';');
        const QByteArray sizeToken = extensionStart < 0
            ? sizeLine
            : sizeLine.left(extensionStart);
        if (sizeToken.isEmpty()) {
            errorMessage = QStringLiteral("chunked body contains an empty chunk size");
            return false;
        }
        for (const char digit : sizeToken) {
            const bool isHexDigit = (digit >= '0' && digit <= '9') ||
                                    (digit >= 'a' && digit <= 'f') ||
                                    (digit >= 'A' && digit <= 'F');
            if (!isHexDigit) {
                errorMessage = QStringLiteral("chunked body contains an invalid chunk size");
                return false;
            }
        }

        // HTTP chunk sizes are hexadecimal byte counts; the ok flag also
        // catches values that cannot fit in Qt's unsigned-long-long type.
        bool sizeOk = false;
        const qulonglong chunkSize = sizeToken.toULongLong(&sizeOk, 16);
        if (!sizeOk) {
            errorMessage = QStringLiteral("chunked body contains an invalid chunk size");
            return false;
        }

        position = sizeLineEnd + 2;
        if (chunkSize == 0) {
            while (true) {
                const qsizetype trailerLineEnd = encoded.indexOf("\r\n", position);
                if (trailerLineEnd < 0) {
                    errorMessage = QStringLiteral("chunked body ended before the trailer terminator");
                    return false;
                }

                if (trailerLineEnd == position) {
                    position += 2;
                    if (position != encoded.size()) {
                        errorMessage = QStringLiteral("unexpected bytes after the chunked body");
                        return false;
                    }
                    return true;
                }

                const QByteArray trailerLine =
                    encoded.mid(position, trailerLineEnd - position);
                if (trailerLine.indexOf(':') <= 0 ||
                    trailerLine.startsWith(' ') ||
                    trailerLine.startsWith('\t')) {
                    errorMessage = QStringLiteral("chunked body contains a malformed trailer");
                    return false;
                }
                position = trailerLineEnd + 2;
            }
        }

        const qsizetype remaining = encoded.size() - position;
        if (remaining < 2 ||
            chunkSize > static_cast<qulonglong>(remaining - 2)) {
            errorMessage = QStringLiteral("chunked body ended before a chunk was complete");
            return false;
        }

        const qsizetype chunkLength = static_cast<qsizetype>(chunkSize);
        // Copy exactly chunkLength raw bytes; chunk contents are not text.
        decoded.append(encoded.constData() + position, chunkLength);
        position += chunkLength;
        if (encoded.mid(position, 2) != "\r\n") {
            errorMessage = QStringLiteral("chunked body is missing a chunk-data terminator");
            return false;
        }
        position += 2;
    }
}

}

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
    errorMessage.clear();

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

    QStringList transferCodings;
    QStringList contentLengthValues;
    for (const auto &header : response.headers) {
        if (header.first.compare(QStringLiteral("Transfer-Encoding"),
                                 Qt::CaseInsensitive) == 0) {
            for (const QString &coding : header.second.split(QLatin1Char(','))) {
                transferCodings.append(coding.trimmed());
            }
        } else if (header.first.compare(QStringLiteral("Content-Length"),
                                        Qt::CaseInsensitive) == 0) {
            for (const QString &length : header.second.split(QLatin1Char(','))) {
                contentLengthValues.append(length.trimmed());
            }
        }
    }

    if (!transferCodings.isEmpty() && !contentLengthValues.isEmpty()) {
        errorMessage = QStringLiteral(
            "response contains both Transfer-Encoding and Content-Length");
        return std::nullopt;
    }

    std::optional<qulonglong> contentLength;
    for (const QString &value : contentLengthValues) {
        if (value.isEmpty()) {
            errorMessage = QStringLiteral("response contains an empty Content-Length");
            return std::nullopt;
        }
        const QByteArray lengthBytes = value.toLatin1();
        for (const char digit : lengthBytes) {
            if (digit < '0' || digit > '9') {
                errorMessage = QStringLiteral("response contains an invalid Content-Length");
                return std::nullopt;
            }
        }

        bool lengthOk = false;
        const qulonglong parsedLength = lengthBytes.toULongLong(&lengthOk);
        if (!lengthOk || (contentLength && *contentLength != parsedLength)) {
            errorMessage = QStringLiteral("response contains an invalid or conflicting Content-Length");
            return std::nullopt;
        }
        contentLength = parsedLength;
    }

    if (!transferCodings.isEmpty()) {
        if (transferCodings.size() != 1 ||
            transferCodings.first().compare(QStringLiteral("chunked"),
                                             Qt::CaseInsensitive) != 0) {
            errorMessage = QStringLiteral("unsupported Transfer-Encoding: %1")
                               .arg(transferCodings.join(QStringLiteral(", ")));
            return std::nullopt;
        }
        if (!decodeChunkedBody(body, response.body, errorMessage)) {
            return std::nullopt;
        }
    } else {
        const bool statusHasNoBody =
            (response.statusCode >= 100 && response.statusCode < 200) ||
            response.statusCode == 204 || response.statusCode == 304;
        if (statusHasNoBody && !body.isEmpty()) {
            errorMessage = QStringLiteral("response status code must not include a body");
            return std::nullopt;
        }
        if (contentLength && response.statusCode != 304 &&
            *contentLength != static_cast<qulonglong>(body.size())) {
            errorMessage = QStringLiteral("response body length does not match Content-Length");
            return std::nullopt;
        }
        response.body = body;
    }

    return response;
}
