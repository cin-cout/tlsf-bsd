/*
 * Copyright (c) 2016 National Cheng Kung University, Taiwan.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style license.
 */

#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

#include "tlsf.h"

static tlsf_t t = TLSF_INIT;

static void usage(const char *name)
{
    printf(
        "run a malloc benchmark.\n"
        "usage: %s [-s blk-size|blk-min:blk-max] [-l loop-count] "
        "[-n num-blocks] [-c]\n",
        name);
    exit(-1);
}

/* Parse an integer argument. */
static size_t parse_int_arg(const char *arg, const char *exe_name)
{
    long ret = strtol(arg, NULL, 0);
    if (errno)
        usage(exe_name);

    return (size_t) ret;
}

/* Parse a size argument, which is either an integer or two integers separated
 * by a colon, denoting a range.
 */
static void parse_size_arg(const char *arg,
                           const char *exe_name,
                           size_t *blk_min,
                           size_t *blk_max)
{
    char *endptr;
    *blk_min = (size_t) strtol(arg, &endptr, 0);

    if (errno)
        usage(exe_name);

    if (endptr && *endptr == ':') {
        *blk_max = (size_t) strtol(endptr + 1, NULL, 0);
        if (errno)
            usage(exe_name);
    }

    if (*blk_min > *blk_max)
        usage(exe_name);
}

/* Get a random block size between blk_min and blk_max. */
static size_t get_random_block_size(size_t blk_min, size_t blk_max)
{
    if (blk_max > blk_min)
        return blk_min + ((size_t) rand() % (blk_max - blk_min));
    return blk_min;
}

static void run_alloc_benchmark(size_t loops,
                                size_t blk_min,
                                size_t blk_max,
                                void **blk_array,
                                size_t num_blks,
                                bool clear)
{
    while (loops--) {
        size_t next_idx = (size_t) rand() % num_blks;
        size_t blk_size = get_random_block_size(blk_min, blk_max);

        if (blk_array[next_idx]) {
            if (rand() % 10 == 0) {
                /* Insert the newly alloced block into the array at a random
                 * point.
                 */
                blk_array[next_idx] =
                    tlsf_realloc(&t, blk_array[next_idx], blk_size);
            } else {
                tlsf_free(&t, blk_array[next_idx]);
                /* Insert the newly alloced block into the array at a random
                 * point.
                 */
                blk_array[next_idx] = tlsf_malloc(&t, blk_size);
            }
        } else {
            /* Insert the newly alloced block into the array at a random point.
             */
            blk_array[next_idx] = tlsf_malloc(&t, blk_size);
        }
        if (clear)
            memset(blk_array[next_idx], 0, blk_size);
    }

    /* Free up all allocated blocks. */
    for (size_t i = 0; i < num_blks; i++) {
        if (blk_array[i])
            tlsf_free(&t, blk_array[i]);
    }
}

static size_t max_size;
static void *mem = 0;

void *tlsf_resize(tlsf_t *_t, size_t req_size)
{
    (void) _t;
    return req_size <= max_size ? mem : 0;
}

