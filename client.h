/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#ifndef CLIENT_H
#define CLIENT_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
struct Thread_data
{
    char *host; 
    char *port;
    char *filename;
    char *mesg;
    unsigned long counts;
    int recvOne;
    int timeout;//0 immediatly close, > 0:wait for timeout, -1: forever wait, -2: 1 send 1 reply 
};
int client(char *host, 
           char *port, 
           char *filename,
           char *mesg);
int stressclient(char *host, 
           char *port, 
           char *filename,
           char *mesg,unsigned long counts,int recvOne,int itimeout,char *outputfilename);
DWORD WINAPI threadstressclient(LPVOID arg);

int udpclient(char *host, 
           char *port, 
           char *filename,
           char *mesg);
int portscan(char *host, char *port,int ctimeout);
#endif /* CLIENT_H */
