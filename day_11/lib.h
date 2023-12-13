#ifndef lib_h
#define lib_h

#include <cfac/darray.h>
#include <cfac/stat.h>

#include "common.h"

typedef enum SpaceType { EMPTY, GALAXY } SpaceType;

static inline char space_type_to_char(SpaceType t) {
  switch(t) {
  case EMPTY: return '.';
  case GALAXY: return '#';
  default: return '?';
  }
}

static inline SpaceType char_to_space_type(char c) {
  switch(c) {
  case '.': return EMPTY;
  case '#': return GALAXY;
  default: return EMPTY;
  }
}

typedef struct Space {
  SpaceType type;
  size_t    horizontal_density;
  size_t    vertical_density;
} Space;

typedef struct Universe {
  DAR_DArray spaces; // contains Space
  size_t     width;
  size_t     height;
} Universe;

typedef struct Position {
  size_t x, y;
} Position;

static inline size_t pos_to_idx(const Universe * universe, Position pos) { return (pos.x + (pos.y * universe->width)); }

static inline Position idx_to_pos(const Universe * universe, size_t idx) {
  Position p = {0};
  p.y        = idx / universe->width;
  p.x        = idx - (p.y * universe->width);
  return p;
}
//  [const] Space * get_space([const] Universe * universe, Position pos)
#define get_space(universe, pos)                                                                                       \
  _Generic((universe), const Universe *: get_space_const, Universe *: get_space_mut)(universe, pos)
static inline Space * get_space_mut(Universe * universe, Position pos) {
  return DAR_get(&universe->spaces, pos_to_idx(universe, pos));
}
static inline const Space * get_space_const(const Universe * universe, Position pos) {
  return DAR_get(&universe->spaces, pos_to_idx(universe, pos));
}

STAT_Val parse_universe(const DAR_DArray * lines, Universe * universe);

STAT_Val increase_space_density_for_gaps(Universe * universe, size_t new_density_of_gaps);

STAT_Val destroy_universe(Universe * universe);

STAT_Val calculate_galaxy_distances(
    const Universe * universe,
    DAR_DArray *     galaxy_positions /* contains Positions */,
    DAR_DArray *     distances /* contains distances ordered [1-1,1-2,...,1-n,2-1,2-2,...,2-n,...]*/);

STAT_Val sum_galaxy_distances(const Universe *   universe,
                              const DAR_DArray * distances,
                              size_t             num_galaxies,
                              size_t *           sum);

void print_universe(const Universe * universe);

static inline bool is_positions_equal(Position a, Position b) { return (a.x == b.x && a.y == b.y); }

#endif