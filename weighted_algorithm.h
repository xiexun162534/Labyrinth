#ifndef __WEIGHTED_ALGORITHM_H
#define __WEIGHTED_ALGORITHM_H

/* Works when count > 2 */
/* build an N-ary tree */
#define N 3
#define PARENT(node) (((node) - 1) / (N))
#define CHILD(i, node) ((node) * (N) + (i))

typedef struct __WEIGHTED_TREE
{
  double *data;
  int count;
  int base;
} weighted_tree;


weighted_tree *weighted_init (int count);

void weighted_destroy (weighted_tree *current_tree);

int weighted_get (weighted_tree *current_tree);

void weighted_adjust (weighted_tree *current_tree, int index);

int weighted_remove (weighted_tree *current_tree, int index);


#endif
