//Matthew Coffman - May 2018
//Compile: ./buildChaser.sh && export LD_LIBRARY_PATH=.
//Run: ./runChaser.sh <n> <filename> for some natural number n (defaults to 1) 
//  and the name of a file containing map information
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

//Drones can be initializing, flying around, engaging an enemy, or dead.
enum State
{
  STATE_INIT,
  STATE_NOENEMY,
  STATE_ENEMY_SPOTTED,
  STATE_DEAD
};

//To compute paths through the map, we have four algorithms available:
//the A* algorithm (computes shortest path, faster than Dijkstra's),
//Breadth-first search,
//Depth-first search, and
//Dijkstra's algorithm (computes shortest path).
enum GraphAlg
{
  ALG_ASTAR,
  ALG_BFS,
  ALG_DFS,
  ALG_DIJKSTRA,
  ALG_DIJKSTRA_EXT
};

//movementInfo structure: stores information regarding the nearest enemy's
//most recent movements, including the following:
//id,
//position (x and y),
//speed, and
//heading (deg).
typedef struct movementInfo 
{
  int id;
  int x;
  int y;
  double spd;
  int deg;
} moveInfo_t;

//macros
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define SIGN(X) ((X) > 0 ? 1 : ((X) < 0 ? -1 : 0))

//global constants
#define EPSILON 1E-6
#define PAST_INFO_NUM 10
#define HUGE_DISTANCE 40000
#define MAX_DEG 360
#define BUFFER_SIZE 255

//global variables
bool init = false;				
int state = STATE_INIT;				
int mapWidth, mapHeight, halfMapWidth, halfMapHeight;
int frameCount = 0;				
int degToAim;					
moveInfo_t mi[PAST_INFO_NUM] = {0};	
int turnLock = 0;
int wallLookAhead = 100, cornerLookAhead = 75, wallAvoidVector = -1;
int cohesionRadius = 400, cohesionVector = -1;
int separationRadius = 50, separationVector = -1;
int pathVector = -1;
graph_t map = {0};
bool atInter = true, atDest = true;
int *path = NULL, pathIndex = -1, interPointId = -1;
int destX, destY;
int preserving = 1, stealthy = 0;


//argv params
int idx;
int tot_idx;
char *filename;


/*****************************************************************************
 * Internal Map Generation
 * ***************************************************************************/

//Open the input file and read its contents into the global graph variable, thereby
//generating an internal representation of the map for this drone.
void getMap()
{
  //Open the given file in read mode.
  FILE *fp = fopen(filename, "r");
  char buf[BUFFER_SIZE];
  char *tok = NULL;
  int i, num_points, num_edges;

  //Read in the number of points on the map.
  fgets(buf, sizeof(buf), fp);
  tok = strtok(buf, " ");
  num_points = atoi(tok);

  //For each vertex in the map, read it in from the file, make it a vertex_t, and
  //add it to the global map variable.
  i = -1;
  while(++i < num_points)
  {
    int id, x, y;
    fgets(buf, sizeof(buf), fp);
    
    tok = strtok(buf, " ");
    id = atoi(tok);

    tok = strtok(NULL, " ");
    x = atoi(tok);

    tok = strtok(NULL, " ");
    y = atoi(tok);

    vertex_t v = (vertex_t) {id, x, y};
    map = add_vertex(map, v);
  }
  
  //Read in the number of edges in the map.
  fgets(buf, sizeof(buf), fp);
  tok = strtok(buf, " ");
  num_edges = atoi(tok);
  
  //For each edge, read it in, make it two vertex_t's and an integer, and add it
  //to the global map variable.
  i = -1;
  while(++i < num_edges)
  {
    int id1, x1, y1, id2, x2, y2;
    fgets(buf, sizeof(buf), fp);

    tok = strtok(buf, " ");
    id1 = atoi(tok);

    tok = strtok(NULL, " ");
    x1 = atoi(tok);

    tok = strtok(NULL, " ");
    y1 = atoi(tok);

    tok = strtok(NULL, " ");
    id2 = atoi(tok);
    
    tok = strtok(NULL, " ");
    x2 = atoi(tok);

    tok = strtok(NULL, " ");
    y2 = atoi(tok);

    vertex_t v1 = (vertex_t) {id1, x1, y1};
    vertex_t v2 = (vertex_t) {id2, x2, y2};
    map = add_edge(map, v1, v2);
  }

  //Close the file.
  fclose(fp);

  //Note: this could break at some point, if there's a lot of doubling back on
  //the path and the size of the path exceeds the number of vertices.
  path = malloc(sizeof(int) * map.num_v + 1);
  if(!path)
  {
    perror("ERROR: couldn't allocate memory for path\n");
    abort();
  }
}


