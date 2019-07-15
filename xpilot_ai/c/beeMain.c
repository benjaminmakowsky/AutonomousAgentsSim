//
// Created by makowskyb on 7/15/19.
//

#include "beeMain.h"
#include "beeAI.h"
#include "bee.h"
#include "boids.h"
#include <ctype.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <limits.h>


//Declares initialization complete and switches to the NOENEMY state
void begin()
{
  //Generate a random initial heading.
  if(degToAim < 0)
  {
    pVector = degToAim = rand() % 360;
  }

  //Turn to the random initial angle.
  turnToDeg(degToAim);

  //Check if we want to follow the leader, or if this is just normal boids.
  if(leaderMode)
  {
    if(idx < 3)
    {
      isLeader = true;
    }
  }

  //Declare initialized and set state to typical flying.
  init = true;
  state = STATE_SEARCHING;
  thrust(1);
}

AI_loop()
{

  //Increment the frame counter.
  frameCount = (frameCount + 1) % INT_MAX;

  //Check the input message buffer to adjust behavior as necessary.
  handleMsgBuffer();

  //Check if we are dead.
  if(!selfAlive())
  {
    state = STATE_DEAD;
  }

  //If we haven't initialized yet, do that.
  if(!init)
  {
    state = STATE_INIT;
  }

  switch(state)
  {
    case(STATE_INIT):
      begin();
      break;

    case(STATE_FLYING):
      flocking();
      break;

    case(STATE_DEAD):
      state = STATE_SEARCHING;
      break;

    case(STATE_SEARCHING):
      searching();
      break;

    case(STATE_FORAGING):
      forage();
      break;

    default:
      state = STATE_SEARCHING;
  }
}

/*****************************************************************************
 * Main (get cmd line info and start AI)
 * ***************************************************************************/

int main(int argc, char *argv[])
{
  //Get info on my idx, tot_idx, and my team number.
  idx = strtol(argv[2], NULL, 10);
  tot_idx = strtol(argv[3], NULL, 10);
  teamNum = strtol(argv[4], NULL, 10);

  //Seed the random number generator.
  srand(time(NULL));

  //Start the AI loop.
  return start(argc, argv);
}