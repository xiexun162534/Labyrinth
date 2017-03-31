#include "labyrinth.h"
#include "weighted_algorithm.h"
#include "generate.h"


weighted_tree *weighted_init (int count)
{
  weighted_tree *current_tree;
  int i, j;
  int base;

  current_tree = malloc (sizeof (weighted_tree));
  current_tree->count = count;

  i = 0;
  j = count;
  while (j > 0)
    {
      j = PARENT (j);
      i = CHILD (1, i);
    }

  base = i;
  current_tree->base = i;

  current_tree->data = (double *) malloc (sizeof (double) * (current_tree->count +  current_tree->base));
  if (!current_tree->data)
    {
      printf ("ERROR: Cannot allocate memory.\n");
      exit (1);
    }

  for (i = 0; i < base + count; i++)
    {
      current_tree->data[i] = 0.0;
    }

  /* Ignore two ends */
  for (i = base + 1; i < base + count - 1; i++)
    {
      j = i;
      while (j >= 0)
        {
          current_tree->data[j] += 1.0;
          j = (j == 0) ? -1 : PARENT (j);
        }
    }

  return current_tree;
}

int weighted_get (weighted_tree *current_tree)
{
  int i;
  int index;
  double num;
  double current_sum;
  num = current_tree->data[0] * (double) rand () / (double) RAND_MAX;
  index = 0;
  current_sum = 0;

  while (index < current_tree->base)
    {
      for (i = 1; i <= N; i++)
        {
          if (current_sum + current_tree->data[CHILD (i, index)] > num)
            {
              index = CHILD (i, index);
              break;
            }
          else
            current_sum += current_tree->data[CHILD (i, index)];
        }
      if (i == N + 1)
        index = CHILD (N, index);
    }

  return index - current_tree->base;
}

void weighted_adjust (weighted_tree *current_tree, int index)
{
  int i;
  int j;
  double delta;

  j = index + current_tree->base;
  delta = FORK_AGAIN_REDUCTION * current_tree->data[j];
  while (j >= 0)
    {
      current_tree->data[j] -= delta;
      j = (j == 0) ? -1 : PARENT (j);
    }

  for (i = -2; i <= 2; i++)
    {
      if (i != 0 && index + i >= 0 && index + i < current_tree->count)
        {
          j = index + i + current_tree->base;
          delta = ADJACENT_FORK_AGAIN_REDUCTION * current_tree->data[j];
          while (j >= 0)
            {
              current_tree->data[j] -= delta;
              j = (j == 0) ? -1 : PARENT (j);
            }
        }
    }
}

/* If all weight in the list falls to zero, return 0. */
int weighted_remove (weighted_tree *current_tree, int index)
{
  int i;
  double delta;
  i = index + current_tree->base;
  delta = current_tree->data[i];
  while (i >= 0)
    {
      current_tree->data[i] -= delta;
      i = (i == 0) ? -1 : PARENT (i);
    }
  
  if (current_tree->data[0] < MIN_PROBABILITY_PRECISION)
    return 0;
  else
    return 1;
}
