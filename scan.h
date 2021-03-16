/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#ifndef SCAN_H
#define SCAN_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#define MIN_PORT 1
#define MAX_PORT 65535
#define NON_BLOCKING 1

int connect_scan(char *host, 
                 int low, 
                 int high, 
                 int timeout);

int ping_scan(char *cidr, 
              int timeout,
              int do_reverse_dns);

#endif /* SCAN_H */
