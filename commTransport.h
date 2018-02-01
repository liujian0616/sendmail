#ifndef COMMTRANSPORT_H
#define COMMTRANSPORT_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <string.h>
#include "unistd.h"
#include "transport.h"
using std::string;

class CommTransport : public Transport
{
    public:
        CommTransport(string sIP, int nPort);
        virtual ~CommTransport();

        virtual int Connect();
        virtual int Disconnect();
        virtual int SafeSend(char *pSendBuf, int nSendBufLen);
        virtual int SafeRecv(char *pRecvBuf, int nRecvBufLen);

};
#endif
