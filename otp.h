/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#ifndef OTP_H
#define OTP_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <stdio.h>

int genOTP(char *output1,char *output2,char *output3);
int checkOTP(char *input);

#endif /* SCAN_H */
