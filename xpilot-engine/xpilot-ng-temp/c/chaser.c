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

//macros
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define SIGN(X) ((X) > 0 ? 1 : ((X) < 0 ? -1 : 0))
#define FAST thrust(1);
#define STOP thrust(0);
#define SLOW (frameCount % 5 < 2) ? thrust(1) : thrust(0)

//global constants
#define ALIGNMENT_RADIUS 400
#define BUFFER_SIZE 255
#define COHESION_RADIUS 400
#define CORNER_LOOK_AHEAD 75
#define EPSILON 1E-6
#define MAX_DEG 360
#define NUM_AVOIDPTS 10
#define NUM_FOLLOWERS 20
#define NUM_LEADERS 20
#define PAST_INFO_NUM 10
#define POINT_PRECISION 50
#define SEPARATION_RADIUS 100
#define WALL_LOOK_AHEAD 100

//function prototypes
void getMap();
void initialize();
int getMPIndex(graph_t, int);
int cpIndexXY(graph_t, int, int);
int cpIndexSelf(graph_t);
void pathToPointId(int, graph_t, int, int *);
void pathToPointXY(int, graph_t, int, int, int *);
void goToXY(int, graph_t, int, int);
void goToId(int, graph_t, int);
void goToRandom(int, graph_t);
void runAway();
void wallAvoidance();
void cohesion();
void separation();
void alignment();
bool nearCornerPeek();
void noEnemyFlying();
void updateEnemyPast(int, int, int, double, int);
void updateAim(int);
void engaging(int, int, int);
void stealth();
void enemySpotted();
void handleMsg();

//enumeration of drone states
enum State
{
  STATE_INIT,
  STATE_NOENEMY,
  STATE_ENEMY_SPOTTED,
  STATE_DEAD
};

//enumeration of path-finding algorithms
enum GraphAlg
{
  ALG_ASTAR,
  ALG_BFS,
  ALG_DFS,
  ALG_DIJKSTRA,
  ALG_DIJKSTRA_EXT
};

//enumeration of talking lines
enum TalkMsgs
{
  MSG_LEADER,
  MSG_DIED
};

//stores info on points that should be avoided
typedef struct avoidpt
{
  int x;
  int y;
  int r;
} avoidpt_t;

//stores info relevant to (an enemy's) movement
typedef struct movementInfo 
{
  int id;
  int x;
  int y;
  double spd;
  int deg;
} moveInfo_t;

//global variables
bool init = false, isLeader = false;				
int state = STATE_INIT, frameCount = 0;				
int mapWidth, mapHeight, halfMapWidth, halfMapHeight;
int degToAim, turnLock = 0;
int wallAvoidVector = -1, pathVector = -1, leaderVector = -1;
int cohesionVector = -1, separationVector = -1, alignmentVector = -1;
int interPointId = -1, *corners_list = NULL;
int preserving = 0, stealthy = 1, mobile = 1, cautious = 0, focused = 1, avoidant = 1;
char *pastMsg = "dummy", *thisMsg = "dummy";
int aWeight = 0, sWeight = 0, cWeight = 0;
int fov = 60, rov = 500;
int *leaders = NULL, *followers = NULL;
char talkMsgs[][10] = {"leader ", "died "};
graph_t map;
moveInfo_t pastMovement;
avoidpt_t dangerPoints[NUM_AVOIDPTS];

//argv params
int idx;
int tot_idx;
char *filename;


/*****************************************************************************
 * Internal Map Generation
 * ***************************************************************************/

