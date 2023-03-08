# 1) vcvarsall x86
# 2) nmake

APPNAME = wnc

CONSOLE_APP = /SUBSYSTEM:CONSOLE
GUI_APP = /SUBSYSTEM:WINDOWS

CC = CL
CFLAGS = /EHsc /openmp
INCS = 

LIBS = user32.lib \
	   Ws2_32.lib \
	   Mswsock.lib \
	   kernel32.lib \
	   Iphlpapi.lib \
	   Advapi32.lib

LFLAGS = /link \
		 /ENTRY:mainCRTStartup

# Default target
all: build


build: objects
	$(CC) $(CFLAGS) /Fe$(APPNAME) $(INCS) *.obj $(LIBS) $(LFLAGS) $(CONSOLE_APP)
	DEL *.obj

daemonize: objectC
	$(CC) $(CFLAGS) /Fe daemonize $(INCS) *.obj $(LIBS) $(LFLAGS) $(CONSOLE_APP)
	DEL *.obj
	MOVE /Y daemonize.exe bin/daemonize.exe
	
hello: HelloC
	$(CC) $(CFLAGS) /Fe hello $(INCS) *.obj $(LIBS) $(LFLAGS) $(CONSOLE_APP)
	DEL *.obj
	MOVE /Y hello.exe bin/hello.exe
otp: OTPC
	$(CC) $(CFLAGS) /Fe otpgenerator $(INCS) *.obj $(LIBS) $(LFLAGS) $(CONSOLE_APP)
	DEL *.obj
	MOVE /Y otpgenerator.exe bin/otpgenerator.exe
service: SERVICEC
	$(CC) $(CFLAGS) /Fe WinNCService $(INCS) *.obj $(LIBS) $(LFLAGS) $(CONSOLE_APP)
	DEL *.obj
	MOVE /Y WinNCService.exe bin/WinNCService.exe
# Build object files
objects:	main.cc otp.cc WjCryptLib_Sha1.cc server.cc client.cc scan.cc udpserver.cc options.cc pipes.cc WinCat.cc AsyncStreamReader.cc
	$(CC) $(CFLAGS) /c $(INCS) $(CFLAGS) $?	

objectC:	daemonize.c
	$(CC) $(CFLAGS) /c $(INCS) $(CFLAGS) $?
	
HelloC:	hello.c
	$(CC) $(CFLAGS) /c $(INCS) $(CFLAGS) $?
	
OTPC:	OTPGenerator.cc otp.cc WjCryptLib_Sha1.cc
	$(CC) $(CFLAGS) /c $(INCS) $(CFLAGS) $?

SERVICEC: WinNCService.cc  otp.cc WjCryptLib_Sha1.cc server.cc client.cc scan.cc udpserver.cc options.cc pipes.cc WinCat.cc AsyncStreamReader.cc
	$(CC) $(CFLAGS) /c $(INCS) $(CFLAGS) $?
	
# Clean any leftover build files and executables
clean:
#	DEL *.exe *.obj
	DEL *.obj & cd bin & DEL *.exe
