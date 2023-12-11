#ifndef COMMON_H
#define COMMON_H

#include <cfac/log.h>
#include <cfac/stat.h>

#include <stdbool.h>

#define OK STAT_OK

#define CHECK(x)                                                                                                       \
  do {                                                                                                                 \
    if(!(x)) return LOG_STAT(STAT_ERR_ASSERTION, "failed assertion: %s", #x);                                          \
  } while(false)

#define CHECK_WITH_MSG(x, ...)                                                                                         \
  do {                                                                                                                 \
    if(!(x)) return LOG_STAT(STAT_ERR_ASSERTION, __VA_ARGS__);                                                         \
  } while(false)

#define TRY(x)                                                                                                         \
  do {                                                                                                                 \
    STAT_Val st = (x);                                                                                                 \
    if(!STAT_is_OK(st)) return LOG_STAT(st, "failed: %s", #x);                                                         \
  } while(false)

static inline size_t min_sz(size_t a, size_t b) { return (a < b) ? a : b; }
static inline size_t max_sz(size_t a, size_t b) { return (a > b) ? a : b; }

#endif