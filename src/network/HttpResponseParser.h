#pragma once

#include "HttpResponse.h"
#include <QString>
#include <QByteArray>
#include <optional>

// Turns raw response bytes straight off the socket into an HttpResponse.
//
// Where it fits in the pipeline:
//
//   TcpConnection recv loop -> accumulated QByteArray -> THIS parser
//                           -> HttpResponse -> navigation/redirect logic
//
// Design notes:
// - parse() is total: it either returns a value or explains via
//   `errorMessage` why the bytes are not a usable response. It never
//   throws and never crashes on garbage.
// - The parser is stateless for now. That is only correct because the
//   browser currently uses "Connection: close" and reads until EOF, so
//   one accumulated buffer == exactly one response. Once we keep
//   connections alive, responses can end mid-buffer and a later version
//   of this class will grow incremental state (a leftover-bytes buffer).
struct HttpResponseParser
{
    // Parses one complete response from `raw`.
    // Returns std::optional<HttpResponse>: a value on success,
    // std::nullopt on failure with `errorMessage` describing why.
    std::optional<HttpResponse> parse(const QByteArray &raw,
                                      QString &errorMessage) const;
};
