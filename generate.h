#ifndef __GENERATE_H
#define __GENERATE_H

#define CHECKED 100
#define AVERAGE_FORK_RATE 0.1
#define SHORT_ROAD 8
#define PROBABILITY_FORK_SHORT_ROAD 0.2
#define FORK_AGAIN_REDUCTION 0.8
#define ADJACENT_FORK_AGAIN_REDUCTION 0.6
#define MIN_PROBABILITY_PRECISION 0.01
#define MAX_FORK_PROBABILITY 0.99
#define PROBABILITY_GO_AHEAD 0.999
#define PROBABILITY_GO_BACK 0.1

#include "map.h"
#include "weighted_algorithm.h"

typedef struct __EXTENDED_MAP
{
  map *labyrinth;
  coordinate *checked_list;
  int checked_count;
} extended_map;

typedef struct __ROAD
{
  int forks;
  int length;
  coordinate *list;
  int *fork_list;
  weighted_tree *weighted;
} road;

map *generate (int width, int height, coordinate entrance_position, coordinate exit_position);

void generate_main_road (extended_map *extended_labyrinth);

void generate_branches_along (extended_map *extended_labyrinth, road *current_road);

int go_ahead (extended_map *extended_labyrinth, road *current_road, coordinate *current_position_p);

int go_back (extended_map *extended_labyrinth, road *current_road, coordinate *current_position_p);

void generate_walls_around (extended_map *extended_labyrinth, coordinate position);

int break_wall (extended_map *extended_labyrinth, road *current_road, coordinate *position_p);


double fall_function (double x);

void clean_checked (extended_map *extended_labyrinth);

int get_random_direction ();

void reset_random_direction ();

int probability_event (double probability);

#endif
