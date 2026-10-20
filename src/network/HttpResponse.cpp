#include "HttpResponse.h"

bool HttpResponse::isRedirect() const
{
    return statusCode == 301 || statusCode == 302 || statusCode == 303 ||
           statusCode == 307 || statusCode == 308;
}

std::optional<QString> HttpResponse::headerValue(const QString &name) const
{
    for (const auto &header : headers) {
        if (header.first.compare(name, Qt::CaseInsensitive) == 0) {
            return header.second;
        }
    }
    return std::nullopt;
}
