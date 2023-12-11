#include <cfac/log.h>

#include "common.h"
#include "lib.h"
#include <stdio.h>

static bool is_valid_sketch_char(char c) {
  return ((c == '.') || (c == '|') || (c == '-') || (c == 'L') || (c == 'J') || (c == '7') || (c == 'F') || (c == 'S'));
}

static STAT_Val init_sketch(PipeSketch * sketch) {
  CHECK(sketch != NULL);
  *sketch = (PipeSketch){0};

  TRY(DAR_create(&sketch->pieces, sizeof(Piece)));

  return OK;
}

STAT_Val parse_sketch(const DAR_DArray * lines, PipeSketch * sketch) {
  CHECK(lines != NULL);
  CHECK(DAR_is_initialized(lines) && !DAR_is_empty(lines));
  CHECK(lines->element_size == sizeof(DAR_DArray));
  CHECK(sketch != NULL);

  TRY(init_sketch(sketch));

  size_t y = 0;
  for(const DAR_DArray * line = DAR_first(lines); line != DAR_end(lines); line++, y++) {
    size_t last_char = line->size - 1;
    while(!is_valid_sketch_char(*(const char *)DAR_get(line, last_char))) {
      if(last_char == 0) break;
      last_char--;
    }
    if(last_char > 0) {
      if((sketch->width != (last_char + 1)) && (sketch->width != 0)) {
        return LOG_STAT(STAT_ERR_READ, "malformed sketch line '%s'", (const char *)line->data);
      }
      sketch->width = (last_char + 1);

      const char * raw_line = line->data;
      for(size_t i = 0; i <= last_char; i++) {
        Piece piece = {.type = char_to_piece_type(raw_line[i]), .dist_from_start = SIZE_MAX, .region_id = 0};
        TRY(DAR_push_back(&sketch->pieces, &piece));

        if(piece.type == ANIMAL_START) sketch->start_pos = (Position){.x = i, .y = y};
      }
      sketch->height++;
    }
  }

  return OK;
}

STAT_Val destroy_sketch(PipeSketch * sketch) {
  CHECK(sketch != NULL);
  CHECK(DAR_is_initialized(&sketch->pieces));

  TRY(DAR_destroy(&sketch->pieces));
  *sketch = (PipeSketch){0};

  return OK;
}

static STAT_Val are_pieces_connected(const PipeSketch * sketch, Position pos_a, Position pos_b, bool * out) {
  CHECK(sketch != NULL);
  CHECK(out != NULL);
  CHECK(DAR_is_initialized(&sketch->pieces));
  CHECK(!DAR_is_empty(&sketch->pieces));
  CHECK_WITH_MSG(pos_a.x < sketch->width, "pos_a.x < sketch->width: (%zu < %zu)", pos_a.x, sketch->width);
  CHECK_WITH_MSG(pos_a.y < sketch->height, "pos_a.y < sketch->height: (%zu < %zu)", pos_a.y, sketch->height);
  CHECK_WITH_MSG(pos_b.x < sketch->width, "pos_b.x < sketch->width: (%zu < %zu)", pos_b.x, sketch->width);
  CHECK_WITH_MSG(pos_b.y < sketch->height, "pos_b.y < sketch->height: (%zu < %zu)", pos_b.y, sketch->height);

  const Piece * a = get_piece(sketch, pos_a);
  const Piece * b = get_piece(sketch, pos_b);

  *out = false;

  if(((pos_a.x + 1) == pos_b.x) && (pos_a.y == pos_b.y)) {
    // a is WEST of b, b is EAST of a
    *out = (is_connected_to_west(b->type) && is_connected_to_east(a->type));
  } else if(((pos_a.x - 1) == pos_b.x) && (pos_a.y == pos_b.y)) {
    // a is EAST of b, b is WEST of a
    *out = (is_connected_to_east(b->type) && is_connected_to_west(a->type));
  } else if(((pos_a.y + 1) == pos_b.y) && pos_a.x == pos_b.x) {
    // a is SOUTH of b, b is NORTH of a
    *out = (is_connected_to_north(b->type) && is_connected_to_south(a->type));
  } else if(((pos_a.y - 1) == pos_b.y) && pos_a.x == pos_b.x) {
    // a is NORTH of b, b is SOUTH of a
    *out = (is_connected_to_south(b->type) && is_connected_to_north(a->type));
  } else {
    return LOG_STAT(STAT_ERR_ARGS, "positions not adjacent: {%zu,%zu} {%zu,%zu}", pos_a.x, pos_a.y, pos_b.x, pos_b.y);
  }

  return OK;
}

