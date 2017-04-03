#include "labyrinth.h"
#include "timestamp.h"
#include "generate.h"


#ifdef __DEBUG
#include "print.h"
#endif



static road main_road;
static int max_road_length;
map *generate (int width, int height, coordinate entrance_position, coordinate exit_position)
{
  extended_map *extended_labyrinth;
  map *labyrinth;
  extended_labyrinth = malloc (sizeof (extended_map));
  labyrinth = create_empty_map (width, height);
  extended_labyrinth->labyrinth = labyrinth;
  extended_labyrinth->checked_list = malloc (sizeof (coordinate) * width * height);
  if (!extended_labyrinth->checked_list)
    {
      printf ("ERROR: Cannot allocate memory.\n");
      exit (1);
    }
  extended_labyrinth->checked_count = 0;
  set_entrance (labyrinth, entrance_position);
  set_exit (labyrinth, exit_position);

  reset_timestamp ();

  #ifdef __DEBUG_LESS
  {
    clock_t seed = clock ();
    seed = 187;
    printf ("WATCH HERE: %ld!!!!!!!!!!!!!!!!!!!\n", seed);
    srand (seed);
  }
  #else
  srand (clock ());
  #endif

  max_road_length = width * height;
  main_road.length = 0;
  main_road.list = (coordinate *) malloc (sizeof (coordinate) * max_road_length);
  if (!main_road.list)
    {
      printf ("ERROR: Cannot allocate memory.\n");
      exit (1);
    }
  generate_main_road (extended_labyrinth);

  /* Ignore entrance and exit */
  set_land_type (labyrinth, labyrinth->entrance_position, ROAD);
  set_land_type (labyrinth, labyrinth->exit_position, ROAD);
  generate_branches_along (extended_labyrinth, &main_road);
  /* Mark entrance and exit */
  set_land_type (labyrinth, labyrinth->entrance_position, ENTRANCE);
  set_land_type (labyrinth, labyrinth->exit_position, EXIT);

  free (main_road.list);
  free (extended_labyrinth->checked_list);
  free (extended_labyrinth);

  return labyrinth;
}

void generate_main_road (extended_map *extended_labyrinth)
{
  coordinate current_position, next_position;
  int i;
  map *labyrinth = extended_labyrinth->labyrinth;

  /* Go from entrance */
  current_position = labyrinth->entrance_position;
  set_land_timestamp (labyrinth, current_position, get_timestamp ());
  generate_walls_around (extended_labyrinth, current_position);
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
              generate_walls_around (extended_labyrinth, next_position);
              main_road.list[main_road.length] = next_position;
              main_road.length++;
              clean_checked (extended_labyrinth);
              return ;
            }
        }

      /* Choose a way randomly */
      if (!go_ahead (extended_labyrinth, &main_road, &current_position))
        /* There's no way to go. We have to go back. */
        {
          if (!go_back (extended_labyrinth, &main_road, &current_position))
            {
              printf ("ERROR: Are you kidding?\n");
              exit (3);
            }
        }
    }
}


void generate_branches_along (extended_map *extended_labyrinth, road *current_road)
{
  road branch;
  coordinate current_position;
  map *labyrinth = extended_labyrinth->labyrinth;
  max_road_length -= current_road->length;
  current_road->forks = 0;
  current_road->weighted = weighted_init (current_road->length);
  /*current_road->fork_list = (int *) malloc (sizeof (int) * TOTAL_DIRECTIONS * current_road->length);*/
  branch.length = 0;
  branch.list = current_road->list + current_road->length;

  while (break_wall (extended_labyrinth, current_road, &current_position))
    {
      branch.list[branch.length] = current_position;
      branch.length++;

      while (probability_event (PROBABILITY_GO_AHEAD))
        {
          #ifdef __DEBUG
          print (labyrinth);
          #endif

          if (!go_ahead (extended_labyrinth, &branch, &current_position))
            {
              if (probability_event (PROBABILITY_GO_BACK))
                {
                  if (!go_back (extended_labyrinth, &branch, &current_position))
                    {
                      clean_checked (extended_labyrinth);
                      branch.length = 0;
                      break;
                    }
                }
              else
                {
                  clean_checked (extended_labyrinth);
                  generate_branches_along (extended_labyrinth, &branch);
                  branch.length = 0;
                  break;
                }
            }
        }
    }
  max_road_length += current_road->length;
  /*free (current_road->fork_list);*/
  weighted_destroy (current_road->weighted);
}

