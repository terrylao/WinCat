/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#include "options.h"
#include "server.h"
#include "udpserver.h"
#include "client.h"
#include "scan.h"
#include "otp.h"
#include "WjCryptLib_Sha1.h"
#include <stdlib.h>
#include <stdio.h>
#include <String.h>
#include <windows.h>
#include <tchar.h>
/*
wnc -l -k -U -P KKK -e "cmd" 8118
wnc -U -P 802163 127.0.0.1 8118
*/
#define DEBUG 1
#define MAX_LINE 4096

const char *version = "WinNetCat - v3.0  by  TerryLao\n";

const char *about = "A simple TCP/IP network debugging utility for Windows.\n"
                    "Inspired by the traditional nc we all know and love.\n";

const char *usage = "usage: wnc [-lakszrwechvtmT] [host] [port]\n";

const char *details =
                    "   l              Listen TCP for incoming connections. It is an error to\n"
                    "                  use this option with a host specified.\n"
                    "\n"
                    "   a              combine with l , can accept only connection which came from.\n"
                    "                  use this option restrict access ip.\n"
                    "\n"
                    "   D              combine with l , Daemon processed.\n"
                    "\n"
                    "   m              client mode send this message and then \"m\" \n"
                    "\n"
                    "   f/F filename   clinet mode same as -m but message read \"f\" line by line/\"F\" Block by Block from file max 8KB.\n"
                    "                  and receive once only e.g.wnc -t 10 -f c:\\foo.txt 127.0.0.1 2222 \n"
                    "\n"
                    "   t              combine with -m/-f to send times of message.\n"
                    "\n"
                    "   T              combine with -t  to with multithreaded send times of message.\n"
                    "\n"
                    "   k              Keep listening. Forces wc to stay listening \n"
                    "                  for another connection after its current\n"
                    "                  connection is completed. It is an error to use\n"
                    "                  this option without -l.\n"
//                    "\n"
//                    "   z              Specify port(s) on the host to scan for listening\n"
//                    "                  daemons using the connect() call. The default\n"
//                    "                  timeout is 750 milliseconds.\n"
//                    "                  e.g.    wnc -z localhost 1-200\n"
//                    "\n"
//                    "   r              Do a reverse DNS lookup with ICMP echo requests.\n"
//                    "                  e.g.    wnc -sr 10.0.0.0/24\n"
//                    "\n"
//                    "   w  timeout     The timeout in milliseconds for pings and connect\n"
//                    "                  scans.\n"
//                    "                  e.g.    wnc -zw 100 localhost 1000-2000\n"
                    "\n"
                    "   e  filename    Specify a filename to execute after connecting\n"
                    "                  (use with caution). See -c for enhanced\n" 
                    "                  functionality. It is an error to use this option\n" 
                    "                  with -c, -s, or -z.\n"
                    "                  e.g.    host A (10.0.0.2)>  wnc -l -k -e \"cmd\" 8118\n"
                    "                          host B (10.0.0.3)>  wnc 10.0.0.2 8118\n"
                    "\n"
                    "   c  command     Specify a command to pass to \"cmd /c\" for\n"
                    "                  execution after connecting. It is an error\n"
                    "                  to use this option with -e, -s, or -z.\n"
                    "                  e.g.    host A (10.0.0.2)>  wnc -l -c \"dir\" 8118\n"
                    "                          host B (10.0.0.3)>  wnc 10.0.0.2 8118\n"
                    "\n"
                    "   U           udp client Mode : wnc -U 10.0.0.2 8118\n"
                    "\n"
                    "   B           udp client Mode BroadCast : wnc -B 172.30.255.255 8118\n"
                    "\n"
                    "   M           udp client Mode MultiCast : wnc -M 239.255.255.250 8118\n"
                    "\n"
                    "   w n           client Mode Connect Send and Receive will: \n"
                    "                 0:close immediatly greater than wait for timeout, \n"
                    "                 F:will wait forever until peer disconnect.\n"
                    "                 O:will receive once and then disconnect.\n"
                    "                 R:receive only and only receive once.\n"
                    "\n"
                    "   h              Print this help page.\n"
                    "\n"
                    "   o  filename    client mode output to file.\n"
                    "\n"
                    "   p n             port scan,timeout n = 1~5 in seconds.\n"
                    "\n"
                    "   s  filename    SHA1_HASH a file.\n"
                    "\n"
                    "   v              verbose.\n"
                    "\n"
                    "   host           Can be a numerical address or a symbolic\n"
                    "                  hostname. If the -s option is specified, CIDR\n"
                    "                  notation (IPv4 only) can be used to specify a\n"
                    "                  range of hosts.\n"
                    "\n"
                    "   port           Must be single integer. If the -z option\n"
                    "                  is specified, a range of ports can be used instead.\n";                 
