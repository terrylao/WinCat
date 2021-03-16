/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#include <stdio.h>
#ifndef OPTIONS_H
#define OPTIONS_H

/* 1st method: Parse once then call 
   opt_count or opt_arg freely */
int pre_parse_opts(int argc, char **argv);
int opt_count(char opt);
char *opt_arg(char opt);
extern int argind;

/* 2nd method: Incremental arg parsing 
   similar to get_opt */
char get_opt(int argc, char **argv);
void printverbose();
int readline(FILE *fptr,char *buf,int bufsize);

extern char *optarg;
extern int optind;
extern int verbose;
extern int readline(FILE *fptr,char *buf,int bufsize);;
#endif /* OPTIONS_H */
