#pragma once
#include "ProtocolPorts.h"
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <QDebug>
#include <QByteArray>
#include <HttpResponse.h>
#include <QString>
#include <QtGlobal>

class HttpClient
{


public:

    QByteArray buildRequest(const QString &host,
                            quint16 port,
                            const QString &path,
                            const QString &query) const;

    bool sendRequest(int socketFd,
                    const QString &host,
                    quint16 port,
                    const QString &path,
                    const QString &query,
                    QString &errorMessage,
                    QString &ser_res) const;
    
    bool receiveRequest(int socketFd,
                        QString &errorMessage,
                        QString &ser_res) const;
                    
    bool Parse_Request(QString &ser_res) const;            
};
// https://uppbeat.io