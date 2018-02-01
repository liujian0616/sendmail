#include <stdint.h>
#include "smtp.h"
#include <time.h>

int main()
{
    Smtp *pSmtp = new Smtp();
    int nRet = pSmtp->Init();
    if (nRet != 0)
    {
        printf("init error\n");
        return -1;
    }
    uint32_t uCorpid = 21299;
    pSmtp->SendCorpWarnMail("test_server1", uCorpid, 1000);
    sleep(2);

    pSmtp->SendCorpWarnMail("test_server2", uCorpid, 1000);

    return 0;
}
