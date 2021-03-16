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
        int launched,udpsocket;
        char *filename;
        char *myrestrictip;
        int Launch();

    public:
        WinCat(char *filename);
        ~WinCat();
        int Process(SOCKET ClientSocket);
        int SyncProcess(SOCKET ClientSocket);
        void setUdp(char * restrictip);
        
};

#endif /* WINCAT_H */