int main(int argc, char **argv)
{
    size_t blk_start = 0, blk_limit = 16384, num_blks = 10000;
    size_t loops = 10000000;
    bool clear = false;
    int opt;

    while ((opt = getopt(argc, argv, "s:l:r:t:n:b:ch")) > 0) {
        switch (opt) {
        case 's':
            parse_size_arg(optarg, argv[0], &blk_start, &blk_limit);
            break;
        case 'l':
            loops = parse_int_arg(optarg, argv[0]);
            break;
        case 'n':
            num_blks = parse_int_arg(optarg, argv[0]);
            break;
        case 'c':
            clear = true;
            break;
        case 'h':
            usage(argv[0]);
            break;
        default:
            usage(argv[0]);
            break;
        }
    }


    FILE *fptr_ra;
    fptr_ra = fopen("./txt/tlsf_bench_range_all.txt", "w");
    FILE *fptr_rb;
    fptr_rb = fopen("./txt/tlsf_bench_range_big.txt", "w");
    FILE *fptr_rs;
    fptr_rs = fopen("./txt/tlsf_bench_range_small.txt", "w");
    FILE *fptr_as;
    fptr_as = fopen("./txt/tlsf_bench_all_size.txt", "w");
/*
    FILE *fptr_ac;
    fptr_ac = fopen("./txt/tlsf_bench_all_size_self_debug.txt", "w");
*/

    max_size = blk_limit * num_blks * 2;
    mem = malloc(max_size);
    if(!mem){
        printf("error\n");
    }

    void **blk_array = (void **) calloc(num_blks, sizeof(void *));
    assert(blk_array);

    struct timespec start, end;

    int err;
    for(int i=0;i<50;i++){
        printf("now blk_min:%zu blk_max:%zu\n", blk_start, blk_limit);
        t = TLSF_INIT;
        err = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        assert(err == 0);
        run_alloc_benchmark(loops, blk_start, blk_limit, blk_array, num_blks, clear);
        err = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
        assert(err == 0);
        double elapsed = (double) (end.tv_sec - start.tv_sec) +
                 (double) (end.tv_nsec - start.tv_nsec) * 1e-9;
        printf("%zu  %zu %.6f %.3f\n", blk_start, blk_limit, elapsed, elapsed / (double) loops * 1e6);
        fprintf(fptr_ra, "%zu  %zu %.6f %.3f\n", blk_start, blk_limit, elapsed, elapsed / (double) loops * 1e6);
        memset(blk_array, 0, sizeof(*blk_array) * num_blks);
        memset(mem, 0, max_size);
    }

    for(int i=0;i<50;i++){
        printf("now blk_min:%zu blk_max:%zu\n", blk_limit/2, blk_limit);
        t = TLSF_INIT;
        err = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        assert(err == 0);
        run_alloc_benchmark(loops, blk_limit/2, blk_limit, blk_array, num_blks, clear);
        err = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
        assert(err == 0);
        double elapsed = (double) (end.tv_sec - start.tv_sec) +
                 (double) (end.tv_nsec - start.tv_nsec) * 1e-9;
        printf("%zu  %zu %.6f %.3f\n", blk_limit/2, blk_limit, elapsed, elapsed / (double) loops * 1e6);
        fprintf(fptr_rb, "%zu  %zu %.6f %.3f\n", blk_limit/2, blk_limit, elapsed, elapsed / (double) loops * 1e6);
        memset(blk_array, 0, sizeof(*blk_array) * num_blks);
        memset(mem, 0, max_size);
    }

    for(int i=0;i<50;i++){
        printf("now blk_min:%zu blk_max:%zu\n", blk_start, blk_limit/2);
        t = TLSF_INIT;
        err = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        assert(err == 0);
        run_alloc_benchmark(loops, blk_start, blk_limit/2, blk_array, num_blks, clear);
        err = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
        assert(err == 0);
        double elapsed = (double) (end.tv_sec - start.tv_sec) +
                 (double) (end.tv_nsec - start.tv_nsec) * 1e-9;
        printf("%zu  %zu %.6f %.3f\n", blk_start, blk_limit/2, elapsed, elapsed / (double) loops * 1e6);
        fprintf(fptr_rs, "%zu  %zu %.6f %.3f\n", blk_start, blk_limit/2, elapsed, elapsed / (double) loops * 1e6);
        memset(blk_array, 0, sizeof(*blk_array) * num_blks);
        memset(mem, 0, max_size);
    }

    while(blk_start<blk_limit){
        printf("now blk_min:%zu blk_max:%zu\n", blk_start, blk_start+64);
        t = TLSF_INIT;
        err = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        assert(err == 0);
        run_alloc_benchmark(loops, blk_start, blk_start+64, blk_array, num_blks, clear);
        err = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
        assert(err == 0);
        double elapsed = (double) (end.tv_sec - start.tv_sec) +
                 (double) (end.tv_nsec - start.tv_nsec) * 1e-9;
        printf("%zu  %zu %.6f %.3f\n", blk_start, blk_start+64, elapsed, elapsed / (double) loops * 1e6);
        fprintf(fptr_as, "%zu  %zu %.6f %.3f\n", blk_start, blk_start+64, elapsed, elapsed / (double) loops * 1e6);
        memset(blk_array, 0, sizeof(*blk_array) * num_blks);
        memset(mem, 0, max_size);
        blk_start+=64;
    }

/* use it after u comment the arena grow/shrink code in free and find free
    while(blk_start<blk_limit){
        printf("now blk_min:%zu blk_max:%zu\n", blk_start, blk_start+64);
        err = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        assert(err == 0);
        tlsf_init(&t, mem, max_size);
        run_alloc_benchmark(loops, blk_start, blk_start+64, blk_array, num_blks, clear);
        err = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
        assert(err == 0);
        double elapsed = (double) (end.tv_sec - start.tv_sec) +
                 (double) (end.tv_nsec - start.tv_nsec) * 1e-9;
        printf("%zu  %zu %.6f %.3f\n", blk_start, blk_start+64, elapsed, elapsed / (double) loops * 1e6);
        fprintf(fptr_ac, "%zu  %zu %.6f %.3f\n", blk_start, blk_start+64, elapsed, elapsed / (double) loops * 1e6);
        memset(blk_array, 0, sizeof(*blk_array) * num_blks);
        memset(mem, 0, max_size);
        blk_start+=64;
    }
*/

    fclose(fptr_rb);
    fclose(fptr_ra);
    fclose(fptr_rs);
    fclose(fptr_as);
    //fclose(fptr_ac);

    free(blk_array);

    return 0;
}
