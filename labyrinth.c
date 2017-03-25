#include "labyrinth.h"
#include "map.h"
#include "generate.h"
#include "print.h"

int main (void)
{
  map *labyrinth;
  coordinate entrance_position = {0,0}, exit_position = {999, 999};
  labyrinth = generate (1000, 1000, entrance_position, exit_position);
  print (labyrinth);

  exit (0);
}
