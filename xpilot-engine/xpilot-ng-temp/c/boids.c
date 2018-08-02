//Matthew Coffman - May 2018
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
#include "graph.c"
#include "dijkstra.c"
#include "astar.c"
#include "bfs.c"
#include "dfs.c"
#include <sys/time.h>

//macros
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define SIGN(X) ((X) > 0 ? 1 : ((X) < 0 ? -1 : 0))
#define SLOW (frameCount % 5 < 2) ? thrust(1) : thrust(0)

//global constants
#define CORNER_LOOK_AHEAD 75
#define MAX_DEG 360
#define WALL_LOOK_AHEAD 100

//function prototypes
void initialize();
void wallAvoidance();
void alignment();
void cohesion();
void separation();
bool nearCornerPeek();
void noEnemyFlying();
void handleMsg();

//enumeration of drone states
enum State
{
  STATE_INIT,
  STATE_FLYING,
  STATE_DEAD
};

//global variables
bool init = false;		//have we initialized yet
int state = STATE_INIT;		//what state are we in (usually STATE_FLYING)
int frameCount = 0;		//how many frames have elapsed
int degToAim;			//what direction do we want to go
int turnLock = 0;		//time not allowed to compute new wall avoidance
int wallVector = -1;		//where to go to avoid crashing into a wall
int pVector = -1;		//information on the most recent (past) heading	
int pWeight = 0;
int aVector = -1; 		//(friend) alignment variables
int aWeight = 0;
int aRadius = 400;
int cVector = -1; 		//(friend) cohesion variables
int cWeight = 0; 
int cRadius = 400;	
int sVector = -1;		//(friend) separation variables
int sWeight = 0;
int sRadius = 100;	
int eVector = -1;		//(enemy) separation variables
int eWeight = 0;
int eRadius = 100;
int fov = 60;			//field (angle) of vision
int rov = 500;			//range (distance) of vision
int mobile = 1;			//allows us to completely anchor all drones	


/*****************************************************************************
 * Initialization
 * ***************************************************************************/

//Declares initialization complete and switches to the NOENEMY state
void initialize()
{
  int i;

  turnToDeg(degToAim);

  //Declare initialized and set state to no enemies.
  init = true;
  state = STATE_FLYING;
  thrust(0);
}


/*****************************************************************************
 * Wall Avoidance
 * ***************************************************************************/

