//Matthew Coffman - July 2018
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

//global constants
#define MAX_DEG 360

//function prototypes
void initialize();
void wallAvoidance();
void alignment();
void cohesion();
void separation();
void flocking();
void cWeightAdjustByDistance();
void sWeightAdjustByDistance();
void eWeightAdjustByDistance();
void handleMsgBuffer();

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
int degToAim = -1;			//what direction do we want to go
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
bool isLeader = false;		//whether this drone is a leader
int *leaders = NULL;		//array of leader id's
int numLeaders = 0;		//how many leaders on our team
bool leaderMode = false;	//whether we care just about leaders for flocking
bool distanceWeighting = false;	//simple averaging vs factoring in distance


/*****************************************************************************
 * Initialization
 * ***************************************************************************/

//Declares initialization complete and switches to the NOENEMY state
void initialize()
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
    //Determine, by some criterion, whether I'm a leader.
    if(idx % tot_idx == 1)
    {
      isLeader = true;
    }
  
    //Allocate space for an array of all the leader id's for later use.
    leaders = malloc(sizeof(int) * tot_idx);
    if(!leaders)
    {
      perror("couldn't allocate leaders memory");
      exit(EXIT_FAILURE);
    }
  }

  //Declare initialized and set state to typical flying.
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
  static int turnLock;
  static int lockNum = 5;

  //If there's no turn lock on, compute wall avoidance and impose a turn lock
  //of some number of frames. This will prevent us from computing new turn angles 
  //too frequently and continually turning back and forth.
  if(!turnLock)
  {
    wallVector = getWallAvoidance();

    if(wallVector != -1)
    {
      turnLock = lockNum;
    }
  }
  //If there's a turn lock on, just decrement the turn lock counter and leave the
  //wall avoidance vector unchanged.
  else
  {
    --turnLock;
  }
}


/*****************************************************************************
 * Friendly Alignment 
 * ***************************************************************************/

