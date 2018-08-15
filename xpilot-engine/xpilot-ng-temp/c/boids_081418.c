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
int idx;			//what's our unique idx number
int tot_idx;			//how many drones are on our team
int teamNum;			//what team do we belong to
bool init = false;		//have we initialized yet
int state = STATE_INIT;		//what state are we in (usually STATE_FLYING)
int frameCount = 0;		//how many frames have elapsed
int degToAim;			//what direction do we want to go
int turnLock = 0;		//time not allowed to compute new wall avoidance
int wallVector = -1;		//where to go to avoid crashing into a wall
int wWeight = 0;		//weight of wall avoidance relative to other vectors
int pVector = -1;		//information on the most recent (past) heading	
int pWeight = 5;
int aVector = -1; 		//(friend) alignment variables
int aWeight = 0;
int aRadius = 400;
int cVector = -1; 		//(friend) cohesion variables
int cWeight = 0; 
int cRadius = 400;	
int sVector = -1;		//(friend) separation variables
int sWeight = 0;
int sRadius = 200;	
int eVector = -1;		//(enemy) separation variables
int eWeight = 0;
int eRadius = 200;
int fov = 60;			//field (angle) of vision
int mobile = 1;			//allows us to completely anchor all drones
bool isLeader = false;
int *leaders = NULL;
int numLeaders = 0;
bool leaderMode = false;
bool advancedWeighting = false;

/*****************************************************************************
 * Initialization
 * ***************************************************************************/

//Declares initialization complete and switches to the NOENEMY state
void initialize()
{
  turnToDeg(degToAim);

  if(leaderMode)
  {
    if(selfID() % tot_idx == 1)
    {
      isLeader = true;
    }
  
    leaders = malloc(sizeof(int) * tot_idx);
    if(!leaders)
    {
      perror("couldn't allocate leaders memory");
      exit(EXIT_FAILURE);
    }
  }

  //Declare initialized and set state to no enemies.
  init = true;
  state = STATE_FLYING;
  thrust(1);
}


/*****************************************************************************
 * Wall Avoidance
 * ***************************************************************************/

