#include "labyrinth.h"
#include "map.h"
#include "generate.h"
#include "print.h"

int main (void)
{
  map *labyrinth;
  coordinate entrance_position = {0,0}, exit_position = {49, 49};
  labyrinth = generate (50, 50, entrance_position, exit_position);
  print (labyrinth);

  exit (0);
}
