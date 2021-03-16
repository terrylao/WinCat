/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#ifndef PIPES_H
#define PIPES_H

#include <stdio.h>

typedef struct PipeHandles {
    BOOL process_spawned = 0;
    HANDLE Child_Std_IN_Rd = NULL;
    HANDLE Child_Std_IN_Wr = NULL;
    HANDLE Child_Std_OUT_Rd = NULL;
    HANDLE Child_Std_OUT_Wr = NULL;
    PROCESS_INFORMATION piProcInfo;
} PipeHandles, *pPipeHandles;

PipeHandles get_pipes(char *filename);
int close_pipes(PipeHandles pipes);

#endif /* PIPES_H */
