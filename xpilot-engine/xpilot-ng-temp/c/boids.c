//Matthew Coffman - May 2018
//This C document is intended as a controller for drones that will flock
//in a manner guided by boid's algorithm. Based largely off of defender.c.
//We will also assume that we're working with drones only, whether they
//are fixed or quad.

// Copied includes over from defender.c, just in case
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
#define PI 3.1415926536

//Since this controller just simulates swarming behavior, we have no need
//to incorporate positions, death, etc. A drone will initialize and then fly
//around as guided by boid's algorithm.
enum State{
  STATE_INIT,
  STATE_READY
};

//Don't think we need this, but I'll include it just in case.
struct pair{
  int first;
  int second;
};

//global variables
bool init = false;
int degToMov;
int state = STATE_INIT;
int angleToTurn;
int state;
//argv params
int idx;
int tot_idx;

const char *printState(enum State s){
  switch(s){
    case STATE_INIT : return "INIT";
    case STATE_READY : return "READY";
  }
}

int distanceFormula(int x1, int x2, int y1, int y2){
  return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

void Initialize(){
  degToMov = rand() % 360;
  state = STATE_READY;
};


AI_loop() {
    //Calculate neighbors' average position
    //Calculate neighbors' average heading
    //Calculate aversion vector(?)
    //Somehow average/scale these vectors
    //Thrust
}

int main(int argc, char *argv[]) {
  idx = strtol( argv[2], NULL, 10 );
  tot_idx = strtol( argv[3], NULL, 10 );

  printf( "idx: %d\n", idx );
  printf( "tot_idx: %d\n", tot_idx );

  return start(argc, argv);
}