int go_ahead (extended_map *extended_labyrinth, road *current_road, coordinate *current_position_p)
{
  #ifdef __DEBUG
  printf ("go_ahead\n");
  #endif
  int i;
  coordinate next_position;
  map *labyrinth = extended_labyrinth->labyrinth;
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
  generate_walls_around (extended_labyrinth, next_position);
  current_road->list[current_road->length] = next_position;
  current_road->length++;
  *current_position_p = next_position;
  return 1;
}

int go_back (extended_map *extended_labyrinth, road *current_road, coordinate *current_position_p)
{
  #ifdef __DEBUG
  printf ("go_back\n");
  #endif
  int i;
  coordinate position;
  map *labyrinth = extended_labyrinth->labyrinth;
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
  extended_labyrinth->checked_list[extended_labyrinth->checked_count] = *current_position_p;
  extended_labyrinth->checked_count++;

  *current_position_p = position;

  return 1;
}

void generate_walls_around (extended_map *extended_labyrinth, coordinate position)
{
  #ifdef __DEBUG
  printf ("generate_walls_around\n");
  #endif
  int i;
  map *labyrinth = extended_labyrinth->labyrinth;
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

void clean_checked (extended_map *extended_labyrinth)
{
  int i, j;
  coordinate position, next_position;
  map *labyrinth = extended_labyrinth->labyrinth;

  for (i = 0; i < extended_labyrinth->checked_count; i++)
    {
      position = extended_labyrinth->checked_list[i];
      for (j = 0; j < TOTAL_DIRECTIONS; j++)
        {
          next_position = get_adjacent (position, j);
          if (is_in_map (labyrinth, next_position) && (get_land_type (labyrinth, next_position) == ROAD || get_land_type (labyrinth, next_position) == ENTRANCE || get_land_type (labyrinth, next_position) == EXIT))
            {
              set_land_type (labyrinth, position, WALL);
              set_land_timestamp (labyrinth, position, get_timestamp ());
              break;
            }
        }
      if (j == TOTAL_DIRECTIONS)
        {
          set_land_type (labyrinth, position, LAND);
          set_land_timestamp (labyrinth, position, 0);
        }
    }
  extended_labyrinth->checked_count = 0;
}

/* Determine whether we have to break a wall alongside the road. If we determine to break a wall, get the position of wall and return 1. Otherwise, return 0. */
int break_wall (extended_map *extended_labyrinth, road *current_road, coordinate *position_p)
{
  #ifdef __DEBUG
  printf ("break_wall\n");
  #endif
  int i;
  coordinate current_position, next_position;
  map *labyrinth = extended_labyrinth->labyrinth;

  if (current_road->length <= 2)
    /* Road too short */
    {
      return 0;
    }
  else if (current_road->length <= SHORT_ROAD)
    {
      if (!probability_event (PROBABILITY_FORK_SHORT_ROAD))
        return 0;
    }
  else if (!probability_event (fall_function ((double) current_road->forks / ((double) current_road->length * 2)) / fall_function (((double) current_road->forks - 1) / ((double) current_road->length * 2))))
    /* Done. */
    return 0;


  while (1)
    {
      int index;
      index = weighted_get (current_road->weighted);
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
              weighted_adjust (current_road->weighted, index);
              generate_walls_around (extended_labyrinth, next_position);
              *position_p = next_position;
              return 1;
            }
        }

      if (i == TOTAL_DIRECTIONS)
        {
          if (!weighted_remove (current_road->weighted, index))
            return 0;
        }
    }
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
