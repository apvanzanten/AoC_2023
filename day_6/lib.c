#include <cfac/log.h>
#include <cfac/span.h>

#include "lib.h"
#include "common.h"

static bool is_winning_input_for_race(Race race, size_t input) {
  if(input > race.time) return false;
  
  const size_t move_time = race.time - input;
  const size_t distance = input * move_time;

  return (distance > race.distance);
}

STAT_Val get_record_beating_input_product(SPN_Span races, size_t * out) {
  CHECK(!SPN_is_empty(races));
  CHECK(races.element_size == sizeof(Race));
  CHECK(out != NULL);

  *out = 1;

  for(const Race * race = SPN_first(races); race != SPN_end(races); race++) {
    // find smallest winning input
    size_t smallest_winning_input = SIZE_MAX;
    for(size_t input = 0; input <= race->time; input++) {
      if(is_winning_input_for_race(*race, input)) {
        smallest_winning_input = input;
        break;
      }
    }

    // find largest winning input
    size_t largest_winning_input = 0;
    for(size_t input = race->time; input > 0; input--) {
      if(is_winning_input_for_race(*race, input)) {
        largest_winning_input = input;
        break;
      }
    }

    CHECK(largest_winning_input >= smallest_winning_input);

    const size_t num_winning_inputs = (largest_winning_input - smallest_winning_input) + 1;

    *out *= num_winning_inputs;
  }

  return OK;
}