/*****************************************************************************
 * Initialization
 * ***************************************************************************/

//Initialization: spends the first 50 frames turning to the desired angle, and then
//declares initialization complete and switches to the NOENEMY state
void Initialize()
{
  turnToDeg(degToAim);

  if(frameCount % 28 == 0)
  {
    //Generate a representation of the map, using the given map file.
    getMap();

    //Get the map dimensions for later use.
    mapWidth = getMapWidth();
    mapHeight = getMapHeight();
    halfMapWidth = mapWidth / 2;
    halfMapHeight = mapHeight / 2;

    //Declare initialized and set state to no enemies.
    init = true;
    state = STATE_NOENEMY;
    thrust(0);
  }
}


/*************************************************************************** 
 * Path Generation
 * *************************************************************************/

//Returns the index of the point with the given id.
int getMapPointIndex(int id)
{
  int i;

  for(i = 0; i < map.num_v; i++)
    if(map.vertices[i].id == id)
      return i;

  return -1;
}

//Returns the index of the vertex closest to the given XY point.
int closestMapPointIndexXY(int x, int y)
{
  int i, minDist = INT_MAX, minIndex = -1, tempDist;

  //For each vertex, compute its distance to the given XY point until we've
  //found the minimum value.
  for(i = 0; i < map.num_v; i++)
  {
    tempDist = distanceFormula(x, map.vertices[i].x, y, map.vertices[i].y);
    if(tempDist < minDist)
    {
      minDist = tempDist;
      minIndex = i;
    }
  }
  
  //Return the index corresponding to that minimum value.
  return minIndex;
}

//Returns the index of the closest point in the graph to this drone's location.
int closestMapPointIndexSelf()
{
  return closestMapPointIndexXY(selfX(), selfY());
}

//Using the given algorithm, finds a path from this drone's current location
//to the point with the given id.
void pathToPointId(int alg, int id)
{
  int i, cpToMe = closestMapPointIndexSelf();
  vertex_t v1, v2;

  //Clear the global path variable to make way for the new path.0
  for(i = 0; i < map.num_v+1; i++)
    path[i] = '\0';

  //Switch on the given path-finding algorithm, enumerated globally above.
  switch(alg)
  {
    case(ALG_ASTAR):
      astar(map, map.vertices[cpToMe], map.vertices[getMapPointIndex(id)], path);  
      break;

    case(ALG_BFS):
      bfs(map, map.vertices[cpToMe], map.vertices[getMapPointIndex(id)], path);  
      break;

    case(ALG_DFS):
      dfs(map, map.vertices[cpToMe], map.vertices[getMapPointIndex(id)], path);  
      break;

    case(ALG_DIJKSTRA):
      dijkstra(map, map.vertices[cpToMe], map.vertices[getMapPointIndex(id)], path);
      break;

    case(ALG_DIJKSTRA_EXT):
      v1 = map.vertices[rand() % map.num_v];
      v2 = map.vertices[rand() % map.num_v];
      printf("MIDPOINTS: %3d %3d\n", v1.id, v2.id);
      dijkstraN(map, map.vertices[cpToMe], map.vertices[getMapPointIndex(id)], path, 2, v1, v2);
      break; 
  }
 
  //Print the path found.
  print_path(path);

  //Set the pathIndex variable to 0, to prepare for stepping through the path
  //incrementally as our drone flies there.
  pathIndex = 0;
}

//Finds a path from this drone's current location to the given XY point, using
//the given path-finding algorithm.
void pathToPointXY(int alg, int x, int y)
{
  pathToPointId(alg, closestMapPointIndexXY(x, y) + 1);
}

