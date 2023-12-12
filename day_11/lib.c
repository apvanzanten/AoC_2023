#include <cfac/darray.h>
#include <cfac/log.h>

#include "common.h"
#include "lib.h"

#include <stdio.h>

static STAT_Val init_universe(Universe * universe) {
  CHECK(universe != NULL);

  *universe = (Universe){0};
  TRY(DAR_create(&universe->spaces, sizeof(Space)));

  return OK;
}

STAT_Val parse_universe(const DAR_DArray * lines, Universe * universe) {
  CHECK(lines != NULL);
  CHECK(DAR_is_initialized(lines));
  CHECK(!DAR_is_empty(lines));
  CHECK(universe != NULL);

  TRY(init_universe(universe));

  Position pos = {0};
  for(const DAR_DArray * line = DAR_first(lines); line != DAR_end(lines); line++, pos.y++) {
    pos.x        = 0;
    size_t width = 0;

    for(const char * p = DAR_first(line); p != DAR_end(line); p++, pos.x++) {
      if(*p == '\n') break;
      Space space = {.type = char_to_space_type(*p)};
      TRY(DAR_push_back(&universe->spaces, &space));
      width++;
    }

    if(universe->width != 0 && universe->width != width) return LOG_STAT(STAT_ERR_ARGS, "malformed universe input");
    if(universe->width == 0) universe->width = width;

    universe->height++;
  }

  return OK;
}

static STAT_Val get_empty_rows(const Universe * universe, DAR_DArray * empty_rows) {
  CHECK(universe != NULL);
  CHECK(empty_rows != NULL);
  CHECK(DAR_is_initialized(empty_rows));
  CHECK(DAR_is_empty(empty_rows));
  CHECK(empty_rows->element_size == sizeof(size_t));

  Position pos = {0};
  for(pos.y = 0; pos.y < universe->height; pos.y++) {
    bool is_row_empty = true;
    for(pos.x = 0; pos.x < universe->width; pos.x++) {
      if(get_space(universe, pos)->type != EMPTY) {
        is_row_empty = false;
        break;
      }
    }
    if(is_row_empty) TRY(DAR_push_back(empty_rows, &pos.y));
  }

  return OK;
}
static STAT_Val get_empty_columns(const Universe * universe, DAR_DArray * empty_columns) {
  CHECK(universe != NULL);
  CHECK(empty_columns != NULL);
  CHECK(DAR_is_initialized(empty_columns));
  CHECK(DAR_is_empty(empty_columns));
  CHECK(empty_columns->element_size == sizeof(size_t));

  Position pos = {0};
  for(pos.x = 0; pos.x < universe->width; pos.x++) {
    bool is_column_empty = true;
    for(pos.y = 0; pos.y < universe->height; pos.y++) {
      if(get_space(universe, pos)->type != EMPTY) {
        is_column_empty = false;
        break;
      }
    }
    if(is_column_empty) TRY(DAR_push_back(empty_columns, &pos.x));
  }

  return OK;
}

