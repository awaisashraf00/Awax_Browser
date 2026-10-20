#pragma once

#include "HttpResponse.h"
#include <QString>
#include <QByteArray>
#include <optional>

struct HttpResponseParser
{
    std::optional<struct HttpResponse> parse(const QByteArray &raw,
                                      QString &errorMessage) const;
};
