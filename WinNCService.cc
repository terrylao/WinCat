/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
/*
Installing the Service : C:\>sc create "WinNCService" binPath= D:\Download\win-cat-master\bin\WinNCService.exe
Uninstalling the Service : C:\>sc delete "WinNCService"  
*/
#define ISSERVICE 1
#include "main.cc"
SERVICE_STATUS        g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler (DWORD);
DWORD WINAPI ServiceWorkerThread (LPVOID lpParam);
FILE* stderr_redirect;
#define SERVICE_NAME  _T("WinNCService")
//today is a char array with 11 bytes of null terminate
void getToday(char* today){
    SYSTEMTIME lt = {0};
    FILETIME ft = {0};
    GetLocalTime(&lt);
    SystemTimeToFileTime(&lt, &ft);
    memset(today,0,11);
    sprintf(today,"%04d-%02d-%02d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour);
}
int _tmain (int argc, TCHAR *argv[])
{
    OutputDebugString(_T("WinNCService: Main: Entry"));

    SERVICE_TABLE_ENTRY ServiceTable[] = 
    {
        {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher (ServiceTable) == FALSE)
    {
       OutputDebugString(_T("WinNCService: Main: StartServiceCtrlDispatcher returned error"));
       return GetLastError ();
    }

    OutputDebugString(_T("WinNCService: Main: Exit"));
    return 0;
}


VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv)
{
    DWORD Status = E_FAIL;

    OutputDebugString(_T("WinNCService: ServiceMain: Entry"));

    g_StatusHandle = RegisterServiceCtrlHandler (SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL) 
    {
        OutputDebugString(_T("WinNCService: ServiceMain: RegisterServiceCtrlHandler returned error"));
        goto EXIT;
    }

    // Tell the service controller we are starting
    ZeroMemory (&g_ServiceStatus, sizeof (g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE) 
    {
        OutputDebugString(_T("WinNCService: ServiceMain: SetServiceStatus returned error"));
    }

    /* 
     * Perform tasks neccesary to start the service here
     */
    OutputDebugString(_T("WinNCService: ServiceMain: Performing Service Start Operations"));

    // Create stop event to wait on later.
    g_ServiceStopEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL) 
    {
        OutputDebugString(_T("WinNCService: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error"));

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
	    {
		    OutputDebugString(_T("WinNCService: ServiceMain: SetServiceStatus returned error"));
	    }
        goto EXIT; 
    }    

    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
	    OutputDebugString(_T("WinNCService: ServiceMain: SetServiceStatus returned error"));
    }

    // Start the thread that will perform the main task of the service
    HANDLE hThread = CreateThread (NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    OutputDebugString(_T("WinNCService: ServiceMain: Waiting for Worker Thread to complete"));
    
    // Wait until our worker thread exits effectively signaling that the service needs to stop
    //WaitForSingleObject (hThread, INFINITE);
    //  Periodically check if the service has been requested to stop
    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {        
        //  Simulate some work by sleeping
        Sleep(3000);
        fflush(stderr_redirect);
    }
    OutputDebugString(_T("WinNCService: ServiceMain: Worker Thread Stop Event signaled"));
    
    
    /* 
     * Perform any cleanup tasks
     */
    OutputDebugString(_T("WinNCService: ServiceMain: Performing Cleanup Operations"));

    CloseHandle (g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
	    OutputDebugString(_T("WinNCService: ServiceMain: SetServiceStatus returned error"));
    }
    
    EXIT:
    OutputDebugString(_T("WinNCService: ServiceMain: Exit"));

    return;
}


VOID WINAPI ServiceCtrlHandler (DWORD CtrlCode)
{
    OutputDebugString(_T("WinNCService: ServiceCtrlHandler: Entry"));

    switch (CtrlCode) 
	{
	  //state: SERVICE_CONTROL_STOP,SERVICE_CONTROL_PAUSE,SERVICE_CONTROL_CONTINUE,SERVICE_CONTROL_SHUTDOWN,SERVICE_CONTROL_INTERROGATE
     //case SERVICE_CONTROL_SHUTDOWN:
     case SERVICE_CONTROL_STOP :

        OutputDebugString(_T("WinNCService: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request"));

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
           break;

        /* 
         * Perform tasks neccesary to stop the service here 
         */
        closeserver();
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    		{
    			OutputDebugString(_T("WinNCService: ServiceCtrlHandler: SetServiceStatus returned error"));
    		}

        // This will signal the worker thread to start shutting down
        SetEvent (g_ServiceStopEvent);

        break;

     default:
         break;
    }

    OutputDebugString(_T("WinNCService: ServiceCtrlHandler: Exit"));
}


DWORD WINAPI ServiceWorkerThread (LPVOID lpParam)
{
    OutputDebugString(_T("WinNCService: ServiceWorkerThread: Entry"));

    //  Periodically check if the service has been requested to stop
    //while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    //{        
        /* 
         * Perform main service function here
         */
    char *argv[] = {"programname", "-l", "-k", "-c","-v","9339", NULL };
    int argc = sizeof(argv) / sizeof(char*) - 1;
		char path[ _MAX_PATH + 1 ];
		
		if ( GetModuleFileName( 0, path, sizeof(path)/sizeof(path[0]) ) > 0 )
		{
      char* p = path;
      while(strchr(p,'\\'))
      {
               p = strchr(p,'\\');
               p++;
      }
 
      *p = '\0';
		}else{
      memset(path,0,sizeof(path));
		  sprintf(path,"%s","c:/");
		}
		char logfilename[ _MAX_PATH + 1 ];
		char today[11];
		getToday(today);
		memset(logfilename,0,sizeof(logfilename));
		sprintf(logfilename,"%s%s%s%s",path,"WinNCService",today,".log");
		//fprintf(stderr, "logfile: %s\n", logfilename);
    stderr_redirect = freopen( logfilename, "w", stderr );
    mainsub(argc,argv);
    fclose( stderr_redirect );

        //  Simulate some work by sleeping
    //    Sleep(3000);
    //}

    OutputDebugString(_T("WinNCService: ServiceWorkerThread: Exit"));

    return ERROR_SUCCESS;
}
/** redirect STDXX to FILE
UNIX:
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
    int out = open("cout.log", O_RDWR|O_CREAT|O_APPEND, 0600);
    if (-1 == out) { perror("opening cout.log"); return 255; }

    int err = open("cerr.log", O_RDWR|O_CREAT|O_APPEND, 0600);
    if (-1 == err) { perror("opening cerr.log"); return 255; }

    int save_out = dup(fileno(stdout));
    int save_err = dup(fileno(stderr));

    if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }
    if (-1 == dup2(err, fileno(stderr))) { perror("cannot redirect stderr"); return 255; }

    puts("doing an ls or something now");

    fflush(stdout); close(out);
    fflush(stderr); close(err);

    dup2(save_out, fileno(stdout));
    dup2(save_err, fileno(stderr));

    close(save_out);
    close(save_err);

    puts("back to normal output");

    return 0;
}

WINDOWS:
DWORD CALLBACK DoDebugThread(void *)
{
    AllocConsole();
    SetConsoleTitle("Copilot Debugger");
    // The following is a really disgusting hack to make stdin and stdout attach
    // to the newly created console using the MSVC++ libraries. I hope other
    // operating systems don't need this kind of kludge.. :)
    stdout->_file = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
    stdin->_file  = _open_osfhandle((long)GetStdHandle(STD_INPUT_HANDLE), _O_TEXT);
    debug();
    stdout->_file = -1;
    stdin->_file  = -1;
    FreeConsole();
    CPU_run();
    return 0;
}

WINDOWS SERVICE:
FILE* stderr_redirect = freopen( "C:/stderr.log", "w", stderr );
fclose( stderr_redirect );
:OR:
If you are modifying the code to the service, you could call SetStdHandle in your ServiceMain:

SetStdHandle(STD_ERROR_HANDLE, logFileHandle);
*/
