#include "labyrinth.h"
#include "map.h"
#include "generate.h"
#include "timestamp.h"

#ifdef __DEBUG
#include "print.h"
#endif



map *generate (int width, int height, coordinate entrance_position, coordinate exit_position)
{
  map *labyrinth;
  labyrinth = create_empty_map (width, height);
  set_entrance (labyrinth, entrance_position);
  set_exit (labyrinth, exit_position);

  srand (clock ());
  generate_random_route (labyrinth);
  generate_branches (labyrinth);

  return labyrinth;
}

void generate_random_route (map *labyrinth)
{
  coordinate current_position, next_position;
  int direction;
  int i;

  current_position = labyrinth->entrance_position;
  set_land_timestamp (labyrinth, current_position, get_timestamp ());
  if (get_land_type (labyrinth, current_position) == EXIT)
    /* We get it. */
    {
      generate_walls_around (labyrinth, next_position);
      return ;
    }
  while (1)
    {
      for (i = 0; i < TOTAL_DIRECTIONS; i++)
        {
          next_position = get_adjacent (current_position, i);
          if (is_in_map (labyrinth, next_position) && get_land_type (labyrinth, next_position) == EXIT)
            /* We get it. */
            {
              if (get_land_type (labyrinth, current_position) != ENTRANCE && get_land_type (labyrinth, current_position) != EXIT)
                {
                  set_land_type (labyrinth, current_position, ROAD);
                  set_land_timestamp (labyrinth, current_position, get_timestamp ());
                }
              generate_walls_around (labyrinth, current_position);
              generate_walls_around (labyrinth, next_position);
              return ;
            }
        }
      /* Choose a way randomly */
      reset_random_direction ();
      for (i = 0; i < TOTAL_DIRECTIONS; i++)
        {
          direction = get_random_direction ();
          next_position = get_adjacent (current_position, direction);
          if (is_in_map (labyrinth, next_position) && get_land_type (labyrinth, next_position) == LAND)
            break;
        }
      if (i == TOTAL_DIRECTIONS)
        /* There's no way to go. We have to go back. */
        {
          if (get_land_type (labyrinth, current_position) == ENTRANCE)
            {
              printf ("ERROR: Are you kidding?\n");
              exit (3);
            }

          /* Look for the way back */
          for (i = 0; i < TOTAL_DIRECTIONS; i++)
            {
              coordinate position;
              position = get_adjacent (current_position, i);
              if (is_in_map (labyrinth, position) && get_land_type (labyrinth, position) == ROAD)
                {
                  set_land_type (labyrinth, current_position, CHECKED);
                  set_land_timestamp (labyrinth, current_position, get_timestamp ());
                  current_position = position;
                  break;
                }
            }

          /* Remove walls that are generated after we go through current position */
          for (i = 0; i < TOTAL_DIRECTIONS; i++)
            {
              coordinate position;
              position = get_adjacent (current_position, i);
              if (is_in_map (labyrinth, position) && get_land_type (labyrinth, position) == WALL && get_land_timestamp (labyrinth, position) > get_land_timestamp (labyrinth, current_position))
                {
                  set_land_type (labyrinth, position, LAND);
                  set_land_timestamp (labyrinth, position, 0);
                }
            }
        }
      else
        {
          /* Go ahead */
          if (get_land_type (labyrinth, current_position) != ENTRANCE && get_land_type (labyrinth, current_position) != EXIT)
            {
              set_land_type (labyrinth, current_position, ROAD);
              set_land_timestamp (labyrinth, current_position, get_timestamp ());
            }
          generate_walls_around (labyrinth, current_position);
          current_position = next_position;
        }

      #ifdef __DEBUG
      print (labyrinth);
      #endif
      
    }
}


void generate_branches (map *labyrinth)
{
}


void generate_walls_around (map *labyrinth, coordinate position)
{
  int i;
  for (i = 0; i < TOTAL_DIRECTIONS; i++)
    {
      coordinate next_position = get_adjacent (position, i);
      if (is_in_map (labyrinth, next_position) && get_land_type (labyrinth, next_position) == LAND)
        {
          set_land_type (labyrinth, next_position, WALL);
          set_land_timestamp (labyrinth, next_position, get_timestamp ());
        }
    }
}


/* Enumerate directions randomly */
static int directions[TOTAL_DIRECTIONS];
static int directions_count;
static int last_direction;
int get_random_direction (void)
{
  int direction, delta;

  if (directions_count >= TOTAL_DIRECTIONS)
    /* Out of range */
    return TOTAL_DIRECTIONS;

  direction = last_direction;
  delta = rand () % (TOTAL_DIRECTIONS - directions_count);
  for (; delta >= 0; delta--)
    {
      do
        {
          direction = (direction + 1) % TOTAL_DIRECTIONS;
        } while (directions[direction]);
    }
  directions[direction] = 1;
  last_direction = direction;
  directions_count++;

  return direction;
}

void reset_random_direction (void)
{
  int i;
  directions_count = 0;
  for (i = 0; i < TOTAL_DIRECTIONS; i++)
    directions[i] = 0;
}


int probability_event (double probability)
{
  if (probability > 1.0 || probability < 0.0)
    {
      printf ("ERROR: Invalid probability.\n");
      exit (4);
    }

  if (rand () < probability * ULONG_MAX)
    return 1;
  else
    return 0;
}
