#include "sslTransport.h"

SslTransport::SslTransport(string sIp, int nPort) : Transport(sIp, nPort)
{
    m_pCtx = NULL;
    m_pSsl = NULL;
}

SslTransport::~SslTransport()
{
}


int SslTransport::Connect()
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

    SSL_library_init();
    const SSL_METHOD *pMethod;
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    pMethod = SSLv23_client_method();  /* Create new client-method instance */
    m_pCtx = SSL_CTX_new(pMethod);   /* Create new context */
    if ( m_pCtx == NULL )
    {
        PrintSslError();
        return -1;
    }


    m_pSsl = SSL_new(m_pCtx);      /* create new SSL connection state */
    SSL_set_fd(m_pSsl, m_nSock);    /* attach the socket descriptor */
    if ( SSL_connect(m_pSsl) == -1)   /* perform the connection */
    {
        PrintSslError();
        return -1;
    }

    printf("Connected with %s encryption\n", SSL_get_cipher(m_pSsl));
    ShowCerts();        /* get any certs */

    return 0;
}

void SslTransport::PrintSslError()
{
    uint32_t errcode = ERR_get_error();
    char errstr[256] = {0};
    ERR_error_string(errcode, errstr);
    printf("[error]%s errcode:%d, errstr:%s\n", __FUNCTION__, errcode, errstr);
}

void SslTransport::ShowCerts()
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(m_pSsl); /* get the server's certificate */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);       /* free the malloc'ed string */
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);       /* free the malloc'ed string */
        X509_free(cert);     /* free the malloc'ed certificate copy */
    }
    else
        printf("No certificates.\n");
}

int SslTransport::Disconnect()
{
    if (m_nSock != -1)
    {
        close(m_nSock);
        m_nSock = -1;
    }

    if (m_pSsl)
    {
        SSL_free(m_pSsl);
        m_pSsl = NULL;
    }

    if (m_pCtx)
    {
        SSL_CTX_free(m_pCtx);
        m_pCtx = NULL;
    }

    return -1; 
}

int SslTransport::SafeSend(char *pSendBuf, int nSendBufLen)
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
                nLen = SSL_write(m_pSsl, pSendBuf, nSendBufLen);
                break;
            }
        }
    }
    return nLen;
}

int SslTransport::SafeRecv(char *pRecvBuf, int nRecvBufLen)
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
                nLen = SSL_read(m_pSsl, pRecvBuf, nRecvBufLen);
                break;
            }
        }
    }
    return nLen;
}
