#include "labyrinth.h"
#include "map.h"
#include "generate.h"
#include "timestamp.h"

#define __DEBUG_LESS
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

  #ifdef __DEBUG_LESS
  {
    clock_t seed = clock ();
    seed = 3337;
    printf ("WATCH HERE: %ld!!!!!!!!!!!!!!!!!!!\n", seed);
    srand (seed);
  }
  #else
  srand (clock ());
  #endif

  max_road_length = labyrinth->width * labyrinth->height;
  main_road.length = 0;
  main_road.list = (coordinate *) malloc (sizeof (coordinate) * max_road_length);
  if (!main_road.list)
    {
      printf ("ERROR: Cannot allocate memory.\n");
      exit (1);
    }
  generate_main_road (labyrinth);

  /* Ignore entrance and exit */
  set_land_type (labyrinth, labyrinth->entrance_position, ROAD);
  set_land_type (labyrinth, labyrinth->exit_position, ROAD);
  generate_branches_along (labyrinth, &main_road);
  /* Mark entrance and exit */
  set_land_type (labyrinth, labyrinth->entrance_position, ENTRANCE);
  set_land_type (labyrinth, labyrinth->exit_position, EXIT);

  free (main_road.list);

  return labyrinth;
}

void generate_main_road (map *labyrinth)
{
  coordinate current_position, next_position;
  int i;

  /* Go from entrance */
  current_position = labyrinth->entrance_position;
  set_land_timestamp (labyrinth, current_position, get_timestamp ());
  generate_walls_around (labyrinth, current_position);
  main_road.list[main_road.length] = current_position;
  main_road.length++;

  if (get_land_type (labyrinth, current_position) == EXIT)
    /* The entrance is also the exit. */
    {
      return ;
    }


  while (1)
    {
      /* Check whether the exit is adjacent to current position. If so, return. */
      for (i = 0; i < TOTAL_DIRECTIONS; i++)
        {
          next_position = get_adjacent (current_position, i);
          if (is_in_map (labyrinth, next_position) && get_land_type (labyrinth, next_position) == EXIT)
            /* We get it. */
            {
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
  /*current_road->fork_list = (int *) malloc (sizeof (int) * TOTAL_DIRECTIONS * current_road->length);*/
  branch.length = 0;
  branch.list = malloc (sizeof (coordinate) * max_road_length);
  if (!branch.list)
    {
      printf ("ERROR: Cannot allocate memory.\n");
      exit (1);
    }

  while (break_wall (labyrinth, current_road, &current_position))
    {
      branch.list[branch.length] = current_position;
      branch.length++;

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
                      clean_checked (labyrinth);
                      branch.length = 0;
                      break;
                    }
                }
              else
                {
                  clean_checked (labyrinth);
                  generate_branches_along (labyrinth, &branch);
                  branch.length = 0;
                  break;
                }
            }
        }
    }
  max_road_length += current_road->length;
  /*  free (current_road->fork_list);*/
  /*free (branch.list);*/
}

int go_ahead (map *labyrinth, road *current_road, coordinate *current_position_p)
{
  #ifdef __DEBUG
  printf ("go_ahead\n");
  #endif
  int i;
  coordinate next_position;
  /* Choose a way randomly */
  reset_random_direction ();
  for (i = 0; i < TOTAL_DIRECTIONS; i++)
    {
      next_position = get_adjacent (*current_position_p, get_random_direction ());
      if (is_in_map (labyrinth, next_position) && get_land_type (labyrinth, next_position) == WALL && get_land_timestamp (labyrinth, next_position) > get_land_timestamp (labyrinth, *current_position_p))
        break;
    }
  if (i == TOTAL_DIRECTIONS)
    /* No way */
    return 0;

  set_land_type (labyrinth, next_position, ROAD);
  set_land_timestamp (labyrinth, next_position, get_timestamp ());
  generate_walls_around (labyrinth, next_position);
  current_road->list[current_road->length] = next_position;
  current_road->length++;
  *current_position_p = next_position;
  return 1;
}

int go_back (map *labyrinth, road *current_road, coordinate *current_position_p)
{
  #ifdef __DEBUG
  printf ("go_back\n");
  #endif
  int i;
  coordinate position;
  if (current_road->length == 1)
    /* No way to go back */
    return 0;

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

  current_road->length--;
  position = current_road->list[current_road->length - 1];

  set_land_type (labyrinth, *current_position_p, CHECKED);
  set_land_timestamp (labyrinth, *current_position_p, get_timestamp ());
  *current_position_p = position;


  return 1;
}

void generate_walls_around (map *labyrinth, coordinate position)
{
  #ifdef __DEBUG
  printf ("generate_walls_around\n");
  #endif
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
  #ifdef __DEBUG
  printf ("break_wall\n");
  #endif
  int i;
  coordinate current_position, next_position;

  if (current_road->length <= SHORT_ROAD)
    {
      if (!probability_event (PROBABILITY_FORK_SHORT_ROAD))
        return 0;
    }
  else if (!probability_event (fall_function ((double) current_road->forks / (double) current_road->length)))
    /* Done. */
    return 0;

  if (current_road->forks == 0)
    {
      weighted_init (current_road->length);
    }

  while (1)
    {
      int index;
      index = weighted_get ();
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
              weighted_adjust (index);
              generate_walls_around (labyrinth, next_position);
              *position_p = next_position;
              return 1;
            }
        }

      if (i == TOTAL_DIRECTIONS)
        {
          if (!weighted_remove (index))
            return 0;
        }
    }
}


static double sum;
static double *weight_list = NULL;
static int count;
void weighted_init (int new_count)
{
  int i;
  count = new_count;
  if (!weight_list)
    free (weight_list);
  weight_list = (double *) malloc (sizeof (double) * count);
  if (!weight_list)
    {
      printf ("ERROR: Cannot allocate memory.\n");
      exit (1);
    }

  /* Ignore two ends */
  weight_list[0] = 0.0;
  weight_list[count - 1] = 0.0;

  for (i = 1; i < count - 1; i++)
    {
      weight_list[i] = 1.0;
    }
  sum = (double) count - 2.0;
}

int weighted_get ()
{
  int i;
  double num;
  double current_sum;
  current_sum = 0.0;
  num = sum * (double) rand () / (double) RAND_MAX;
  for (i = 0; i < count; i++)
    {
      current_sum += weight_list[i];
      if (current_sum > num)
        break;
    }
  if (i >= count -1)
    i = count - 2;
  return i;
}

void weighted_adjust (int index)
{
  int i;
  sum -= weight_list[index] * FORK_AGAIN_REDUCTION;
  weight_list[index] = weight_list[index] - FORK_AGAIN_REDUCTION * weight_list[index];
  for (i = -2; i <= 2; i++)
    {
      if (i != 0 && index + i >= 0 && index + i < count)
        {
          sum -= weight_list[index + i] * ADJACENT_FORK_AGAIN_REDUCTION;
          weight_list[index + i] = weight_list[index + i] - ADJACENT_FORK_AGAIN_REDUCTION * weight_list[index + i];
        }
    }
}

/* If all weight in the list falls to zero, return 0. */
int weighted_remove (int index)
{
  sum -= weight_list[index];
  weight_list[index] = 0.0;
  if (sum < MIN_PROBABILITY_PRECISION)
    return 0;
  else
    return 1;
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