//Open the given file, generate an internal map representation.
void getMap()
{
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
  for(i = 0; i < num_points; i++)
  {
    int j, a[3];
    
    fgets(buf, sizeof(buf), fp);
    
    for(j = 0; j < 3; j++)
    {
      if(!j)
      {
        tok = strtok(buf, " ");
      }
      else
      {
        tok = strtok(NULL, " ");
      }

      a[j] = atoi(tok);
    }

    vertex_t v = (vertex_t) {a[0], a[1], a[2]};
    map = add_vertex(map, v);
  }
  
  //Read in the number of edges in the map.
  fgets(buf, sizeof(buf), fp);
  tok = strtok(buf, " ");
  num_edges = atoi(tok);
  
  //For each edge, read it in, make it two vertex_t's and an integer, and add it
  //to the global map variable.
  for(i = 0; i < num_edges; i++)
  {
    int j, a[6];
    
    fgets(buf, sizeof(buf), fp);

    for(j = 0; j < 6; j++)
    {
      if(!j)
      {
        tok = strtok(buf, " ");
      }
      else
      {
        tok = strtok(NULL, " ");
      }

      a[j] = atoi(tok);
    }

    vertex_t v1 = (vertex_t) {a[0], a[1], a[2]};
    vertex_t v2 = (vertex_t) {a[3], a[4], a[5]};
    map = add_edge(map, v1, v2);
  }

  //Close the file.
  fclose(fp);

  int num_corners = 0;
  while(map.vertices[num_corners].y 
          >= map.vertices[num_corners++ + 1].y);

  corners_list = malloc(sizeof(int) * num_corners);
  for(i = 0; i < num_corners; i++)
  {
    corners_list[i] = map.vertices[i].id;
  }
}


/*****************************************************************************
 * Initialization
 * ***************************************************************************/

//Declares initialization complete and switches to the NOENEMY state
void initialize()
{
  int i;

  turnToDeg(degToAim);

  //Generate a representation of the map, using the given map file.
  getMap();

  //Get the map dimensions for later use.
  mapWidth = getMapWidth();
  mapHeight = getMapHeight();
  halfMapWidth = mapWidth / 2;
  halfMapHeight = mapHeight / 2;

  //Set up leader/follower arrays.
  leaders = malloc(sizeof(int) * tot_idx);
  followers = malloc(sizeof(int) * tot_idx);
  if(!leaders || !followers)
  {
    perror("error: couldn't allocate memory for leaders/followers");
    abort();
  }
  for(i = 0; i < tot_idx; i++)
  {
    leaders[i] = followers[i] = 0;
  }
 
  //initialize the map to have one point that drones want to avoid
  for(i = 0; i < NUM_AVOIDPTS; i++)
  {
    dangerPoints[i] = (avoidpt_t) {-1, -1, -1};
  }
  dangerPoints[0] = (avoidpt_t) {1820, 3780, 500};
  dangerPoints[1] = (avoidpt_t) {2835, 2870, 300};

  //Declare initialized and set state to no enemies.
  init = true;
  state = STATE_NOENEMY;
  thrust(0);
}


/*************************************************************************** 
 * Path Generation
 * *************************************************************************/

//Returns the index of the point with the given id.
int getMPIndex(graph_t g, int id)
{
  int i;

  for(i = 0; i < g.num_v; i++)
  {
    if(g.vertices[i].id == id)
    {
      return i;
    }
  }

  return -1;
}