//Provides a mechanism for drones to spot walls ahead and steer away from them.
void wallAvoidance()
{
  double currHeadingRad;
  int currHeadingDeg;
  int currX, currY, newX, newY, delX, delY;
  int lHead, rHead;
  bool seeWallX, seeWallY, seeWallAhead;
  bool seeWallL, seeWallR, cornerClose;
  int r;

  //Get the current heading, in degrees and radians, and current position info. 
  currHeadingRad = selfHeadingRad();
  currHeadingDeg = (int)radToDeg(currHeadingRad);
  currX = selfX();
  currY = selfY(); 

  //Check if we will crash into a wall if we carry on the current course for some
  //distance.
  delX = (int)(WALL_LOOK_AHEAD * cos(currHeadingRad)); 
  delX = SIGN(delX) * MAX(abs(delX), 50);
  delY = (int)(WALL_LOOK_AHEAD * sin(currHeadingRad)); 
  delY = SIGN(delY) * MAX(abs(delY), 50);
  
  newX = currX + delX;
  newY = currY + delY;

  //Check straight in front, and then check the x and y directions individually.
  seeWallAhead = wallBetween(currX, currY, newX, newY, 1, 1);
  seeWallX = wallBetween(currX, currY, newX, currY, 1, 1);
  seeWallY = wallBetween(currX, currY, currX, newY, 1, 1);

  //Check 15 degrees to the left and right of the current heading.
  lHead = modm(currHeadingDeg + 15, MAX_DEG);
  seeWallL = wallFeeler(WALL_LOOK_AHEAD, lHead, 1, 1);
  rHead = modm(currHeadingDeg - 15, MAX_DEG);
  seeWallR = wallFeeler(WALL_LOOK_AHEAD, rHead, 1, 1);
 
  //Finally, check if we are close to one of the corners of the map.
  cornerClose = (wallFeeler(CORNER_LOOK_AHEAD, 0, 1, 1) 
                   || wallFeeler(CORNER_LOOK_AHEAD, 180, 1, 1)) 
                && 
                (wallFeeler(CORNER_LOOK_AHEAD, 90, 1, 1) 
                   || wallFeeler(CORNER_LOOK_AHEAD, 270, 1, 1));

  //If turnLock is off, we are allowed to set a new turn angle.
  if(!turnLock)
  {
    //If we are close to a corner...
    if(cornerClose)
    {
      //Turn somewhere between 65 and 115 degrees from the current heading.
      r = 0; //(rand() % 50) - 25;
      wallVector = modm(currHeadingDeg + 90 + r, MAX_DEG);

      //Set a turnLock of X frames: we will not be able to set a new wall avoidance
      //angle for X frames, whatever X happens to be.
      turnLock = 5;
      SLOW;
    }

    //If we see a vertical wall, mirror our heading on the y-axis, and set a turnLock
    //of 5 frames.
    else if(seeWallX)
    {
      wallVector = modm(180 - currHeadingDeg, MAX_DEG);
      turnLock = 5;
    }

    //If we see a horizontal wall, mirror our heading on the x-axis, and set a turnLock
    //of 5 frames.
    else if(seeWallY)
    {
      wallVector = modm(-currHeadingDeg, MAX_DEG);
      turnLock = 5;
    }
    
    //If the wall isn't perfectly horizontal or vertical, turn a little bit
    //(note that this solution is not optimal, still bumps into walls occasionally),
    //and set a turnLock of 5 frames.
    else if(seeWallAhead || seeWallR)
    {
      wallVector = modm(currHeadingDeg + 90, MAX_DEG);
      turnLock = 5;      
    }

    else if(seeWallL)
    {
      wallVector = modm(currHeadingDeg - 90, MAX_DEG);
      turnLock = 5;
    }

    //If we see no walls at all, indicate this by a value of -1 for wall avoidance.
    else
    {
      wallVector = -1;
    }
  }
  //If there's a turnLock on, count down.
  else
  {
    turnLock--;
  }
}


/*****************************************************************************
 * Friendly Alignment 
 * ***************************************************************************/

//Compute the angle to stay aligned with all friends within a certain radius.
void alignment()
{
  aVector = avgFriendlyDir(aRadius, fov); 
}
 

/*****************************************************************************
 * Friendly Cohesion
 * ***************************************************************************/

//Compute the angle toward the average location of all friends within a certain radius.
void cohesion()
{
  int avgFriendX = averageFriendRadarX(cRadius, fov);
  int avgFriendY = averageFriendRadarY(cRadius, fov);
 
  if(avgFriendX != -1 && avgFriendY != -1)
  {
    cVector = getAngle(selfX(), avgFriendX, selfY(), avgFriendY);
  }
  else
  {
    cVector = -1;
  }
}


/*****************************************************************************
 * Friend Separation
 * ***************************************************************************/

//Compute the angle to stay away from the average position of all friends nearby.
void separation()
{
  int avgFriendX = averageFriendRadarX(sRadius, fov);
  int avgFriendY = averageFriendRadarY(sRadius, fov);
  int sepVec = getAngle(selfX(), avgFriendX, selfY(), avgFriendY);

  if(avgFriendX != -1 && avgFriendY != -1)
  {
    sVector = modm(sepVec + 180, 360);
  }
  else
  {
    sVector = -1;
  }
}


/*****************************************************************************
 * Enemy Separation
 * ***************************************************************************/

//Compute the angle to stay away from the average position of all friends nearby.
void enemySeparation()
{
  int avgEnemyX = averageEnemyRadarX(eRadius, fov);
  int avgEnemyY = averageEnemyRadarY(eRadius, fov);
  int sepVec = getAngle(selfX(), avgEnemyX, selfY(), avgEnemyY);

  if(avgEnemyX != -1 && avgEnemyY != -1)
  {
    eVector = modm(sepVec + 180, 360);
  }
  else
  {
    eVector = -1;
  }
}


/*****************************************************************************
 * No Enemy Flying (Controller)
 * ***************************************************************************/

