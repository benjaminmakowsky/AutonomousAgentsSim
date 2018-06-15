//Matthew Coffman - May 2018
//Compile: ./buildChaser.sh && export LD_LIBRARY_PATH=.
//Run: ./runChaser.sh <n> <filename> for some natural number n (defaults to 1 
//  if no number given) and the name of a file containing map information
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

//drones can be initializing, flying around, engaging, or dead
enum State
{
  STATE_INIT,
  STATE_NOENEMY,
  STATE_ENGAGED,
  STATE_DEAD
};

enum GraphAlg
{
  ALG_ASTAR,
  ALG_BFS,
  ALG_DFS,
  ALG_DIJKSTRA
};

//structure to store information on the nearest enemy's recent movements
struct movementInfo 
{
  int id;
  int x;
  int y;
  double spd;
  int deg;
};

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
int frameCount = 0;				//keeps track of time by counting frames
bool init = false;				//not initialized for the first 40 frames
int degToAim;					//the angle at which we want to head
int state = STATE_INIT;				//the current drone state (enum above)
struct movementInfo mi[PAST_INFO_NUM] = {0};	//stores info on enemy's past movements
int turnLock = 0;
int mapWidth, mapHeight, halfMapWidth, halfMapHeight;
int wallLookAhead = 100, cornerDist = 75;
int cohesionRadius = 400, separationRadius = 50;
int wallAvoidVector = -1, cohesionVector = -1, separationVector = -1;
graph_t map = {0};
int num_points, num_edges;
bool atInter = true, atDest = true;
int *path = NULL, pathIndex = -1, interPointId = -1;

//argv params
int idx;
int tot_idx;
char *filename;

//resets the array that stores info on past enemy movements: we might want to use
//this at the start, and later when a chaser picks up a new target
void resetMI()
{
  int i;
  for(i = 0; i < PAST_INFO_NUM; i++)
    mi[i] = (struct movementInfo) {0, 0, 0, 0.0, 0.0};
}

void getMap()
{
  FILE *fp = fopen(filename, "r");
  char buf[BUFFER_SIZE];
  char *tok = NULL;
  int i;

  fgets(buf, sizeof(buf), fp);
  tok = strtok(buf, " ");
  num_points = atoi(tok);

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
  
  fgets(buf, sizeof(buf), fp);
  tok = strtok(buf, " ");
  num_edges = atoi(tok);
  
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

  fclose(fp);

  path = malloc(sizeof(int) * map.num_v + 1);
  if(!path)
  {
    perror("ERROR: couldn't allocate memory for path\n");
    abort();
  }
}

int closestMapPointSelf()
{
  return closestMapPointXY(selfX(), selfY());
}

int closestMapPointXY(int x, int y)
{
  int i, minDist = INT_MAX, minIndex = -1, tempDist;

  for(i = 0; i < map.num_v; i++)
  {
    tempDist = distanceFormula(x, map.vertices[i].x, y, map.vertices[i].y);
    if(tempDist < minDist)
    {
      minDist = tempDist;
      minIndex = i;
    }
  }
  
  return minIndex;
}

void pathToPointId(int alg, int id)
{
  int i, cpToMe = closestMapPointSelf();

  for(i = 0; i < map.num_v+1; i++)
    path[i] = '\0';

  switch(alg)
  {
    case(ALG_ASTAR):
      astar(map, map.vertices[cpToMe], map.vertices[id-1], path);  
      break;

    case(ALG_BFS):
      bfs(map, map.vertices[cpToMe], map.vertices[id-1], path);  
      break;

    case(ALG_DFS):
      dfs(map, map.vertices[cpToMe], map.vertices[id-1], path);  
      break;

    case(ALG_DIJKSTRA):
      dijkstra(map, map.vertices[cpToMe], map.vertices[id-1], path); 
      break; 
  }
 
  print_path(path);
}

void pathToPointXY(int alg, int x, int y)
{
  pathToPointId(alg, closestMapPointXY(x, y) + 1);
}

void goToXY(int alg, int x, int y)
{
  if(atDest)
  {
    if(distanceFormula(selfX(), x, selfY(), y) < 30)
      return;

    pathToPointXY(alg, x, y);
    pathIndex = 0;
    atDest = false;
  }

  if(interPointId != -1)
  {
    int interX = map.vertices[interPointId-1].x;
    int interY = map.vertices[interPointId-1].y;
    
    if(distanceFormula(selfX(), interX, selfY(), interY) < 30)
    {
      if(pathIndex == length(path)-1)
      {
        printf("PATH DONE!!!\n");
        atDest = true;
        return;
      }  
      interPointId = path[++pathIndex];
      interX = map.vertices[interPointId-1].x;
      interY = map.vertices[interPointId-1].y;
      printf("1. NEW TARGET %4d %4d\n", interX, interX);
    }
    
    degToAim = angleToXY(interX, interY);
 
  }
  else
  {
    interPointId = path[++pathIndex];
    degToAim = angleToXY(map.vertices[interPointId-1].x, map.vertices[interPointId-1].y);
    printf("2. NEW TARGET %4d %4d\n", map.vertices[interPointId-1].x, map.vertices[interPointId-1].y);
  }
}

//initialization: spends the first 50 frames turning to the desired angle, and then
//declares initialization complete and switches to the NOENEMY state
void Initialize()
{
  turnToDeg(degToAim);

  if(frameCount % 50 == 0)
  {
    //generate a representation of the map, using the given map file
    getMap();

    //get the map dimensions for later use
    mapWidth = getMapWidth();
    mapHeight = getMapHeight();
    halfMapWidth = mapWidth / 2;
    halfMapHeight = mapHeight / 2;

    //declare initialized and set state to no enemies
    init = true;
    state = STATE_NOENEMY;
  }
}