//Returns the index of the vertex closest to the given XY point.
int cpIndexXY(graph_t g, int x, int y)
{
  int i, tempX, tempY, tempDist;
  int minIndex = -1, minDist = INT_MAX;

  //For each vertex, compute its distance to the given XY point until we've
  //found the minimum value.
  for(i = 0; i < g.num_v; i++)
  {
    tempX = g.vertices[i].x;
    tempY = g.vertices[i].y;
    tempDist = distForm(x, tempX, y,tempY);

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
int cpIndexSelf(graph_t g)
{
  return cpIndexXY(g, selfX(), selfY());
}


//Using the given algorithm, finds a path from this drone's current location
//to the point with the given id.
void pathToPointId(int alg, graph_t g, int id, int *path)
{
  int i, cpToMe = cpIndexSelf(g), x, y, deg, r, angleToDest, tempHead;
  vertex_t v1, v2, v3, v4;
  
  v1 = g.vertices[cpToMe];
  v2 = g.vertices[getMPIndex(g, id)];

  //Switch on the given path-finding algorithm, enumerated globally above.
  switch(alg)
  {
    case(ALG_ASTAR):
      astar(g, v1, v2, path);  
      break;

    case(ALG_BFS):
      bfs(g, v1, v2, path);  
      break;

    case(ALG_DFS):
      dfs(g, v1, v2, path);  
      break;

    case(ALG_DIJKSTRA):
      dijkstra(g, v1, v2, path);
      break;

    case(ALG_DIJKSTRA_EXT):
      angleToDest = selfAngleToXY(v2.x, v2.y); 
      tempHead = modm(angleToDest + 60, MAX_DEG);
      x = selfX() + 1500 * cos(degToRad(tempHead));
      y = selfY() + 1500 * sin(degToRad(tempHead));
      v3 = g.vertices[cpIndexXY(g, x, y)];
      angleToDest = selfAngleToXY(v2.x, v2.y);
      tempHead = modm(angleToDest + 60, MAX_DEG);
      x = v3.x + 1500 * cos(degToRad(tempHead));
      y = v3.y + 1500 * sin(degToRad(tempHead));
      v4 = g.vertices[cpIndexXY(g, x, y)];
/*      
      v3 = g.vertices[rand() % g.num_v];
      deg = getAngle(v3.x, v2.x, v3.y, v2.y);
      r = rand() % 40 - 20;
      x = v3.x + (rand() % 1000) - 500;
      y = v3.y + (rand() % 1000) - 500;
      v4 = g.vertices[cpIndexXY(g, x, y)];
*/
      printf("MIDPOINTS: %3d %3d\n", v3.id, v4.id);
      dijkstraN(g, v1, v2, path, 2, v3, v4);
      break; 
  }
 
  //Print the path found.
  print_path(path);
}


//Finds a path from this drone's current location to the given XY point, using
//the given path-finding algorithm.
void pathToPointXY(int alg, graph_t g, int x, int y, int *path)
{
  int id = g.vertices[cpIndexXY(g, x, y)].id;
  pathToPointId(alg, g, id, path);
}


//Establishes a path to the given XY point using the given path algorithm, and
//sets the pathVector value so the drone can fly there.
void goToXY(int alg, graph_t g, int x, int y)
{
  static int *path = NULL;
  static bool atDest = true;
  static int interX = -1, interY = -1, pathIndex = 0;
  bool badPath, edgeNotFound; 
  graph_t tg;
  int i;
  vertex_t v1, v2;

  if(!path)
  {
    path = malloc(sizeof(int) * map.num_v);
    if(!path)
    {
      perror("ERROR: couldn't allocate memory\n");
      abort();
    }
  }
  
  if(atDest)
  {
    if(distForm(selfX(), x, selfY(), y) < POINT_PRECISION)
    {
      return;
    }
      
    if(avoidant)
    {
      tg = g;
      do
      {
        badPath = false;
        pathToPointXY(alg, tg, x, y, path);
        pathIndex = 0;
        while(pathIndex + 1 < length(path))
        {
          edgeNotFound = true;
          v1 = tg.vertices[getMPIndex(tg, path[pathIndex])];
          v2 = tg.vertices[getMPIndex(tg, path[pathIndex + 1])];
        
          i = -1;
          while(edgeNotFound && ++i < NUM_AVOIDPTS)
          {
            if(dangerPoints[i].x != -1
               && lineInCircle(v1.x, v1.y, v2.x, v2.y, 
                  dangerPoints[i].x, dangerPoints[i].y, dangerPoints[i].r))
            {
              tg = remove_edge(tg, v1, v2);
              badPath = true;
              edgeNotFound = false;
            }
          }
          pathIndex++;
        }
      }
      while(badPath);
/*
       tg = g;
       for(i = 0; i < g.num_e; i++)
       {
         vertex_t v1 = g.edges[i].v1;
         vertex_t v2 = g.edges[i].v2;
         if(lineInCircle(v1.x, v1.y, v2.x, v2.y, 1820, 3780, 500))
         {
           tg = remove_edge(tg, v1, v2);
         }
       }
       pathToPointXY(alg, tg, x, y, path);
*/
    }
    else
    {
      pathToPointXY(alg, g, x, y, path);
    }

    atDest = false;    
    pathIndex = 0;
    interPointId = path[pathIndex];
    interX = g.vertices[getMPIndex(g, interPointId)].x;
    interY = g.vertices[getMPIndex(g, interPointId)].y;    
  }

  if(distForm(selfX(), interX, selfY(), interY) < POINT_PRECISION 
     && pathIndex < length(path)-1)
  {
    interPointId = path[++pathIndex];
    interX = g.vertices[getMPIndex(g, interPointId)].x;
    interY = g.vertices[getMPIndex(g, interPointId)].y;
  }
  else if(distForm(selfX(), interX, selfY(), interY) < POINT_PRECISION)
  {
    atDest = true;
    return;
  }

  pathVector = selfAngleToXY(interX, interY);
}


//Go to the map vertex with the given id.
void goToId(int alg, graph_t g, int id)
{
  int x = g.vertices[getMPIndex(g, id)].x;
  int y = g.vertices[getMPIndex(g, id)].y;

  goToXY(alg, g, x, y);
}


//If we currently don't have anywhere to go, pick a random point and go there.
void goToRandom(int alg, graph_t g)
{
  static int destX, destY;

  destX = rand() % mapWidth;
  destY = rand() % mapHeight;
 
  goToXY(alg, g, destX, destY);
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
  delX = SIGN(delX) * MAX(abs(delX), 40);
  delY = (int)(WALL_LOOK_AHEAD * sin(currHeadingRad)); 
  delY = SIGN(delY) * MAX(abs(delY), 40);
  
  newX = currX + delX;
  newY = currY + delY;

  seeWallX = wallBetween(currX, currY, newX, currY, 1, 1);
  seeWallY = wallBetween(currX, currY, currX, newY, 1, 1);
  seeWallAhead = wallBetween(currX, currY, newX, newY, 1, 1);

  lHead = modm(currHeadingDeg + 15, MAX_DEG);
  seeWallL = wallFeeler(WALL_LOOK_AHEAD, lHead, 1, 1);
  rHead = modm(currHeadingDeg - 15, MAX_DEG);
  seeWallR = wallFeeler(WALL_LOOK_AHEAD, rHead, 1, 1);
 
  //Check if we are close to one of the corners of the map.
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
      r = (rand() % 50) - 25;
      wallAvoidVector = modm(currHeadingDeg + 90 + r, MAX_DEG);

      //Set a turnLock of 28 frames: we will not be able to set a new wall avoidance
      //angle for 28 frames, approximately 2 seconds.
      turnLock = 14;
      SLOW;
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
    else if(seeWallAhead || seeWallR)
    {
      wallAvoidVector = modm(currHeadingDeg + 90, MAX_DEG);
      turnLock = 5;      
    }

    else if(seeWallL)
    {
      wallAvoidVector = modm(currHeadingDeg - 90, MAX_DEG);
      turnLock = 5;
    }

    else
    {
      wallAvoidVector = -1;
    }
  }
  //If there's a turnLock on, count down.
  else
  {
    turnLock--;
  }
}


//Check if we are near a designated corner point, and return a boolean value.
bool nearCornerPeek()
{
  if(interPointId != -1)
  {
    int x = map.vertices[getMPIndex(map, interPointId)].x;
    int y = map.vertices[getMPIndex(map, interPointId)].y;
    if(found_in_array(corners_list, interPointId) 
       && distForm(selfX(), x, selfY(), y) < 200)
    {
      return true;
    }
  }
 
  return false;
}


/*****************************************************************************
 * Friendly Cohesion
 * ***************************************************************************/

//Computes the angle toward the average location of all friends within a certain radius.
void cohesion()
{
  int avgFriendX = averageFriendRadarX(COHESION_RADIUS, fov);
  int avgFriendY = averageFriendRadarY(COHESION_RADIUS, fov);
 
  if(avgFriendX != -1 && avgFriendY != -1)
  {
    cohesionVector = getAngle(selfX(), avgFriendX, selfY(), avgFriendY);
  }
  else
  {
    cohesionVector = -1;
  }
}


/*****************************************************************************
 * Enemy Separation
 * ***************************************************************************/

void separation()
{
  int avgFriendX = averageFriendRadarX(SEPARATION_RADIUS, fov);
  int avgFriendY = averageFriendRadarY(SEPARATION_RADIUS, fov);
  int sepVec = getAngle(selfX(), avgFriendX, selfY(), avgFriendY);

  if(avgFriendX != -1 && avgFriendY != -1)
  {
    separationVector = modm(sepVec + 180, 360);
  }
  else
  {
    separationVector = -1;
  }
}


/*****************************************************************************
 * Friendly Alignment 
 * ***************************************************************************/

void alignment()
{
  alignmentVector = avgFriendlyDir(ALIGNMENT_RADIUS, fov); 
}
 

/*****************************************************************************
 * No Enemy Flying (Controller)
 * ***************************************************************************/

//TODO: find a way to balance wa ll avoidance, neighbor cohesion, and other vectors
void noEnemyFlying()
{
  int numVec = 0, totalVec = 0;

  wallAvoidance();
  cohesion();
  separation();  
  alignment();

  if(focused)
  {
    goToId(ALG_DIJKSTRA, map, 94);

    degToAim = pathVector;
  }

  if(wallAvoidVector != -1)
  {
    degToAim = wallAvoidVector;
  }
  else if(!isLeader)
  { 
    if(alignmentVector != -1)
    {
      totalVec += alignmentVector * aWeight;
      numVec += aWeight;
    }
    
    if(separationVector != -1)
    {
      totalVec += separationVector * sWeight;
      numVec += sWeight;
    }
   
    if(cohesionVector != -1)
    {
      totalVec += cohesionVector * cWeight;
      numVec += cWeight;
    }

    if(numVec > 0)
    {
      degToAim = totalVec / numVec;
    }
  }

  turnToDeg(degToAim);

  if(mobile)
  {
    if(cautious && nearCornerPeek())
    {
      SLOW;
    }
    else
    {
      FAST;
    }
  }
  else
  {
    STOP;
  }
}


/*****************************************************************************
 * Update Enemy Info
 * ***************************************************************************/

//update the enemy's past movement info to include the most recent info on speed,
//location, angle, and id
void updateEnemyPast(int id, int x, int y, double spd, int deg)
{
  pastMovement.x = x;
  pastMovement.y = y;
   
  //if one of these component values is -1, replace it with the last meaningful entry
  if(id != -1) 
  {
    pastMovement.id = id;
  }

  if(deg != -1) 
  {
    pastMovement.deg = deg;
  }

  if(abs(spd + 1) > EPSILON) 
  {
    pastMovement.spd = spd;
  }
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

  enemyHeadingToMe = getAngle(pastMovement.x, selfX(), pastMovement.y, selfY());
  enemyHeading = pastMovement.deg;
  headingDiff = abs(enemyHeadingToMe - enemyHeading);
  
  if(headingDiff < 30 || headingDiff > 330)
  {
    distInFront = 0;
  }
  else if(pastMovement.spd < 3)
  {
    distInFront = 0;
  }
  else
  {
    distInFront = MIN(2 * (dist - (dist % 50)), 300);
  }

  //compute the new target location and turn
  int newX = pastMovement.x + (distInFront * cos(degToRad(pastMovement.deg))); 
  int newY = pastMovement.y + (distInFront * sin(degToRad(pastMovement.deg)));

  aimInRad = atan2(newY - selfY(), newX - selfX());
  degToAim = radToDeg(aimInRad);
}


/*****************************************************************************
 * Engaging Enemy (Controller)
 * ***************************************************************************/

//engaging: compute the id, speed, and heading of the nearest enemy, if possible,
//and adjust course as necessary
void engaging(int enemyX, int enemyY, int enemyDist)
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


/*****************************************************************************
 * Run Away 
 * ***************************************************************************/

void runAway()
{
  static bool done = true;
  static int newX, newY;

  int enemyX, enemyY;
  int angleToEnemy, otherWay;
  int lookAhead = 300;
  int closestPoint;

  enemyX = getNearestEnemyX();
  enemyY = getNearestEnemyY();
  angleToEnemy = selfAngleToXY(enemyX, enemyY);
  otherWay = modm(angleToEnemy + 180, MAX_DEG);

  newX = selfX() + lookAhead * cos(degToRad(otherWay));
  newY = selfY() + lookAhead * sin(degToRad(otherWay));

  wallAvoidance();

  if(wallAvoidVector != -1)
  {
    degToAim = wallAvoidVector;
  }
  else
  {
    degToAim = selfAngleToXY(newX, newY);
  }

  turnToDeg(degToAim);
  thrust(1);
}


/*****************************************************************************
 * Stealth
 * ***************************************************************************/

void stealth()
{
  thrust(0);
}


/*****************************************************************************
 * Enemy Spotted (Controller) 
 * ***************************************************************************/

void enemySpotted()
{
  if(preserving)
  {
    runAway();
  }
  else if(stealthy)
  {
    stealth(); 
  }
  else
  {
    int x = getNearestEnemyX();
    int y = getNearestEnemyY();
    int d = distForm(selfX(), x, selfY(), y);
    engaging(x, y, d);
  }
}


/*****************************************************************************
 * Message Handler (through chat feature) 
 * ***************************************************************************/

void handleMsg()
{
  if(strcmp(thisMsg, scanMsg(0)))
  {
    char buf[100];
    char *tok = NULL;

    pastMsg = thisMsg;
    thisMsg = scanMsg(0);
    
    strcpy(buf, thisMsg);
    tok = strtok(buf, " ");

    if(!strcmp(tok, "mobile"))
    {
      tok = strtok(NULL, " ");
      mobile = atoi(tok);
    }
    else if(!strcmp(tok, "cautious"))
    {
      tok = strtok(NULL, " ");
      cautious = atoi(tok);
    }
    else if(!strcmp(tok, "preserving"))
    {
      tok = strtok(NULL, " ");
      preserving = atoi(tok);
    }
    else if(!strcmp(tok, "stealthy"))
    {
      tok = strtok(NULL, " ");
      stealthy = atoi(tok);
    }
    else if(!strcmp(tok, "focused"))
    {
      tok = strtok(NULL, " ");
      focused = atoi(tok);
    }
    else if(!strcmp(tok, "aweight"))
    {
      tok = strtok(NULL, " ");
      aWeight = atoi(tok);
    }
    else if(!strcmp(tok, "sweight"))
    {
      tok = strtok(NULL, " ");
      sWeight = atoi(tok);
    }
    else if(!strcmp(tok, "cweight"))
    {
      tok = strtok(NULL, " ");
      cWeight = atoi(tok);
    }
    else if(!strcmp(tok, "fov"))
    {
      tok = strtok(NULL, " ");
      fov = atoi(tok);
    }
    else if(!strcmp(tok, "rov"))
    {
      tok = strtok(NULL, " ");
      rov = atoi(tok);
    }
    else if(!strcmp(tok, "avoidant"))
    {
      tok = strtok(NULL, " ");
      avoidant = atoi(tok);
    }
    else if(!strcmp(tok, "leader"))
    {
      tok = strtok(NULL, " ");

      int i = -1;
      if(!found_in_array(leaders, atoi(tok)))
      {
        while(leaders[++i]);
        leaders[i] = atoi(tok);
      }
/*
      printf("ALL LEADERS CURRENTLY\n");
      for(i = 0; i < NUM_LEADERS; i++)
        if(leaders[i])
          printf("%d\n", leaders[i]);
*/
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
  static int enemySpottedFC = 0;
  char numMsg[4], talkMsg[10];
  char buf[100], *tok = NULL, *tm = NULL;

  if(frameCount == 300 && isLeader)
  {
    strcpy(talkMsg, talkMsgs[MSG_LEADER]);
    sprintf(numMsg, "%d", selfID());
    talk(strcat(talkMsg, numMsg));
  }

  //by default, don't thrust, and increment the frame counter
  frameCount = (frameCount + 1) % INT_MAX;

  //also by default, assume no enemy is nearby
  state = STATE_NOENEMY;

  //check the input buffer to try changes in behavior
  handleMsg();

  //if there is an enemy closeby on the map, engage
  if(radarEnemyInView(fov, rov) || enemySpottedFC)
  {
    state = STATE_ENEMY_SPOTTED;
  }

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

    case(STATE_ENEMY_SPOTTED):
      enemySpotted();
      if(enemySpottedFC == 0)
      {
        enemySpottedFC = 56;
      }
      else
      {
        enemySpottedFC--;
      }
      break;

    case(STATE_NOENEMY):
      noEnemyFlying();
      break;

    case(STATE_DEAD):
      strcpy(buf, scanMsg(0));
      tok = strtok(buf, " ");
      if(strcmp(tok, "died"))
      {
        tm = talkMsgs[MSG_DIED];
        sprintf(numMsg, "%d", selfID());
        talk(strcat(tm, numMsg));
      }
      state = STATE_NOENEMY;
      break;
  }
}


/*****************************************************************************
 * Main (get cmd line info and start AI) 
 * ***************************************************************************/

int main(int argc, char *argv[]) 
{
  idx = strtol(argv[2], NULL, 10);
  tot_idx = strtol(argv[3], NULL, 10);
  filename = argv[4];  

  printf("idx:      %d\n", idx);
  printf("tot_idx:  %d\n", tot_idx);
  printf("filename: %s\n", filename);

  if(idx % 4 == 1)
  {
    isLeader = true;
    printf("IS LEADER\n");
  }

  //generate a random initial heading
  srand(time(NULL));
  degToAim = rand() % 360;

  return start(argc, argv);
}
