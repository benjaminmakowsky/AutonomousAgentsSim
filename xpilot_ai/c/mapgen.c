//Matthew Coffman - May 2018
//Compile: ./buildMapgen.sh && export LD_LIBRARY_PATH=.
//Run:     ./runMapgen.sh <n> <points_file> for some natural number n (defaults to 1)
//         and some file containing the needed points information
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

//global constants and macros
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define BUFFER_SIZE 255
#define OFFSET 25

//drones can be initializing, flying around, engaging, or dead
enum State
{
  STATE_COMPUTING,
  STATE_INIT,
  STATE_DEAD
};

//point_t structure: stores an id and x and y coordinates
typedef struct point
{
  int id;
  int x;
  int y;
} point_t;

//edge_t structure: stores info on both vertices needed to make the edge
typedef struct edge
{
  int id1;
  int x1;
  int y1;
  int id2;
  int x2;
  int y2;
} edge_t;

//global variables
int frameCount = 0;			//keeps track of time by counting frames
bool init = false;			
int state = STATE_INIT;			//the current state of our drone (enum above)

//argv params
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

  //after opening the file, from the first line get the number of points that will
  //be stored in the graph
  fgets(buf, sizeof(buf), fp);
  char *tok = strtok(buf, " ");
  num_points = atoi(tok);

  //loop through the next N lines of the given file, where N is the number of points,
  //and store each line in an array of points, where a point has an id, an x, and a y
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

  //after reading in all the points, close the file
  fclose(fp);

  //for any graph, there can be at most N choose 2 edges, where N is the number of
  //points in the graph
  int num_edges_possible = num_points * (num_points-1) / 2;
  int num_edges = 0;
  edge_t edges[num_edges_possible];

  //for each possible pairing of points in the graph, determine whether there's a wall
  //between them, and if not, store the resulting edge in an array of edges
  for(i = 0; i < num_points-1; i++)
  {
    int x1 = vertices[i].x;
    int y1 = vertices[i].y;

    for(j = i+1; j < num_points; j++)
    {
      int x2 = vertices[j].x;
      int y2 = vertices[j].y;
     
      int dx = x1 - x2;
      int dy = y1 - y2;
      bool offsetWorks = true;

      //we want to know whether there's a wall between or close to the line segment
      //between the two points in question, so, given the x and y coordinates of the
      //two points, we check the area around the line segment between them by adding
      //or subtracting some offset value. We will change how this offset value is
      //used based on the slope of the line segment.
      //if dx * dy > 0, the line segment in question has a positive slope
      if(dx * dy > 0)
      {
        offsetWorks = !wallBetween(x1+OFFSET, y1-OFFSET, x2+OFFSET, y2-OFFSET, 1, 1)
                      && !wallBetween(x1-OFFSET, y1+OFFSET, x2-OFFSET, y2+OFFSET, 1, 1);
      }
      //if dx * dy < 0, the line segment has a negative slope
      else if(dx * dy < 0)
      {
        offsetWorks = !wallBetween(x1+OFFSET, y1+OFFSET, x2+OFFSET, y2+OFFSET, 1, 1)
                      && !wallBetween(x1-OFFSET, y1-OFFSET, x2-OFFSET, y2-OFFSET, 1, 1);
      }
      //if !dx, no change in x indicates a vertical line segment
      else if(!dx)
      {
        offsetWorks = !wallBetween(x1-OFFSET, y1, x2-OFFSET, y2, 1, 1)
                      && !wallBetween(x1+OFFSET, y1, x2+OFFSET, y2, 1, 1);
      }
      //if !dy, the segment is horizontal
      else if(!dy)
      {
        offsetWorks = !wallBetween(x1, y1-OFFSET, x2, y2-OFFSET, 1, 1)
                      && !wallBetween(x1, y1+OFFSET, x2, y2+OFFSET, 1, 1);
      }
      //if we somehow get here, something clearly has gone wrong, so quit with an error
      else
      {
        perror("ERROR: something went wrong with wall-checks");
        exit(1);
      }
 
      //if there's no wall between the two points or even nearby, add an edge
      if(offsetWorks && !wallBetween(x1, y1, x2, y2, 1, 1))
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

  //open a new file, called map.csv, where we'll store our map info for drones to use
  fp = fopen("map.csv", "w");
  
  //add all the point info to the file
  fprintf(fp, "%d\n", num_points);
  for(i = 0; i < num_points; i++)
  {
    fprintf(fp, "%d %d %d\n", vertices[i].id, vertices[i].x, vertices[i].y);
  }

  //add the edge info
  fprintf(fp, "%d\n", num_edges);
  for(i = 0; i < num_edges; i++)
  {
    fprintf(fp, "%d %d %d %d %d %d\n", 
                edges[i].id1, edges[i].x1, edges[i].y1, 
                edges[i].id2, edges[i].x2, edges[i].y2);
  }

  //include a print statement indicating completion of the map.csv file, so the user
  //knows to quit this mapgen drone
  printf("\n\n\n/****************** MAP GENERATION COMPLETE *******************/\n\n\n");
}


//this is where the magic happens (i.e., this loop fires every frame and decides
//which state the chaser is currently in and acts accordingly)
AI_loop()
{
  frameCount = MIN(frameCount + 1, INT_MAX);
  thrust(0);
 
  switch(state)
  {
    //before anything else, initialize
    case(STATE_INIT):
      Initialize();
      break;

    //after everything is initialized correctly, generate the map
    case(STATE_COMPUTING):
      if(frameCount == 50)
      {
        Compute();
      }
      break;

    //if we ever get to this state, something went wrong
    case(STATE_DEAD):
      state = STATE_COMPUTING;
      break;
  }
}


//gets initial input and starts the AI loop
int main(int argc, char *argv[]) 
{
  filename = argv[3];
  printf("filename: %s\n", filename);
  
  return start(argc, argv);
}
