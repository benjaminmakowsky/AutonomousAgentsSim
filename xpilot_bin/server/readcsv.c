#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

int main(void)
{
  FILE *fp = fopen("points.csv", "r");

  char buff[255];
  fgets(buff, 255, fp);

  int length = atoi(buff);
  int points[length][2];
  char *tok;

  int i;
  for(i = 0; i < length; i++)
  {
    fgets(buff, 255, fp);

    tok = strtok(buff, ",");
    points[i][0] = atoi(tok);

    tok = strtok(NULL, ",");
    points[i][1] = atoi(tok);

    //printf("X: %d\tY: %d\n", points[i][0], points[i][1]);
  }

  return 0;
}