//provides a mechanism for drones to spot walls ahead and steer away from them
void wallAvoidance()
{
  double currHeadingRad;
  int currHeadingDeg;
  int currX, currY, newX, newY, delX, delY;
  bool seeWallX, seeWallY, seeWallAhead, cornerClose;
  int r;

  //get the current heading, in degrees and radians, and current position info 
  currHeadingRad = selfHeadingRad();
  currHeadingDeg = (int)radToDeg(currHeadingRad);
  currX = selfX();
  currY = selfY(); 

  //check if we will be off the map if we carried on the current course for some 
  //number of pixels ("some number" defined globally above)
  delX = (int)(wallLookAhead * cos(currHeadingRad)); 
  delY = (int)(wallLookAhead * sin(currHeadingRad)); 
  newX = currX + delX;
  newY = currY + delY;
  //seeWallX = newX < 0 || newX > mapWidth;
  //seeWallY = newY < 0 || newY > mapHeight;
  seeWallX = wallBetween(currX, currY, currX + delX, currY, 1, 1);
  seeWallY = wallBetween(currX, currY, currX, currY + delY, 1, 1);
  seeWallAhead = wallBetween(currX, currY, currX + delX, currY + delY, 1, 1);
 
  //check if we are close to one of the corners of the map
  //cornerClose = closeToCorner(cornerDist);
  cornerClose = (wallFeeler(cornerDist, 0, 1, 1) || wallFeeler(cornerDist, 180, 1, 1)) && (wallFeeler(cornerDist, 90, 1, 1) || wallFeeler(cornerDist, 270, 1, 1));

  //if turnLock is off, meaning we are allowed to set a new turn angle
  if(!turnLock)
  {
    //if we are close to a corner
    if(cornerClose)
    {
      //turn toward the midpoint of the map and fly in that direction, give or take
      //25 degrees
      r = (rand() % 50) - 25;
      wallAvoidVector = radToDeg(atan2(halfMapHeight - currY, halfMapWidth - currX)) + r;
      wallAvoidVector = modm(wallAvoidVector, MAX_DEG);

      //set a turnLock of 28 frames: we will not be able to set a new turn angle for
      //28 frames, approximately 2 seconds
      turnLock = 28;
    }

    // if we see a vertical wall, mirror our heading on the y-axis
    else if(seeWallX)
      wallAvoidVector = modm(180 - currHeadingDeg, MAX_DEG);
    
    //if we see a horizontal wall, mirror our heading on the x-axis
    else if(seeWallY)
      wallAvoidVector = modm(-currHeadingDeg, MAX_DEG);
    
    //if the wall isn't perfectly horizontal or vertical, turn a little bit
    //(note that this solution is not optimal, still bumps into walls occasionally)
    else if(seeWallAhead)
    {
      wallAvoidVector = modm(currHeadingDeg + 90, MAX_DEG);
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
  //if there's a turnLock on, count down
  else
    turnLock--;
}


//computes the vector toward the average location of all friends within a certain
//radius
void cohesion()
{
  int avgFriendX = averageFriendRadarX(cohesionRadius);
  int avgFriendY = averageFriendRadarY(cohesionRadius);
 
  if(avgFriendX != -1 && avgFriendY != -1)
  {
    cohesionVector = getHeading(selfX(), avgFriendX, selfY(), avgFriendY);
  }
  else
  {
    cohesionVector = -1;
  }
}

void separation()
{
  int avgFriendX = averageFriendRadarX(separationRadius);
  int avgFriendY = averageFriendRadarY(separationRadius);

  if(avgFriendX != -1 && avgFriendY != -1)
  {
    separationVector = modm(getHeading(selfX(), avgFriendX, selfY(), avgFriendY)+180,360);
  }
  else
  {
    separationVector = -1;
  }
}

//TODO: find a way to balance wall avoidance, neighbor cohesion, and other vectors
void NoEnemyFlying()
{
  wallAvoidance();
  cohesion();
  separation(); 
  
  if(wallAvoidVector != -1)
    degToAim = wallAvoidVector;

  else if(separationVector != -1)
    degToAim = separationVector;

  else if(cohesionVector != -1)
    degToAim = cohesionVector;    

  goToXY(ALG_DIJKSTRA, 4000, 200);

  turnToDeg(degToAim);
  if(!atDest)
    thrust(1);
}


//update the enemy's past movement info to include the most recent info on speed,
//location, angle, and id
void updateEnemyPast(int id, int x, int y, double spd, int deg)
{
  int i;
  
  //shift old array entries down to make room for the newest data point
  for(i = PAST_INFO_NUM-1; i > 0; i--)
    mi[i] = mi[i-1];

  //store the most recent state of the enemy's motion
  mi[0] = (struct movementInfo) {id, x, y, spd, deg};
  
  //if one of these component values is -1, replace it with the last meaningful entry
  if(id == -1) 
    mi[0].id = mi[1].id; 

  if(deg == -1) 
    mi[0].deg = mi[1].deg; 

  if(abs(spd + 1) < EPSILON) 
    mi[0].spd = mi[1].spd;
}


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


//this is where the magic happens (i.e., this loop fires every frame and decides
//which state the chaser is currently in and acts accordingly)
AI_loop()
{
  //by default, don't thrust, and increment the frame counter
  thrust(0);
  frameCount = (frameCount + 1) % 100;
 
  //also by default, assume no enemy is nearby
  state = STATE_NOENEMY;

  //if there is an enemy closeby on the map, engage
  int distToEnemy = distanceToNearestEnemy();
  if(distToEnemy > 0 && distToEnemy < 500)
    state = STATE_ENGAGED;

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

    case(STATE_ENGAGED):
      Engaging(getNearestEnemyX(), getNearestEnemyY(), distToEnemy);
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
