/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#include "otp.h"
#include "WjCryptLib_Sha1.h"
#include <stdlib.h>
#include <stdio.h>
#include <String.h>
#include <windows.h>

int main(int argc, char **argv) {
    fprintf(stderr, "OTP generator version: 1.0\n");
    char ipt1[41],ipt2[41],ipt3[41];
    char userinput[7];
    memset(&ipt1[0],0,41);
    memset(&ipt2[0],0,41);
    memset(&ipt3[0],0,41);
    memset(&userinput,0,7);
    genOTP(&ipt1[0],&ipt2[0],&ipt3[0]);
    memcpy(userinput,ipt1,6);
    printf("OTP:%s\n",userinput);
    return 0;
}