static STAT_Val set_distance_from_start(PipeSketch * sketch, Position pos) {
  CHECK(sketch != NULL);
  CHECK(DAR_is_initialized(&sketch->pieces));
  CHECK(!DAR_is_empty(&sketch->pieces));
  CHECK(pos.x < sketch->width);
  CHECK(pos.y < sketch->height);

  Piece * target = DAR_get(&sketch->pieces, pos_to_idx(sketch, pos));

  const Position adjacents[] = {
      {.x = ((pos.x == 0) ? 0 : (pos.x - 1)), .y = pos.y},      // WEST
      {.x = pos.x, .y = ((pos.y == 0) ? 0 : (pos.y - 1))},      // NORTH
      {.x = min_sz(sketch->width - 1, pos.x + 1), .y = pos.y},  // EAST
      {.x = pos.x, .y = min_sz(sketch->height - 1, pos.y + 1)}, // SOUTH
  };

  size_t min_adjacent_dist = SIZE_MAX;
  for(size_t i = 0; i < (sizeof(adjacents) / sizeof(adjacents[0])); i++) {
    Position adjacent_pos = adjacents[i];
    if(!is_positions_equal(pos, adjacent_pos)) {
      bool is_connected = false;
      TRY(are_pieces_connected(sketch, pos, adjacent_pos, &is_connected));

      if(is_connected) {
        const Piece * adjacent      = get_piece(sketch, adjacent_pos);
        size_t        adjacent_dist = adjacent->dist_from_start;
        min_adjacent_dist           = (adjacent_dist < min_adjacent_dist) ? adjacent_dist : min_adjacent_dist;
      }
    }
  }

  if((min_adjacent_dist != SIZE_MAX) && (target->dist_from_start > (min_adjacent_dist + 1))) {
    target->dist_from_start = min_adjacent_dist + 1;
  }

  return OK;
}

// static void print_sketch_distances(const PipeSketch * sketch) {
//   Position pos = {0};
//   for(pos.y = 0; pos.y < sketch->height; pos.y++) {
//     for(pos.x = 0; pos.x < sketch->width; pos.x++) { printf("%.16zx ", get_piece(sketch, pos)->dist_from_start); }
//     putc('\n', stdout);
//   }
// }

STAT_Val calculate_distances_from_start(PipeSketch * sketch) {
  CHECK(sketch != NULL);
  CHECK(DAR_is_initialized(&sketch->pieces));

  get_piece(sketch, sketch->start_pos)->dist_from_start = 0;

  // fill in all distances repeatedly until we reach a fixed point (i.e. the values stop changing)
  // TODO spiral out from the start position, as that is much more efficient
  size_t num_iterations = 0;
  bool   is_fixed_point = false;
  do {
    is_fixed_point = true;

    Position pos = {0};
    for(pos.y = 0; pos.y < sketch->height; pos.y++) {
      for(pos.x = 0; pos.x < sketch->width; pos.x++) {
        const Piece * piece = get_piece(sketch, pos);
        if(piece->type != GROUND) {
          const size_t orig_dist = piece->dist_from_start;

          TRY(set_distance_from_start(sketch, pos));

          if(piece->dist_from_start != orig_dist) is_fixed_point = false;
        }
      }
    }
    num_iterations++;

    if((num_iterations & 0xff) == 0) LOG_STAT(STAT_OK, "iteration %zu", num_iterations);
  } while(!is_fixed_point);

  LOG_STAT(STAT_OK, "took %zu iterations to get to fixed point", num_iterations);

  return OK;
}

STAT_Val get_max_distance_from_start(const PipeSketch * sketch, size_t * o_dist, Position * o_pos) {
  CHECK(sketch != NULL);
  CHECK(DAR_is_initialized(&sketch->pieces));
  CHECK(o_dist != NULL);
  CHECK(o_pos != NULL);

  *o_pos  = (Position){0};
  *o_dist = 0;

  Position pos = {0};
  for(pos.y = 0; pos.y < sketch->height; pos.y++) {
    for(pos.x = 0; pos.x < sketch->width; pos.x++) {
      const size_t dist = get_piece(sketch, pos)->dist_from_start;
      if((dist != SIZE_MAX) && (dist > *o_dist)) {
        *o_dist = dist;
        *o_pos  = pos;
      }
    }
  }

  return OK;
}

