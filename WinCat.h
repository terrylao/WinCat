/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#ifndef WINCAT_H
#define WINCAT_H

#include <windows.h>
#include <stdio.h>
#include "AsyncStreamReader.h"
#include "pipes.h"

class WinCat {
    private:
        AsyncStreamReader *input = NULL;
        FILE *output;
        struct PipeHandles pipes;
        int launched,udpsocket,otppass,udpAddrSize;
        char *filename;
        char *myrestrictip;
				char key[33];
        int Launch();
				struct sockaddr_in udpAddr;
    public:
        WinCat(char *filename);
        ~WinCat();
        int Process(SOCKET ClientSocket);
        int SyncProcess(SOCKET ClientSocket);
        void setUdp(char * restrictip,struct sockaddr_in audpAddr);
				void setOTP(char* akey);
        
};

#endif /* WINCAT_H */