STAT_Val expand_universe(Universe * universe) {
  CHECK(universe != NULL);

  DAR_DArray empty_rows    = {0};
  DAR_DArray empty_columns = {0};
  TRY(DAR_create(&empty_rows, sizeof(size_t)));
  TRY(DAR_create(&empty_columns, sizeof(size_t)));
  TRY(get_empty_rows(universe, &empty_rows));
  TRY(get_empty_columns(universe, &empty_columns));

  DAR_DArray new_spaces = {0};
  TRY(DAR_create(&new_spaces, sizeof(Space)));
  TRY(DAR_reserve(&new_spaces, universe->spaces.size)); // not quite enough, but a good start

  Position pos           = {0};
  size_t   empty_row_idx = 0;
  for(pos.y = 0; pos.y < universe->height; pos.y++) {
    const bool is_empty_row =
        ((empty_row_idx < empty_rows.size) && (*(size_t *)DAR_get(&empty_rows, empty_row_idx) == pos.y));
    if(is_empty_row) empty_row_idx++;

    size_t num_row_occurrences = (is_empty_row ? 2 : 1);
    for(size_t row_occurrence = 0; row_occurrence < num_row_occurrences; row_occurrence++) {
      size_t empty_column_idx = 0;

      for(pos.x = 0; pos.x < universe->width; pos.x++) {
        const bool is_empty_column = ((empty_column_idx < empty_columns.size) &&
                                      (*(size_t *)DAR_get(&empty_columns, empty_column_idx) == pos.x));

        TRY(DAR_push_back(&new_spaces, get_space(universe, pos)));
        if(is_empty_column) {
          TRY(DAR_push_back(&new_spaces, get_space(universe, pos)));
          empty_column_idx++;
        }
      }
    }
  }

  TRY(DAR_destroy(&universe->spaces));
  universe->spaces = new_spaces;
  universe->height += empty_rows.size;
  universe->width += empty_columns.size;

  TRY(DAR_destroy(&empty_rows));
  TRY(DAR_destroy(&empty_columns));

  return OK;
}

STAT_Val destroy_universe(Universe * universe) {
  CHECK(universe != NULL);
  CHECK(DAR_is_initialized(&universe->spaces));

  TRY(DAR_destroy(&universe->spaces));
  *universe = (Universe){0};

  return OK;
}

void print_universe(const Universe * universe) {
  if(universe != NULL && DAR_is_initialized(&universe->spaces)) {
    Position pos = {0};
    for(pos.y = 0; pos.y < universe->height; pos.y++) {
      for(pos.x = 0; pos.x < universe->width; pos.x++) {
        putc(space_type_to_char(get_space(universe, pos)->type), stdout);
      }
      putc('\n', stdout);
    }
  }
}

static STAT_Val get_galaxies(const Universe * universe, DAR_DArray * positions) {

  Position pos = {0};
  for(pos.y = 0; pos.y < universe->height; pos.y++) {
    for(pos.x = 0; pos.x < universe->width; pos.x++) {
      if(get_space(universe, pos)->type == GALAXY) TRY(DAR_push_back(positions, &pos));
    }
  }

  return OK;
}

static void print_distance_map(const Universe * universe, Position galaxy_pos, const DAR_DArray * distance_map) {
  Position pos = {0};
  for(pos.y = 0; pos.y < universe->height; pos.y++) {
    for(pos.x = 0; pos.x < universe->width; pos.x++) {
      if(is_positions_equal(pos, galaxy_pos)) {
        printf("XX ");
      } else {
        const size_t * dist = DAR_get(distance_map, pos_to_idx(universe, pos));
        if(*dist > 0xff) {
          printf("__ ");
        } else {
          printf("%2.2zx ", *dist);
        }
      }
    }
    printf("\n");
  }
}

static void update_distance(const Universe * universe,
                            Position         orig_pos,
                            Position         pos,
                            DAR_DArray *     distance_map,
                            size_t           distance) {
  if(pos.x < universe->width && pos.y < universe->height) {
    size_t * dist_in_map = DAR_get(distance_map, pos_to_idx(universe, pos));
    if(distance < *dist_in_map) {
      LOG_STAT(STAT_OK,
               "from {%zu,%zu}, updating {%zu,%zu}: %zu -> %zu",
               orig_pos.x,
               orig_pos.y,
               pos.x,
               pos.y,
               *dist_in_map,
               distance);
      *dist_in_map = distance;
    }
  }
}

