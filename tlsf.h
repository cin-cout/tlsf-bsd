/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

/* Inhibit C++ name-mangling for tlsf functions */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <stdint.h>

#define _TLSF_SL_COUNT 16
#if __SIZE_WIDTH__ == 64
#define _TLSF_FL_COUNT 32
#define _TLSF_FL_MAX 38
#else
#define _TLSF_FL_COUNT 25
#define _TLSF_FL_MAX 30
#endif
#define TLSF_MAX_SIZE (((size_t) 1 << (_TLSF_FL_MAX - 1)) - sizeof(size_t))
#define TLSF_INIT ((tlsf_t){.size = 0})

typedef struct {
    uint32_t fl, sl[_TLSF_FL_COUNT];
    struct tlsf_block *block[_TLSF_FL_COUNT][_TLSF_SL_COUNT];
    size_t size;
} tlsf_t;

void *tlsf_resize(tlsf_t *, size_t);
void *tlsf_aalloc(tlsf_t *, size_t, size_t);

bool tlsf_add_pool(tlsf_t *t, void* mem, size_t mem_size);
void tlsf_init(tlsf_t *t, void* mem, size_t mem_size);

/**
 * Allocates the requested @size bytes of memory and returns a pointer to it.
 * On failure, returns NULL.
 */
void *tlsf_malloc(tlsf_t *, size_t size);
void *tlsf_realloc(tlsf_t *, void *, size_t);

/**
 * Releases the previously allocated memory, given the pointer.
 */
void tlsf_free(tlsf_t *, void *);

#ifdef TLSF_ENABLE_CHECK
void tlsf_check(tlsf_t *);
#else
static inline void tlsf_check(tlsf_t *t)
{
    (void) t;
}
#endif

#ifdef __cplusplus
}
#endif
