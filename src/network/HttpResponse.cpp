#include "HttpResponse.h"

// Redirect detection is the browser's trigger for "start a new navigation
// with the Location URL instead of treating the body as the document".
// 301/308 = permanent move (bookmarks should be updated),
// 302/307 = temporary move (come back to this URL next time).
bool HttpResponse::isRedirect() const
{
    return statusCode == 301 || statusCode == 302 ||
           statusCode == 307 || statusCode == 308;
}

// Linear scan for the first header whose name matches case-insensitively.
// A linear search is fine: responses have a handful of headers. The
// comparison uses QString::compare with Qt::CaseInsensitive, which returns
// 0 when the two strings are equal ignoring case.
//
// std::nullopt is the "no value" state of std::optional - the caller can
// distinguish "header missing" from "header present but empty".
std::optional<QString> HttpResponse::headerValue(const QString &name) const
{
    for (const auto &header : headers) {
        if (header.first.compare(name, Qt::CaseInsensitive) == 0) {
            return header.second;
        }
    }
    return std::nullopt;
}
