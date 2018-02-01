#include "smtp.h"

Smtp::Smtp()
{
    m_sSender.clear();
    m_sPasswd.clear();
    m_vRecver.clear();
    m_pTransport = NULL;
}

Smtp::~Smtp()
{

}

int Smtp::Init()
{

    string sIp = string("smtp.exmail.qq.com");
    //int nPort = 25;
    int nPort = 465;
    m_sSender = string("xxxxxx@ecqun.com");
    m_sPasswd = string("xxxxx");
    string sRecver = string("xxxxxn@ecqun.com");
    if (nPort == 25)
        m_pTransport = new CommTransport(sIp, nPort);
    else if (nPort == 465)
        m_pTransport = new SslTransport(sIp, nPort);

    if (m_pTransport == NULL)
        return -1;

    size_t nBeginPos = 0;
    size_t nEndPos = 0;
    while ((nEndPos = sRecver.find(",", nBeginPos)) != string::npos )
    {
        string sTemp = sRecver.substr(nBeginPos, nEndPos - nBeginPos);
        nBeginPos = nEndPos + 1;

        m_vRecver.push_back(sTemp);
    }
    if (nBeginPos < sRecver.size())
    {
        string sTemp = sRecver.substr(nBeginPos);
        m_vRecver.push_back(sTemp);
    }

#if 1
    printf("[trace]%s ip:%s, port:%d, sender:%s, passwd:%s, recvcount:%d, recver0:%s\n", 
            __FUNCTION__, sIp.c_str(), nPort, m_sSender.c_str(), m_sPasswd.c_str(), m_vRecver.size(), m_vRecver[0].c_str());
#endif
    return 0;
}



int Smtp::SendRecv(char *pSendBuf, int nSendBufLen)
{
    char szRecvBuf[1024] = {0};
    int nRet = 0;

    printf("[trace]%s sendbuf:%s\n", __FUNCTION__, pSendBuf);
    nRet = m_pTransport->SafeSend(pSendBuf, nSendBufLen);
    if (nRet == -1)
    {
        printf("[error]%s send error\n", __FUNCTION__);
        return -1;
    }

    nRet = m_pTransport->SafeRecv(szRecvBuf, sizeof(szRecvBuf));
    if (nRet == -1)
    {
        printf("[error]%s recv error\n", __FUNCTION__);
        return -1;
    }

    printf("[trace]%s recvbuf:%s\n", __FUNCTION__, szRecvBuf);
    nRet = CheckRecvStatus(szRecvBuf);
    if (nRet == -1)
    {
        printf("[error]%s CheckRecvStatus error\n", __FUNCTION__);
        return -1;
    }

    return nRet;
}

int Smtp::CheckRecvStatus(char *pRecvBuf)
{
    char szStatus[4] = {0};
    strncpy(szStatus, pRecvBuf, 3);
    int nStatus = atoi(szStatus);

    switch (nStatus)
    {
        case 250:
        case 235:
        case 354:
        case 334:
        case 221:
        case 220:
            break;
        default:
            printf("[error]%s, get error status: %s\n", __FUNCTION__, pRecvBuf);
            return -1;
    }

    return 0;
}

int Smtp::AuthLogin()
{
    char szWriteBuf[1024] = {0};
    int nRet = 0;
    int nSendLen = 0;

    //send EHLO
    nSendLen = snprintf(szWriteBuf, sizeof(szWriteBuf), "HELO localhost\r\n");
    nRet = SendRecv(szWriteBuf, nSendLen);
    if (nRet == -1)
    {
        printf("[error]%s SendRecv error. writebuf:%s\n", __FUNCTION__, szWriteBuf);
        return -1;
    }

    //send auth login
    nSendLen = snprintf(szWriteBuf, sizeof(szWriteBuf), "AUTH LOGIN\r\n");
    nRet = SendRecv(szWriteBuf, nSendLen);
    if (nRet == -1)
    {
        printf("[error]%s SendRecv error. writebuf:%s\n", __FUNCTION__, szWriteBuf);
        return -1;
    }

    //send username
    base64_encode(szWriteBuf, sizeof(szWriteBuf), (const unsigned char *)m_sSender.c_str(), m_sSender.size());
    strcat(szWriteBuf, "\r\n");
    nRet = SendRecv(szWriteBuf, strlen(szWriteBuf));
    if (nRet == -1)
    {
        return -1;
        printf("[error]%s SendRecv error. writebuf:%s\n", __FUNCTION__, szWriteBuf);
    }

    //send passwd
    base64_encode(szWriteBuf, sizeof(szWriteBuf), (const unsigned char *)m_sPasswd.c_str(), m_sPasswd.size());
    strcat(szWriteBuf, "\r\n");
    nRet = SendRecv(szWriteBuf, strlen(szWriteBuf));
    if (nRet == -1)
    {
        return -1;
        printf("[error]%s SendRecv error. writebuf:%s\n", __FUNCTION__, szWriteBuf);
    }

    return 0;
}

