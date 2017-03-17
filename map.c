#include "labyrinth.h"
#include "map.h"


map *create_empty_map (int width, int height)
{
  map *labyrinth;
  int i;

  labyrinth = malloc (sizeof (map));
  if (!labyrinth)
    {
      printf ("ERROR: Cannot allocate memory.\n");
      exit (1);
    }

  labyrinth->width = width;
  labyrinth->height = height;
  labyrinth->data = malloc (sizeof (land) * width * height);
  if (!labyrinth->data)
    {
      printf ("ERROR: Cannot allocate memory.\n");
      exit (1);
    }

  for (i = 0; i < width * height; i++)
    {
      labyrinth->data[i].type = LAND;
      labyrinth->data[i].timestamp = 0;
    }

  return labyrinth;
}

unsigned long int get_land_timestamp (map *labyrinth, coordinate position)
{
  return labyrinth->data[labyrinth->width * position.y + position.x].timestamp;
}

void set_land_timestamp (map *labyrinth, coordinate position, unsigned long int timestamp)
{
  labyrinth->data[labyrinth->width * position.y + position.x].timestamp = timestamp;
}

int get_land_type (map *labyrinth, coordinate position)
{
  return labyrinth->data[labyrinth->width * position.y + position.x].type;
}

void set_land_type (map *labyrinth, coordinate position, int type)
{
  labyrinth->data[labyrinth->width * position.y + position.x].type = type;
}

void set_entrance (map *labyrinth, coordinate entrance_position)
{
  labyrinth->entrance_position = entrance_position;
  set_land_type (labyrinth, entrance_position, ENTRANCE);
}

void set_exit (map *labyrinth, coordinate exit_position)
{
  labyrinth->exit_position = exit_position;
  set_land_type (labyrinth, exit_position, EXIT);
}

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

int is_in_map (map *labyrinth, coordinate position)
{
  if (position.x >= 0 && position.y >= 0 && position.x < labyrinth->width && position.y < labyrinth->height)
    return 1;
  else
    return 0;
}
