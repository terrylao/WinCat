/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#include "scan.h"
#include <omp.h>
#include <stdlib.h>
#include <math.h>
#include <Icmpapi.h>
#include <fcntl.h> // non-blocking sockets

#define DEBUG 0

int reverse_service_lookup(char *result, char *dotted_ip, int port) {
    DWORD dwRetval;

    struct sockaddr_in saGNI;
    char hostname[NI_MAXHOST];
    char servInfo[NI_MAXSERV];

    saGNI.sin_family = AF_INET;
    saGNI.sin_addr.s_addr = inet_addr(dotted_ip);
    saGNI.sin_port = htons(port);
    
    dwRetval = getnameinfo((struct sockaddr *) &saGNI,
                           sizeof (struct sockaddr),
                           hostname,
                           NI_MAXHOST, servInfo, NI_MAXSERV, NI_NOFQDN);

    if (dwRetval != 0) {
        if (DEBUG) fprintf(stderr, "getnameinfo failed with error # %ld\n", WSAGetLastError());
        return 1;
    } 

    sprintf(result, "%s", servInfo);
    return 0;
}

int reverse_dns_lookup(char *result, char *dotted_ip) {
    DWORD dwRetval;

    struct sockaddr_in saGNI;
    char hostname[NI_MAXHOST];
    u_short port = 7;

    saGNI.sin_family = AF_INET;
    saGNI.sin_addr.s_addr = inet_addr(dotted_ip);
    saGNI.sin_port = htons(port);
    
    dwRetval = getnameinfo((struct sockaddr *) &saGNI,
                           sizeof (struct sockaddr),
                           hostname,
                           NI_MAXHOST, NULL, NI_MAXSERV, NI_NOFQDN);

    if (dwRetval != 0) {
        if (DEBUG) fprintf(stderr, "getnameinfo failed with error # %ld\n", WSAGetLastError());
        return 1;
    } 

    sprintf(result, "%s", hostname);
    return 0;
}

int long_to_dotted_ipv4(char *dest, unsigned long ip_addr) {
    ip_addr = htonl(ip_addr);
    
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    
    d = ip_addr & 0xFF;
    c = (ip_addr >> 8) & 0xFF;
    b = (ip_addr >> 16) & 0xFF;
    a = (ip_addr >> 24) & 0xFF;

    sprintf(dest, "%d.%d.%d.%d", a, b, c, d);
    return 0;
}


char *get_dotted_ipv4(char *hostname) {
    char *ip = NULL;
    DWORD dwRetval;

    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;

    struct sockaddr_in  *sockaddr_ipv4;
    struct sockaddr_in6  *sockaddr_ipv6;

    char ipstringbuffer[46];
    DWORD ipbufferlength = 46;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    /* "echo" for ICMP echo/reply relaying service */
    dwRetval = getaddrinfo(hostname, "echo", &hints, &result);
    if ( dwRetval != 0 ) {
        if (DEBUG) fprintf(stderr, "getaddrinfo failed with error: %d\n", dwRetval);
        fprintf(stderr, "Unable to resolve host.\n");      
        return NULL;
    }

    /* Retrieve each address and print out the hex bytes */
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        switch (ptr->ai_family) {
            case AF_UNSPEC:
                if (DEBUG) fprintf(stderr, "Unspecified\n");
                break;
            case AF_INET:
                sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
                ip = inet_ntoa(sockaddr_ipv4->sin_addr);
                if (DEBUG) fprintf(stderr, "IPv4 address %s\n", ip);
                break;
            case AF_INET6:
                sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr; 
                if (DEBUG) fprintf(stderr, "IPv6 address %s\n", 
                    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46));
                break;
            case AF_NETBIOS:
                fprintf(stderr, "AF_NETBIOS (NetBIOS)\n");
                break;
            default:
                if (DEBUG) fprintf(stderr, "Other %ld\n", ptr->ai_family);
                break;
        }
    }
    return ip;
}

