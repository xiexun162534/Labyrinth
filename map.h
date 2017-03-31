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

/*
unsigned long int get_land_timestamp (map *labyrinth, coordinate position)
{
  return labyrinth->data[labyrinth->width * position.y + position.x].timestamp;
}
*/
#define get_land_timestamp(labyrinth, position) \
  ((labyrinth)->data[(labyrinth)->width * (position).y + (position).x].timestamp)

/*
void set_land_timestamp (map *labyrinth, coordinate position, unsigned long int timestamp)
{
  labyrinth->data[labyrinth->width * position.y + position.x].timestamp = timestamp;
}
*/
#define set_land_timestamp(labyrinth, position, get_timestamp) \
  ((labyrinth)->data[(labyrinth)->width * (position).y + (position).x].timestamp = (get_timestamp))

/*
int get_land_type (map *labyrinth, coordinate position)
{
  return labyrinth->data[labyrinth->width * position.y + position.x].type;
}
*/
#define get_land_type(labyrinth, position)                              \
  ((labyrinth)->data[(labyrinth)->width * (position).y + (position).x].type)

/*
void set_land_type (map *labyrinth, coordinate position, int type)
{
  labyrinth->data[labyrinth->width * position.y + position.x].type = type;
}
*/
#define set_land_type(labyrinth, position, land_type)                   \
  ((labyrinth)->data[(labyrinth)->width * (position).y + (position).x].type = (land_type))

/*
void set_entrance (map *labyrinth, coordinate entrance_position)
{
  labyrinth->entrance_position = entrance_position;
  set_land_type (labyrinth, entrance_position, ENTRANCE);
}
*/
#define set_entrance(labyrinth, position)                     \
  do                                                          \
    {                                                         \
      (labyrinth)->entrance_position = (position);            \
      set_land_type ((labyrinth), (position), ENTRANCE);      \
    } while (0)

/*
void set_exit (map *labyrinth, coordinate exit_position)
{
  labyrinth->exit_position = exit_position;
  set_land_type (labyrinth, exit_position, EXIT);
}
*/
#define set_exit(labyrinth, position)                     \
  do                                                      \
    {                                                     \
      (labyrinth)->exit_position = (position);            \
      set_land_type ((labyrinth), (position), EXIT);      \
    } while (0)

coordinate get_adjacent (coordinate position, int direction);
/*
coordinate get_adjacent (coordinate position, int direction)
{
  switch (direction)
    {
    case LEFT:
      position.x -= 1;
      break;
    case RIGHT:
      position.x += 1;
      break;
    case UP:
      position.y += 1;
      break;
    case DOWN:
      position.y -= 1;
      break;
    }
  return position;
}
*/
/*
#define get_adjacent(position, direction)         \
  ((direction) == LEFT) ?                         \
  ((position).x -= 1, (position)) :               \
  ((direction) == RIGHT) ?                        \
  ((position).x += 1, (position)) :               \
  ((direction) == UP) ?                           \
  ((position).y += 1, (position)) :               \
  ((direction) == DOWN) ?                         \
  ((position).y -= 1, (position)) :               \
  (position)
*/

/*
int is_in_map (map *labyrinth, coordinate position)
{
  if (position.x >= 0 && position.y >= 0 && position.x < labyrinth->width && position.y < labyrinth->height)
    return 1;
  else
    return 0;
}
*/
#define is_in_map(labyrinth, position) \
  (((position).x >= 0 && (position).y >= 0 && (position).x < (labyrinth)->width && (position).y < (labyrinth)->height) ? 1 : 0)


#endif
