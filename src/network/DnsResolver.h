#pragma once

#include <QObject>
#include <QString>
class DnsResolver : public QObject
{
    Q_OBJECT

public:
    explicit DnsResolver(QObject *parent = nullptr);
    void resolve(const QString &host);

signals:
    
    // void resolved(const QString &host, const QString &address);
    // void failed(const QString &host, const QString &reason);

private:
    QString m_host;
    QString m_address;
};
