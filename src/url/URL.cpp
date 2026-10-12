#include "URL.h"
#include <ProtocolPorts.h>
#include <QDebug>

URL::URL(QObject *parent)
    : QObject(parent)
{
}

bool URL::parse(const QString &raw)
{
    m_parsed = ParsedUrl{};

    QString s = raw.trimmed();
    if (s.isEmpty()) {
        qDebug() << "URL: empty input";
        return false;
    }
    for (const QChar c : s) {
        if (c.isSpace() || c.unicode() < 0x20 || c.unicode() == 0x7F) {
            qDebug() << "URL: whitespace/control character in:" << s;
            return false;
        }
    }

    if (!s.contains("://")) {
        s.prepend("https://");
    }
    const qsizetype schemeEnd = s.indexOf("://");
    m_parsed.scheme = s.left(schemeEnd).toLower();

    // Our network stack only speaks http and https for now.
    if (m_parsed.scheme != "http" && m_parsed.scheme != "https") {
        qDebug() << "URL: unsupported scheme:" << m_parsed.scheme;
        return false;
    }

    QString rest = s.mid(schemeEnd + 3);

    // Fragment: the FIRST '#' starts it; everything after belongs to
    // the fragment even if it contains '?' or '/'.
    const qsizetype hash = rest.indexOf('#');
    if (hash != -1) {   
        m_parsed.fragment = rest.mid(hash + 1);
        rest.truncate(hash);
    }

    const qsizetype qmark = rest.indexOf('?');
    if (qmark != -1) {
        m_parsed.query = rest.mid(qmark + 1);
        rest.truncate(qmark);
    }

    QString authority = rest;
    const qsizetype slash = rest.indexOf('/');
    if (slash != -1) {
        m_parsed.path = rest.mid(slash);
        authority = rest.left(slash);
    }
    if (m_parsed.path.isEmpty()) {
        m_parsed.path = "/";
    }

    QString host = authority;
    const qsizetype colon = authority.lastIndexOf(':');
    if (colon != -1) {
        const QString portStr = authority.mid(colon + 1);
        host = authority.left(colon);
        if (!portStr.isEmpty()) {
            bool ok = false;
            const int port = portStr.toInt(&ok);
            if (!ok || port < 1 || port > 65535) {
                qDebug() << "URL: invalid port:" << portStr;
                return false;
            }
            m_parsed.port = static_cast<quint16>(port);
        }
    }
    if (host.isEmpty()) {
        qDebug() << "URL: missing host";
        return false;
    }
    m_parsed.host = host.toLower();

    if (m_parsed.port == 0) {
        m_parsed.port = (m_parsed.scheme == "https") ? HTTPS_PORT : HTTP_PORT;
    }

    m_parsed.isValid = true;
    qDebug() << "URL parsed:" << m_parsed.scheme << m_parsed.host
             << m_parsed.port << m_parsed.path
             << "query:" << m_parsed.query
             << "fragment:" << m_parsed.fragment;
            
    return true;
}

void URL::url_write(const QString &new_url)
{
    if (new_url == m_url) {
        return;
    }

    m_url = new_url;

    const bool ok = parse(m_url);
    emit url_changed(m_url);
    if (ok) {
        emit navigationRequested(m_parsed);
    }
}