//Establishes a path to the given XY point using the given path algorithm, and
//sets the pathVector value so the drone can fly there.
void goToXY(int alg, int x, int y)
{
  int interX = -1, interY = -1;

  if(atDest)
  {
    //If we're already supposedly at our destination, check if we really are by
    //asking if we're within 30 pixels of the target.
    if(distanceFormula(selfX(), x, selfY(), y) < 30)
      return;

    //If we're actually far away from the target, compute the path to the target
    //and go there, and indicate that we're not at our destination yet.
    pathToPointXY(alg, x, y);
    atDest = false;
  }

  //Since interPointId is globally initialized to -1, check this before trying to
  //index with it.
  if(interPointId != -1)
  {
    interX = map.vertices[interPointId-1].x;
    interY = map.vertices[interPointId-1].y;    
  }

  //If we're at our intermediate destination but still have more points in the path,
  //update pathIndex and interPointId accordingly.
  if(interPointId == -1 || (distanceFormula(selfX(), interX, selfY(), interY) < 30 && pathIndex != length(path)-1))
  {
    interPointId = path[++pathIndex];
    interX = map.vertices[interPointId-1].x;
    interY = map.vertices[interPointId-1].y;
  }
  //Otherwise, if we're close to the goal, we must have finished the path.
  else if(distanceFormula(selfX(), interX, selfY(), interY) < 30)
  {
    printf("PATH DONE!!!\n");
    atDest = true;
    return;
  }

  //Assuming we aren't at the goal yet, aim toward the next target point.
  pathVector = angleToXY(interX, interY);
}

//If we currently don't have anywhere to go, pick a random point and go there.
void goToRandom(int alg)
{
  if(atDest)
  {
    destX = rand() % mapWidth;
    destY = rand() % mapHeight;
  }
  goToXY(alg, destX, destY);
}


//Run away from the closest enemy until we're out of range or out of sight.
void RunAway()
{
  int cpToMe = closestMapPointIndexSelf();
  int enemyX = getNearestEnemyX();
  int enemyY = getNearestEnemyY();

  int angleToEnemy = angleToXY(enemyX, enemyY);
  int angleAwayFromEnemy = modm(angleToEnemy + 180, MAX_DEG);

  int newX = selfX() + 200 * cos(degToRad(angleAwayFromEnemy));
  int newY = selfY() + 200 * sin(degToRad(angleAwayFromEnemy));

  goToXY(ALG_DIJKSTRA, newX, newY);
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
  bool seeWallX, seeWallY, seeWallAhead, cornerClose;
  int r;

  //Get the current heading, in degrees and radians, and current position info. 
  currHeadingRad = selfHeadingRad();
  currHeadingDeg = (int)radToDeg(currHeadingRad);
  currX = selfX();
  currY = selfY(); 

  //Check if we will crash into a wall if we carry on the current course for some
  //distance.
  delX = (int)(wallLookAhead * cos(currHeadingRad)); 
  delY = (int)(wallLookAhead * sin(currHeadingRad)); 
  delX = SIGN(delX) * MAX(abs(delX), 40);
  delY = SIGN(delY) * MAX(abs(delY), 40);
  newX = currX + delX;
  newY = currY + delY;
  seeWallX = wallBetween(currX, currY, currX + delX, currY, 1, 1);
  seeWallY = wallBetween(currX, currY, currX, currY + delY, 1, 1);
  seeWallAhead = wallBetween(currX, currY, currX + delX, currY + delY, 1, 1);
 
  //Check if we are close to one of the corners of the map.
  cornerClose = (wallFeeler(cornerLookAhead, 0, 1, 1) 
                   || wallFeeler(cornerLookAhead, 180, 1, 1)) 
                && 
                (wallFeeler(cornerLookAhead, 90, 1, 1) 
                   || wallFeeler(cornerLookAhead, 270, 1, 1));

  //If turnLock is off, we are allowed to set a new turn angle.
  if(!turnLock)
  {
    //If we are close to a corner...
    if(cornerClose)
    {
      //Turn toward the midpoint of the map and fly in that direction, give or take
      //25 degrees.
      r = (rand() % 50) - 25;
      wallAvoidVector = radToDeg(atan2(halfMapHeight - currY, halfMapWidth - currX)) + r;
      wallAvoidVector = modm(wallAvoidVector, MAX_DEG);

      //Set a turnLock of 28 frames: we will not be able to set a new wall avoidance
      //angle for 28 frames, approximately 2 seconds.
      turnLock = 28;
    }

    //If we see a vertical wall, mirror our heading on the y-axis, and set a turnLock
    //of 5 frames.
    else if(seeWallX)
    {
      wallAvoidVector = modm(180 - currHeadingDeg, MAX_DEG);
      turnLock = 5;
    }

    //If we see a horizontal wall, mirror our heading on the x-axis, and set a turnLock
    //of 5 frames.
    else if(seeWallY)
    {
      wallAvoidVector = modm(-currHeadingDeg, MAX_DEG);
      turnLock = 5;
    }
    
    //If the wall isn't perfectly horizontal or vertical, turn a little bit
    //(note that this solution is not optimal, still bumps into walls occasionally),
    //and set a turnLock of 5 frames.
    else if(seeWallAhead)
    {
      wallAvoidVector = modm(currHeadingDeg + 90, MAX_DEG);
      turnLock = 5;
      /*
      int i, lastWallMin = -1, lastWallMax = INT_MAX, thisWallDist;
      bool inc = true;
      bool dec = true;
      for(i = -20; i <= 20; i++)
      {
        thisWallDist = wallFeeler(wallLookAhead, radToDeg(selfHeadingRad()) + i, 1, 1);
        if(thisWallDist != 0)
        {
          if(thisWallDist < lastWallMin)
            inc = false;
          else
            lastWallMin = thisWallDist;
          
          if(thisWallDist > lastWallMax)
            dec = false;
          else
            lastWallMax = thisWallDist;
        }
      }

      if(inc)
        //do something
      else if(dec)
        //do something
      else
        //do something
      */
    }

    else
      wallAvoidVector = -1;
  }
  //If there's a turnLock on, count down.
  else
    turnLock--;
}


