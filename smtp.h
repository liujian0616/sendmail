#ifndef SMTP_H
#define SMTP_H

#include "stdio.h"
#include "stdlib.h"
#include "base64.h"
#include "transport.h"
#include "commTransport.h"
#include "sslTransport.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
using std::string;
using std::stringstream;
using std::vector;
using std::ifstream;


#define TEXT_BOUNDARY     "KkK170891tpbkKk__FV_KKKkkkjjwq"

class Smtp
{
    public:
        Smtp();
        virtual ~Smtp();

        int Init();

        int AuthLogin();
        int SendMail(const string &sMailText);

        int SendRecv(char *pSendBuf, int nSendBufLen);
        int CheckRecvStatus(char *pRecvBuf);

        int MailText(const string sSubject, const string sMailBody, string &sMailText);
        int MailAttachment(string &sMailText, string sFilePath);
        int MailEnd(string &sMailText);


        int SendHostWarnMail(const string sHostname, int nGreaterThan);
        int SendCorpWarnMail(string sHostname, uint32_t uCorpid, int nGreaterThan);


    private:
        string          m_sSender;
        string          m_sPasswd;
        vector<string>  m_vRecver;

        Transport       *m_pTransport;
};

#endif
