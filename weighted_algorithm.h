#ifndef __WEIGHTED_ALGORITHM_H
#define __WEIGHTED_ALGORITHM_H

typedef struct __WEIGHTED_DATA
{
  double sum;
  double *weight_list;
  int count;
} weighted_data;


weighted_data *weighted_init (int count);

int weighted_get (weighted_data *current_data);

void weighted_adjust (weighted_data *current_data, int index);

int weighted_remove (weighted_data *current_data, int index);


#endif