//TODO: find a way to balance wa ll avoidance, neighbor cohesion, and other vectors
void noEnemyFlying()
{
  static int numVec, totalVec;

  numVec = totalVec = 0;
  
  if(frameCount % 14 == 0)
  {
    pVector = degToAim;
  }

  wallAvoidance();
  alignment();
  cohesion();
  separation();  
  enemySeparation();

  //If there is a wall nearby, aim away from it. Wall avoidance takes precedence over
  //all the other vectors below.
  if(wallVector != -1)
  {
    degToAim = wallVector;
  }
  //If this drone is a follower rather than a leader, enact boids behavior.
  else 
  { 
    //Get the alignment vector and weight it.
    if(aVector != -1)
    {
      totalVec += aVector * aWeight;
      numVec += aWeight;
    }
    
    //Get the cohesion vector and weight it.
    if(cVector != -1)
    {
      totalVec += cVector * cWeight;
      numVec += cWeight;
    }

    //Get the friend separation vector and weight it.
    if(sVector != -1)
    {
      totalVec += sVector * sWeight;
      numVec += sWeight;
    }
    
    //Get the enemy separation vector and weight it.
    if(eVector != -1)
    {
      totalVec += eVector * eWeight;
      numVec += eWeight;
    }
   
    totalVec += pVector * pWeight;
    numVec += pWeight; 

    //Compute the weighted average of the four vectors above, and point in that
    //direction.
    if(numVec > 0)
    {
      degToAim = totalVec / numVec;
    }
  }

  //Whatever direction we've decided to turn to, do so.
  turnToDeg(degToAim);

  //Now that we've turned, we can decide how much to thrust.
  if(mobile)
  {
    thrust(1);
  }
  //If we're especially anchored, just don't move.
  else
  {
    thrust(0);
  }
}


/*****************************************************************************
 * Message Handler (through chat feature) 
 * ***************************************************************************/

//Handle whatever messages pop up through the chat feature.
void handleMsg()
{
  static char buf[100];
  static char *tok = NULL;
  static char *thisMsg = "";
  static int num = 0;
 
  if(strcmp(thisMsg, scanMsg(0)))
  {
    thisMsg = scanMsg(0);
    
    strcpy(buf, thisMsg);
    tok = strtok(buf, " ");
    if(tok)
    {
      num = atoi(strtok(NULL, " "));
    }

    if(!strcmp(tok, "mobile"))
    {
      mobile = num; 
    }
    else if(!strcmp(tok, "aweight"))
    {
      aWeight = num; 
    }
    else if(!strcmp(tok, "aradius"))
    {
      aRadius = num; 
    }
    else if(!strcmp(tok, "cweight"))
    {
      cWeight = num; 
    }
    else if(!strcmp(tok, "cradius"))
    {
      cRadius = num;
    }
    else if(!strcmp(tok, "sweight"))
    {
      sWeight = num;
    }
    else if(!strcmp(tok, "sradius"))
    {
      sRadius = num;
    }
    else if(!strcmp(tok, "eweight"))
    {
      eWeight = num; 
    }
    else if(!strcmp(tok, "eradius"))
    {
      eRadius = num; 
    }
    else if(!strcmp(tok, "pweight"))
    {
      pWeight = num; 
    }
    else if(!strcmp(tok, "fov"))
    {
      fov = num; 
    }
    else if(!strcmp(tok, "rov"))
    {
      rov = num; 
    }
  }
}


/*****************************************************************************
 * AI Loop
 * ***************************************************************************/

//this is where the magic happens (i.e., this loop fires every frame and decides
//which state the chaser is currently in and acts accordingly)
AI_loop()
{
  //by default, don't thrust, and increment the frame counter
  frameCount = (frameCount + 1) % INT_MAX;

  //also by default, assume no enemy is nearby
  state = STATE_FLYING;

  //check the input buffer to try changes in behavior
  handleMsg();

  //check if we are dead
  if(!selfAlive())
  {
    state = STATE_DEAD;
  }

  //if we haven't initialized yet
  if(!init)
  {
    state = STATE_INIT;
  }

  thrust(0);
 
  switch(state)
  {
    case(STATE_INIT):
      initialize();
      break;

    case(STATE_FLYING):
      noEnemyFlying();
      break;

    case(STATE_DEAD):
      state = STATE_FLYING;
      break;
  }
}


/*****************************************************************************
 * Main (get cmd line info and start AI) 
 * ***************************************************************************/

int main(int argc, char *argv[]) 
{
  //generate a random initial heading
  srand(time(NULL));
  degToAim = rand() % 360;

  return start(argc, argv);
}
