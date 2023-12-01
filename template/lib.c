#include <cfac/log.h>

#include "lib.h"

STAT_Val do_a_thing(void) {
  return LOG_STAT(STAT_OK, "wow amaze!");
}