static STAT_Val grow_distance_rec(const Universe * universe,
                                  Position         pos,
                                  DAR_DArray *     distance_map,
                                  size_t           curr_distance) {
  if(pos.x >= universe->width || pos.y >= universe->height) return OK;

  size_t * dist_in_map = DAR_get(distance_map, pos_to_idx(universe, pos));
  if(*dist_in_map < curr_distance) return OK;

  const Position adjacents[] = {
      {.x = pos.x - 1, .y = pos.y},
      {.x = pos.x, .y = pos.y - 1},
      {.x = pos.x, .y = pos.y + 1},
      {.x = pos.x + 1, .y = pos.y},
  };

  // LOG_STAT(STAT_OK, "updating positions around {%zu,%zu}", pos.x, pos.y);
  for(size_t i = 0; i < sizeof(adjacents) / sizeof(adjacents[0]); i++) {
    update_distance(universe, pos, adjacents[i], distance_map, curr_distance + 1);
  }

  for(size_t i = 0; i < sizeof(adjacents) / sizeof(adjacents[0]); i++) {
    TRY(grow_distance_rec(universe, adjacents[i], distance_map, curr_distance + 1));
  }

  if(curr_distance < 4) print_distance_map(universe, pos, distance_map);

  return OK;
}

static STAT_Val make_distance_map(const Universe * universe, Position galaxy_pos, DAR_DArray * distance_map) {
  CHECK(galaxy_pos.x < universe->width);
  CHECK(galaxy_pos.y < universe->height);
  CHECK(distance_map != NULL);
  CHECK(DAR_is_initialized(distance_map));
  CHECK(DAR_is_empty(distance_map));
  CHECK(distance_map->element_size == sizeof(size_t));

  const size_t max = SIZE_MAX;
  TRY(DAR_resize_with_value(distance_map, universe->spaces.size, &max));

  TRY(grow_distance_rec(universe, galaxy_pos, distance_map, 0));

  return OK;
}

STAT_Val calculate_galaxy_distances(
    const Universe * universe,
    DAR_DArray *     galaxy_positions /* contains Positions */,
    DAR_DArray *     distances /* contains distances ordered [1-1,1-2,...,1-n,2-1,2-2,...,2-n,...]*/) {
  CHECK(universe != NULL);
  CHECK(DAR_is_initialized(&universe->spaces));
  CHECK(galaxy_positions != NULL);
  CHECK(DAR_is_initialized(galaxy_positions));
  CHECK(galaxy_positions->element_size == sizeof(Position));
  CHECK(distances != NULL);
  CHECK(DAR_is_initialized(distances));
  CHECK(distances->element_size == sizeof(size_t));

  TRY(get_galaxies(universe, galaxy_positions));
  const size_t num_galaxies = galaxy_positions->size;

  DAR_DArray distance_maps = {0};
  TRY(DAR_create(&distance_maps, sizeof(DAR_DArray)));
  TRY(DAR_reserve(&distance_maps, num_galaxies));

  for(size_t i = 0; i < num_galaxies; i++) {
    DAR_DArray distance_map = {0};
    TRY(DAR_create(&distance_map, sizeof(size_t)));
    TRY(make_distance_map(universe, *(Position *)DAR_get(galaxy_positions, i), &distance_map));
    TRY(DAR_push_back(&distance_maps, &distance_map));
  }

  TRY(DAR_reserve(distances, galaxy_positions->size * galaxy_positions->size));
  for(size_t i = 0; i < num_galaxies; i++) {
    Position           pos_i    = *(Position *)DAR_get(galaxy_positions, i);
    const DAR_DArray * dist_map = DAR_get(&distance_maps, pos_to_idx(universe, pos_i));

    for(size_t j = 0; j < num_galaxies; j++) {
      Position       pos_j = *(Position *)DAR_get(galaxy_positions, j);
      const size_t * dist  = DAR_get(dist_map, pos_to_idx(universe, pos_j));
      TRY(DAR_push_back(distances, dist));
    }
  }

  for(size_t i = 0; i < num_galaxies; i++) {
    Position     pos_i    = *(Position *)DAR_get(galaxy_positions, i);
    DAR_DArray * dist_map = DAR_get(&distance_maps, pos_to_idx(universe, pos_i));
    TRY(DAR_destroy(dist_map));
  }
  TRY(DAR_destroy(&distance_maps));

  return OK;
}