int print_version() {
    fprintf(stdout, "%s", version);
    return 0;
}
int print_usage() {
    print_version();
    fprintf(stderr, "%s", usage);
    return 1;
}

int print_help() {
    fprintf(stdout, "%s\n%s\n%s\n%s", version, about, usage, details);
    return 0;
}

void replacecontrolchar(char *c){
    int i=0;
    while (c[i]!=NULL){
        if (c[i]=='\\'){
            i++;
            switch(c[i]){
                case 'n':
                   c[i-1]=0x0d;
                   c[i]=0x0a;
                   break;
                case 't':
                    c[i-1]=0x09;
                    c[i]=' ';
                    break; 
            }
        }
        i++;
    }
}
void closeListenserver(){
  closeserver();
}
#ifdef ISSERVICE
int mainsub(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{	
    fprintf(stderr, "version: %s\n", version);
    int total_unique_opts = pre_parse_opts(argc, argv);
    char opt;
    while (opt = get_opt(argc, argv)) {
        switch(opt) {
            case 'l':
                {   
                    if (opt_count('s') || opt_count('z')) return print_usage();
                    if (opt_count('e') && opt_count('c') && opt_count('a')) return print_usage();

                    //fprintf(stderr, "argind+1: %d=%d has a %s\n", argind+1,argc,opt_arg('a'));
                    
                    argind += opt_arg('e')? 1 : 0;
                    //argind += opt_arg('c')? 1 : 0;
                    argind += ((!opt_arg('e')&&!opt_arg('c'))&&opt_arg('a'))? 1 : 0;
                    if (opt_count('v')){ 
                        verbose=1;
                    }
                    fprintf(stderr, "argind+: %d=%d\n", argind+1,argc); //because of server not bind ip
                    if (argind+1 != argc) return print_usage();

                    if (DEBUG) {
                        if (opt_count('k')) fprintf(stderr, "-k\n");
                        if (opt_count('e')) fprintf(stderr, "-e %s\n", opt_arg('e'));
                        if (opt_count('c')) fprintf(stderr, "-c %s\n", opt_arg('c'));
                        if (opt_count('a')) fprintf(stderr, "-a %s\n", opt_arg('a'));
												if (opt_count('P')) fprintf(stderr, "-P %s\n", opt_arg('P'));
                        if (opt_count('v')) fprintf(stderr, "-v on\n");
												if (opt_count('U')) fprintf(stderr, "-U\n");
                        if (opt_count('k')) fprintf(stderr, "-k\n");
                        fprintf(stderr, "port: %s\n", argv[argind]);
                    }
                    if (opt_count('D')) {
                      FreeConsole();
                    }
                    if (opt_count('e')) {
											if (opt_count('U')){
												return udpserver(argv[argind], opt_arg('e'), opt_count('k') ? 1 : 0, opt_arg('a'),opt_arg('P'));
											}else{
												return server(argv[argind], opt_arg('e'), opt_count('k') ? 1 : 0, opt_arg('a'),opt_arg('P'));
											}
                    } 

                    if (opt_count('c')) {
                        char command[MAX_LINE+1] = {0};
                        memset(command,0,MAX_LINE+1);
                        if (opt_arg('c')){
                            memcpy(command, "cmd /c ", 7);
                            memcpy(command+7, opt_arg('c'), strlen(opt_arg('c')));
														if (opt_count('U')){
															return udpserver(argv[argind], command, opt_count('k') ? 1 : 0, opt_arg('a'),opt_arg('P'));
														}else{
															return server(argv[argind], command, opt_count('k') ? 1 : 0, opt_arg('a'),opt_arg('P'));
														}
                            
                        }else{
                            memcpy(command, "cmd /c ", 7);
														if (opt_count('U')){
															return udpserver(argv[argind], command, opt_count('k') ? 1 : 0, opt_arg('a'),opt_arg('P'));
														}else{
															return server(argv[argind], command, opt_count('k') ? 1 : 0, opt_arg('a'),opt_arg('P'));
														}
                        }
                    }
										if (opt_count('U')){
											return udpserver(argv[argind], NULL, opt_count('k') ? 1 : 0,opt_arg('a'),opt_arg('P'));
										}else{
											return server(argv[argind], NULL, opt_count('k') ? 1 : 0,opt_arg('a'),opt_arg('P'));
										}
                    
                }
            case 's':
                {
                    SHA1_HASH digest;
                    fprintf(stderr, "hashed: %s\n", opt_arg('s'));
                    Sha1File(opt_arg('s'),&digest);
                    int i;
                    for( i=0; i<sizeof(digest); i++ )
                    {
                        printf( "%02x", digest.bytes[i] );
                    }
                    return 0;
                }
            case 'z': 
                {   
                    if (opt_count('w') && !opt_arg('w')) return print_usage();
                    
                    argind += opt_arg('w')? 1 : 0;

                    if (opt_arg('z') && opt_arg('w')) {
                        if (strcmp(opt_arg('w'), opt_arg('z'))) argind--;
                    }

                    if (DEBUG) {
                        if (opt_count('z')) fprintf(stderr, "-z\n");
                        if (opt_arg('w')) fprintf(stderr, "-w arg: %s\n", opt_arg('w'));
                        if (argind < argc-1) fprintf(stderr, "host: %s\nport: %s\n", argv[argind], argv[argind+1]);
                    }

                    if (argind+2 != argc) return print_usage();

                    int low;
                    int high;
                    char *start = strtok(argv[argind+1], "-");
                    low = atoi(start);
                    char *end = strtok(NULL, "-");
                    if (end == NULL)  high = low;
                    else high = atoi(end);
                    
                    return connect_scan(argv[argind], low, high, opt_arg('w') ? atoi(opt_arg('w')) : 750);
                }
            case 'h': 
                return print_help();
								
            case 'c':
                {
                    if (opt_count('e') || opt_count('z') || opt_count('s')) return print_usage();
                    if (!opt_count('l')) {
                        
                        if (opt_arg('c')) argind++;

                        if (argind+2 != argc) return print_usage();

                        if (DEBUG) {
                            fprintf(stderr, "-c arg: %s\n", opt_arg('c'));
                            fprintf(stderr, "host: %s\nport: %s\n", argv[argind], argv[argind+1]);
                        }

                        char command[MAX_LINE+1] = {0};
                        memcpy(command, "cmd /c ", 7);
                        memcpy(command+7, opt_arg('c'), strlen(opt_arg('c')));
												if (opt_arg('U')){
													return udpclient(argv[argind], argv[argind+1], command,NULL,opt_arg('P'));
												}else{
													return client(argv[argind], argv[argind+1], command,NULL,opt_arg('P'));
												}
                        
                    }

                }
                break;
            case 'e':
                {
                    if (opt_count('c') || opt_count('z') || opt_count('s')) return print_usage();
                    if (!opt_count('l')) {
                        
                        if (opt_arg('e')) argind++;

                        if (argind+2 != argc) return print_usage();

                        if (DEBUG) {
                            fprintf(stderr, "-e arg: %s\n", opt_arg('e'));
                            fprintf(stderr, "host: %s\nport: %s\n", argv[argind], argv[argind+1]);
                        }
												if (opt_arg('U')){
													return udpclient(argv[argind], argv[argind+1], opt_arg('e'),NULL,opt_arg('P'));
												}else{
													return client(argv[argind], argv[argind+1], opt_arg('e'),NULL,opt_arg('P'));
												}
                        
                    }
                }
                break;
            case 'p':
            {
              argind ++;
              fprintf(stderr, "host: %s\nport: %s\n", argv[argind], argv[argind+1]);
              int portscantimeout=atoi(opt_arg('p'));
              if (portscantimeout>5){
                portscantimeout=5;
              }
              portscan(argv[argind], argv[argind+1], portscantimeout);
              return 1;
            }
						case 'P':
							argind += opt_arg('P')? 1 : 0;
							break;
            case 't':
            case 'T':
            case 'm':
            case 'f':
            case 'F':
            case 'w':
            {
                if (opt_count('v')){ 
                    verbose=1;
                }
                if (opt_count('f') && opt_count('m')) return print_usage();
                if (!opt_count('l')) {
                    //they has parameters
                    //argind += opt_arg('m')? 1 : 0;
                    argind += opt_arg('t')? 1 : 0;
                    argind += opt_arg('f')? 1 : 0;
                    argind += opt_arg('T')? 1 : 0;
                    argind += opt_arg('w')? 1 : 0;
                    //argind += opt_arg('o')? 1 : 0;
                    
                    fprintf(stderr, "argind: %d=%d has m %s\n", argind,argc,opt_arg('m'));
                    if (argind+2 != argc) return print_usage();
                    unsigned long ul=1;
                    if (opt_arg('t')){
                        ul=atol(opt_arg('t'));
                    }
                    if (ul==0L){
                        return fprintf(stderr, "-t with non digits: %s\n", opt_arg('t'));
                    }
                    if (DEBUG) {
                        fprintf(stderr, "-m arg: %s\n", opt_arg('m'));
                        fprintf(stderr, "-t arg: %lu\n", ul);
                        fprintf(stderr, "-f arg: %s\n", opt_arg('f'));
                        fprintf(stderr, "-F arg: %s\n", opt_arg('F'));
                        fprintf(stderr, "-T arg: %s\n", opt_arg('T'));
                        fprintf(stderr, "-C arg: %s\n", opt_arg('C'));
                        fprintf(stderr, "host: %s\nport: %s\n", argv[argind], argv[argind+1]);
                    }
                    char *c=NULL;
                    char *outputfile=NULL;
                    int recvOne=1;
                    int timeout=0;
                    if (opt_arg('o'))
                        outputfile=opt_arg('o');
                        
                    if (opt_arg('m'))
                        c=opt_arg('m');
                    if (opt_arg('w')){
                        recvOne=0;
                        if (opt_arg('w')[0]=='F'){
                          timeout=-1;
                        }else
                        if (opt_arg('w')[0]=='O'){
                          timeout=-2;
                        }else
                        if (opt_arg('w')[0]=='R'){
                          timeout=-3;
                        }else{
                          timeout=atol(opt_arg('w'));
                        }
                    }
                    if (c)
                        replacecontrolchar(c);
                    if (opt_arg('T')){
                        unsigned long threads=atol(opt_arg('T'));
                        unsigned long slab=ul/threads;
                        fprintf(stderr, "-T arg: %lu,%lu\n", threads,slab);
                        Thread_data* tt=(Thread_data*)malloc(threads*sizeof(Thread_data));
                        int i;
                        for (i=0;i<threads;i++){
                            tt[i].host=argv[argind];
                            tt[i].port=argv[argind+1];
                            if (opt_arg('F')){
                                recvOne=2;
                                tt[i].filename=opt_arg('F');
                            }else{
                                tt[i].filename=opt_arg('f');
                            }
                            tt[i].mesg=c;
                            tt[i].counts=slab;
                            tt[i].recvOne=recvOne;
                            DWORD dwThreadId = 0;
                            HANDLE thread=CreateThread(NULL, 0, threadstressclient,  (LPVOID)&tt[i], 0, &dwThreadId);
                            fprintf(stderr, "createthread: %d\n", dwThreadId);
                        }
                        getchar();
                        free(tt);
                        return 0;
                    }else{
                        fprintf(stderr, "c: %s\n", c);
                        if (opt_arg('F')){
                            return stressclient(argv[argind], argv[argind+1],opt_arg('F'), c, ul,2,timeout,outputfile);
                        }else{
                            return stressclient(argv[argind], argv[argind+1],opt_arg('f'), c, ul,recvOne,timeout,outputfile);
                        }
                    }
                }
            }
						case 'v':
						{
							verbose=1;
							break;
						}
						case 'U':
						{
							break;
						}
            default:
                if (DEBUG) fprintf(stderr, "? -%c\n", opt);
                break;
        }
    }

    /* No options specified - client connect */
    if (argind >= argc-1)  {return print_usage();}
    if (DEBUG) fprintf(stderr, "host: %s\nport: %s\n", argv[argind], argv[argind+1]);
		if (opt_arg('U')){
			return udpclient(argv[argind], argv[argind+1], NULL,NULL,opt_arg('P'));
		}else{
			return client(argv[argind], argv[argind+1], NULL,NULL,opt_arg('P'));
		}
    
}

//int main(int argc1, char **argv1){
//    char *argv[] = {"mememe","-l", "-k", "-c","-v","9339", NULL };
//    int argc = sizeof(argv) / sizeof(char*) - 1;;
//    int q= mainsub(argc,argv);
//}
