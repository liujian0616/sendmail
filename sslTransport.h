#ifndef SSLTRANSPORT_H
#define SSLTRANSPORT_H


#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "transport.h"
using std::string;

class SslTransport : public Transport
{
    public:
        SslTransport(string sIp, int nPort);
        virtual ~SslTransport();


        virtual int Connect();
        virtual int Disconnect();
        virtual int SafeSend(char *pSendBuf, int nSendBufLen);
        virtual int SafeRecv(char *pRecvBuf, int nRecvBufLen);

    private:
        void ShowCerts();
        void PrintSslError();

    private:
        SSL_CTX     *m_pCtx;
        SSL         *m_pSsl;
};
#endif