/*****************************************************************************
 * Friendly Cohesion
 * ***************************************************************************/

//Computes the angle toward the average location of all friends within a certain radius.
void cohesion()
{
  int avgFriendX = averageFriendRadarX(cohesionRadius);
  int avgFriendY = averageFriendRadarY(cohesionRadius);
 
  if(avgFriendX != -1 && avgFriendY != -1)
    cohesionVector = getHeading(selfX(), avgFriendX, selfY(), avgFriendY);
  else
    cohesionVector = -1;
}


/*****************************************************************************
 * Enemy Separation
 * ***************************************************************************/

void separation()
{
  int avgFriendX = averageFriendRadarX(separationRadius);
  int avgFriendY = averageFriendRadarY(separationRadius);

  if(avgFriendX != -1 && avgFriendY != -1)
    separationVector = modm(getHeading(selfX(), avgFriendX, selfY(), avgFriendY)+180,360);
  else
    separationVector = -1;
}


/*****************************************************************************
 * No Enemy Flying (Controller)
 * ***************************************************************************/

//TODO: find a way to balance wall avoidance, neighbor cohesion, and other vectors
void NoEnemyFlying()
{
  wallAvoidance();
  cohesion();
  separation();  
  goToRandom(ALG_DIJKSTRA_EXT);
   
  degToAim = pathVector;
  if(wallAvoidVector != -1)
    degToAim = wallAvoidVector;
  else if(separationVector != -1)
    degToAim = separationVector;
  else if(cohesionVector != -1)
    degToAim = cohesionVector;    

  turnToDeg(degToAim);
  
  if(!atDest)
    thrust(1);
}


/*****************************************************************************
 * Update Enemy Info
 * ***************************************************************************/

//update the enemy's past movement info to include the most recent info on speed,
//location, angle, and id
void updateEnemyPast(int id, int x, int y, double spd, int deg)
{
  int i;
  
  //shift old array entries down to make room for the newest data point
  for(i = PAST_INFO_NUM-1; i > 0; i--)
    mi[i] = mi[i-1];

  //store the most recent state of the enemy's motion
  mi[0] = (moveInfo_t) {id, x, y, spd, deg};
  
  //if one of these component values is -1, replace it with the last meaningful entry
  if(id == -1) 
    mi[0].id = mi[1].id; 

  if(deg == -1) 
    mi[0].deg = mi[1].deg; 

  if(abs(spd + 1) < EPSILON) 
    mi[0].spd = mi[1].spd;
}


