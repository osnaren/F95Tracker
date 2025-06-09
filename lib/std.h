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
#include <mlib/m-array.h>
#include <mlib/m-bitset.h>
#include <mlib/m-bstring.h>
#include <mlib/m-buffer.h>
#include <mlib/m-dict.h>
#include <mlib/m-list.h>
#include <mlib/m-string.h>
#include <mlib/m-thread.h>
#include <mlib/m-tuple.h>
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

#define OS_WINDOWS 1
#define OS_LINUX   2
#define OS_MACOS   3
#if defined(_WIN32)
#define OS OS_WINDOWS
#elif defined(linux) || defined(__linux) || defined(__linux__)
#define OS OS_LINUX
#elif defined(__APPLE__)
#define OS OS_MACOS
#else
#error "Platform not supported!"
#endif

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

#define M_ARRAY_EX_DEF(name, name_t, ...)                  \
    M_ARRAY_DEF_AS(name, name_t, name_t##_it, __VA_ARGS__) \
    typedef name##_ptr name_t##_ptr;
#define M_ARRAY_EX_OPL(name, type_t) M_ARRAY_OPLIST(name, M_GLOBAL_OPLIST_OR_DEF(type_t)())

#define M_BUFFER_EX_DEF(name, name_t, ...)     \
    M_BUFFER_DEF_AS(name, name_t, __VA_ARGS__) \
    typedef name##_ptr name_t##_ptr;
#define M_BUFFER_EX_OPL(name, type_t) M_BUFFER_OPLIST(name, M_GLOBAL_OPLIST_OR_DEF(type_t)())

#define M_DICT_OA_EX_DEF(name, name_t, ...)                                   \
    M_DICT_OA_DEF2_AS(name, name_t, name_t##_it, name_t##_itref, __VA_ARGS__) \
    typedef name##_ptr name_t##_ptr;                                          \
    typedef name##_pair_ct name_t##_pair;
#define M_DICT_OA_EX_OPL(name, key_t, value_t) \
    M_DICT_OPLIST(name, M_GLOBAL_OPLIST_OR_DEF(key_t)(), M_GLOBAL_OPLIST_OR_DEF(value_t)())

#define M_LIST_DUAL_PUSH_EX_DEF(name, name_t, ...)                  \
    M_LIST_DUAL_PUSH_DEF_AS(name, name_t, name_t##_it, __VA_ARGS__) \
    typedef name##_ptr name_t##_ptr;
#define M_LIST_DUAL_PUSH_EX_OPL(name, type_t) M_LIST_OPLIST(name, M_GLOBAL_OPLIST_OR_DEF(type_t)())

#define M_TUPLE_EX_DEF(name, name_t, ...)      \
    M_TUPLE_DEF2_AS(name, name_t, __VA_ARGS__) \
    typedef name##_ptr name_t##_ptr;
#define M_GLOBAL_OPLIST_OR_DEF_CALL(type_t) M_GLOBAL_OPLIST_OR_DEF(type_t)()
#define M_TUPLE_EX_OPL(name, ...) \
    M_TUPLE_OPLIST(name, M_MAP_C(M_GLOBAL_OPLIST_OR_DEF_CALL, __VA_ARGS__))

// Based on M_EACH() but also dereferences item pointer for convenience;
// container and container_t are swapped to resemble variable declaration.
#define each(item_t, item, container_t, container) \
    eachi_oplist(item_t, item, container, M_GLOBAL_OPLIST(container_t))

// clang-format off
/* Internal for each */
#define eachi_oplist(item_t, item, container, oplist)                         \
  M_IF_METHOD(IT_REF, oplist)(eachi, eachi_const)                             \
  (item_t, item, container, oplist, M_C(local_iterator_, __LINE__),           \
   M_C(local_cont_, __LINE__), M_C(local_ptr_, __LINE__))

/* Internal for each with M_GET_IT_REF operator */
#define eachi(item_t,item,container,oplist, iterator, cont, ptr)              \
  (bool cont = true; cont; cont = false)                                      \
  for(M_GET_SUBTYPE oplist *ptr; cont ; cont = false)                         \
    for(item_t item; cont ; cont = false)                                     \
      for(M_GET_IT_TYPE oplist iterator; cont ; cont = false)                 \
        for(M_GET_IT_FIRST oplist (iterator, container) ;                     \
            !M_GET_IT_END_P oplist (iterator)                                 \
              && (ptr = M_GET_IT_REF oplist (iterator), item = *ptr, true) ;  \
            M_GET_IT_NEXT oplist (iterator))

/* Internal for each with M_GET_IT_CREF operator */
#define eachi_const(item_t,item,container,oplist, iterator, cont, ptr)        \
  (bool cont = true; cont; cont = false)                                      \
  for(const M_GET_SUBTYPE oplist *ptr; cont ; cont = false)                   \
    for(item_t item; cont ; cont = false)                                     \
      for(M_GET_IT_TYPE oplist iterator; cont ; cont = false)                 \
        for(M_GET_IT_FIRST oplist (iterator, container) ;                     \
            !M_GET_IT_END_P oplist (iterator)                                 \
              && (ptr = M_GET_IT_CREF oplist (iterator), item = *ptr, true) ; \
            M_GET_IT_NEXT oplist (iterator))
// clang-format on

M_LIST_DUAL_PUSH_DEF_AS(m_string_list, MStringList, MStringListIt, m_string_t)
#define M_OPL_MStringList() M_LIST_OPLIST(m_string_list)

typedef struct m_eflag_s {
    m_mutex_t mutex;
    m_cond_t cond;
} m_eflag_t[1];
typedef struct m_eflag_s* m_eflag_ptr;

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
