#pragma once

// IWYU pragma: begin_exports
#include <assert.h>
// #include <ctype.h>
#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <json-c/json.h>
#include <limits.h>
// #include <math.h>
#include <mlib/m-bstring.h>
#include <mlib/m-buffer.h>
#include <mlib/m-string.h>
#include <mlib/m-thread.h>
// #include <stdarg.h>
// #include <stdbool.h>
// #include <stddef.h>
// #include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
// #include <string.h>
// #include <time.h>
#include <unistd.h>
// IWYU pragma: end_exports

static_assert(
    sizeof(float) * CHAR_BIT == 32,
    "float is not 32 bit on this architecture, fix the f32 typedef.");
typedef float flt32_t;

static_assert(
    sizeof(double) * CHAR_BIT == 64,
    "float is not 32 bit on this architecture, fix the f32 typedef.");
typedef double flt64_t;

static_assert(
    sizeof(long double) * CHAR_BIT == 128,
    "float is not 32 bit on this architecture, fix the f32 typedef.");
typedef long double flt128_t;

#define CLOCKS_TO_USEC(clocks) (clocks * 1'000'000 / CLOCKS_PER_SEC)

#ifndef MAX
#define MAX(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })
#endif

#ifndef MIN
#define MIN(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })
#endif

#ifndef ABS
#define ABS(a) ({ (a) < 0 ? -(a) : (a); })
#endif

#ifndef ROUND_UP_TO
#define ROUND_UP_TO(a, b)       \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a / _b + !!(_a % _b);  \
    })
#endif

#ifndef CLAMP
#define CLAMP(x, upper, lower) (MIN(upper, MAX(x, lower)))
#endif

#ifndef COUNT_OF
#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef UNUSED
#define UNUSED(X) (void)(X)
#endif

#define STR(x)  #x
#define XSTR(x) STR(x)

typedef struct {
    m_mutex_t mutex;
    m_cond_t cond;
} m_eflag_s;

typedef m_eflag_s m_eflag_t[1];

static inline void m_eflag_init(m_eflag_t eflag) {
    m_mutex_init(eflag->mutex);
    m_cond_init(eflag->cond);
}

static inline void m_eflag_wait(m_eflag_t eflag) {
    m_cond_wait(eflag->cond, eflag->mutex);
}

static inline void m_eflag_broadcast(m_eflag_t eflag) {
    m_cond_broadcast(eflag->cond);
}

static inline void m_eflag_clear(m_eflag_t eflag) {
    m_mutex_clear(eflag->mutex);
    m_cond_clear(eflag->cond);
}

static inline void custom_perror(const char* s, const char* e) {
    fprintf(stderr, "%s: %s\n", s, e);
}
