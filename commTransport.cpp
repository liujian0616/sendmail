#include "commTransport.h"

CommTransport::CommTransport(string sIp, int nPort) : Transport(sIp, nPort)
{

}

CommTransport::~CommTransport()
{
}

int CommTransport::Connect()
{
    struct hostent  *pHost = NULL;
    struct sockaddr_in   stServAddr;
    pHost = gethostbyname(m_sIp.c_str());
    int nRet = 0;

    if (pHost == NULL)
    {
        printf("[error]%s gethostname error. %s\n", __FUNCTION__, m_sIp.c_str());
        return -1;
    }

    memset(&stServAddr, 0, sizeof(stServAddr));
    stServAddr.sin_family = AF_INET;
    stServAddr.sin_port = htons(m_nPort);
    stServAddr.sin_addr = *((struct in_addr*) pHost->h_addr);
    m_nSock = socket(PF_INET, SOCK_STREAM, 0);
    if (m_nSock == -1)
    {
        printf("[error]%s socket error\n", __FUNCTION__);
        return -1;
    }

    nRet = connect(m_nSock, (struct sockaddr *)&stServAddr, sizeof(struct sockaddr));
    if (nRet == -1)
    {
        close(m_nSock);
        printf("[error]%s connect error\n", __FUNCTION__);
        return -1;
    }

    return 0;

}

int CommTransport::Disconnect()
{
    if (m_nSock != -1)
    {
        close(m_nSock);
        m_nSock = -1;
    }

    return -1; 
}

int CommTransport::SafeSend(char *pSendBuf, int nSendBufLen)
{
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(m_nSock, &wfds);
    int nRet = 0;
    int nLen = 0;
    while (1)
    {
        nRet = select(m_nSock + 1, NULL, &wfds, NULL, NULL);
        if (nRet == 0)
        {
            continue;
        }
        else if (nRet < 0)
        {
            printf("[error]%s select return -1\n", __FUNCTION__);
            return -1;
        }
        else 
        {
            if (FD_ISSET(m_nSock, &wfds))
            {
                nLen = send(m_nSock, pSendBuf, nSendBufLen, 0);
                break;
            }
        }
    }
    return nLen;
}

int CommTransport::SafeRecv(char *pRecvBuf, int nRecvBufLen)
{
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(m_nSock, &rfds);
    int nRet = 0;
    int nLen = 0;
    while (1)
    {
        nRet = select(m_nSock + 1, &rfds, NULL, NULL, NULL);
        if (nRet == 0)
        {
            continue;
        }
        else if (nRet < 0)
        {
            printf("[error]%s select return -1\n", __FUNCTION__);
            return -1;
        }
        else 
        {
            if (FD_ISSET(m_nSock, &rfds))
            {
                nLen = recv(m_nSock, pRecvBuf, nRecvBufLen, 0);
                break;
            }
        }
    }
    return nLen;
}
