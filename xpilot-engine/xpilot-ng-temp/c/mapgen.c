//Matthew Coffman - May 2018
//Compile: ./buildChaser.sh && export LD_LIBRARY_PATH=.
//Run: ./runChaser.sh <n> for some natural number n (defaults to 1 if no number given)
#include "cAI.h"
#include <sys/time.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <limits.h>

//drones can be initializing, flying around, engaging, or dead
enum State
{
  STATE_COMPUTING,
  STATE_INIT,
  STATE_DEAD
};

typedef struct point
{
  int id;
  int x;
  int y;
} point_t;

typedef struct edge
{
  int id1;
  int x1;
  int y1;
  int id2;
  int x2;
  int y2;
} edge_t;

//global constants
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define BUFFER_SIZE 255

//global variables
int frameCount = 0;			//keeps track of time by counting frames
bool init = false;			//not initialized for the first 40 frames
int state = STATE_INIT;			//the current state of our drone (enum above)

//argv params
int idx;
char *filename;

//initialization: spends the first 40 frames turning to the desired angle, and then
//declares initialization complete and switches to the NOENEMY state
void Initialize()
{
  init = true;
  state = STATE_COMPUTING;
}

void Compute()
{
  FILE *fp = fopen(filename, "r");
  char buf[BUFFER_SIZE];
  int num_points;
  int i, j;

  fgets(buf, sizeof(buf), fp);

  char *tok = strtok(buf, " ");
  num_points = atoi(tok);

  point_t vertices[num_points];
  
  i = -1;
  while(++i < num_points)
  {
    fgets(buf, sizeof(buf), fp);

    char *tok = strtok(buf, " ");
    vertices[i].id = i+1;
    vertices[i].x = atoi(tok);

    tok = strtok(NULL, " ");
    vertices[i].y = atoi(tok);
  }

  fclose(fp);

  int num_edges_possible = num_points * (num_points-1) / 2;
  int num_edges = 0;
  edge_t edges[num_edges_possible];

  for(i = 0; i < num_points-1; i++)
  {
    int x1 = vertices[i].x;
    int y1 = vertices[i].y;

    for(j = i+1; j < num_points; j++)
    {
      int x2 = vertices[j].x;
      int y2 = vertices[j].y;
      
      if(!wallBetween(x1, y1, x2, y2, 1, 1))
      {
        edges[num_edges].id1 = vertices[i].id;
        edges[num_edges].x1 = vertices[i].x;
        edges[num_edges].y1 = vertices[i].y;
        edges[num_edges].id2 = vertices[j].id;
        edges[num_edges].x2 = vertices[j].x;
        edges[num_edges].y2 = vertices[j].y;
        num_edges++;
      }
    }
  }

  fp = fopen("map.csv", "w");
  
  fprintf(fp, "%d\n", num_points);
  
  for(i = 0; i < num_points; i++)
    fprintf(fp, "%d %d %d\n", vertices[i].id, vertices[i].x, vertices[i].y);

  fprintf(fp, "%d\n", num_edges);

  for(i = 0; i < num_edges; i++)
    fprintf(fp, "%d %d %d %d %d %d\n", edges[i].id1, edges[i].x1, edges[i].y1, edges[i].id2, edges[i].x2, edges[i].y2);

  printf("\n\n\n/****************** MAP GENERATION COMPLETE *******************/\n\n\n");

  //exit(0);
}

//this is where the magic happens (i.e., this loop fires every frame and decides
//which state the chaser is currently in and acts accordingly)
AI_loop()
{
  //by default, don't thrust, and increment the frame counter
  thrust(0);
  frameCount = MIN(frameCount + 1, INT_MAX);
 
  switch(state)
  {
    case(STATE_INIT):
      Initialize();
      break;

    case(STATE_COMPUTING):
      if(frameCount == 100)
        Compute();
      break;

    case(STATE_DEAD):
      state = STATE_COMPUTING;
      break;
  }
}


//gets initial input and starts the AI loop
int main(int argc, char *argv[]) 
{
  idx = strtol(argv[2], NULL, 10);
  filename = argv[3];

  printf("idx: %d\n", idx);
  printf("filename: %s\n", filename);
  
  return start(argc, argv);
}
