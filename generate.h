#ifndef __GENERATE_H
#define __GENERATE_H

#define CHECKED 100
#define AVERAGE_FORK_RATE 0.30
#define PROBABILITY_FORK_AGAIN 0.2
#define MIN_PROBABILITY_PRECISION 0.01
#define MAX_FORK_PROBABILITY 0.99
#define PROBABILITY_GO_AHEAD 0.99
#define PROBABILITY_GO_BACK 0.1


typedef struct __ROAD
{
  int forks;
  int length;
  coordinate *list;
  int *fork_list;
} road;

map *generate (int width, int height, coordinate entrance_position, coordinate exit_position);

void generate_main_road (map *labyrinth);

void generate_branches_along (map *labyrinth, road *current_road);

int go_ahead (map *labyrinth, road *current_road, coordinate *current_position_p);

int go_back (map *labyrinth, road *current_road, coordinate *current_position_p);

void generate_walls_around (map *labyrinth, coordinate position);

int break_wall (map *labyrinth, road *current_road, coordinate *position_p);

int weighted_get_num (int count, double sum, double *probability_list);

double fall_function (double x);

void clean_checked (map *labyrinth);

int get_random_direction ();

void reset_random_direction ();

int probability_event (double probability);

#endif