int Smtp::SendMail(const string &sMailText)
{
    char szWriteBuf[1024] = {0};
    int nRet = 0;
    int nSendLen = 0;

    //send: mail from
    nSendLen = snprintf(szWriteBuf, sizeof(szWriteBuf), "mail from:<%s>\r\n", m_sSender.c_str());
    nRet = SendRecv(szWriteBuf, nSendLen);
    if (nRet == -1)
    {
        printf("[error]%s SendRecv error. writebuf:%s\n", __FUNCTION__, szWriteBuf);
        return -1;
    }

    for (size_t i = 0; i < m_vRecver.size(); i++)
    {
        //send: rcpt to
        nSendLen = snprintf(szWriteBuf, sizeof(szWriteBuf), "rcpt to:<%s>\r\n", m_vRecver[i].c_str());

        nRet = SendRecv(szWriteBuf, nSendLen);
        if (nRet == -1)
        {
            printf("[error]%s SendRecv error. writebuf:%s\n", __FUNCTION__, szWriteBuf);
            return -1;
        }
    }

    //send: data
    nSendLen = snprintf(szWriteBuf, sizeof(szWriteBuf), "data\r\n");
    nRet = SendRecv(szWriteBuf, nSendLen);
    if (nRet == -1)
    {
        printf("[error]%s SendRecv error. writebuf:%s\n", __FUNCTION__, szWriteBuf);
        return -1;
    }

    //send:mail text
    nRet = SendRecv((char *)sMailText.c_str(), sMailText.size());
    if (nRet == -1)
    {
        printf("[error]%s SendRecv error. writebuf:%.*s\n", __FUNCTION__, 50, szWriteBuf);
        return -1;
    }

    //send: quit
    nSendLen = snprintf(szWriteBuf, sizeof(szWriteBuf), "quit\r\n");
    nRet = SendRecv(szWriteBuf, nSendLen);
    if (nRet == -1)
    {
        printf("[error]%s SendRecv error. writebuf:%s\n", __FUNCTION__, szWriteBuf);
        return -1;
    }

    return 0;
}

int Smtp::MailText(const string sSubject, const string sMailBody, string &sMailText)
{
    string sRecverName; //不包括@ecqun.com
    string sSenderName; //不包括@ecqun.com
    size_t nPos = 0;

    nPos = m_sSender.find("@");
    if (nPos != string::npos)
        sSenderName = m_sSender.substr(0, nPos);

    stringstream ss;
    ss << "From:" << sSenderName << "<" << m_sSender  << ">\r\n";

    for (size_t i = 0; i < m_vRecver.size(); i++)
    {
        nPos = m_vRecver[i].find("@");
        if (nPos != string::npos)
            sRecverName = m_vRecver[i].substr(0, nPos);
        else
            sRecverName = m_vRecver[i];

        ss << "To:" << sRecverName << "<" << m_vRecver[i] << ">\r\n";
    }

    ss << "Subject:" << sSubject << "\r\n";
    ss << "MIME-Version:1.0\r\n";
    ss << "Content-Type:multipart/mixed;boundary=" << TEXT_BOUNDARY << "\r\n";
    ss << "\r\n--" << TEXT_BOUNDARY << "\r\n";
    ss << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
    ss << "\r\n" << sMailBody << "\r\n";

    sMailText.assign(ss.str());

    return 0;
}

int Smtp::MailAttachment(string &sMailText, string sFilePath)
{
    string sFileName;
    size_t nPos = sFilePath.rfind("/");
    if (nPos != string::npos)
    {
        sFileName = sFilePath.substr(nPos + 1);
    }
    else
    {
        sFileName = sFilePath;
    }

    stringstream ss;
    ss << "\r\n--" << TEXT_BOUNDARY << "\r\n" 
        << "Content-Type: application/octet-stream;name=" << sFileName << "\r\n"
        << "Content-Transfer-Encoding: base64\r\n"
        << "Content-Disposition: attachment;filename=" << sFileName << "\r\n\r\n";

    ifstream is(sFilePath.c_str(), std::ifstream::binary);
    if (!is)
    {
        printf("[error]%s ifstream open file error. filepath:%s\n", __FUNCTION__, sFilePath.c_str());
        return -1;
    }

    is.seekg(0, is.end);
    int nAttachLen = is.tellg();
    is.seekg(0, is.beg);
    char *pFileContent = new char[nAttachLen];
    if (pFileContent == NULL)
    {
        printf("[error]%s new char[%d] error\n", __FUNCTION__, nAttachLen);
        return -1;
    }
    is.read(pFileContent, nAttachLen);
    is.close();

    int nBase64FileContentLen = BASE64_SIZE(nAttachLen);
    char *pBase64FileContent = new char [nBase64FileContentLen];
    if (pBase64FileContent == NULL)
    {
        printf("[error]%s new char[%d] error\n", __FUNCTION__, nBase64FileContentLen);
        return -1;
    }

    base64_encode(pBase64FileContent, nBase64FileContentLen, (const unsigned char *)pFileContent, nAttachLen);

    ss << pBase64FileContent;

    sMailText = sMailText + ss.str();

    delete pFileContent;
    delete pBase64FileContent;

    return 0;
}

int Smtp::MailEnd(string &sMailText)
{
    stringstream ss;
    ss << "\r\n--" << TEXT_BOUNDARY << "--\r\n\r\n.\r\n";
    sMailText = sMailText + ss.str();
    return 0;
}



int Smtp::SendCorpWarnMail(string sHostname, uint32_t uCorpid, int nGreaterThan)
{
    int nRet = 0;
    nRet = m_pTransport->Connect();
    if (nRet != 0)
    {
        printf("[error]%s Connect() error\n", __FUNCTION__);
        return -1;
    }

    nRet = AuthLogin();
    if (nRet != 0)
    {
        printf("[error]%s AuthLogin() error\n", __FUNCTION__);
        return -1;
    }

    string sMailText;
    string sSubject("Warn Webimsignserver");
    string sMailBody;
    stringstream ss;
    ss << "Dear Sir,\r\n\r\n    Warn. 主机 "<< sHostname << " 上企业id " << uCorpid << " 总的请求量已经超过了  " << nGreaterThan << ", 将会拒绝当天内更多的请求.";
    sMailBody = ss.str();

    MailText(sSubject, sMailBody, sMailText);
    MailEnd(sMailText);
    printf("[trace]%s mailtext:%s\n", __FUNCTION__, sMailText.c_str());

    nRet = SendMail(sMailText);
    
    m_pTransport->Disconnect();
    return nRet;
}
