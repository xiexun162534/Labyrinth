#ifndef __GENERATE_H
#define __GENERATE_H

#define CHECKED 100
#define PROBABILITY_CONTINUE_FORKING 0.85
#define PROBABILITY_GO_AHEAD 0.9
#define PROBABILITY_GO_BACK 0.1


typedef struct __ROAD
{
  int length;
  coordinate *list;
} road;

map *generate (int width, int height, coordinate entrance_position, coordinate exit_position);

void generate_main_road (map *labyrinth);

void generate_branches_along (map *labyrinth, road current_road);

int go_ahead (map *labyrinth, road *current_road_p, coordinate *current_position_p);

int go_back (map *labyrinth, road *current_road_p, coordinate *current_position_p);

void generate_walls_around (map *labyrinth, coordinate position);

void clean_checked (map *labyrinth);

int get_random_direction ();

void reset_random_direction ();

int probability_event (double probability);

#endif
