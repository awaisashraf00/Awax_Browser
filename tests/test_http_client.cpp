#include "HttpClient.h"

#include <cstdio>


int main()
{
    const HttpClient client;
    const QByteArray request = client.buildRequest(
        QStringLiteral("example.com"),
        80,
        QStringLiteral("/search"),
        QStringLiteral("q=browser"));
    const QByteArray expected =
        "GET /search?q=browser HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Connection: close\r\n\r\n";

    if (request != expected) {
        std::printf("HTTP request serialization did not match the expected wire format.\n");
        return 1;
    }

    const QByteArray nonDefaultPortRequest = client.buildRequest(
        QStringLiteral("example.com"),
        8080,
        QStringLiteral("/"),
        QString());
    if (!nonDefaultPortRequest.contains("Host: example.com:8080\r\n")) {
        std::printf("HTTP Host header omitted a non-default port.\n");
        return 1;
    }

    std::printf("All HTTP client tests passed.\n");
    return 0;
}
