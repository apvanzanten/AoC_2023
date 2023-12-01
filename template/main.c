#include <stdio.h>

#include <cfac/stat.h>
#include <cfac/log.h>

#include "lib.h"

int main(void) {
  return LOG_STAT(do_a_thing(), "so amaze");
}