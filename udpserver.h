/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#ifndef UDPSERVER_H
#define UDPSERVER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

int udpserver(char *port, 
           char *filename, 
           int keep_listening, char *restrictip, char *akey);

#endif /* SERVER_H */
