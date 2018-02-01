#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <string>
using std::string;

class Transport
{
    public:
        Transport(string sIp, int nPort) : m_sIp(sIp), m_nPort(nPort), m_nSock(-1) { };
        virtual ~Transport(){};

        virtual int Connect() = 0;
        virtual int Disconnect() = 0;
        virtual int SafeSend(char *pSendBuf, int nSendBufLen) = 0;
        virtual int SafeRecv(char *pRecvBuf, int nRecvBufLen) = 0;

    protected:
        string  m_sIp;
        int     m_nPort;
        int     m_nSock;
};
#endif
