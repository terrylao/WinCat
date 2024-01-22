/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#include "client.h"
#include "options.h"
#include "WinCat.h"
#include <io.h>
#include <fcntl.h>
#define DEBUG 1
int connect_timeout(SOCKET cClient,struct sockaddr *server,int sockaddr_lens,unsigned long timeout_seconds) { 
  unsigned long ul = 1;
  //set to non-block mode
  int ret = ioctlsocket(cClient, FIONBIO, (unsigned long*)&ul);
  if(ret==SOCKET_ERROR)
    return -1;

  connect(cClient,server,sockaddr_lens);

  struct timeval timeout ;
  fd_set r;

  FD_ZERO(&r);
  FD_SET(cClient, &r);
  timeout.tv_sec = timeout_seconds; 
  timeout.tv_usec =0;
  ret = select(0, 0, &r, 0, &timeout);
  if ( ret <= 0 ){
    return -1;
  }
  //back to block mode
  unsigned long ul1= 0 ;
  ret = ioctlsocket(cClient, FIONBIO, (unsigned long*)&ul1);
  return ret;
}
int portscan(char *host, char *port,int ctimeout) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    int iResult;
    struct sockaddr_in  client_info = {0};
    int addrsize = sizeof(client_info);
    
    /* Initialize Winsock */
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", iResult);
        return -1;
    }
    DWORD dwError;
    int i = 0;
    struct hostent *remoteHost;
    char **pAlias;
    char hostname[NI_MAXHOST];
    char servInfo[NI_MAXSERV];
    int fromport,toport;
    // If the user input is an alpha name for the host, use gethostbyname()
    // If not, get host by addr (assume IPv4)
		fromport=atoi(port);
		toport=fromport;
		for (int i=0;i<strlen(port);i++){
			if (port[i]=='-'){
				toport=atoi(&port[i+1]);
				fprintf(stderr,"from %d  to %d\n" , fromport,toport);
				break;
			}
		}
    if (isalpha(host[0]))
    {
        // host address is a name
        remoteHost = gethostbyname(host);
        if (remoteHost == NULL)
        {
            dwError = WSAGetLastError();
            if (dwError != 0)
            {
                if (dwError == WSAHOST_NOT_FOUND)
                {
                    fprintf(stderr,"Host not found!\n");
                    WSACleanup();
                    return -1;
                }
                else if (dwError == WSANO_DATA)
                {
                    fprintf(stderr,"No data record found!\n");
                    WSACleanup();
                    return -1;
                }
                else
                {
                    fprintf(stderr,"Function failed with error code %ld\n", dwError);
                    WSACleanup();
                    return -1;
                }
            }
        }
        else
        {
            fprintf(stderr,"Function returned:\n");
            fprintf(stderr,"\tOfficial name: %s\n", remoteHost->h_name);
            for (pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++)
            {
                fprintf(stderr,"\tAlternate name #%d: %s\n", ++i, *pAlias);
            }

            fprintf(stderr,"\tAddress type: ");
            switch (remoteHost->h_addrtype)
            {
                case AF_INET:
                fprintf(stderr,"AF_INET\n");
                break;
            case AF_INET6:
                fprintf(stderr,"AF_INET6\n");
                break;
            case AF_NETBIOS:
                fprintf(stderr,"AF_NETBIOS\n");
                break;
            default:
                fprintf(stderr," %d\n", remoteHost->h_addrtype);
                break;
            }
            fprintf(stderr,"\tAddress length: %d\n", remoteHost->h_length);
            i = 0;
            while (remoteHost->h_addr_list[i] != 0)
            {
                client_info.sin_addr.s_addr = *(u_long *) remoteHost->h_addr_list[i++];
                fprintf(stderr,"\tIP Address #%d: %s\n", i, inet_ntoa(client_info.sin_addr));
              	for (int j=fromport;j<=toport;j++){
                	if((ConnectSocket = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
                	{
                		fprintf(stderr,"Could not create socket : %d" , WSAGetLastError());
                		WSACleanup();
                		return -1;
                	}
                	client_info.sin_port = htons( j );
                	fprintf(stderr,"connected port: %d" , j);
                	//iResult = connect( ConnectSocket, (struct sockaddr *) &client_info, addrsize);
                  iResult = connect_timeout( ConnectSocket, (struct sockaddr *) &client_info, addrsize,ctimeout);
                  if (iResult < 0) {
                      //fprintf(stderr,"Could not connect socket : %d" , WSAGetLastError());
                      closesocket(ConnectSocket);
                      ConnectSocket = INVALID_SOCKET;
                      fprintf(stderr," fail\n");
                      continue;
                  }
                  fprintf(stderr," success\n");
              	}
            }
        }
    }
    else
    {
        client_info.sin_addr.s_addr = inet_addr(host);
      	client_info.sin_family = AF_INET;
      	
        if (client_info.sin_addr.s_addr == INADDR_NONE)
        {
            fprintf(stderr,"The IPv4 address entered must be a legal address!\n");
            WSACleanup();
            return -1;
        }
        else
        if (getnameinfo((struct sockaddr *) &client_info, sizeof (client_info), hostname, NI_MAXHOST, servInfo, NI_MAXSERV, NI_NUMERICSERV)==0){
          fprintf(stderr,"Domain Name : %s\n" , hostname);
        }
      	for (int i=fromport;i<=toport;i++){
        	if((ConnectSocket = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
        	{
        		fprintf(stderr,"Could not create socket : %d" , WSAGetLastError());
        		WSACleanup();
        		return -1;
        	}
        	client_info.sin_port = htons( i );
        	fprintf(stderr,"connected port: %d" , i);
        	//iResult = connect( ConnectSocket, (struct sockaddr *) &client_info, addrsize);
          iResult = connect_timeout( ConnectSocket, (struct sockaddr *) &client_info, addrsize,ctimeout);
          if (iResult < 0) {
              //fprintf(stderr,"Could not connect socket : %d" , WSAGetLastError());
              closesocket(ConnectSocket);
              ConnectSocket = INVALID_SOCKET;
              fprintf(stderr," fail\n");
              continue;
          }
          fprintf(stderr," success\n");
      	}
    }
    WSACleanup();
    return 0;
}
int client(char *host, char *port, char *filename,char *mesg, char* otp) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    int iResult;
    int err;
    char msgbuf [256];   // for a message up to 255 bytes.
    msgbuf [0] = '\0';    // Microsoft doesn't guarantee this on man page.
    sockaddr_in  client_info = {0};
    int addrsize = sizeof(client_info);
    
    /* Initialize Winsock */
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
      fprintf(stderr, "WSAStartup failed with error: %d\n", iResult);
      return 1;
    }
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    /* Resolve the server address and port */
    iResult = getaddrinfo(host, port, &hints, &result);
    if ( iResult != 0 ) {
      fprintf(stderr, "Unable to resolve host and/or port.\n");
      if (DEBUG) fprintf(stderr, "Getaddrinfo failed with error: %d\n", iResult);
      WSACleanup();
      return 1;
    }
    /* Attempt to connect to an address until one succeeds */
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
      /* Create a SOCKET for connecting to server */
      ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
          ptr->ai_protocol);
      if (ConnectSocket == INVALID_SOCKET) {
        err = WSAGetLastError ();
        FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
                       NULL,                // lpsource
                       err,                 // message id
                       MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
                       msgbuf,              // output buffer
                       sizeof (msgbuf),     // size of msgbuf, bytes
                       NULL);               // va_list of arguments
        if (DEBUG) {
            fprintf(stderr, "Socket failed with error: %ld,%s\n",  err,msgbuf);
        }
        WSACleanup();
        return 1;
      }
      /* Connect to server */
      //iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
      iResult = connect_timeout( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen,3);
      if (iResult < 0) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
        continue;
      }
      break;
    }
    freeaddrinfo(result);
    if (ConnectSocket == INVALID_SOCKET) {
      fprintf(stderr, "Unable to connect to host.\n");
      WSACleanup();
      return 1;
    }else{
      getpeername(ConnectSocket, (struct sockaddr *)&client_info, &addrsize);
      char *ip = inet_ntoa(client_info.sin_addr);
      fprintf(stderr, "connected to host %s.\n",ip);
    }
    if (mesg!=NULL){
      ULONGLONG NTickInitial,NTickShowEnd;
      char recvbuf[DEFAULT_BUFLEN];
      memset(recvbuf, '\0', DEFAULT_BUFLEN);
      NTickInitial=GetTickCount64();
      int iResult = send(ConnectSocket, mesg, strlen(mesg), 0);
      iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
      NTickShowEnd= GetTickCount64();
      fprintf(stderr, "reply %s in $llu ms\n",  recvbuf,NTickShowEnd-NTickInitial);
      iResult = shutdown(ConnectSocket, SD_BOTH);
      closesocket(ConnectSocket);
      WSACleanup();
      return 1;
    }
		if (otp!=NULL){
      char recvbuf[DEFAULT_BUFLEN];
      memset(recvbuf, '\0', DEFAULT_BUFLEN);
			int iResult = send(ConnectSocket, otp, strlen(otp), 0);
			iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
			if (iResult>0){
				fprintf(stderr, "OTP: %s\n",  recvbuf);
				if (iResult<3){
  				closesocket(ConnectSocket);
  				WSACleanup();
  				return -3;
				}
			}else{
				closesocket(ConnectSocket);
				WSACleanup();
				return -3;
			}
			
		}
    /* Where the magic happens */
    WinCat *wincat = new WinCat(filename);
    wincat->Process(ConnectSocket);
    /* Shut down the connection since we're done */
    iResult = shutdown(ConnectSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR) {
      err = WSAGetLastError();
      FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
                     NULL,                // lpsource
                     err,                 // message id
                     MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
                     msgbuf,              // output buffer
                     sizeof (msgbuf),     // size of msgbuf, bytes
                     NULL);               // va_list of arguments
      if (DEBUG) {
      	fprintf(stderr, "Shutdown failed with error: %ld,%s\n",  err,msgbuf);
      }
      closesocket(ConnectSocket);
      WSACleanup();
      return 1;
    }
    
    /* Cleanup */
    delete wincat;
    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
}
int stressclient(char *host, char *port, char *filename,char *mesg,unsigned long counts,int recvOne,int itimeout,char *outputfilename) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    int iResult;
    int err;
    char msgbuf [DEFAULT_BUFLEN];   // for a message up to 255 bytes.
    msgbuf [0] = '\0';    // Microsoft doesn't guarantee this on man page.
    sockaddr_in  client_info = {0};
    int addrsize = sizeof(client_info);
    DWORD timeout = itimeout * 1000;

    /* Initialize Winsock */
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    /* Resolve the server address and port */
    iResult = getaddrinfo(host, port, &hints, &result);
    if ( iResult != 0 ) {
        fprintf(stderr, "Unable to resolve host and/or port.\n");
        if (DEBUG) fprintf(stderr, "Getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    
    if (filename!=NULL){
        counts=1;
    }
    ULONGLONG gNTickInitial,gNTickShowEnd;
    ULONGLONG NTickInitial,NTickShowEnd;
    gNTickInitial=GetTickCount64();
    while (counts>0){
        /* Attempt to connect to an address until one succeeds */
        for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
            /* Create a SOCKET for connecting to server */
            ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
                ptr->ai_protocol);
            if (ConnectSocket == INVALID_SOCKET) {
                err = WSAGetLastError ();
                FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
                               NULL,                // lpsource
                               err,                 // message id
                               MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
                               msgbuf,              // output buffer
                               sizeof (msgbuf),     // size of msgbuf, bytes
                               NULL);               // va_list of arguments
                if (DEBUG) {
                    fprintf(stderr, "Socket failed with error: %ld,%s\n",  err,msgbuf);
                }
                WSACleanup();
                return 1;
            }
            /* Connect to server */
            iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
            if (iResult == SOCKET_ERROR) {
                closesocket(ConnectSocket);
                ConnectSocket = INVALID_SOCKET;
                continue;
            }
            break;
        }
        
        if (ConnectSocket == INVALID_SOCKET) {
            fprintf(stderr, "Unable to connect to host.\n");
            WSACleanup();
            return 1;
        }else{
            getpeername(ConnectSocket, (struct sockaddr *)&client_info, &addrsize);
            char *ip = inet_ntoa(client_info.sin_addr);
            fprintf(stderr, "connected to host %s.\n",ip);
        }
        
        if (itimeout>0)
            setsockopt(ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        char recvbuf[DEFAULT_BUFLEN];
        FILE *fout;
        if (outputfilename!=NULL){
          fout = fopen(outputfilename,"wb");
        }else{
          fout = stdout;
        }
        
        memset(recvbuf, '\0', DEFAULT_BUFLEN);
        NTickInitial=GetTickCount64();
        if (itimeout==-3){
            fprintf(stderr, "receive only!");
            int iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
            if (iResult<=0){
                fprintf(stderr, "recv failed with error: %d\n", WSAGetLastError());
            }
            //fprintf(stderr, "%s",  recvbuf);
            //write(1,recvbuf,iResult);
            fwrite(recvbuf,1,iResult,fout);
        }else
        if (filename!=NULL){
            FILE *fptr;
            fptr = fopen(filename,"rb");
            if(fptr == NULL)
            {
               if (DEBUG) {
                fprintf(stderr, "open file Error!");
               }   
               exit(1);             
            }
            memset(msgbuf, '\0', DEFAULT_BUFLEN);
            int sendcount=0;
            if (recvOne==2){
              while((sendcount=fread(msgbuf,   1,DEFAULT_BUFLEN, fptr)) >0){
                int iResult = send(ConnectSocket, msgbuf, sendcount, 0);
                if (iResult<=0){
                    fprintf(stderr, "send failed with error: %d\n", WSAGetLastError());
                    break;
                }
                memset(msgbuf, '\0', DEFAULT_BUFLEN);
              }
              if (itimeout!=0){
                memset(msgbuf, '\0', DEFAULT_BUFLEN);
                do{
                  //fprintf(stderr, "start recv\n");
                  iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
                  if (iResult<=0){
                      fprintf(stderr, "recv failed with error: %d\n", WSAGetLastError());
                      break;
                  }
                  //fprintf(stderr, "%s",  recvbuf);
                  //write(1,recvbuf,iResult);
                  fwrite(recvbuf,1,iResult,fout);
                  memset(recvbuf,0,DEFAULT_BUFLEN);
                }while(iResult>0&&itimeout!=-2);
              }
            }else{
                int fr=0;
                do{
                    fr=readline(fptr,msgbuf,DEFAULT_BUFLEN);
                    if (fr==0){
                        if (DEBUG) {
                            fprintf(stderr, "read file content error\n");
                        }
                        break;
                    }else{
                        int iResult = send(ConnectSocket, msgbuf, strlen(msgbuf), 0);
                        if (iResult<=0){
                            fprintf(stderr, "send failed with error: %d\n", iResult);
                            break;
                        }
                        if (itimeout!=0){
                            memset(msgbuf, '\0', DEFAULT_BUFLEN);
                            do{
                              iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
                              if (iResult<=0){
                                  fprintf(stderr, "recv failed with error: %d\n", WSAGetLastError());
                                  break;
                              }
                              //fprintf(stderr, "%s\n",  recvbuf);
                              //write(1,recvbuf,iResult);
                              fwrite(recvbuf,1,iResult,fout);
                              memset(recvbuf,0,DEFAULT_BUFLEN);
                            }while(iResult>0&&itimeout!=-2);
                        }
                    }
                }while (fr);
            }
            fclose(fptr);
        }else{
            int iResult = send(ConnectSocket, mesg, strlen(mesg), 0);
            if (iResult>0){
                do{
                    iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
                    if (iResult<=0){
                      fprintf(stderr, "recv failed with error: %d\n", iResult);
                      break;
                    }
                    //fprintf(stderr, "%s",  recvbuf);
                    //write(1,recvbuf,iResult);
                    fwrite(recvbuf,1,iResult,fout);
                    memset(recvbuf,0,DEFAULT_BUFLEN);
                }while (iResult&&recvOne);
            }else{
                fprintf(stderr, "send failed with error: %d\n", iResult);
            }
        }
        if (outputfilename!=NULL){
          fclose(fout);
        }
        NTickShowEnd= GetTickCount64();
        fprintf(stderr, "end within %llu ms\n",  NTickShowEnd-NTickInitial);
        /* Shut down the connection since we're done */
        iResult = shutdown(ConnectSocket, SD_BOTH);
        if (iResult == SOCKET_ERROR) {
            err = WSAGetLastError();
            FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
                           NULL,                // lpsource
                           err,                 // message id
                           MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
                           msgbuf,              // output buffer
                           sizeof (msgbuf),     // size of msgbuf, bytes
                           NULL);               // va_list of arguments
            if (DEBUG) {
                fprintf(stderr, "Shutdown failed with error: %ld,%s\n",  err,msgbuf);
            }
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
        closesocket(ConnectSocket);
        counts--;
    }
    gNTickShowEnd= GetTickCount64();
    if (DEBUG) {
        fprintf(stderr, "total elapsed: %llums\n",  gNTickShowEnd-gNTickInitial);
    }
    freeaddrinfo(result);
    WSACleanup();
    return 0;
}
//this function will be removed
int stressclient2(char *host, char *port, char *filename,char *mesg,unsigned long counts,int recvOne) {
    
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    int iResult;
    int err;
    char msgbuf [DEFAULT_BUFLEN];   // for a message up to 255 bytes.
    msgbuf [0] = '\0';    // Microsoft doesn't guarantee this on man page.
    sockaddr_in  client_info = {0};
    int addrsize = sizeof(client_info);
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    /* Resolve the server address and port */
    iResult = getaddrinfo(host, port, &hints, &result);
    if ( iResult != 0 ) {
        fprintf(stderr, "Unable to resolve host and/or port.\n");
        if (DEBUG) fprintf(stderr, "Getaddrinfo failed with error: %d\n", iResult);
        return 1;
    }
    
    if (filename!=NULL){
        counts=1;
    }
    ULONGLONG gNTickInitial,gNTickShowEnd;
    ULONGLONG NTickInitial,NTickShowEnd;
    gNTickInitial=GetTickCount64();
    while (counts>0){
        /* Attempt to connect to an address until one succeeds */
        for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
            /* Create a SOCKET for connecting to server */
            ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
                ptr->ai_protocol);
            if (ConnectSocket == INVALID_SOCKET) {
                err = WSAGetLastError ();
                FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
                               NULL,                // lpsource
                               err,                 // message id
                               MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
                               msgbuf,              // output buffer
                               sizeof (msgbuf),     // size of msgbuf, bytes
                               NULL);               // va_list of arguments
                if (DEBUG) {
                    fprintf(stderr, "Socket failed with error: %ld,%s\n",  err,msgbuf);
                }
                return 1;
            }
            /* Connect to server */
            iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
            if (iResult == SOCKET_ERROR) {
                closesocket(ConnectSocket);
                ConnectSocket = INVALID_SOCKET;
                continue;
            }
            break;
        }
        
        if (ConnectSocket == INVALID_SOCKET) {
            fprintf(stderr, "Unable to connect to host.\n");
            return 1;
        }else{
            getpeername(ConnectSocket, (struct sockaddr *)&client_info, &addrsize);
            char *ip = inet_ntoa(client_info.sin_addr);
            fprintf(stderr, "connected to host %s.\n",ip);
        }
        
        
        char recvbuf[DEFAULT_BUFLEN];
        memset(recvbuf, '\0', DEFAULT_BUFLEN);
        NTickInitial=GetTickCount64();
        if (filename!=NULL){
            FILE *fptr;
            fptr = fopen(filename,"rb");
            if(fptr == NULL)
            {
               if (DEBUG) {
                fprintf(stderr, "open file Error!");
               }   
               exit(1);             
            }
            if (recvOne==2){
              while(fgets(msgbuf, DEFAULT_BUFLEN-1, fptr) != NULL){
                int iResult = send(ConnectSocket, msgbuf, strlen(mesg), 0);
                if (iResult<=0){
                    fprintf(stderr, "send failed with error: %d\n", iResult);
                    break;
                }
              }
            }else{
                int fr=0;
                do{
                    fr=readline(fptr,msgbuf,DEFAULT_BUFLEN);
                    if (fr==0){
                        if (DEBUG) {
                            fprintf(stderr, "read file content error\n");
                        }
                        break;
                    }else{
                        mesg=&msgbuf[0];
                        int iResult = send(ConnectSocket, mesg, strlen(mesg), 0);
                        if (iResult<=0){
                            fprintf(stderr, "send failed with error: %d\n", iResult);
                            break;
                        }
                    }
                }while (fr);
            }
            fclose(fptr);
        }else{
            int iResult = send(ConnectSocket, mesg, strlen(mesg), 0);
            if (iResult>0){
                do{
                    iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
                    fprintf(stderr, "%s",  recvbuf);
                    memset(recvbuf,0,DEFAULT_BUFLEN);
                }while (iResult&&recvOne);
            }else{
                fprintf(stderr, "send failed with error: %d\n", iResult);
            }
        }
        NTickShowEnd= GetTickCount64();
        fprintf(stderr, "end within %llu ms\n",  NTickShowEnd-NTickInitial);
        /* Shut down the connection since we're done */
        iResult = shutdown(ConnectSocket, SD_BOTH);
        if (iResult == SOCKET_ERROR) {
            err = WSAGetLastError ();
            FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
                           NULL,                // lpsource
                           err,                 // message id
                           MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
                           msgbuf,              // output buffer
                           sizeof (msgbuf),     // size of msgbuf, bytes
                           NULL);               // va_list of arguments
            if (DEBUG) {
                fprintf(stderr, "Shutdown failed with error: %ld,%s\n",  err,msgbuf);
            }
            closesocket(ConnectSocket);
            return 1;
        }
        closesocket(ConnectSocket);
        counts--;
    }
    gNTickShowEnd= GetTickCount64();
    if (DEBUG) {
        fprintf(stderr, "total elapsed: %llums\n",  gNTickShowEnd-gNTickInitial);
    }
    freeaddrinfo(result);
    return 0;
}
DWORD WINAPI threadstressclient(LPVOID arg)
{
    WSADATA wsaData;
    struct Thread_data* data = (Thread_data *)arg;
    /* Initialize Winsock */
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    stressclient(data->host, data->port,data->filename, data->mesg, data->counts,data->recvOne,data->timeout,NULL);
    WSACleanup();
    //free(data);
    return 0;
}
int udpclient(char *host, char *port, char *filename,char *mesg, char* otp)
{
	struct sockaddr_in server_address;
	int slen=sizeof(server_address);
	int iResult;
	char buf[DEFAULT_BUFLEN];
	WSADATA wsa;
	//Initialise winsock
	fprintf(stderr, "\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		fprintf(stderr, "Failed. Error Code : %d",WSAGetLastError());
		return -1;
	}
	fprintf(stderr, "Initialised.\n");
	
	//create socket
	SOCKET clientSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if ( clientSocket == SOCKET_ERROR)
	{
		fprintf(stderr, "socket() failed with error code : %d" , WSAGetLastError());
		return -1;
	}
	
	//setup address structure
	memset((char *) &server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(port));
	server_address.sin_addr.S_un.S_addr = inet_addr(host);
	
  if (mesg!=NULL){
    ULONGLONG NTickInitial,NTickShowEnd;
    char recvbuf[DEFAULT_BUFLEN];
    memset(recvbuf, '\0', DEFAULT_BUFLEN);
    NTickInitial=GetTickCount64();
  	//send the message
  	if (sendto(clientSocket, mesg, strlen(mesg) , 0 , (struct sockaddr *) &server_address, slen) == SOCKET_ERROR)
  	{
  		fprintf(stderr,"sendto() failed with error code : %d" , WSAGetLastError());
  		return -1;
  	}
		
  	//receive a reply and print it
  	//clear the buffer by filling null, it might have previously received data
  	memset(buf,0, DEFAULT_BUFLEN);
  	//try to receive some data, this is a blocking call
  	if (recvfrom(clientSocket, buf, DEFAULT_BUFLEN, 0, (struct sockaddr *) &server_address, &slen) == SOCKET_ERROR)
  	{
  		fprintf(stderr,"recvfrom() failed with error code : %d" , WSAGetLastError());
  		return -1;
  	}
    NTickShowEnd= GetTickCount64();
    fprintf(stderr, "reply %s in $llu ms\n",  recvbuf,NTickShowEnd-NTickInitial);
    closesocket(clientSocket);
    WSACleanup();
    return 1;
  }
    
  WinCat *wincat = new WinCat(filename);
  wincat->setUdp(NULL,server_address);
  wincat->Process(clientSocket);
    
  delete wincat;
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}
int broadcastUDPSender(char *subnet, char *port, char *filename,char *mesg) {
    WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
	{
		fprintf(stderr, "Failed. Error Code : %d",WSAGetLastError());
		return -1;
	}
 
  SOCKET sock;
  sock = socket(AF_INET,SOCK_DGRAM,0);
  char broadcast = '1';
  if(setsockopt(sock,SOL_SOCKET,SO_BROADCAST,&broadcast,sizeof(broadcast)) < 0)
  {
      fprintf(stderr,"Error in setting Broadcast option");
      closesocket(sock);
      return -1;
  }
 
  struct sockaddr_in Recv_addr;  
  struct sockaddr_in Sender_addr;
  int len = sizeof(struct sockaddr_in);
  char recvbuff[DEFAULT_BUFLEN] = "";
  int recvbufflen = DEFAULT_BUFLEN;
 
  Recv_addr.sin_family       = AF_INET;        
  Recv_addr.sin_port         = htons(atoi(port));   
  //  Recv_addr.sin_addr.s_addr  = INADDR_BROADCAST; // this isq equiv to 255.255.255.255
  // better use subnet broadcast (for our subnet is 172.30.255.255)
  Recv_addr.sin_addr.s_addr = inet_addr(subnet);
  sendto(sock,mesg,strlen(mesg)+1,0,(sockaddr *)&Recv_addr,sizeof(Recv_addr));
  recvfrom(sock,recvbuff,recvbufflen,0,(sockaddr *)&Recv_addr,&len);
  fprintf(stderr,"\n\n\tReceived message from Reader is => %s\n",recvbuff);
  fprintf(stderr,"\n\n\tpress any key to CONT...");
  //_getch();
 
  closesocket(sock);
  WSACleanup();
  return 0;
}

int multicastUDPSender(char *group, char *port, char *filename,char *mesg){
  //if (argc != 3) {
  //   printf("Command line args should be multicast group and port\n");
  //   printf("(e.g. for SSDP, `sender 239.255.255.250 1900`)\n");
  //   return 1;
  //}

  //char* group = argv[1]; // e.g. 239.255.255.250 for SSDP
  //int port = atoi(argv[2]); // 0 if error, which is an invalid port

  // !!! If test requires, make these configurable via args
  //
  const int delay_secs = 1;

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

  // set up destination address
  //
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(group);
  addr.sin_port = htons(atoi(port));

    // now just sendto() our destination!
  //
  while (1) {
    char ch = 0;
    int nbytes = sendto(
        fd,
        mesg,
        strlen(mesg),
        0,
        (struct sockaddr*) &addr,
        sizeof(addr)
    );
    if (nbytes < 0) {
        perror("sendto");
        return 1;
    }

   #ifdef _WIN32
   	Sleep(delay_secs * 1000); // Windows Sleep is milliseconds
   #else
   	sleep(delay_secs); // Unix sleep is seconds
   #endif
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
