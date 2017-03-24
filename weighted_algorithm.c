#include "labyrinth.h"
#include "weighted_algorithm.h"
#include "generate.h"


weighted_data *weighted_init (int count)
{
  weighted_data *current_data;
  int i;

  current_data = malloc (sizeof (weighted_data));
  current_data->count = count;
  current_data->weight_list = (double *) malloc (sizeof (double) * current_data->count);
  if (!current_data->weight_list)
    {
      printf ("ERROR: Cannot allocate memory.\n");
      exit (1);
    }

  /* Ignore two ends */
  current_data->weight_list[0] = 0.0;
  current_data->weight_list[count - 1] = 0.0;

  for (i = 1; i < count - 1; i++)
    {
      current_data->weight_list[i] = 1.0;
    }
  current_data->sum = (double) count - 2.0;
  return current_data;
}

int weighted_get (weighted_data *current_data)
{
  int i;
  double num;
  double current_sum;
  current_sum = 0.0;
  num = current_data->sum * (double) rand () / (double) RAND_MAX;
  for (i = 0; i < current_data->count; i++)
    {
      current_sum += current_data->weight_list[i];
      if (current_sum > num)
        break;
    }
  if (i >= current_data->count -1)
    i = current_data->count - 1;
  return i;
}

void weighted_adjust (weighted_data *current_data, int index)
{
  int i;
  current_data->sum -= current_data->weight_list[index] * FORK_AGAIN_REDUCTION;
  current_data->weight_list[index] = current_data->weight_list[index] - FORK_AGAIN_REDUCTION * current_data->weight_list[index];
  for (i = -2; i <= 2; i++)
    {
      if (i != 0 && index + i >= 0 && index + i < current_data->count)
        {
          current_data->sum -= current_data->weight_list[index + i] * ADJACENT_FORK_AGAIN_REDUCTION;
          current_data->weight_list[index + i] = current_data->weight_list[index + i] - ADJACENT_FORK_AGAIN_REDUCTION * current_data->weight_list[index + i];
        }
    }
}

/* If all weight in the list falls to zero, return 0. */
int weighted_remove (weighted_data *current_data, int index)
{
  current_data->sum -= current_data->weight_list[index];
  current_data->weight_list[index] = 0.0;
  if (current_data->sum < MIN_PROBABILITY_PRECISION)
    return 0;
  else
    return 1;
}
