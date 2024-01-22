/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#include "wincat.h"
#include "pipes.h"
#include "options.h"
#include "otp.h"
#include <windows.h>
#include <io.h>
#include <fcntl.h>

#define DEBUG 1

WinCat::WinCat(char *filename) {
  this->filename = NULL;
  if (filename != NULL) {
    this->filename = (char *) malloc((strlen(filename) + 1) * sizeof(char));
    strcpy(this->filename, filename);
  }
  this->launched = 0;
  udpsocket = 0;
	otppass = 0;
	memset(key,0,32);
}
void WinCat::setUdp(char * restrictip,struct sockaddr_in audpAddr) {
	udpsocket = 1;
	myrestrictip=restrictip;
	udpAddr=audpAddr;
	udpAddrSize=sizeof(udpAddr);
}
void WinCat::setOTP(char* akey){
	otppass=1;
	if (strlen(akey)<32){
		memcpy(key,akey,strlen(akey));
	}else{
		memcpy(key,akey,32);
	}
}
WinCat::~WinCat() { 
  if (pipes.process_spawned) close_pipes(pipes);
  free(filename);
  if (input) delete input;
  _setmode(fileno(stdout), _O_TEXT);
}

int WinCat::SyncProcess(SOCKET ClientSocket) {
  STARTUPINFO sinfo;
  PROCESS_INFORMATION pinfo;
  SECURITY_ATTRIBUTES sattr;
  HANDLE readfh, writefh;

	// Initialize the STARTUPINFO struct
  ZeroMemory(&sinfo, sizeof(STARTUPINFO));
  sinfo.cb = sizeof(STARTUPINFO);

  sinfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

  // Uncomment this if you want to hide the other app's
  // DOS window while it runs
  //    sinfo.wShowWindow = SW_HIDE;

  sinfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  sinfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  sinfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);

  // Initialize security attributes to allow the launched app to
  // inherit the caller's STDOUT, STDIN, and STDERR
  DWORD dwRead, dwWritten,dwAvail;
  char chBuf[DEFAULT_BUFLEN];
  char cmd[DEFAULT_BUFLEN];
  int iResult;
  sattr.nLength = sizeof(SECURITY_ATTRIBUTES);
  sattr.lpSecurityDescriptor = 0;
  sattr.bInheritHandle = TRUE;
  struct sockaddr_in SenderAddr;
  int SenderAddrSize = sizeof(SenderAddr);
  if (DEBUG) fprintf(stderr, "Wincat: enter sync processing.\n");
  if (udpsocket==1){
  	iResult = recvfrom(ClientSocket, chBuf, DEFAULT_BUFLEN,0, (SOCKADDR *)&SenderAddr, &SenderAddrSize);
  }else{
  	iResult = recv(ClientSocket, chBuf, DEFAULT_BUFLEN, 0);
  }
  if (DEBUG) fprintf(stderr, "Wincat: recv: %s.\n",chBuf);
  if ( iResult <= 0 ) {
    printverbose();
    if (DEBUG) fprintf(stderr, "Wincat: Connection closed on other end.\n");
    return -1;
  }
  if (udpsocket==1){
    getpeername(ClientSocket, (SOCKADDR *)&SenderAddr, &SenderAddrSize);
    char *ip =inet_ntoa(SenderAddr.sin_addr);
    if (myrestrictip!=NULL){
      if (strcmp(myrestrictip,ip)==0){
                
      }else{
        if (verbose) {
          printverbose();
          fprintf(stderr, "ban connect from: %s\n",  ip);
        }
        closesocket(ClientSocket);
        return -3;
      }
    }else{
      if (verbose) {
        printverbose();
        fprintf(stderr, "connect from: %s\n",  ip);
      }    
    }
  }
  if (memchr(chBuf,'\n',iResult)==NULL){
  	return -2;
  }
  //One Time Pass verification
  //char ipt1[41],ipt2[41],ipt3[41];
  char userinput[7];
  //memset(&ipt1[0],0,41);
  //memset(&ipt2[0],0,41);
  //memset(&ipt3[0],0,41);
  memset(&userinput,0,7);
  //genOTP(&ipt1[0],&ipt2[0],&ipt3[0]);
  //memcpy(userinput,ipt1,6);
  //printf("OTP:%s\n",userinput);
  //printf("checkOTP:%d\n",checkOTP(userinput));
  memcpy(userinput,chBuf,6);
	if (otppass){
    if (checkOTP(userinput,key)==0){
      fprintf(stderr, "Wincat: checkOTP Error");
      closesocket(ClientSocket);
      return -3;
    }
		otppass=1;
	}
    
  memset(cmd,0,sizeof(cmd));
  memcpy(cmd,this->filename,strlen(this->filename));
  //if (DEBUG) fprintf(stderr, "Wincat: show %s.\n",&chBuf[6]);
  //if (DEBUG) fprintf(stderr, "Wincat: cmd %s.\n",cmd);
  //if (DEBUG) fprintf(stderr, "Wincat: iResult: %d.\n",iResult);
  //chBuf[7] because of one time pass is 6 bytes, the 7th is empty space
  memcpy(&cmd[strlen(this->filename)],&chBuf[6],iResult-7);
  if (DEBUG) {
    printverbose();
    fprintf(stderr, "Wincat: execute:%s\n",cmd);
  }
    
  // Get a pipe from which we read
  // output from the launched app
  if (!CreatePipe(&readfh, &sinfo.hStdOutput, &sattr, 0))
  {
		// Error opening the pipe
    if (DEBUG) {
      printverbose();
      fprintf(stderr, "Wincat: Error opening the pipe\n");
    }
    closesocket(ClientSocket);
    return -1;
  }
  HANDLE CallerToken= NULL;
  if ( !OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS , &CallerToken) ) {
    printverbose();
    fprintf(stderr, "OpenProcessToken failed - 0x%08x\n", GetLastError());
    closesocket(ClientSocket);
    return -1;
  }
	// Launch the app. We should return immediately (while the app is running)
  //if (!CreateProcess(0, cmd, 0, 0, TRUE, 0, 0, 0, &sinfo, &pinfo))
  if (!CreateProcessAsUser(CallerToken,0, cmd, 0, 0, TRUE, 0, 0, 0, &sinfo, &pinfo))
  {
    if (DEBUG) {
      printverbose();
      fprintf(stderr, "Wincat: Error CreateProcess\n");
    }
    CloseHandle(sinfo.hStdInput);
    //CloseHandle(writefh);
    CloseHandle(readfh);
    CloseHandle(sinfo.hStdOutput);
    closesocket(ClientSocket);
    return -1;
  }

	// Don't need the read access to these pipes
  CloseHandle(sinfo.hStdInput);
  CloseHandle(sinfo.hStdOutput);

	// We haven't yet read app's output
  sinfo.dwFlags = 0;

	// Input and/or output still needs to be done?
  while (readfh)
  {
    // Capture more output of the app?
    // Read in upto DEFAULT_BUFLEN bytes
    if (!ReadFile(readfh, chBuf, DEFAULT_BUFLEN, &dwRead, 0))
    {
      // If we aborted for any reason other than that the
      // app has closed that pipe, it's an
      // error. Otherwise, the program has finished its
      // output apparently
      if (GetLastError() != ERROR_BROKEN_PIPE && dwRead)
      {
				// An error reading the pipe
        if (DEBUG) {
          printverbose();
          fprintf(stderr, "Wincat: An error reading the pipe: %d\n",GetLastError());
        }
      }
      break;
    }
    if (udpsocket==1){
			iResult = sendto(ClientSocket, chBuf, dwRead, 0, (struct sockaddr*) &SenderAddr, SenderAddrSize);
    }else{
			iResult = send(ClientSocket, chBuf, dwRead, 0);
    }
    if ( iResult == 0 ) {
      if (DEBUG) {
        printverbose();
        fprintf(stderr, "Wincat: Connection closed on other end.\n");
      }
      break;
    } else if (iResult < 0 ) {
      if (DEBUG) {
        printverbose();
        fprintf(stderr, "Wincat: Send() failed with error: %d\n", WSAGetLastError());
      }
      break;
    }
  }