/*****************************************************************************
 * Update Aim (Toward Enemy)
 * ***************************************************************************/

//after updating the movement info, figure out where the chaser drone should be
//heading and turn in that direction
void updateAim(int dist)
{
  double aimInRad;
  int enemyHeadingToMe, enemyHeading, headingDiff, distInFront;  

  enemyHeadingToMe = getHeading(mi[0].x, selfX(), mi[0].y, selfY());
  enemyHeading = mi[0].deg;
  headingDiff = abs(enemyHeadingToMe - enemyHeading);
  
  if(headingDiff < 30 || headingDiff > 330)
    distInFront = 0;

  else if(mi[0].spd < 3)
    distInFront = 0;

  else
    distInFront = MIN(2 * (dist - (dist % 50)), 300);

  //compute the new target location and turn
  int newX = mi[0].x + (distInFront * cos(degToRad(mi[0].deg))); 
  int newY = mi[0].y + (distInFront * sin(degToRad(mi[0].deg)));

  aimInRad = atan2(newY - selfY(), newX - selfX());
  degToAim = radToDeg(aimInRad);
}


/*****************************************************************************
 * Engaging Enemy (Controller)
 * ***************************************************************************/

//engaging: compute the id, speed, and heading of the nearest enemy, if possible,
//and adjust course as necessary
void Engaging(int enemyX, int enemyY, int enemyDist)
{
  int enemyId, enemyDeg = -1;
  double enemySpd = -1.0;

  enemyId = closestEnemyShipId();
  
  if(enemyId != -1)
  {
    enemySpd = enemySpeedId(enemyId);
    enemyDeg = (int)enemyHeadingDegId(enemyId);
  }

  //check that the x- and y-coordinates of the enemy are valid
  if(enemyX != -1 && enemyY != -1)
  {
    //update the arrays of past enemy info
    updateEnemyPast(enemyId, enemyX, enemyY, enemySpd, enemyDeg);

    //calculate the angle we should be heading based on the enemy's position
    updateAim(enemyDist);

    //make sure the angle we need to turn to is between 0 and 360
    degToAim = modm(degToAim, MAX_DEG);

    //turn to the desired heading
    turnToDeg(degToAim);
     
    thrust(1);
  }
}


void EnemySpotted()
{
  if(preserving)
    RunAway();
  //else if(stealthy)
    //Stealth();
  //else
    //Engaging()
}


/*****************************************************************************
 * AI Loop
 * ***************************************************************************/

//this is where the magic happens (i.e., this loop fires every frame and decides
//which state the chaser is currently in and acts accordingly)
AI_loop()
{
  //by default, don't thrust, and increment the frame counter
  frameCount = (frameCount + 1) % 100;
 
  //also by default, assume no enemy is nearby
  state = STATE_NOENEMY;

  //if there is an enemy closeby on the map, engage
  int distToEnemy = distanceToNearestEnemy();
  int enemyX = getNearestEnemyX();
  int enemyY = getNearestEnemyY();
  if(distToEnemy != -1 && distToEnemy < 500 && !wallBetween(selfX(), enemyX, selfY(), enemyY, 1, 1))
    state = STATE_ENEMY_SPOTTED;

  //check if we are dead
  if(!selfAlive())
    state = STATE_DEAD;

  //if we haven't initialized yet
  if(!init)
    state = STATE_INIT;

  switch(state)
  {
    case(STATE_INIT):
      Initialize();
      break;

    case(STATE_ENEMY_SPOTTED):
      EnemySpotted();
      //Engaging(getNearestEnemyX(), getNearestEnemyY(), distToEnemy);
      break;

    case(STATE_NOENEMY):
      NoEnemyFlying();
      break;

    case(STATE_DEAD):
      state = STATE_NOENEMY;
      break;
  }
}


//gets initial input and starts the AI loop
int main(int argc, char *argv[]) 
{
  idx = strtol(argv[2], NULL, 10);
  tot_idx = strtol(argv[3], NULL, 10);
  filename = argv[4];  

  printf("idx:      %d\n", idx);
  printf("tot_idx:  %d\n", tot_idx);
  printf("filename: %s\n", filename);

  //generate a random initial heading
  srand(time(NULL));
  degToAim = rand() % 360;

  return start(argc, argv);
}
