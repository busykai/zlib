/* deflate_benchmark.c -- simple benchmark for measuring deflate performance.
 * Copyright (C) 1995-2006, 2010, 2011, 2016 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "zlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <time.h>

#define BUFLEN      16384
#define MAX_NAME_LEN 1024


static char *prog;
typedef struct timespec ts_t;

/* ===========================================================================
 * Display error message and exit
 */
void error(msg)
    const char *msg;
{
    fprintf(stderr, "%s: %s\n", prog, msg);
    exit(1);
}

int64_t elapsed(ts_t *start, ts_t *end)
{
    int64_t result;

    if (end->tv_nsec < start->tv_nsec) {
        int sec = (start->tv_nsec - end->tv_nsec) / 1000000000 + 1;
        start->tv_nsec -= sec * 1000000000;
        start->tv_sec += sec;
    }
    if (start->tv_nsec - end->tv_nsec > 1000000000) {
        int sec = (end->tv_nsec - start->tv_nsec) / 1000000000;
        start->tv_nsec += sec * 1000000000;
        start->tv_sec -= sec;
    }
    result = (end->tv_sec - start->tv_sec) * 1000000000;
    result += (end->tv_nsec - start->tv_nsec);
    return result;
}

int main(argc, argv)
    int argc;
    char *argv[];
{
    FILE  *in;
    gzFile out;
    char buf[BUFLEN];
    char file[MAX_NAME_LEN];
    int len;
    int err;
    int i;
    int times = 100;
    struct timespec start, end;

    prog = basename(argv[0]);

    if (argc != 2) {
        error("Must specify exactly one file to compress");
        exit(1);
    }

    len = strlen(argv[1]);
    if (len > MAX_NAME_LEN) {
        fprintf(stderr, "Filename too long: %s\n", argv[1]);
        exit(1);
    }
    strncpy(file, argv[1], len);
    file[len] = '\0';

    in = fopen(file, "rb");
    if (in == NULL) {
        perror(file);
        exit(1);
    }
    out = gzopen("/dev/null", "wb");
    if (out == NULL) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, "/dev/null");
        exit(1);
    }

    if (clock_gettime(CLOCK_MONOTONIC, &start))
        perror("clock_gettime");

    for (i = 0; i < times; i++) {
        for (;;) {
            len = (int)fread(buf, 1, sizeof(buf), in);
            if (ferror(in)) {
                perror("fread");
                exit(1);
            }
            if (len == 0) break;

            if (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
        }
        fseek(in, 0, SEEK_SET);
    }

    if (clock_gettime(CLOCK_MONOTONIC, &end))
        perror("clock_gettime");

    printf("Elapsed time: %.2f sec\n", elapsed(&start, &end) / 1000000000.0);

    return 0;
}
