#pragma once

#include <QObject>
#include <QString>
#include <QtTypes>


struct ParsedUrl {
    QString scheme;
    QString host;
    quint16 port = 0;    
    QString path;
    QString query;       
    QString fragment;    
    bool isValid = false;
};

class URL : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE url_write NOTIFY url_changed)

public:
    URL(QObject *parent = nullptr);

    QString url() const { return m_url; }
    const ParsedUrl &parsed() const { return m_parsed; }

    bool parse(const QString &raw);
    void url_write(const QString &new_url);

signals:
    void url_changed(const QString &url);
    void navigationRequested(const ParsedUrl &url);

private:
    QString m_url;       
    ParsedUrl m_parsed;
};
