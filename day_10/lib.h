#ifndef lib_h
#define lib_h

#include <cfac/darray.h>
#include <cfac/log.h>
#include <cfac/stat.h>

typedef enum PieceType {
  GROUND = 0,         // '.'
  VERTICAL,           // '|',
  HORIZONTAL,         // '-',
  BEND_NORTH_TO_EAST, // 'L',
  BEND_NORTH_TO_WEST, // 'J',
  BEND_SOUTH_TO_WEST, // '7',
  BEND_SOUTH_TO_EAST, // 'F',
  ANIMAL_START,       // 'S',
} PieceType;

static inline bool is_connected_to_west(PieceType t) {
  return (t == HORIZONTAL) || (t == BEND_NORTH_TO_WEST) || (t == BEND_SOUTH_TO_WEST) || (t == ANIMAL_START);
}
static inline bool is_connected_to_east(PieceType t) {
  return (t == HORIZONTAL) || (t == BEND_NORTH_TO_EAST) || (t == BEND_SOUTH_TO_EAST) || (t == ANIMAL_START);
}
static inline bool is_connected_to_north(PieceType t) {
  return (t == VERTICAL) || (t == BEND_NORTH_TO_EAST) || (t == BEND_NORTH_TO_WEST) || (t == ANIMAL_START);
}
static inline bool is_connected_to_south(PieceType t) {
  return (t == VERTICAL) || (t == BEND_SOUTH_TO_WEST) || (t == BEND_SOUTH_TO_EAST) || (t == ANIMAL_START);
}

static inline PieceType char_to_piece_type(char c) {
  switch(c) {
  case '.': return GROUND;
  case '|': return VERTICAL;
  case '-': return HORIZONTAL;
  case 'L': return BEND_NORTH_TO_EAST;
  case 'J': return BEND_NORTH_TO_WEST;
  case '7': return BEND_SOUTH_TO_WEST;
  case 'F': return BEND_SOUTH_TO_EAST;
  case 'S': return ANIMAL_START;
  }
  LOG_STAT(STAT_ERR_INTERNAL, "bad piece type char: %c", c);
  return GROUND;
}

static inline char piece_type_to_char(PieceType t) {
  switch(t) {
  case GROUND: return '.';
  case VERTICAL: return '|';
  case HORIZONTAL: return '-';
  case BEND_NORTH_TO_EAST: return 'L';
  case BEND_NORTH_TO_WEST: return 'J';
  case BEND_SOUTH_TO_WEST: return '7';
  case BEND_SOUTH_TO_EAST: return 'F';
  case ANIMAL_START: return 'S';
  }
  LOG_STAT(STAT_ERR_INTERNAL, "bad piece type %u", t);
  return '.';
}

typedef struct Piece {
  PieceType type;
  size_t    dist_from_start;
  size_t    region_id;
} Piece;

typedef struct Position {
  size_t x, y;
} Position;

static inline bool is_positions_equal(Position a, Position b) { return (a.x == b.x && a.y == b.y); }

typedef struct PipeSketch {
  DAR_DArray pieces; // will contain Piece
  size_t     width;
  size_t     height;
  Position   start_pos;
} PipeSketch;

static inline size_t pos_to_idx(const PipeSketch * sketch, Position pos) { return (pos.x + (pos.y * sketch->width)); }

static inline Position idx_to_pos(const PipeSketch * sketch, size_t idx) {
  Position p = {0};
  p.y        = idx / sketch->width;
  p.x        = idx - (p.y * sketch->width);
  return p;
}

//  [const] Piece * get_piece([const] PipeSketch * sketch, Position pos)
#define get_piece(sketch, pos)                                                                                         \
  _Generic((sketch), const PipeSketch *: get_piece_const, PipeSketch *: get_piece_mut)(sketch, pos)
static inline Piece * get_piece_mut(PipeSketch * sketch, Position pos) {
  return DAR_get(&sketch->pieces, pos_to_idx(sketch, pos));
}
static inline const Piece * get_piece_const(const PipeSketch * sketch, Position pos) {
  return DAR_get(&sketch->pieces, pos_to_idx(sketch, pos));
}

STAT_Val parse_sketch(const DAR_DArray * lines, PipeSketch * sketch);

STAT_Val calculate_distances_from_start(PipeSketch * sketch);

STAT_Val get_max_distance_from_start(const PipeSketch * sketch, size_t * o_dist, Position * o_pos);

STAT_Val determine_enclosed_tiles(PipeSketch * sketch, size_t * num_enclosed);

STAT_Val destroy_sketch(PipeSketch * sketch);

#endif