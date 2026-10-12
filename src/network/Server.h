#pragma once
#include <ProtocolPorts.h>
#include <Server.h>
#include <cstdlib>
#include <TcpConnection.h>
#include <QDebug>


enum class Server_State {
    NotCreated = -1 ,
    Created = 1
};

class REQUEST_SERVER
{
    private:
        TcpConnection Prepare_Request;
    public:
        REQUEST_SERVER(bool HTTP_PROTOCOL);
        ~REQUEST_SERVER();
};
