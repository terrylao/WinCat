/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#ifndef MAIN_H
#define MAIN_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "options.h"
#include "server.h"
#include "client.h"
#include "scan.h"
#include "otp.h"
#include "WjCryptLib_Sha1.h"
#include <stdlib.h>
#include <stdio.h>
#include <String.h>
#include <windows.h>
#include <tchar.h>
#ifdef ISSERVICE
#define INSERVICE 1

void closeserver();
int mainsub(int argc, char **argv);

#else

#endif
#endif /* MAIN */
