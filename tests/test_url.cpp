// Verifies URL::parse() against the cases a real browser must handle
// before anything reaches the network: scheme defaulting, component
// splitting, default ports/paths, and rejection of malformed input.
#include "URL.h"
#include <cstdio>

static int g_failures = 0;

#define CHECK(cond)                                                     \
    do {                                                                \
        if (!(cond)) {                                                  \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                               \
        }                                                               \
    } while (false)

static void checkParsed(URL &url, const char *raw, const char *scheme,
                        const char *host, quint16 port, const char *path)
{
    if (!url.parse(raw)) {
        std::printf("FAIL: could not parse \"%s\"\n", raw);
        ++g_failures;
        return;
    }
    const ParsedUrl &p = url.parsed();
    CHECK(p.scheme == scheme);
    CHECK(p.host == host);
    CHECK(p.port == port);
    CHECK(p.path == path);
}

int main()
{
    URL url;

    // Simple https URL, no path -> default port 443 and path "/".
    checkParsed(url, "https://example.com", "https", "example.com", 443, "/");

    // Explicit port, mixed-case host (must be lowercased), path,
    // query and fragment all present.
    checkParsed(url, "http://Example.COM:8080/a/b?x=1#top",
                "http", "example.com", 8080, "/a/b");
    CHECK(url.parsed().query == "x=1");
    CHECK(url.parsed().fragment == "top");

    // Redirect references are resolved against the current page before the
    // result enters the normal URL parsing and DNS navigation pipeline.
    QString redirectError;
    url.url_write(QStringLiteral("https://example.com/dir/page?old=1"));
    const std::optional<QString> absoluteRedirect =
        url.resolveReference(QStringLiteral("https://other.example/new"), redirectError);
    CHECK(absoluteRedirect == QStringLiteral("https://other.example/new"));
    const std::optional<QString> rootRelativeRedirect =
        url.resolveReference(QStringLiteral("/login"), redirectError);
    CHECK(rootRelativeRedirect == QStringLiteral("https://example.com/login"));
    const std::optional<QString> pathRelativeRedirect =
        url.resolveReference(QStringLiteral("../next"), redirectError);
    CHECK(pathRelativeRedirect == QStringLiteral("https://example.com/next"));
    const std::optional<QString> unsupportedRedirect =
        url.resolveReference(QStringLiteral("ftp://example.com/file"), redirectError);
    CHECK(!unsupportedRedirect.has_value());
    CHECK(!redirectError.isEmpty());

    // Plain http gets the default port 80.
    checkParsed(url, "http://example.com", "http", "example.com", 80, "/");

    // Scheme-less input is completed to https.
    checkParsed(url, "example.com", "https", "example.com", 443, "/");
    checkParsed(url, "localhost:3000", "https", "localhost", 3000, "/");

    // Empty port ("host:") means "use the scheme default".
    checkParsed(url, "https://example.com:/", "https", "example.com", 443, "/");

    // Rejections: malformed input must never reach the network stage.
    CHECK(!url.parse(""));
    CHECK(!url.parse("   "));
    CHECK(!url.parse("https:///path"));            // empty host
    CHECK(!url.parse("https://exa mple.com"));     // space in host
    CHECK(!url.parse("https://example.com:99999")); // port out of range
    CHECK(!url.parse("https://example.com:abc"));   // port not numeric
    CHECK(!url.parse("ftp://example.com"));         // unsupported scheme (v1)

    if (g_failures == 0) {
        std::printf("All URL tests passed.\n");
        return 0;
    }
    std::printf("%d URL test(s) failed.\n", g_failures);
    return 1;
}
