#include <stdio.h>

#include <cfac/log.h>
#include <cfac/stat.h>

#include "common.h"
#include "lib.h"

int main(void) {
  Race races_arr_part1[] = {
      {.time = 59, .distance = 597},
      {.time = 79, .distance = 1234},
      {.time = 65, .distance = 1032},
      {.time = 75, .distance = 1328},
  };

  Race races_arr_part2[] = {
      {.time = 59796575ll, .distance = 597123410321328ll},
  };

  SPN_Span races_part1 = {.begin        = races_arr_part1,
                          .element_size = sizeof(races_arr_part1[0]),
                          .len          = sizeof(races_arr_part1) / sizeof(races_arr_part1[0])};

  SPN_Span races_part2 = {.begin        = races_arr_part2,
                          .element_size = sizeof(races_arr_part2[0]),
                          .len          = sizeof(races_arr_part2) / sizeof(races_arr_part2[0])};

  size_t product_part1 = 0;
  TRY(get_record_beating_input_product(races_part1, &product_part1));

  size_t product_part2 = 0;
  TRY(get_record_beating_input_product(races_part2, &product_part2));

  return LOG_STAT(OK, "product_part1 = %zu, product_part2 = %zu", product_part1, product_part2);
}