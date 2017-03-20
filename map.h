#ifndef __MAP_H
#define __MAP_H

#define LAND 0
#define ROAD 1
#define WALL 2
#define ENTRANCE 3
#define EXIT 4

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3
#define TOTAL_DIRECTIONS 4


typedef struct __COORDINATE
{
  int x;
  int y;
} coordinate;

typedef struct __LAND
{
  int type;
  unsigned long int timestamp;
} land;

typedef struct __MAP
{
  int width;
  int height;
  coordinate entrance_position;
  coordinate exit_position;
  land *data;
} map;

map *create_empty_map (int width, int height);

unsigned long int get_land_timestamp (map *labyrinth, coordinate position);

void set_land_timestamp (map *labyrinth, coordinate position, unsigned long int timestamp);

int get_land_type (map *labyrinth, coordinate position);

void set_land_type (map *labyrinth, coordinate position, int type);

void set_entrance (map *labyrinth, coordinate entrance_position);

void set_exit (map *labyrinth, coordinate exit_position);

coordinate get_adjacent (coordinate position, int direction);

int is_in_map (map *labyrinth, coordinate position);

#endif
