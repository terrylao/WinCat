/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#include "server.h"
#include "WinCat.h"
#include "options.h"
#include <io.h>
#include <fcntl.h>

#define DEBUG 1
int hasAction;
SOCKET ListenSocket = INVALID_SOCKET;
void closeserver(){
  closesocket(ListenSocket);
  ListenSocket=-1;
}
DWORD WINAPI ListenClock(LPVOID arg)
{
  //Sleep(5* 60 * 1000);// five minutes
  Sleep(30 * 1000);
  int* listenSocket=(int*)arg;
  fprintf(stderr, "ListenClock %d,%d\n",*listenSocket,hasAction);
  if (hasAction==0){
    closesocket(*listenSocket);
  }
  return 0;
}
int server(char *port, char *filename, int keep_listening, char *restrictip, char *akey) {

  WSADATA wsaData;
  int iResult;
  int err;
  int singlecmd=0;
  char msgbuf [256];   // for a message up to 255 bytes.
  msgbuf [0] = '\0';    // Microsoft doesn't guarantee this on man page.
    
    
  SOCKET ClientSocket = INVALID_SOCKET;
    
  sockaddr_in client_info = {0};
  int addrsize = sizeof(client_info);
    
  struct addrinfo *result = NULL;
  struct addrinfo hints;
    
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
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  /* Resolve the server address and port */
  iResult = getaddrinfo(NULL, port, &hints, &result);
  if ( iResult != 0 ) {
      if (verbose) {
          printverbose();
          fprintf(stderr, "Getaddrinfo failed with error: %d\n", iResult);
      }
      WSACleanup();
      return 1;
  }

  /* Create a SOCKET for connecting to server */
  ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
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
        
      freeaddrinfo(result);
      WSACleanup();
      return 1;
  }

  /* Setup the TCP listening socket */
  iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
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
      freeaddrinfo(result);
      closesocket(ListenSocket);
      WSACleanup();
      return 1;
  }

  freeaddrinfo(result);

  iResult = listen(ListenSocket, SOMAXCONN);
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
      fprintf(stderr, "Listen failed with error: %d,%s\n",  err,msgbuf);
    }
        
    closesocket(ListenSocket);
    WSACleanup();
    return 1;
  }
  WinCat *wincat =new WinCat(filename);
	if (akey!=NULL){
		wincat->setOTP(akey);
	}
  if (filename!=NULL&&memcmp(filename,"cmd /c ",7)==0){
    singlecmd=1;
    fprintf(stderr, "Single CMD Mode\n");
  }else{
    fprintf(stderr, "Server Mode\n");
  }
  if (keep_listening==0){
    DWORD dwThreadId = 0;
    hasAction=0;
    fprintf(stderr, "Server Mode ListenSocket %d\n",ListenSocket);
    HANDLE thread=CreateThread(NULL, 0, ListenClock,  (LPVOID)&ListenSocket, 0, &dwThreadId);
  }
  do {

    ClientSocket = accept(ListenSocket, NULL, NULL);
    hasAction=1;
    if (ClientSocket == INVALID_SOCKET) {
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
        fprintf(stderr, "Accept failed with error: %d,%s\n",  err,msgbuf);
      }
      closesocket(ListenSocket);
      WSACleanup();
      return 1;
    }
    getpeername(ClientSocket, (struct sockaddr *)&client_info, &addrsize);
    char *ip = inet_ntoa(client_info.sin_addr);
    //if (verbose) {
    //    printverbose();
    //    fprintf(stderr, "connect from: %s\n",  ip);
    //}
    if (restrictip!=NULL){
      if (strstr(restrictip,ip)!=0){
                
      }else{
        if (verbose) {
          printverbose();
          fprintf(stderr, "ban connect from: %s\n",  ip);
        }
        closesocket(ClientSocket);
        continue;
      }
    }else{
      if (verbose) {
        printverbose();
        fprintf(stderr, "connect from: %s\n",  ip);
      }    
    }
    if (singlecmd==1){
    	wincat->SyncProcess(ClientSocket);
    }else{
    	wincat->Process(ClientSocket);
    }
  } while (keep_listening);
    
  /* No longer need server socket */
  closesocket(ListenSocket);

  /* Shut down the connection since we're done */
  iResult = shutdown(ClientSocket, SD_BOTH);
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
    closesocket(ClientSocket);
    WSACleanup();
    return 1;
  }
    
  /* Cleanup */
  delete wincat;
  closesocket(ClientSocket);
  WSACleanup();

  return 0;
}
