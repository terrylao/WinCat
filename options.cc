/*
  publish with BSD Licence.
	Copyright (c) Terry Lao
*/
#include "options.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>


typedef struct OPTION {
    int count = 0;
    char *arg = NULL;
} OPTION;

OPTION opts[58];
int argind = 1;

int optind = 1;
char *optarg = NULL;
int i = 0;
char *pos = NULL;
int moreopts = 0;
int verbose=0;

void free_args() {
    for (int i = 0; i < 58; i++) {
        if (opts[i].arg)
            free(opts[i].arg);
    }
}

void free_optarg() {
    free (optarg);
}

int opt_index(char opt) {
    return opt-65;
}

int pre_parse_opts(int argc, char **argv) {
    /* fill the opt array with number of times each opt is found */
    atexit(free_args);
    int total_unique = 0;
    for (int i = 1; i < argc; i++) {
        char *pos = argv[i];
        if (*pos == '-') {
            argind = i+1;
            while (*++pos) {
                int index = opt_index(*pos);
                if (index < 0) 
                    continue;
                if (!opts[index].count) 
                    total_unique++;
                opts[index].count++;
                if (i+1 >= argc) 
                    continue;
                if (*argv[i+1] == '-') 
                    continue;
                int size = sizeof(char) * (strlen(argv[i+1]));
                opts[index].arg = (char *) malloc(size + 1);
                memset(opts[index].arg, '\0', size + 1);
                memcpy(opts[index].arg, argv[i+1], size);
            }
        }
    }
    return total_unique;
}

char get_opt(int argc, char **argv) { 
    /* incremental arg parsing similar to get_opt.h */
    free(optarg);
    optarg = NULL;

    if (!moreopts) {
        i++;
        if (i >= argc) {
            atexit(free_optarg);
            return NULL;
        }
        pos = argv[i];
    }

    if (*pos == '-' || moreopts) {
        if (*++pos) {
            optind = i + 1;
            if (i+1 < argc) {
                if (*argv[i+1] != '-') {
                    int size = sizeof(char) * (strlen(argv[i+1]));
                    optarg = (char *) malloc(size + 1);
                    memset(optarg, '\0', size + 1);
                    memcpy(optarg, argv[i+1], size);
                }
            }
            if (*(pos+1)) moreopts = 1;
            return *pos;
        }
    }

    moreopts = 0;
    return get_opt(argc, argv);
}

int opt_count(char opt) {
    /* return the option count */
    int index = opt_index(opt);
    return index < 0 ? 0 : opts[index].count;
}

char *opt_arg(char opt) {
    /* return the option argument (can be NULL) */
    int index = opt_index(opt);
    return index < 0 ? NULL : opts[index].arg;
}

void printverbose(){
  time_t rawtime;
  struct tm * timeinfo;
  char buffer [80];

  time (&rawtime);
  timeinfo = localtime (&rawtime);

  strftime (buffer,80,"%F %T:",timeinfo);
  fprintf(stderr, buffer);
}
int readline(FILE *fptr,char *buf,int bufsize) {
	if(0 <= bufsize) { // not initialized yet
		return 0;
	}
	int pos = 0;
	char getcResult = getc(fptr);
	if(EOF==getcResult) {
		return 0;
	}
	while((EOF != getcResult) && ('\n' != getcResult)) {
		buf[pos++] = getcResult;
		getcResult = getc(fptr);
		if (bufsize==pos){
			break;
		}
	}
	buf[pos] = 0;
	return pos;
}