static STAT_Val grow_region_of_ground(PipeSketch * sketch, Position pos, size_t region_id, size_t * region_size) {
  CHECK(sketch != NULL);
  CHECK(DAR_is_initialized(&sketch->pieces));
  CHECK_WITH_MSG(pos.x < sketch->width, "pos.x < sketch->width: (%zu < %zu)", pos.x, sketch->width);
  CHECK_WITH_MSG(pos.y < sketch->height, "pos.y < sketch->height: (%zu < %zu)", pos.y, sketch->height);

  Piece * piece = get_piece(sketch, pos);
  if(piece->type != GROUND || piece->region_id != 0) return OK;

  const Position adjacents[] = {
      {.x = ((pos.x == 0) ? 0 : (pos.x - 1)), .y = pos.y},      // WEST
      {.x = pos.x, .y = ((pos.y == 0) ? 0 : (pos.y - 1))},      // NORTH
      {.x = min_sz(sketch->width - 1, pos.x + 1), .y = pos.y},  // EAST
      {.x = pos.x, .y = min_sz(sketch->height - 1, pos.y + 1)}, // SOUTH
  };

  piece->region_id = region_id;
  (*region_size)++;

  for(size_t i = 0; i < (sizeof(adjacents) / sizeof(adjacents[0])); i++) {
    if(!is_positions_equal(adjacents[i], pos)) {
      TRY(grow_region_of_ground(sketch, adjacents[i], region_id, region_size));
    }
  }

  return OK;
}

static void print_sketch(const PipeSketch * sketch) {
  Position pos = {0};
  for(pos.y = 0; pos.y < sketch->height; pos.y++) {
    for(pos.x = 0; pos.x < sketch->width; pos.x++) {
      const Piece * piece = get_piece(sketch, pos);
      if(piece->type == GROUND) {
        printf("%4.4zu ", piece->region_id);
      } else {
        printf("  %c  ", piece_type_to_char(piece->type));
      }
    }
    putc('\n', stdout);
  }
}

STAT_Val determine_enclosed_tiles(PipeSketch * sketch, size_t * num_enclosed) {
  CHECK(sketch != NULL);
  CHECK(DAR_is_initialized(&sketch->pieces));
  CHECK(num_enclosed != NULL);

  // TODO find way to exclude outside-of-pipe-inclusion better
  // 1. grow all ground regions as potentially enclosed, and collect their size by region id
  // 2. go along edges of field, eliminating all seen regions from the list
  // 3. sum up remaining region sizes

  const size_t zero = 0;

  DAR_DArray regions = {0}; // contains size_t, being the size of the region
  TRY(DAR_create(&regions, sizeof(size_t)));
  TRY(DAR_push_back(&regions, &zero)); // start at size 1, so region id 0 is skipped

  size_t region_id = 1;

  bool     is_inside = false;
  Position pos       = {0};
  for(pos.y = 0; pos.y < sketch->height; pos.y++) {
    is_inside = false;
    for(pos.x = 0; pos.x < sketch->width; pos.x++) {
      Piece * piece = get_piece(sketch, pos);
      if((piece->type == GROUND) && (piece->region_id == 0) && is_inside) {
        TRY(DAR_resize_zeroed(&regions, regions.size + 1));
        TRY(grow_region_of_ground(sketch, pos, region_id, DAR_last(&regions)));
        LOG_STAT(STAT_OK, "growing from (%zu,%zu) : %zu", pos.x, pos.y, region_id);
        region_id++;
      } else if(piece->dist_from_start != SIZE_MAX) {
        // part of pipe
        // we're moving west to east
        if(!is_connected_to_east(piece->type)) is_inside = !is_inside;
      }
    }
  }

  print_sketch(sketch);

  for(pos.x = 0; pos.x < sketch->width; pos.x++) {
    is_inside = false;
    for(pos.y = 0; pos.y < sketch->height; pos.y++) {
      Piece * piece = get_piece(sketch, pos);
      if((piece->type == GROUND) && (piece->region_id == 0) && is_inside) {
        TRY(DAR_resize_zeroed(&regions, regions.size + 1));
        TRY(grow_region_of_ground(sketch, pos, region_id, DAR_last(&regions)));
        LOG_STAT(STAT_OK, "growing from (%zu,%zu) : %zu", pos.x, pos.y, region_id);
        region_id++;
      } else if(piece->dist_from_start != SIZE_MAX) {
        // part of pipe
        // we're moving north to south
        if(!is_connected_to_south(piece->type)) is_inside = !is_inside;
      }
    }
  }

  print_sketch(sketch);

  TRY(DAR_destroy(&regions));

  return OK;
}