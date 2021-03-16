/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#include "udpserver.h"
#include "WinCat.h"
#include "options.h"
#include <io.h>
#include <fcntl.h>
#define DEBUG 1
int udpserver(char *port, char *filename, int keep_listening, char *restrictip) {
    WSADATA wsaData;
    int iResult;
    int err;
    int singlecmd=0;
    char msgbuf [256];   // for a message up to 255 bytes.
    msgbuf [0] = '\0';    // Microsoft doesn't guarantee this on man page.
    
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;
    
    struct sockaddr_in server,client_info = {0};
    int addrsize = sizeof(client_info);
    
    
    /* Initialize Winsock */
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    //fprintf(stderr, "verbose value: %d\n",  verbose);
    if (verbose) {
        printverbose();
        fprintf(stderr, "restrictip from: %s\n",  restrictip);
    }
    /* Create a SOCKET for connecting to server */
    ListenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ListenSocket == INVALID_SOCKET) {
        err = WSAGetLastError ();
        FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
                       NULL,                // lpsource
                       err,                 // message id
                       MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
                       msgbuf,              // output buffer
                       sizeof (msgbuf),     // size of msgbuf, bytes
                       NULL);               // va_list of arguments
        if (verbose) {
            printverbose();
            fprintf(stderr, "Socket failed with error: %ld,%s\n",  err,msgbuf);
        }
        
        WSACleanup();
        return 1;
    }
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons( atoi(port) );
    /* Setup the TCP listening socket */
    iResult = bind( ListenSocket, (struct sockaddr *)&server, sizeof(server));
    if (iResult == SOCKET_ERROR) {
        if (verbose) {
            printverbose();
            fprintf(stderr, "Port %s is already in use.\n", port);
        }
        err = WSAGetLastError ();
        FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
                       NULL,                // lpsource
                       err,                 // message id
                       MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
                       msgbuf,              // output buffer
                       sizeof (msgbuf),     // size of msgbuf, bytes
                       NULL);               // va_list of arguments
        if (verbose) {
            printverbose();
            fprintf(stderr, "Bind failed with error: %d,%s\n",  err,msgbuf);
        }
        if (DEBUG)
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    WinCat *wincat =new WinCat(filename);
    if (filename!=NULL&&memcmp(filename,"cmd /c ",7)==0){
        singlecmd=1;
        fprintf(stderr, "Single CMD Mode\n");
    }else{
        fprintf(stderr, "Server Mode\n");
    }
		wincat->setUdp(restrictip);
    do {
        if (singlecmd==1){
            wincat->SyncProcess(ClientSocket);
        }else{
            wincat->Process(ClientSocket);
        }
    } while (keep_listening);
    
    /* No longer need server socket */
    closesocket(ListenSocket);
    /* Shut down the connection since we're done */
    iResult = shutdown(ListenSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR) {
        err = WSAGetLastError ();
        FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
                       NULL,                // lpsource
                       err,                 // message id
                       MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
                       msgbuf,              // output buffer
                       sizeof (msgbuf),     // size of msgbuf, bytes
                       NULL);               // va_list of arguments
        if (verbose) {
            printverbose();
            fprintf(stderr, "Shutdown failed with error: %d,%s\n",  err,msgbuf);
        }
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    
    /* Cleanup */
    delete wincat;
    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}
int broadcastUDPReceiver(char *port, char *filename, int keep_listening, char *restrictip) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
 
    SOCKET sock;
    sock = socket(AF_INET,SOCK_DGRAM,0);
 
    char broadcast = '1';
 
