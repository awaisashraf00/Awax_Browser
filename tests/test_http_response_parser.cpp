#include "HttpResponseParser.h"

#include <cstdio>

// Returns the header value or "MISSING" for test assertions.
static QString valueOrMissing(const HttpResponse &response, const char *name)
{
    const std::optional<QString> value = response.headerValue(QString::fromLatin1(name));
    return value.value_or(QStringLiteral("MISSING"));
}

int main()
{
    HttpResponseParser parser;

    // ---- Test 1: the real 308 response received from uppbeat.io ---------
    const QByteArray redirectResponse =
        "HTTP/1.0 308 Permanent Redirect\r\n"
        "Content-Type: text/plain\r\n"
        "Location: https://uppbeat.io/\r\n"
        "Refresh: 0;url=https://uppbeat.io/\r\n"
        "server: Vercel\r\n"
        "\r\n"
        "Redirecting...";

    QString errorMessage;
    const std::optional<HttpResponse> redirect = parser.parse(redirectResponse, errorMessage);
    if (!redirect.has_value()) {
        std::printf("Real-world 308 response failed to parse: %s\n",
                    errorMessage.toUtf8().constData());
        return 1;
    }
    if (redirect->statusCode != 308 || redirect->version != "HTTP/1.0" ||
        redirect->reasonPhrase != "Permanent Redirect") {
        std::printf("308 status line was parsed incorrectly.\n");
        return 1;
    }
    if (!redirect->isRedirect()) {
        std::printf("308 was not recognized as a redirect.\n");
        return 1;
    }
    if (redirect->headers.size() != 4) {
        std::printf("Expected 4 headers, got %lld\n",
                    static_cast<long long>(redirect->headers.size()));
        return 1;
    }
    // The server sent lowercase "server:" - lookup must still be
    // case-insensitive in both directions.
    if (valueOrMissing(*redirect, "SERVER") != "Vercel" ||
        valueOrMissing(*redirect, "Location") != "https://uppbeat.io/") {
        std::printf("Case-insensitive header lookup failed.\n");
        return 1;
    }
    // A header that does not exist must read as "absent", not crash.
    if (redirect->headerValue(QStringLiteral("Content-Encoding")).has_value()) {
        std::printf("Missing header wrongly reported as present.\n");
        return 1;
    }
    if (redirect->body != "Redirecting...") {
        std::printf("Redirect body was mangled: %s\n", redirect->body.constData());
        return 1;
    }

    // ---- Test 2: ordinary 200 response with a body ----------------------
    const QByteArray okResponse =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: 11\r\n"
        "\r\n"
        "<h1>Hi</h1>";

    const std::optional<HttpResponse> ok = parser.parse(okResponse, errorMessage);
    if (!ok.has_value()) {
        std::printf("200 response failed to parse: %s\n",
                    errorMessage.toUtf8().constData());
        return 1;
    }
    if (ok->statusCode != 200 || ok->isRedirect()) {
        std::printf("200 response misclassified.\n");
        return 1;
    }
    if (ok->body != "<h1>Hi</h1>") {
        std::printf("200 body was mangled: %s\n", ok->body.constData());
        return 1;
    }

    // ---- Test 3: status line without a reason phrase --------------------
    const std::optional<HttpResponse> bare =
        parser.parse(QByteArray("HTTP/1.1 204\r\n\r\n"), errorMessage);
    if (!bare.has_value() || bare->statusCode != 204 ||
        !bare->reasonPhrase.isEmpty() || bare->body.isEmpty()) {
        std::printf("Reason-less status line should parse with empty reason.\n");
        return 1;
    }

    // ---- Test 4: garbage must fail with a message, not crash ------------
    const std::optional<HttpResponse> incomplete =
        parser.parse(QByteArray("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"),
                     errorMessage);
    if (incomplete.has_value() || errorMessage.isEmpty()) {
        std::printf("Response without header/body boundary should fail.\n");
        return 1;
    }

    const std::optional<HttpResponse> notHttp =
        parser.parse(QByteArray("GARBAGE DATA\r\n\r\n"), errorMessage);
    if (notHttp.has_value()) {
        std::printf("Non-HTTP bytes should fail.\n");
        return 1;
    }

    std::printf("All HTTP response parser tests passed.\n");
    return 0;
}
