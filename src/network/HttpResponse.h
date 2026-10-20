#pragma once
#include <QString>
#include <QByteArray>
#include <QList>
#include <QPair>
#include <QMetaType>
#include <optional>

struct HttpResponse
{
    QString version;

    int statusCode{0};

    QString reasonPhrase;

    QList<QPair<QString, QString>> headers;

    QByteArray body;

    bool isRedirect() const;

    std::optional<QString> headerValue(const QString &name) const;
};

// Q_DECLARE_METATYPE tells Qt's meta-object system that HttpResponse is a
// copyable type it must support in QVariant and in QUEUED signal/slot
// connections. This matters here: TcpConnection lives on the TCP worker
// thread and emits responseReceived toward objects on the main thread, so
// the argument is copied into a queued event. Without this declaration Qt
// fails at runtime with "Cannot queue arguments of type 'HttpResponse'".
Q_DECLARE_METATYPE(HttpResponse)