// Close input pipe if it's still open
  //if (writefh) CloseHandle(writefh);

// Close output pipe
  if (readfh) CloseHandle(readfh);
    
  closesocket(ClientSocket);
// Return the output
  return 0;
}
int WinCat::Launch() {
  int reader = TEXT_READER;

  if (this->filename == NULL) {
    if (!_isatty(fileno(stdin)) ) {
    	reader = BINARY_READER;
    }

    /* Create thread for non-blocking stdin reader */
    AsyncStreamReader *input = new AsyncStreamReader(stdin, reader);
    this->input = input;

    /* Enable VT100 and similar control character sequences 
       that control cursor movement, color/font mode, 
       and other operations */
    //SetConsoleMode( GetStdHandle(STD_OUTPUT_HANDLE), 
    //                ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING); 
    fprintf(stderr,"lauch no filename");
    this->output = stdout;

  } else {
    /* Create a new process */
    this->pipes = get_pipes(this->filename);
    if (!pipes.process_spawned) {
    	return 1;
    } 
    
    int in_fd = _open_osfhandle((intptr_t)this->pipes.Child_Std_OUT_Rd, _O_BINARY | _O_RDONLY);
    int out_fd = _open_osfhandle((intptr_t)this->pipes.Child_Std_IN_Wr, _O_BINARY | _O_APPEND);

    FILE *in = _fdopen(in_fd, "rb");
    FILE *out = _fdopen(out_fd, "wb");  

    /* Create thread for non-blocking stream reader */
    AsyncStreamReader *input = new AsyncStreamReader(in, reader);
    this->input = input;
    this->output = out;
  }

  _setmode(fileno(stdout), _O_BINARY);
  this->launched = 1;
  return 0;
}
int WinCat::Process(SOCKET ClientSocket) {
  if (!this->launched) {
  	if(Launch()) 
			return 1;
  }
    
  int iResult;

  fd_set readfds;
  fd_set writefds;
  fd_set exceptfds;
  int err;
  char msgbuf [256];   // for a message up to 255 bytes.
  msgbuf [0] = '\0';    // Microsoft doesn't guarantee this on man page.
  char sendbuf[DEFAULT_BUFLEN];
  char recvbuf[DEFAULT_BUFLEN];
  struct sockaddr_in SenderAddr;
  int SenderAddrSize = sizeof(SenderAddr);
  memset(recvbuf, '\0', DEFAULT_BUFLEN);
  memset(sendbuf, '\0', DEFAULT_BUFLEN);
  int passchecked=0;
	char* precvbuf;
	int udpClientSocket=ClientSocket+1;
  /* Main run-forever loop: */
  for(;;) {
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);

    FD_SET(ClientSocket, &readfds);
    FD_SET(ClientSocket, &writefds);
    FD_SET(ClientSocket, &exceptfds);
		int rs = 0;
		if (udpsocket==1){
			rs = select(udpClientSocket, &readfds, &writefds, NULL, NULL);
		}else{
			rs = select(1, &readfds, &writefds, &exceptfds, NULL);
		}
    
    if (rs == SOCKET_ERROR) {
      err = WSAGetLastError();
      FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
                     NULL,                // lpsource
                     err,                 // message id
                     MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
                     msgbuf,              // output buffer
                     sizeof (msgbuf),     // size of msgbuf, bytes
                     NULL);               // va_list of arguments
      if (DEBUG) {
      	fprintf(stderr, "Wincat: Select() failed with error: %d,%s\n", err,msgbuf);
      }
      break;
    }

    if (FD_ISSET(ClientSocket, &readfds)) {
      /* SOCKET IS READY FOR READING */
      if (udpsocket==1){
      	iResult = recvfrom(ClientSocket, recvbuf, DEFAULT_BUFLEN,0, (SOCKADDR *)&udpAddr, &udpAddrSize);
      }else{
      	iResult = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
      }
            
      if ( iResult > 0 ) {
        if (udpsocket==1){
          getpeername(ClientSocket, (SOCKADDR *)&SenderAddr, &SenderAddrSize);
          char *ip =inet_ntoa(SenderAddr.sin_addr);
          if (myrestrictip!=NULL){
            if (strcmp(myrestrictip,ip)==0){
                
            }else{
              if (verbose) {
                printverbose();
                fprintf(stderr, "ban connect from: %s\n",  ip);
              }
              closesocket(ClientSocket);
              return -3;
            }
          }else{
            if (verbose) {
              printverbose();
              fprintf(stderr, "connect from: %s\n",  ip);
            }    
          }
      	}
        if (verbose) {
          printverbose();
          fprintf(stderr, "Wincat: recv: %s-\n",recvbuf);
        }
  			if (otppass&&passchecked==0){
          char userinput[7];
          memset(&userinput,0,7);
          memcpy(userinput,recvbuf,6);
          if (checkOTP(userinput,key)==0){
            fprintf(stderr, "Wincat: checkOTP Error");
            closesocket(ClientSocket);
            return -3;
          }
  				passchecked=1;
  				otppass=0;
  				precvbuf=&recvbuf[6];
  				iResult-=6;
  			}else{
  				precvbuf=&recvbuf[0];
  			}
        fwrite(&precvbuf[0], 1, iResult, output);
        fflush(output);
      } else if ( iResult == 0 ) {
        if (DEBUG) fprintf(stderr, "Wincat: Connection closed on other end.\n");
        break;
      } else {
        err = WSAGetLastError();
        FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
                       NULL,                // lpsource
                       err,                 // message id
                       MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
                       msgbuf,              // output buffer
                       sizeof (msgbuf),     // size of msgbuf, bytes
                       NULL);               // va_list of arguments
        if (DEBUG) {
         fprintf(stderr, "Wincat: Recv() failed with error: %d.%s\n", err,msgbuf);
        }
        break;
      }
      memset(recvbuf, '\0', DEFAULT_BUFLEN);

    } else if (FD_ISSET(ClientSocket, &writefds)) {
      /* SOCKET IS READY FOR WRITING */
			if (passchecked==1){
				sendbuf[0]='O';
				sendbuf[1]='K';
				sendbuf[2]='.';
				iResult = send(ClientSocket, sendbuf, 3, 0);
				passchecked=2;
			}
      int bytes_read = this->input->Read(sendbuf);
      if (bytes_read > 0) {
        if (udpsocket==1){
        	iResult = sendto(ClientSocket, sendbuf, bytes_read, 0, (struct sockaddr*) &udpAddr, udpAddrSize);
        }else{
        	iResult = send(ClientSocket, sendbuf, bytes_read, 0);
        }
        if ( iResult == 0 ) {
          if (DEBUG) fprintf(stderr, "Wincat: Connection closed on other end.\n");
          break;
        } else if (iResult < 0 ) {
          if (DEBUG) fprintf(stderr, "Wincat: Send() failed with error: %d\n", WSAGetLastError());
          break;
        }
        if (verbose) {
          printverbose();
          fprintf(stderr, "Wincat: Send(): %s-", sendbuf);
        }
      } else if (bytes_read < 0) {
        if (DEBUG) fprintf(stderr, "Wincat: Input reading complete.\n");
        break;
      }
      memset(sendbuf, '\0', DEFAULT_BUFLEN);
    } else if (FD_ISSET(ClientSocket, &exceptfds)) {
      /* SOCKET EXCEPTION */
      if (DEBUG) fprintf(stderr, "Wincat: Socket exception.\n");
      break;
    }
    /* Free up some CPU time for the OS */
    Sleep(1);
  }
  return 0;
}
