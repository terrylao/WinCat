/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#include "otp.h"
#include <omp.h>
#include <stdlib.h>
#include <math.h>
#include "WjCryptLib_Sha1.h"
#define DEBUG 0
#define NSECS 60*60*1  // 1hours
//http://zetcode.com/gui/winapi/datetime/
int genOTP(char *output1,char *output2,char *output3) {
    SYSTEMTIME lt = {0};
    FILETIME ft = {0};
    GetLocalTime(&lt);
    SystemTimeToFileTime(&lt, &ft);
    ULARGE_INTEGER u = {0};
    char input[20];
    int i,j;
    memset(&input,0,sizeof(input));
    sprintf(input,"Today:%04d-%02d-%02d_%02d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour);
    //printf("Today:%04d-%02d-%02d_%02d\n", lt.wYear, lt.wMonth, lt.wDay, lt.wHour);
    Sha1Context     sha1Context;
    SHA1_HASH       sha1Hash;
    Sha1Initialise( &sha1Context );
    Sha1Update( &sha1Context, input, (uint32_t)strlen(input) );
    Sha1Finalise( &sha1Context, &sha1Hash );
    j=0;
    for( i=0; i<sizeof(sha1Hash); i++ )
    {
        //printf( "%2.2x", sha1Hash.bytes[i] );
        sprintf(&output1[j],"%2.2x", sha1Hash.bytes[i] );
        j+=2;
    }
    //printf( "\n" );
    //sub one hour
    memcpy(&u, &ft, sizeof(u));
    u.QuadPart -= NSECS * 10000000LLU;
    memcpy(&ft, &u, sizeof(ft));
    FileTimeToSystemTime(&ft, &lt);
    sprintf(input,"Today:%04d-%02d-%02d_%02d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour);
    //printf("Today:%04d-%02d-%02d_%02d\n", lt.wYear, lt.wMonth, lt.wDay, lt.wHour);
    Sha1Initialise( &sha1Context );
    Sha1Update( &sha1Context, input, (uint32_t)strlen(input) );
    Sha1Finalise( &sha1Context, &sha1Hash );
    j=0;
    for( i=0; i<sizeof(sha1Hash); i++ )
    {
        //printf( "%2.2x", sha1Hash.bytes[i] );
        sprintf(&output2[j],"%2.2x", sha1Hash.bytes[i] );
        j+=2;
    }
    //printf( "\n" );
    //add one hour 
    u.QuadPart += NSECS * 10000000LLU;
    u.QuadPart += NSECS * 10000000LLU;
    memcpy(&ft, &u, sizeof(ft));
    FileTimeToSystemTime(&ft, &lt);
    sprintf(input,"Today:%04d-%02d-%02d_%02d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour);
    //printf("Today:%04d-%02d-%02d_%02d\n", lt.wYear, lt.wMonth, lt.wDay, lt.wHour);
    Sha1Initialise( &sha1Context );
    Sha1Update( &sha1Context, input, (uint32_t)strlen(input) );
    Sha1Finalise( &sha1Context, &sha1Hash );
    j=0;
    for( i=0; i<sizeof(sha1Hash); i++ )
    {
        //printf( "%2.2x", sha1Hash.bytes[i] );
        sprintf(&output3[j],"%2.2x", sha1Hash.bytes[i] );
        j+=2;
    }
    //printf( "\n" );
    //wash output
    //printf("output1:%s\n",output1);
    //printf("output2:%s\n",output2);
    //printf("output3:%s\n",output3);
    for (i=0;i<20;i++){
      if (output1[i]>='a'){
        output1[i]-=48;
      }
      if (output2[i]>='a'){
        output2[i]-=48;
      }
      if (output3[i]>='a'){
        output3[i]-=48;
      }
    }
    //printf("output1:%s\n",output1);
    //printf("output2:%s\n",output2);
    //printf("output3:%s\n",output3);
    return 3;
}
int checkOTP(char *input){
  char ipt1[41],ipt2[41],ipt3[41];
  memset(&ipt1[0],0,41);
  memset(&ipt2[0],0,41);
  memset(&ipt3[0],0,41);
  genOTP(&ipt1[0],&ipt2[0],&ipt3[0]);
  if ((memcmp(input,ipt1,strlen(input))==0) || (memcmp(input,ipt2,strlen(input))==0) || (memcmp(input,ipt3,strlen(input))==0)){
    return 1;
  }
  return 0;
}