//int send_arp(char *dest, char *dotted) {
//    DWORD dwRetVal;
//    IPAddr DestIp = inet_addr(dotted);
//    IPAddr SrcIp = 0;       /* default for src ip */
//    ULONG MacAddr[2];       /* for 6-byte hardware addresses */
//    ULONG PhysAddrLen = 6;  /* default to length of six bytes */
//    BYTE *bPhysAddr;
//    unsigned int i;
//    memset(&MacAddr, 0xff, sizeof (MacAddr));
//
//    if (DEBUG) fprintf(stderr, "Sending ARP request for IP address: %s\n", dotted);
//    dwRetVal = SendARP(DestIp, SrcIp, &MacAddr, &PhysAddrLen);
//    if (dwRetVal == ERROR_BAD_NET_NAME) {
//        if (DEBUG) fprintf(stderr, "Error: Cannot send Arp request to a network on different subnet\n");
//    }
//
//    if (dwRetVal == NO_ERROR) {
//        bPhysAddr = (BYTE *) & MacAddr;
//        if (PhysAddrLen) {
//            for (i = 0; i < (int) PhysAddrLen; i++) {
//                if (i == (PhysAddrLen - 1))
//                    sprintf(dest, "%.2X", (int) bPhysAddr[i]);
//                else
//                    sprintf(dest, "%.2X-", (int) bPhysAddr[i]);
//                    dest = dest + 3;
//            }
//            return 0;
//        } else {
//            if (DEBUG) fprintf(stderr, "Warning: SendArp completed successfully, but returned length=0\n");
//        }
//    }
//    return 1; 
//}
//
//int ping_host(char *host, int timeout, int *latency) {
//    HANDLE hIcmpFile;
//    unsigned long ipaddr = INADDR_NONE;
//    DWORD dwRetVal = 0;
//    char SendData[32] = "Data Buffer";
//    LPVOID ReplyBuffer = NULL;
//    DWORD ReplySize = 0;
//
//    ipaddr = inet_addr(host);
//    
//    hIcmpFile = IcmpCreateFile();
//    if (hIcmpFile == INVALID_HANDLE_VALUE) {
//        if (DEBUG) {
//            fprintf(stderr, "\tUnable to open handle.\n");
//            fprintf(stderr, "IcmpCreatefile returned error: %ld\n", GetLastError());
//        }
//        return 1;
//    }    
//
//    ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
//    ReplyBuffer = (VOID*) malloc(ReplySize);
//    
//    dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), 
//        NULL, ReplyBuffer, ReplySize, timeout);
//
//    int exit_code = 1;
//
//    if (dwRetVal) {
//        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
//        struct in_addr ReplyAddr;
//        ReplyAddr.S_un.S_addr = pEchoReply->Address;
//
//        if (!strcmp(host, inet_ntoa( ReplyAddr ))) {
//            *latency = pEchoReply->RoundTripTime;
//            exit_code = 0;
//        }
//    }
//
//    free(ReplyBuffer);
//    return exit_code;
//}
//
//void display_ping(char *host, char *name, int latency) {
//    if (strlen(name) && strcmp(host, name)) {
//        fprintf(stdout, "%4ld ms %16s [%s]\n", latency, host, name);
//    } else {
//        fprintf(stdout, "%4ld ms %16s\n", latency, host);
//    }
//
//    /* Send Arp requests */
//    //char mac[18];
//    //if (!send_arp(mac, host)) {
//    //    fprintf(stdout, "\tMAC: %s", mac);
//    //}
//}
//
//int ping_scan(char *cidr, int timeout, int do_reverse_dns) {
//    WSADATA wsaData;
//    int iResult;
//    
//    /* Initialize Winsock */
//    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//    if (iResult != 0) {
//        printf("WSAStartup failed: %d\n", iResult);
//        return 1;
//    }
//    
//    char *hostname = strtok(cidr, "/\\");
//    char *dotted_ip = get_dotted_ipv4(hostname);
//
//    if (dotted_ip == NULL) {
//        /* Failure */
//        return 1;
//    }
//
//    unsigned long ip =  inet_addr(dotted_ip);
//    char *network_bits = strtok(NULL, "/\\");
//
//    if (network_bits == NULL) {
//        /* User supplied ip IS NOT using CIDR notation */
//        char host[NI_MAXHOST];
//        long_to_dotted_ipv4(host, ip);
//        int latency = 0;
//
//        /* Print result */
//        if (!ping_host(host, timeout, &latency)) {
//            char name[NI_MAXHOST];
//            memset(name, '\0' , NI_MAXHOST);
//            if (do_reverse_dns) reverse_dns_lookup(name, host);
//            display_ping(host, name, latency);
//        }
//
//    } else {
//        /* User supplied ip IS using CIDR notation */
//        int host_bits = 32 - atoi(network_bits);
//        if (host_bits < 0 || host_bits > 32 ) {
//            fprintf(stderr, "Invalid CIDR notation.\n");
//            return 1;
//        }
//
//        unsigned long subnet_mask = host_bits == 32 ? 0 : htonl((0xFFFFFFFF << host_bits));
//        unsigned long network = ip & subnet_mask;
//
//        char subnet_mask_str[NI_MAXHOST];
//        char network_str[NI_MAXHOST];
//        long_to_dotted_ipv4(subnet_mask_str, subnet_mask);
//        long_to_dotted_ipv4(network_str, network);
//        unsigned long long num_hosts = (unsigned long long) 1 << host_bits; //pow(2, host_bits);
//        fprintf(stderr, "Network: %s\nSubnet mask: %s\n", network_str, subnet_mask_str);
//        fprintf(stderr, "--- %llu address sweep ---\n", num_hosts);
//
//        /* The number of threads created is not guaranteed 
//           to be the number of threads requested */
//        omp_set_num_threads(num_hosts);
//
//        /* Multithreaded for-loop */
//        #pragma omp parallel for ordered
//        for (long long int i = 0; i < num_hosts; i++) {
//
//                unsigned long current = network + htonl(i);
//                char host[NI_MAXHOST];
//                long_to_dotted_ipv4(host, current);
//                int latency;
//
//                if (!ping_host(host, timeout, &latency)) {
//                    /* Successful PING */
//                    char name[NI_MAXHOST];
//                    memset(name, '\0' , NI_MAXHOST);
//                    if (do_reverse_dns) reverse_dns_lookup(name, host);
//                    #pragma omp ordered 
//                    {
//                        display_ping(host, name, latency);
//                    }
//                }
//        }    
//    }
//    /* Cleanup */
//    WSACleanup();
//    return 0;
//}

