#ifndef lib_h
#define lib_h

#include <cfac/stat.h>

#include <cfac/span.h>

typedef enum Color {
  RED = 0, GREEN, BLUE, 
  NUM_COLORS,
  COLOR_FIRST = 0,
  COLOR_END = NUM_COLORS,
} Color;

typedef struct GameResult {
  int game_id;
  int min_color_occurrences[NUM_COLORS];
} GameResult;

STAT_Val get_game_result_from_line(const SPN_Span line, GameResult * out);

#endif
