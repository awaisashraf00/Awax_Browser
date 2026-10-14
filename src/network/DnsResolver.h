#pragma once

#include <QObject>
#include <QThread>
#include <QHostAddress>
#include <QString>
#include <QtDebug>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

class DnsResolver : public QObject
{
    Q_OBJECT

public:
    explicit DnsResolver(QObject *parent = nullptr);
    void resolve(const QString &host);

signals:
    
    void resolved(const QHostAddress &address);
    // void failed(const QString &host, const QString &reason);

private:
    QString m_host;
    QString m_address;
};
