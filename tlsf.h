#pragma once

/* Inhibit C++ name-mangling for tlsf functions */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <stdint.h>

#define _TLSF_SL_COUNT 16
#define _TLSF_FL_COUNT (sizeof(size_t) == 8 ? 32 : 25)
#define _TLSF_FL_MAX (sizeof(size_t) == 8 ? 38 : 30)
#define TLSF_MAX_SIZE (((size_t) 1 << (_TLSF_FL_MAX - 1)) - sizeof(size_t))
#define TLSF_INIT ((tlsf_t){.size = 0})

typedef struct {
    uint32_t fl, sl[_TLSF_FL_COUNT];
    struct tlsf_block *block[_TLSF_FL_COUNT][_TLSF_SL_COUNT];
    size_t size;
} tlsf_t;

void *tlsf_resize(tlsf_t *, size_t);
void *tlsf_aalloc(tlsf_t *, size_t, size_t);
void *tlsf_malloc(tlsf_t *, size_t);
void *tlsf_realloc(tlsf_t *, void *, size_t);
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
