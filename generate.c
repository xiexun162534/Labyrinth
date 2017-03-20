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
  generate_branches_along (labyrinth, &main_road);
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
      generate_walls_around (labyrinth, current_position);
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


void generate_branches_along (map *labyrinth, road *current_road)
{
  road branch;
  coordinate current_position;
  max_road_length -= current_road->length;
  current_road->forks = 0;
  current_road->fork_list = (int *) malloc (sizeof (int) * TOTAL_DIRECTIONS * current_road->length);
  branch.length = 0;
  branch.list = malloc (sizeof (coordinate) * max_road_length);
  if (!current_road->fork_list || !branch.list)
    {
      printf ("ERROR: Cannot allocate memory.\n");
      exit (1);
    }

  while (break_wall (labyrinth, current_road, &current_position))
    {
      while (probability_event (PROBABILITY_GO_AHEAD))
        {
          #ifdef __DEBUG
          print (labyrinth);
          #endif
 
          branch.list[branch.length] = current_position;
          branch.length++;

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
                  generate_branches_along (labyrinth, &branch);
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

  max_road_length += current_road->length;
}

int go_ahead (map *labyrinth, road *current_road, coordinate *current_position_p)
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
  current_road->list[current_road->length] = *current_position_p;
  current_road->length++;
  return 1;
}

int go_back (map *labyrinth, road *current_road, coordinate *current_position_p)
{
  int i;
  coordinate position;
  if (current_road->length == 1)
    /* No way to go back */
    return 0;

  current_road->length--;
  position = current_road->list[current_road->length - 1];

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

/* Determine whether we have to break a wall alongside the road. If we determine to break a wall, get the position of wall and return 1. Otherwise, return 0. */
int break_wall (map *labyrinth, road *current_road, coordinate *position_p)
{
  int i;
  static double sum;
  static double *probability_list = NULL;
  coordinate current_position, next_position;

  if (!probability_event (fall_function ((double) current_road->forks / (double) current_road->length)))
    /* Done. */
    return 0;

  if (current_road->forks == 0)
    {
      if (!probability_list)
        free (probability_list);
      probability_list = (double *) malloc (sizeof (double) * current_road->length);
      if (!probability_list)
        {
          printf ("ERROR: Cannot allocate memory.\n");
          exit (1);
        }

      /* Ignore two ends of the road */
      probability_list[0] = 0.0;
      probability_list[current_road->length - 1] = 0.0;

      for (i = 1; i < current_road->length - 1; i++)
        {
          probability_list[i] = 1.0;
        }
      sum = (double) current_road->length - 2.0;
    }

  while (1)
    {
      int index;
      index = weighted_get_num (current_road->length, sum, probability_list);
      current_position = current_road->list[index];

      reset_random_direction ();
      for (i = 0; i < TOTAL_DIRECTIONS; i++)
        {
          next_position = get_adjacent (current_position, get_random_direction ());
          if (is_in_map (labyrinth, next_position) && get_land_type (labyrinth, next_position) == WALL)
            {
              set_land_type (labyrinth, next_position, ROAD);
              set_land_timestamp (labyrinth, next_position, get_timestamp ());
              current_road->forks++;
              sum -= probability_list[index] - PROBABILITY_FORK_AGAIN;
              probability_list[index] = PROBABILITY_FORK_AGAIN;
              *position_p = next_position;
              return 1;
            }
        }

      if (i == TOTAL_DIRECTIONS)
        {
          sum -= probability_list[index] - 0.0;
          probability_list[index] = 0.0;
          if (sum < MIN_PROBABILITY_PRECISION)
            return 0;
        }
    }
}


int weighted_get_num (int count, double sum, double *probability_list)
{
  int i;
  double num;
  double current_sum;
  current_sum = 0.0;
  num = sum * (double) rand () / (double) RAND_MAX;
  for (i = 0; i < count; i++)
    {
      current_sum += probability_list[i];
      if (current_sum > num)
        break;
    }
  return i;
}


/*
It's something like this:

1==============_-----------------------------------
   |            \                       |
   |             \                      |
   |             \                      |
   |              \                     |
   |               -\_____________________________
0--|------------------------------------+----------
   0                                    1

 */
double fall_function (double x)
{
  double k = -log (1 / MAX_FORK_PROBABILITY - 1) / AVERAGE_FORK_RATE;
  return 1 / (exp (k * (x - AVERAGE_FORK_RATE)) + 1);
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