//Provides a mechanism for drones to spot walls ahead and steer away from them.
void wallAvoidance()
{
  static int wallLookAhead = 100;
  static int cornerLookAhead = 75;
  static int minLookAside = 50;
  static int dummyVal = 1;
  static int lookAngle = 15;
  static int lookRight = 0;
  static int lookUp = 90;
  static int lookLeft = 180;
  static int lookDown = 270;
  static int turnLeft = 90;
  static int turnRight = -90;
  static int lockNum = 5;

  double currHeadingRad;
  int currHeadingDeg, currHeadingDegDiv90;
  int currX, currY, newX, newY, delX, delY;
  int lHead, rHead;
  bool seeWallX, seeWallY, seeWallAhead;
  bool seeWallL, seeWallR, closeToCorner;

  //Get the current heading, in degrees and radians, and current position info. 
  currHeadingRad = selfHeadingRad();
  currHeadingDeg = (int)radToDeg(currHeadingRad);
  currX = selfX();
  currY = selfY(); 

  //We want to look ahead some number of pixels (given by wallLookAhead) in some
  //direction (that being our current heading). Split up this vector into its x-
  //and y-components.
  delX = (int)(wallLookAhead * cos(currHeadingRad)); 
  delY = (int)(wallLookAhead * sin(currHeadingRad)); 
  
  //Now that we've computed two vectors corresponding to the x- and y-components
  //of our look-ahead vector, make sure that the magnitude of these two vectors
  //is at least some minimum value. Even if we're flying at a heading of 0 degrees
  //(meaning the y-component vector should be 0), we still want to check for walls
  //in the y direction a little bit, so our wings don't accidentally clip walls as
  //we fly by.
  delX = SIGN(delX) * MAX(abs(delX), minLookAside);
  delY = SIGN(delY) * MAX(abs(delY), minLookAside);
  
  //Having generated x- and y- component vectors, add these to our current position
  //to get the x- and y-coordinates of the point where we're now looking.
  newX = currX + delX;
  newY = currY + delY;

  //using the new x- and y-coordinates generated above, check straight in front,
  //and then check the x and y directions individually.
  seeWallAhead = wallBetween(currX, currY, newX, newY, dummyVal, dummyVal);
  seeWallX = wallBetween(currX, currY, newX, currY, dummyVal, dummyVal);
  seeWallY = wallBetween(currX, currY, currX, newY, dummyVal, dummyVal);

  //As well as checking straight in front, check also a little to the left and to
  //the right of our current heading. This incorporates the fact that we don't ever
  //just look directly in front of us, we have a field of view that catches objects
  //some number of degrees off from where we're really looking.
  lHead = modm(currHeadingDeg + lookAngle, MAX_DEG);
  seeWallL = wallFeeler(wallLookAhead, lHead, dummyVal, dummyVal);
  rHead = modm(currHeadingDeg - lookAngle, MAX_DEG);
  seeWallR = wallFeeler(wallLookAhead, rHead, dummyVal, dummyVal);
 
  //Finally, check if we are close to one of the corners of the map. Depending on
  //which direction we're pointing, check up, down, left, or right as necessary to
  //determine whether there's a corner close by.
  currHeadingDegDiv90 = currHeadingDeg / 90;
  switch(currHeadingDegDiv90)
  {
    case(0):
      closeToCorner = wallFeeler(cornerLookAhead, lookRight, dummyVal, dummyVal)
                      && wallFeeler(cornerLookAhead, lookUp, dummyVal, dummyVal);
      break;
  
    case(1):
      closeToCorner = wallFeeler(cornerLookAhead, lookUp, dummyVal, dummyVal)
                      && wallFeeler(cornerLookAhead, lookLeft, dummyVal, dummyVal);
      break;

    case(2):
      closeToCorner = wallFeeler(cornerLookAhead, lookLeft, dummyVal, dummyVal)
                      && wallFeeler(cornerLookAhead, lookDown, dummyVal, dummyVal);
      break;
    
    case(3):
      closeToCorner = wallFeeler(cornerLookAhead, lookDown, dummyVal, dummyVal)
                      && wallFeeler(cornerLookAhead, lookRight, dummyVal, dummyVal);
      break;
   
    default:
      printf("ERROR: something's weird with the current heading\n");
      wallVector = -1;
      return;
  }

  //If turnLock is off, we are allowed to set a new turn angle.
  if(!turnLock)
  {
    //If we are close to a corner...
    if(closeToCorner)
    {
      //Turn some number of degrees to the left of the current heading.
      wallVector = modm(currHeadingDeg + turnLeft, MAX_DEG);

      //Set a turn lock of some number of frames, so that we don't adjust our turn
      //angle too frequently and just end up turning back and forth endlessly.
      turnLock = lockNum;

      //Go slow as long as we're near a corner, so we don't crash into it as we turn.
      if(frameCount % 5 < 2)
      {
        thrust(1);
      }
      else
      {
        thrust(0);
      }
    }

    //If we see a vertical wall, mirror our heading on the y-axis, and set a turn
    //lock of some number of frames.
    else if(seeWallX)
    {
      wallVector = modm(180 - currHeadingDeg, MAX_DEG);
      turnLock = lockNum;
    }

    //If we see a horizontal wall, mirror our heading on the x-axis, and set a turn
    //lock of some number of frames.
    else if(seeWallY)
    {
      wallVector = modm(-currHeadingDeg, MAX_DEG);
      turnLock = lockNum;
    }
    
    //If we see a wall that's directly in front of us or a little to the right, turn
    //left a bit, and set a turn lock.
    else if(seeWallAhead || seeWallR)
    {
      wallVector = modm(currHeadingDeg + turnLeft, MAX_DEG);
      turnLock = lockNum;
    }

    //Similarly, if we see a wall that's just a little to the left of us, turn right
    //a little, and set a turn lock.
    else if(seeWallL)
    {
      wallVector = modm(currHeadingDeg - turnRight, MAX_DEG);
      turnLock = lockNum;
    }

    //If we see no walls at all, indicate this by a value of -1.
    else
    {
      wallVector = -1;
    }
  }
  
  //Finally, if there's a turn lock on, we aren't supposed to turn for some number of
  //frames still. So, just decrement the turn lock counter.
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
  if(leaderMode)
  {
    aVector = avgFriendlyDirWithLeader(aRadius, fov, leaders, numLeaders);
  }
  else
  {
    aVector = avgFriendlyDir(aRadius, fov);
  }
}
 

/*****************************************************************************
 * Friendly Cohesion
 * ***************************************************************************/

