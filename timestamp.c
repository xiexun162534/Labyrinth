#include "labyrinth.h"
#include "time.h"

static unsigned long int timestamp = 1;

unsigned long int get_timestamp (void)
{
  if (timestamp == ULONG_MAX)
    {
      printf ("ERROR: Time out.\n");
      exit (2);
    }

  return timestamp++;
}

void reset_timestamp (void)
{
  timestamp = 1;
}