//     This option is needed on the socket in order to be able to receive broadcast messages
//   If not set the receiver will not receive broadcast messages in the local network.
    if(setsockopt(sock,SOL_SOCKET,SO_BROADCAST,&broadcast,sizeof(broadcast)) < 0)
    {
        fprintf(stderr,"Error in setting Broadcast option");
        closesocket(sock);
        return 0;
    }
 
    struct sockaddr_in Recv_addr;  
    struct sockaddr_in Sender_addr;
 
    int len = sizeof(struct sockaddr_in);
 
    char recvbuff[50];
    int recvbufflen = 50;
    char sendMSG[]= "Broadcast message from READER";
 
    Recv_addr.sin_family       = AF_INET;        
    Recv_addr.sin_port         = htons(atoi(port));   
    Recv_addr.sin_addr.s_addr  = INADDR_ANY;
 
    if (bind(sock,(sockaddr*)&Recv_addr, sizeof (Recv_addr)) < 0)
    {
        fprintf(stderr,"Error in BINDING %d\n",WSAGetLastError());
        //_getch();
        closesocket(sock);
        return 0;
    }
 
    recvfrom(sock,recvbuff,recvbufflen,0,(sockaddr *)&Sender_addr,&len);
 
    fprintf(stderr,"\n\n\tReceived Message is : %s\n",recvbuff);
    fprintf(stderr,"\n\n\tPress Any to send message");
    //_getch();
    if(sendto(sock,sendMSG,strlen(sendMSG)+1,0,(sockaddr *)&Sender_addr,sizeof(Sender_addr)) < 0)
    {
        fprintf(stderr,"Error in Sending %d\n.",WSAGetLastError());
        fprintf(stderr,"\n\n\t\t Press any key to continue....");
        //_getch();
        closesocket(sock);
        return 0;
    }
    else
        fprintf(stderr,"\n\n\n\tREADER sends the broadcast message Successfully");
 
 
    fprintf(stderr,"\n\n\tpress any key to CONT...");
    //_getch();
 
    closesocket(sock);
    WSACleanup();
		return 0;
}
#define HELLO_PORT 12345
#define HELLO_GROUP "225.0.0.37"
#define MSGBUFSIZE 256
//http://www.steves-internet-guide.com/introduction-multicasting/
//https://www.drdobbs.com/web-development/internet-multicasting/184410294
int multicastUDPReceiver(int argc, char *argv[])
{
     if (argc != 3) {
       printf("Command line args should be multicast group and port\n");
       printf("(e.g. for SSDP, `listener 239.255.255.250 1900`)\n");
       return 1;
    }

    char* group = argv[1]; // e.g. 239.255.255.250 for SSDP
    int port = atoi(argv[2]); // 0 if error, which is an invalid port


#ifdef _WIN32
    //
    // Initialize Windows Socket API with given VERSION.
    //
    WSADATA wsaData;
    if (WSAStartup(0x0101, &wsaData)) {
        perror("WSAStartup");
        return 1;
    }
#endif

    // create what looks like an ordinary UDP socket
    //
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    // allow multiple sockets to use the same PORT number
    //
    u_int yes = 1;
    if (
        setsockopt(
            fd, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes)
        ) < 0
    ){
       perror("Reusing ADDR failed");
       return 1;
    }

        // set up destination address
    //
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender
    addr.sin_port = htons(port);

    // bind to receive address
    //
    if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    // use setsockopt() to request that the kernel join a multicast group
    //
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(group);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (
        setsockopt(
            fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)
        ) < 0
    ){
        perror("setsockopt");
        return 1;
    }

    // now just enter a read-print loop
    //
    while (1) {
        char msgbuf[MSGBUFSIZE];
        int addrlen = sizeof(addr);
        int nbytes = recvfrom(
            fd,
            msgbuf,
            MSGBUFSIZE,
            0,
            (struct sockaddr *) &addr,
            &addrlen
        );
        if (nbytes < 0) {
            perror("recvfrom");
            return 1;
        }
        msgbuf[nbytes] = '\0';
        puts(msgbuf);
     }

#ifdef _WIN32
    //
    // Program never actually gets here due to infinite loop that has to be
    // canceled, but since people on the internet wind up using examples
    // they find at random in their own code it's good to show what shutting
    // down cleanly would look like.
    //
    WSACleanup();
#endif

    return 0;
}