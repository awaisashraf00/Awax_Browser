#pragma once

#include <QObject>
#include <QHostAddress>
#include <QString>
class DnsResolver : public QObject
{
    Q_OBJECT

public:
    explicit DnsResolver(QObject *parent = nullptr);
    void resolve(const QString &host);

signals:
    
    void resolved(int port_num,const QHostAddress &address);
    // void failed(const QString &host, const QString &reason);

private:
    QString m_host;
    QString m_address;
};