int connect_scan(char *host, int low, int high, int timeout) {
    if (low < MIN_PORT || high > MAX_PORT) {
        fprintf(stderr, "Ports must be within the range 1-65535.\n");
        return 1;
    }

    WSADATA wsaData;
    int iResult;

    /* Initialize Winsock */
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    /*  Request as many threads as ports. If number of ports is large, 
    the OS will not guarantee requested number of threads */
    int num_ports = high - low + 1;
    if (DEBUG) fprintf(stderr, "Requesting %d threads.\n", num_ports);
    omp_set_num_threads(num_ports);

    fd_set writefds;
    FD_ZERO(&writefds);
  
    /* Multithreaded for-loop */
    #pragma omp parallel for ordered
    for (int i = low; i <= high; i++) {
        char port[6];
        sprintf(port, "%d", i);

        SOCKET ConnectSocket = INVALID_SOCKET;
        struct addrinfo *result = NULL,
                        *ptr = NULL,
                        hints;

        ZeroMemory( &hints, sizeof(hints) );
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        /* Resolve the server address and port */
        iResult = getaddrinfo(host, port, &hints, &result);
        if ( iResult != 0 ) {
            /* Unable to resolve host and/or port: do nothing */
            if (DEBUG) fprintf(stderr, "Getaddrinfo failed with error: %d\n", iResult);
        } else {
            /* Attempt to connect to an address until one succeeds */
            for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {            

                /* Create a SOCKET for connecting to server */
                ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
                if (ConnectSocket == INVALID_SOCKET) {
                    fprintf(stderr, "Socket failed with error: %ld\n", WSAGetLastError());
                }

                /* set socket to non-blocking if flag present */
                unsigned long non_blocking = NON_BLOCKING;
                if (ioctlsocket(ConnectSocket, FIONBIO, &non_blocking)) {
                    fprintf(stderr, "Failed to set socket mode.\n");
                }
                
                /* Connect to server */ 
                connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

                fd_set writefds;
                FD_ZERO(&writefds);
                FD_SET(ConnectSocket, &writefds);

                /* the timeout parameter is given in ms */
                int seconds = timeout >= 1000? timeout/1000 : 0;
                int micro_seconds = (timeout - 1000*seconds) * 1000;

                struct timeval s_timeout; 
                s_timeout.tv_sec = seconds;
                s_timeout.tv_usec = micro_seconds;

                int rs = select(1, NULL, &writefds, NULL, &s_timeout);
                if (FD_ISSET(ConnectSocket, &writefds)) {
                    /* port is OPEN */
                    char service[NI_MAXSERV];
                    reverse_service_lookup(service, get_dotted_ipv4(host), i);

                    #pragma omp ordered 
                    {  
                        fprintf(stdout, "Open: %5d ", i);
                        if (i != atoi(service)) {
                            fprintf(stdout, " [%s]", service);
                        }
                        fprintf(stdout, "\n");   
                    }
                    break;
                }
            }  
            freeaddrinfo(result);
        }
        closesocket(ConnectSocket);
    } 
    /* Cleanup */
    WSACleanup();
    return 0;
}