//Compute the angle toward the average location of all friends within a certain radius.
void cohesion()
{
  int avgFriendX = -1, avgFriendY = -1;

  if(leaderMode)
  {
    avgFriendX = averageLeaderX(cRadius, fov, leaders, numLeaders);
    avgFriendY = averageLeaderY(cRadius, fov, leaders, numLeaders);
  }
  
  if(avgFriendX == -1 || avgFriendY == -1)
  {
    avgFriendX = averageFriendRadarX(cRadius, fov);
    avgFriendY = averageFriendRadarY(cRadius, fov);
  }
 
  if(avgFriendX != -1 && avgFriendY != -1)
  {
    cVector = getAngleBtwnPoints(selfX(), avgFriendX, selfY(), avgFriendY);
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
  sVector = getFriendSeparation(sRadius, fov);
}


/*****************************************************************************
 * Enemy Separation
 * ***************************************************************************/

//Compute the angle to stay away from the average position of all enemies  nearby.
void enemySeparation()
{
  eVector = getEnemySeparation(eRadius, fov);
}


/*****************************************************************************
 * No Enemy Flying (Controller)
 * ***************************************************************************/

//TODO: find a way to balance wa ll avoidance, neighbor cohesion, and other vectors
void noEnemyFlying()
{
  static int numVec, totalVec;
  static double xComp, yComp;
  int avgFriendX, avgFriendY, avgFriendDist = -1;

  numVec = totalVec = 0;
  
  //Update the past movement vector every 7 frames (roughly every half-second).
  if(frameCount % 7 == 0)
  {
    pVector = degToAim;
  }

  //Compute all relevant vectors, like wall avoidance, cohesion, separation, etc.
  wallAvoidance();
  alignment();
  cohesion();
  separation();  
  enemySeparation();

  //If there is a wall nearby, aim away from it.
  if(wallVector != -1)
  {
    totalVec += wallVector * wWeight;
    numVec += wWeight;
  }

  //Get the alignment vector and weight it.
  if(aVector != -1)
  {
    totalVec += aVector * aWeight;
    numVec += aWeight;
  }
  
  //Get the cohesion vector and weight it.
  if(cVector != -1)
  {
    if(advancedWeighting)
    {
      avgFriendX = averageFriendRadarX();
      avgFriendY = averageFriendRadarY(); 
      if(avgFriendX != -1 && avgFriendY == -1)
      {
        avgFriendDist = computeDistance(selfX(), avgFriendX, selfY(), avgFriendY);
      }
      cWeight = pow(1 + avgFriendDist / 100, 2);
    }

    totalVec += cVector * cWeight;
    numVec += cWeight;
  }

  //Get the friend separation vector and weight it.
  if(sVector != -1)
  {
    if(advancedWeighting)
    {
      sWeight = pow(4 - closestFriendDist() / 50, 2);
    }

    totalVec += sVector * sWeight;
    numVec += sWeight;
  }
  
  //Get the enemy separation vector and weight it.
  if(eVector != -1)
  {
    if(advancedWeighting)
    {
      eWeight = pow(4 - closestEnemyDist() / 50, 2);
    }

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
  static int team = 0; 
  static int value = 0;

  if(strcmp(thisMsg, scanMsg(0)))
  {
    thisMsg = scanMsg(0);
    strcpy(buf, thisMsg);
    tok = strtok(buf, " ");

    if(tok)
    {
      team = atoi(tok);
    }
    else
    {
      team = -1;
    }

    if(team == teamNum || team == 0)
    {
      tok = strtok(NULL, " ");

      if(tok)
      {
        value = atoi(strtok(NULL, " "));
      }

      if(!strcmp(tok, "mobile"))
      {
        mobile = value; 
      }
      else if(!strcmp(tok, "wweight"))
      {
        wWeight = value; 
      }
      //alignment
      else if(!strcmp(tok, "aweight"))
      {
        aWeight = value; 
      }
      else if(!strcmp(tok, "aradius"))
      {
        aRadius = value; 
      }
      //cohesion
      else if(!strcmp(tok, "cweight"))
      {
        cWeight = value; 
      }
      else if(!strcmp(tok, "cradius"))
      {
        cRadius = value;
      }
      //(friendly) separation
      else if(!strcmp(tok, "sweight"))
      {
        sWeight = value;
      }
      else if(!strcmp(tok, "sradius"))
      {
        sRadius = value;
      }
      //enemy (separation)
      else if(!strcmp(tok, "eweight"))
      {
        eWeight = value; 
      }
      else if(!strcmp(tok, "eradius"))
      {
        eRadius = value; 
      }
      //past direction
      else if(!strcmp(tok, "pweight"))
      {
        pWeight = value; 
      }
      //field of vision
      else if(!strcmp(tok, "fov"))
      {
        fov = value; 
      }
      //leader id's
      else if(!strcmp(tok, "leader"))
      {
        leaders[value - (teamNum - 1) * tot_idx - 1] = value;
        ++numLeaders;
      }
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

  if(frameCount == 300 + 5 * selfID())
  {
    if(isLeader)
    {
      char leaderMsg[20];
      char team[5], idNum[5];
      sprintf(team, "%d", teamNum);
      sprintf(idNum, "%d", selfID());
      sprintf(leaderMsg, "%s %s %s", team, "leader", idNum);
      talk(leaderMsg);     
    }
  }

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
  //get info on my idx and my team number
  idx = strtol(argv[2], NULL, 10);
  tot_idx = strtol(argv[3], NULL, 10);
  teamNum = strtol(argv[4], NULL, 10);

  //generate a random initial heading
  srand(time(NULL)); 
  pVector = degToAim = rand() % 360;

  return start(argc, argv);
}





/*
  Friendly separation: old code

  int avgFriendX = averageFriendRadarX(sRadius, fov);
  int avgFriendY = averageFriendRadarY(sRadius, fov);
  int sepVec = getAngleBtwnPoints(selfX(), avgFriendX, selfY(), avgFriendY);

  if(avgFriendX != -1 && avgFriendY != -1)
  {
    sVector = modm(sepVec + 180, 360);
  }
  else
  {
    sVector = -1;
  }
*/

/*
  Enemy separation: old code

  int avgEnemyX = averageEnemyRadarX(eRadius, fov);
  int avgEnemyY = averageEnemyRadarY(eRadius, fov);
  int sepVec = getAngleBtwnPoints(selfX(), avgEnemyX, selfY(), avgEnemyY);

  if(avgEnemyX != -1 && avgEnemyY != -1)
  {
      eVector = modm(sepVec + 180, 360);
  }
  else
  {
    eVector = -1;
  }
*/
