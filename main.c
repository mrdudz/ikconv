
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "karaoke.h"

unsigned int verbose = 0;

#include "lrc.c"
#include "ultrastar.c"
#include "ikmod.c"
#include "sid.c"

void usage(void)
{
    printf(
        "--verbose\n"
#ifdef TESTING
        "--readlrc <file>\n"
        "--writelrc <file>\n"
#endif
        "--readusc <file>\n"
        "--writeusc <file>\n"
        "--readikmod <file>\n"
        "--writeikmod <file>\n"
        "--writeikasm <file>\n"
        "--readsid <file>\n"
        "--writesid <file>\n"
        "--start-with-countdown     use countdown at song start (default: no)\n"
        "--volume-addr <addr>       addr in player to patch for volume fade (default: use $d418)\n"
        "--fade-speed <speed>       speed of fade out when sid ends (default: $02)\n"
        "--fade-delay <delay>       number of frames to play music when lyrics are done (default: $0202)\n"
    );
}

int main(int argc, char *argv[] )
{
    int i;
    if (argc < 2) {
        usage();
        exit(-1);
    }
    for(i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--verbose")) {
            verbose++;
        } else if (!strcmp(argv[i], "--start-with-countdown")) {
            start_counter = 1;
        } else if (!strcmp(argv[i], "--volume-addr")) {
            i++;
            volume_addr = strtoul(argv[i], NULL, 0);
        } else if (!strcmp(argv[i], "--fade-speed")) {
            i++;
            fade_speed = strtoul(argv[i], NULL, 0);
        } else if (!strcmp(argv[i], "--fade-delay")) {
            i++;
            fade_delay = strtoul(argv[i], NULL, 0);
#ifdef TESTING
        } else if (!strcmp(argv[i], "--readlrc")) {
            i++;
            if(readlrc(argv[i]) != 0) {
                fprintf(stderr, "error: could not read file '%s'\n", argv[i]);
                exit(-1);
            }
        } else if (!strcmp(argv[i], "--writelrc")) {
            i++;
            if(writelrc(argv[i]) != 0) {
                fprintf(stderr, "error: could not write file '%s'\n", argv[i]);
                exit(-1);
            }
#endif
        } else if (!strcmp(argv[i], "--readusc")) {
            i++;
            if(readusc(argv[i]) != 0) {
                fprintf(stderr, "error: could not read file '%s'\n", argv[i]);
                exit(-1);
            }
        } else if (!strcmp(argv[i], "--writeusc")) {
            i++;
            if(writeusc(argv[i]) != 0) {
                fprintf(stderr, "error: could not write file '%s'\n", argv[i]);
                exit(-1);
            }
        } else if (!strcmp(argv[i], "--readikmod")) {
            i++;
            if(readikmod(argv[i]) != 0) {
                fprintf(stderr, "error: could not read file '%s'\n", argv[i]);
                exit(-1);
            }
        } else if (!strcmp(argv[i], "--writeikmod")) {
            i++;
            if(writeikmod(argv[i]) != 0) {
                fprintf(stderr, "error: could not write file '%s'\n", argv[i]);
                exit(-1);
            }
        } else if (!strcmp(argv[i], "--writeikasm")) {
            i++;
            if(writeikasm(argv[i]) != 0) {
                fprintf(stderr, "error: could not write file '%s'\n", argv[i]);
                exit(-1);
            }
        } else if (!strcmp(argv[i], "--readsid")) {
            i++;
            if(readsid(argv[i]) != 0) {
                fprintf(stderr, "error: could not read file '%s'\n", argv[i]);
                exit(-1);
            }
        } else if (!strcmp(argv[i], "--writesid")) {
            i++;
            if(writesid(argv[i]) != 0) {
                fprintf(stderr, "error: could not write file '%s'\n", argv[i]);
                exit(-1);
            }
        } else {
            usage();
            exit(-1);
        }
    }
    return 0;
}
