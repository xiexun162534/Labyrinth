#include "labyrinth.h"
#include "map.h"
#include "generate.h"
#include "timestamp.h"

#ifdef __DEBUG
#include "print.h"
#endif


static road main_road;
static int max_road_length;

map *generate (int width, int height, coordinate entrance_position, coordinate exit_position)
{
  map *labyrinth;
  labyrinth = create_empty_map (width, height);
  set_entrance (labyrinth, entrance_position);
  set_exit (labyrinth, exit_position);

  reset_timestamp ();
  srand (clock ());

  max_road_length = labyrinth->width * labyrinth->height;
  generate_main_road (labyrinth);

  /* Ignore entrance and exit */
  set_land_type (labyrinth, labyrinth->entrance_position, ROAD);
  set_land_type (labyrinth, labyrinth->exit_position, ROAD);
  generate_branches_along (labyrinth, main_road);
  /* Mark entrance and exit */
  set_land_type (labyrinth, labyrinth->entrance_position, ENTRANCE);
  set_land_type (labyrinth, labyrinth->exit_position, EXIT);

  return labyrinth;
}

void generate_main_road (map *labyrinth)
{
  coordinate current_position, next_position;
  int i;

  main_road.length = 0;
  main_road.list = (coordinate *) malloc (sizeof (coordinate) * max_road_length);
  if (!main_road.list)
    {
      printf ("ERROR: Cannot allocate memory.\n");
      exit (1);
    }

  current_position = labyrinth->entrance_position;
  set_land_timestamp (labyrinth, current_position, get_timestamp ());
  main_road.list[main_road.length] = current_position;
  main_road.length++;
  if (get_land_type (labyrinth, current_position) == EXIT)
    /* We get it. */
    {
      generate_walls_around (labyrinth, next_position);
      clean_checked (labyrinth);
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
              main_road.list[main_road.length] = next_position;
              main_road.length++;
              clean_checked (labyrinth);
              return ;
            }
        }

      /* Choose a way randomly */
      if (!go_ahead (labyrinth, &main_road, &current_position))
        /* There's no way to go. We have to go back. */
        {
          if (!go_back (labyrinth, &main_road, &current_position))
            {
              printf ("ERROR: Are you kidding?\n");
              exit (3);
            }
        }
    }
}


void generate_branches_along (map *labyrinth, road current_road)
{
  road branch;
  max_road_length -= current_road.length;
  branch.length = 0;
  branch.list = malloc (sizeof (coordinate) * max_road_length);
  while (probability_event (PROBABILITY_CONTINUE_FORKING))
    {
      int i;
      coordinate current_position, next_position;
      current_position = current_road.list[(rand () % current_road.length)];
      reset_random_direction ();
      for (i = 0; i < TOTAL_DIRECTIONS; i++)
        {
          next_position = get_adjacent (current_position, get_random_direction ());
          if (is_in_map (labyrinth, next_position) && get_land_type (labyrinth, next_position) == WALL)
            {
              set_land_type (labyrinth, next_position, ROAD);
              set_land_timestamp (labyrinth, next_position, get_timestamp ());
              branch.list[branch.length] = next_position;
              branch.length++;
              current_position = next_position;
              break;
            }
        }
      if (i == TOTAL_DIRECTIONS)
        continue;

      while (probability_event (PROBABILITY_GO_AHEAD))
        {
          #ifdef __DEBUG
          print (labyrinth);
          #endif
          if (!go_ahead (labyrinth, &branch, &current_position))
            {
              if (probability_event (PROBABILITY_GO_BACK))
                {
                  if (!go_back (labyrinth, &branch, &current_position))
                    {
                      generate_walls_around (labyrinth, current_position);
                      clean_checked (labyrinth);
                      branch.length = 0;
                      break;
                    }
                }
              else
                {
                  generate_walls_around (labyrinth, current_position);
                  clean_checked (labyrinth);
                  generate_branches_along (labyrinth, branch);
                  branch.length = 0;
                  break;
                }
            }
          else
            {
              generate_walls_around (labyrinth, current_position);
            }
        }
    }

  max_road_length += current_road.length;
}

int go_ahead (map *labyrinth, road *current_road_p, coordinate *current_position_p)
{
  int i;
  coordinate next_position;
  /* Choose a way randomly */
  reset_random_direction ();
  for (i = 0; i < TOTAL_DIRECTIONS; i++)
    {
      next_position = get_adjacent (*current_position_p, get_random_direction ());
      if (is_in_map (labyrinth, next_position) && get_land_type (labyrinth, next_position) == LAND)
        break;
    }
  if (i == TOTAL_DIRECTIONS)
    /* No way */
    return 0;

  if (get_land_type (labyrinth, *current_position_p) != ENTRANCE && get_land_type (labyrinth, *current_position_p) != EXIT)
    {
      set_land_type (labyrinth, *current_position_p, ROAD);
      set_land_timestamp (labyrinth, *current_position_p, get_timestamp ());
    }
  generate_walls_around (labyrinth, *current_position_p);
  *current_position_p = next_position;
  current_road_p->list[current_road_p->length] = *current_position_p;
  current_road_p->length++;
  return 1;
}

int go_back (map *labyrinth, road *current_road_p, coordinate *current_position_p)
{
  int i;
  coordinate position;
  if (current_road_p->length == 1)
    /* No way to go back */
    return 0;

  current_road_p->length--;
  position = current_road_p->list[current_road_p->length - 1];

  set_land_type (labyrinth, *current_position_p, CHECKED);
  set_land_timestamp (labyrinth, *current_position_p, get_timestamp ());
  *current_position_p = position;

  /* Remove walls that are generated after we go through current position */
  for (i = 0; i < TOTAL_DIRECTIONS; i++)
    {
      position = get_adjacent (*current_position_p, i);
      if (is_in_map (labyrinth, position) && get_land_type (labyrinth, position) == WALL && get_land_timestamp (labyrinth, position) > get_land_timestamp (labyrinth, *current_position_p))
        {
          set_land_type (labyrinth, position, LAND);
          set_land_timestamp (labyrinth, position, 0);
        }
    }

  return 1;
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

void clean_checked (map *labyrinth)
{
  int i;
  coordinate position, next_position;
  for (position.y = 0; position.y < labyrinth->height; position.y++)
    for (position.x = 0; position.x < labyrinth->width; position.x++)
      {
        if (get_land_type (labyrinth, position) == CHECKED)
          {
            for (i = 0; i < TOTAL_DIRECTIONS; i++)
              {
                next_position = get_adjacent (position, i);
                if (is_in_map (labyrinth, next_position) && (get_land_type (labyrinth, next_position) == ROAD || get_land_type (labyrinth, next_position) == ENTRANCE || get_land_type (labyrinth, next_position) == EXIT))
                  {
                    set_land_type (labyrinth, position, WALL);
                    set_land_timestamp (labyrinth, position, get_timestamp ());
                    break;
                  }
              }
            if (i == TOTAL_DIRECTIONS)
              {
                set_land_type (labyrinth, position, LAND);
                set_land_timestamp (labyrinth, position, 0);
              }
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

  if (rand () < probability * RAND_MAX)
    return 1;
  else
    return 0;
}
