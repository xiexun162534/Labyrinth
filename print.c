#include "print.h"

void print (map *labyrinth)
{
  int i, j;
  for (j = 0; j < labyrinth->height; j++)
    {
      for (i = 0; i < labyrinth->width; i++)
        {
          coordinate position;
          position.x = i;
          position.y = j;
          switch (get_land_type (labyrinth, position))
            {
            case ROAD:
              putchar ('R');
              break;
            case WALL:
              putchar ('W');
              break;
            case LAND:
              putchar ('L');
              break;
            case ENTRANCE:
              putchar ('I');
              break;
            case EXIT:
              putchar ('O');
              break;
            default:
              putchar ('D');
            }
        }
      putchar ('\n');
    }
}
