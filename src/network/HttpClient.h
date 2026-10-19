#pragma once
#include "ProtocolPorts.h"
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <QDebug>
#include <QByteArray>
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
                     QString &errorMessage) const;
            
};
