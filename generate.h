#ifndef __GENERATE_H
#define __GENERATE_H

#define CHECKED 100

map *generate (int width, int height, coordinate entrance_position, coordinate exit_position);

void generate_random_route (map *labyrinth);

void generate_branches (map *labyrinth);

void generate_walls_around (map *labyrinth, coordinate position);

int get_random_direction ();

void reset_random_direction ();

#endif