//Compute the angle to stay aligned with all friends within a certain radius.
void alignment()
{
  //If we're in leader mode, just get the average heading of all leaders nearby.
  if(leaderMode)
  {
    if(!isLeader)
    {
      aVector = avgFriendlyDirWithLeader(aRadius, fov, leaders, numLeaders);
    }
    else
    {
      aVector = -1;
    }
  }
  //Otherwise, get the average direction of ALL friends nearby.
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

  //If we're in leader mode, try getting the average position of all leaders nearby.
  if(leaderMode)
  {
    avgFriendX = averageLeaderX(cRadius, fov, leaders, numLeaders);
    avgFriendY = averageLeaderY(cRadius, fov, leaders, numLeaders);
  }
  
  //If we aren't in leader mode, or if our leader-mode calculation gave us bad (-1)
  //values for x or y, get the average position of ALL friends nearby.
  else //if(avgFriendX == -1 || avgFriendY == -1)
  {
    avgFriendX = averageFriendRadarX(cRadius, fov);
    avgFriendY = averageFriendRadarY(cRadius, fov);
  }
 
  //If we have valid (i.e. not -1) values for x and y, update the cohesion vector
  //accordingly. Otherwise, return -1.
  if(avgFriendX != -1 && avgFriendY != -1 && (!leaderMode || !isLeader))
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
 * Flocking (Controller)
 * ***************************************************************************/

//Generates emergent flocking behavior based on boids algorithm by computing and
//balancing vectors related to wall avoidance, cohesion, alignment, and friendly
//and enemy separation. For each of these vectors, splits it into its x- and y-
//components and takes a weighted average of each of these components, then uses
//arctangent to get the resulting angle.
void flocking()
{
  static int totalWt, newAngle;
  static double xComp, yComp;
  int avgFriendX, avgFriendY, avgFriendDist = -1;

  xComp = yComp = 0.0;
  totalWt = 0;
  
  //Update the past movement vector
  pVector = degToAim;
  
  //Compute all the aforementioned vectors.
  wallAvoidance();
  alignment();
  cohesion();
  separation();  
  enemySeparation();

  //Add the past-movement vector, which keeps track of our most recent heading, to
  //our running total for x-component, y-component, and weight total. We will be adding
  //to these values as we check wall avoidance, cohesion, alignment, etc. in succession. 
  scaleVector(pVector, pWeight, &xComp, &yComp, &totalWt);

  //If there is a wall nearby, aim away from it.
  if(wallVector != -1)
  {
    scaleVector(wallVector, wWeight, &xComp, &yComp, &totalWt);
  }
  
  //Get the alignment vector and weight it.
  if(aVector != -1)
  {
    scaleVector(aVector, aWeight, &xComp, &yComp, &totalWt);
  }
  
  //Get the cohesion vector and weight it.
  if(cVector != -1)
  {
    if(distanceWeighting)
    {
      cWeightAdjustByDistance();
    }

    scaleVector(cVector, cWeight, &xComp, &yComp, &totalWt);
  }

  //Get the friend separation vector and weight it.
  if(sVector != -1)
  {
    if(distanceWeighting)
    {
      sWeightAdjustByDistance();
    }

    scaleVector(sVector, sWeight, &xComp, &yComp, &totalWt);
  }
  
  //Get the enemy separation vector and weight it.
  if(eVector != -1)
  {
    if(distanceWeighting)
    {
      eWeightAdjustByDistance();    
    }

    scaleVector(eVector, eWeight, &xComp, &yComp, &totalWt);
  }
  
  //Compute the weighted average of the four vectors above, and point in that
  //direction.
  if(totalWt > 0)
  {
    //Average the accumulated, weighted x- and y-components.
    xComp /= totalWt;
    yComp /= totalWt;

    //Generate a new angle from the resulting x- and y-components, and update
    //the degree to aim.
    //Notice that, if the resulting angle is less than 0, we add MAX_DEG - 1 = 359 
    //to it to get it between 0 and 360. This is because, for some reason, if we
    //add MAX_DEG = 360 to the angle, at the beginning of the simulation before the
    //boids start flocking, some of their headings will slowly but steadily taper
    //to 0 when they should stay constant. This problem corrects itself when we mod
    //by 359 instead.
    //TODO: Figure out exactly why modding by 360 doesn't work, but 359 does.
    degToAim = modm(radToDeg(atan2(yComp, xComp)), MAX_DEG - 1);
  }

  //Whatever direction we've decided to turn to, do so.
  turnToDeg(degToAim);

  //Now that we've turned, we can decide how much to thrust.
  if(mobile)
  {
    thrust(1);
  }
  //If we're anchored, just don't move.
  else
  {
    thrust(0);
  }
}


/*****************************************************************************
 * Scaling Cohesion and (Enemy/Friendly) Separation
 * ***************************************************************************/

//Scale the cohesion weight by how far away we are from our friends' center of mass.
void cWeightAdjustByDistance()
{
  int avgFriendX, avgFriendY, avgFriendDist;

  avgFriendX = averageFriendRadarX(cRadius, fov);
  avgFriendY = averageFriendRadarY(cRadius, fov); 

  if(avgFriendX != -1 && avgFriendY != -1)
  {
    avgFriendDist = computeDistance(selfX(), avgFriendX, selfY(), avgFriendY);
    cWeight = pow(avgFriendDist / 100, 2);
  }
}

//Scale the friendly separation weight by how close we are to our closest friend.
void sWeightAdjustByDistance()
{
  int dist = closestFriendDist();

  if(dist != -1)
  {
    sWeight = pow(4 - dist / 75, 2);
  }
}

//Scale the enemy separation weight by how close we are to our closest enemy.
void eWeightAdjustByDistance()
{
  int dist = closestEnemyDist();

  if(dist != -1)
  {
    eWeight = pow(4 - dist / 50, 2);
  }
}


/*****************************************************************************
 * Message Handler (through chat feature) 
 * ***************************************************************************/

//Handle whatever messages pop up through the chat feature. Messages should take
//the following form:
//  <team number> <keyword> <new value>
//For example, a valid message might be "0 aweight 4". The team number allows
//you to send messages just to one particular team, or else use 0 if you want to
//broadcast your message to everyone. There is a case for each keyword below, 
//and in each case the new value is assigned to some global variable defined above.
void handleMsgBuffer()
{
  static char buf[100];
  static char *tok = NULL;
  static char *oldMsg = "";
  static int team = 0; 
  static int value = 0;

  //Check if the most recent message available has changed since the last frame.
  if(strcmp(oldMsg, scanMsg(0)))
  {
    //Update the most recent message. Then, copy it into a character buffer and
    //tokenize it by spaces.
    oldMsg = scanMsg(0);
    strcpy(buf, oldMsg);
    tok = strtok(buf, " ");

    //If the token is not NULL, meaning the message given was not empty, convert
    //it to an integer and store it as the team number.
    //TODO: Possibly add more error trapping, in case the first value in the message
    //is not an integer.
    if(tok)
    {
      team = atoi(tok);
    }
    else
    {
      team = -1;
    }

    //Check if the given team number matches our team number, or if the message has 
    //been sent to everyone on the map, 
    if(team == teamNum || team == 0)
    {
      //Tokenize again, so now our token pointer points to the keyword.
      tok = strtok(NULL, " ");

      //If the keyword-pointing token is not NULL, get the value that follows.
      if(tok)
      {
        value = atoi(strtok(NULL, " "));
      }

      //mobile: toggle mobility on and off
      if(!strcmp(tok, "mobile"))
      {
        mobile = value; 
      }
      //wall weight: adjust how much we want to get away from walls
      else if(!strcmp(tok, "wweight"))
      {
        wWeight = value; 
      }
      //alignment weight: adjust how much we want to align with nearby friends
      else if(!strcmp(tok, "aweight"))
      {
        aWeight = value; 
      }
      //alignment radius: adjust how far away we check for friends to align with
      else if(!strcmp(tok, "aradius"))
      {
        aRadius = value; 
      }
      //cohesion weight: adjust how much we want to be close to our friends
      else if(!strcmp(tok, "cweight"))
      {
        cWeight = value; 
      }
      //cohesion radius: adjust how far away we check for friends to cohere with
      else if(!strcmp(tok, "cradius"))
      {
        cRadius = value;
      }
      //(friendly) separation weight: adjust how much we want to avoid our friends
      else if(!strcmp(tok, "sweight"))
      {
        sWeight = value;
      }
      //(friendly) separation radius: adjust how far away we check for friends to avoid
      else if(!strcmp(tok, "sradius"))
      {
        sRadius = value;
      }
      //enemy (separation) weight: adjust how much we want to avoid our enemies
      else if(!strcmp(tok, "eweight"))
      {
        eWeight = value; 
      }
      //enemy (separation) radius: adjust how far away we check for enemies to avoid
      else if(!strcmp(tok, "eradius"))
      {
        eRadius = value; 
      }
      //past direction weight: adjust how much we want to stick to our past direction
      else if(!strcmp(tok, "pweight"))
      {
        pWeight = value; 
      }
      //field of vision: adjust how far around us we can see
      else if(!strcmp(tok, "fov"))
      {
        fov = value; 
      }
      //leader: add a leader to the array of leaders to follow, if we're in leader mode
      else if(!strcmp(tok, "leader"))
      {
        leaders[value - (teamNum - 1) * tot_idx - 1] = value;
        ++numLeaders;
      }
      //TODO: possibly add a way to remove leaders from the list
    }
  }
}


/*****************************************************************************
 * AI Loop
 * ***************************************************************************/

//This AI loop runs every frame, keeps track of what state we're in, and acts
//accordingly.
AI_loop()
{
  //Increment the frame counter.
  frameCount = (frameCount + 1) % INT_MAX;

  //If about 300 frames have gone by and we're a leader, broadcast a message that
  //indicates this, so others on my team can follow me in leader mode.
  if(frameCount == 300 && isLeader)
  {
    broadcastMessage(teamNum, "leader", selfID());
  }

  //Check the input message buffer to adjust behavior as necessary.
  handleMsgBuffer();

  //By default, assume we're just flying around.
  state = STATE_FLYING;

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
      initialize();
      break;

    case(STATE_FLYING):
      flocking();
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
  //Get info on my idx, tot_idx, and my team number.
  idx = strtol(argv[2], NULL, 10);
  tot_idx = strtol(argv[3], NULL, 10);
  teamNum = strtol(argv[4], NULL, 10);

  //Seed the random number generator.
  srand(time(NULL)); 

  //Start the AI loop.
  return start(argc, argv);
}

