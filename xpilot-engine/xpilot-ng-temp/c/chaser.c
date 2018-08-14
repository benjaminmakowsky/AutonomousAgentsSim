//Matthew Coffman - May 2018
//Compile: ./buildChaser.sh && export LD_LIBRARY_PATH=.
//Run:     ./runChaser.sh <n> <filename> for some natural number n (defaults to 1) 
//         and the name of a file containing map information
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
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define SIGN(X) ((X) > 0 ? 1 : ((X) < 0 ? -1 : 0))

//global constants
#define BUFFER_SIZE 255
#define EPSILON 1E-6
#define MAX_DEG 360
#define NUM_AVOIDPTS 10
#define NUM_FOLLOWERS 20
#define NUM_LEADERS 20
#define PAST_INFO_NUM 10
#define POINT_PRECISION 100

//function prototypes
int vertex_cmp_by_id(const void *a, const void *b);
void getMap();
void modMap();
void initialize();
int getMPIndex(graph_t, int);
int cpIndexXY(graph_t, int, int);
int cpIndexSelf(graph_t);
void pathToPointId(int, graph_t, int, int *);
void pathToPointXY(int, graph_t, int, int, int *);
void goToXY(int, graph_t, int, int);
void goToId(int, graph_t, int);
void goToRandom(int, graph_t);
void interruptPath();
void runAway();
void wallAvoidance();
void cohesion();
void separation();
void alignment();
bool nearCornerPoint();
void noEnemyFlying();
void updateEnemyPast(int, int, int, double, int);
void updateAim(int);
void engaging(int, int, int);
void stealth();
void enemySpotted();
void printDangerPoints();
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
bool init = false;		//whether we have initialized
bool isLeader = false;		//whether we are a leader
int state = STATE_INIT;		//what state we're in (enumerated above)
int frameCount = 0;		//enables us to keep track of time
int mapWidth;			//dimensions of the map
int mapHeight;
int degToAim;			//the angle we want to be going
int turnLock = 0;		//how long before we can turn again (prevents
				//turning too often, getting disoriented)
int wallAvoidVector = -1;	//the direction we must go to avoid walls
int pathVector = -1;		//the direction to follow a path to some destination
int interPointId = -1; 		//if we're following a path in focused flying, this
				//stores the id of the next point in the path
int *corners_list = NULL;	//list of corner points for cautious flying
int preserving = 0;		//run away from enemies
int stealthy = 1;		//"cloak" when enemies spotted
int mobile = 1;			//whether we move at all; as opposed to anchored
int cautious = 0;		//go slow around corner points
int focused = 1;		//fly directly to some point, rather than aimlessly
int avoidant = 1;		//avoid marked "dangerous" points
char *currMsg = "dummy";	//most recent message in the buffer
int fov = 60;			//field of vision (degrees away from current heading)
int rov = 500;			//range of vision
int *leaders = NULL;		//list of leaders on one's team
int *followers = NULL;		//list of one's followers
graph_t map;			//internal representation of the map
graph_t safeMap;		//representation of map with dangerous edges removed
moveInfo_t pastMovement;	//keeps track of closest enemy's past movement info
avoidpt_t dangerPoints[NUM_AVOIDPTS];	//keeps track of points on the map to avoid

//argv params
int idx;			//this drone's id
int tot_idx;			//the total number of drones on my team 
char *filename;			//file containing all the points and edges in the map


/*****************************************************************************
 * Internal Map Generation
 * ***************************************************************************/

//Given two vertices, sorts them by id in ascending order.
//This function will be used in the call to qsort() below in map generation.
int vertex_cmp_by_id(const void *a, const void *b)
{
  vertex_t *ia = (vertex_t *)a;
  vertex_t *ib = (vertex_t *)b;

  return ia->id - ib->id;
}


//Open the given file and generate an internal map representation.
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
  
  //Though they should already be sorted, ensure that the map's vertices are
  //sorted by id.
  qsort(map.vertices, num_points, sizeof(vertex_t), vertex_cmp_by_id);

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

  //Find out which points in the map have been specifically labeled corner points
  //(i.e. they were labeled with 'c's in the ascii map), and keep track of them
  //for flying slowly around corners as part of the cautious trait.
  int num_corners = 0;
  while(map.vertices[num_corners].y 
          >= map.vertices[num_corners++ + 1].y);

  corners_list = malloc(sizeof(int) * num_corners);
  for(i = 0; i < num_corners; i++)
  {
    corners_list[i] = map.vertices[i].id;
  }
}


//Generate a modified "safe map" that removes edges that go through points labeled
//dangerous that we want to avoid.
//This function will be called whenever the dangerPoints array is modified, allowing
//drones with the avoidant trait to fly around dangerous areas.
void modMap()
{
  vertex_t v1, v2;
  int i, j;
  struct timeval start, end;

  gettimeofday(&start, NULL);

  //Initially, the safe map is just the whole map.
  safeMap = map; 

  //For every edge in the map, check if it intersects any of the dangerPoints, and if
  //so remove it from the safe map.
  for(i = 0; i < map.num_e; i++)
  {
    v1 = map.edges[i].v1;
    v2 = map.edges[i].v2;

    j = -1;
    while(++j < NUM_AVOIDPTS)
    {
      if(lineInCircle(v1.x, v1.y, v2.x, v2.y,
                      dangerPoints[j].x, dangerPoints[j].y, dangerPoints[j].r))
      {
        safeMap = remove_edge(safeMap, v1, v2);
        break;
      }
    }
  }

  gettimeofday(&end, NULL);

  //Display how long it took to generate this safe map (usually about 0.3 - 0.5sec).
  printf("Time to modify map (usec): %ld\n",
          (end.tv_sec - start.tv_sec)*1000000L
           + end.tv_usec - start.tv_usec);
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

  //Set up leader/follower arrays (not really being used right now).
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
 
  //Initialize the map to have points the drones want to avoid.
  for(i = 0; i < NUM_AVOIDPTS; i++)
  {
    dangerPoints[i] = (avoidpt_t) {-1, -1, -1};
  }

  //Generate the "safe map" for avoidant behavior.
  modMap();

  //Declare initialized and set state to no enemies.
  init = true;
  state = STATE_NOENEMY;
  thrust(0);
}


/*************************************************************************** 
 * Path Generation
 * *************************************************************************/

//Finds the index of the point with the given id, using a binary search on a
//(presumably) sorted array of vertices.
int getMPIndex(graph_t g, int id)
{
  //We'll use l and r to refer to the boundaries of the array chunk we're still
  //searching, and m will refer to the midpoint of this chunk. Hence, l and r are
  //initially 0 and one less than the total number of vertices, respectively.
  int l = 0, r = g.num_v - 1, m;

  while(l <= r)
  {
    //Compute the midpoint of the array chunk.
    m = l + (r - l)/2;
    
    //If we've found the desired id, return its index.
    if(g.vertices[m].id == id)
    {
      return m;
    }

    //If the id we're looking for is too big to be in the left half of the chunk,
    //update the left boundary l accordingly.
    if(g.vertices[m].id < id)
    {
      l = m + 1;
    }
    //Conversely, if the desired id is in the left chunk, update the right boundary
    //r accordingly.
    else
    {
      r = m - 1;
    }
  }

  //If we've gotten this far, it must be the case that l > r, so the desired id is
  //not in the array at all. Return an invalid value of -1.
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
    tempDist = computeDistance(x, tempX, y,tempY);

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
  int path_failure = 0;
  int i, cpToMe = cpIndexSelf(g), x, y, deg, r, angleToDest, tempHead;
  vertex_t v1, v2, v3, v4;
  
  v1 = g.vertices[cpToMe];
  v2 = g.vertices[getMPIndex(g, id)];

  //Switch on the given path-finding algorithm, enumerated globally above.
  switch(alg)
  {
    case(ALG_ASTAR):
      path_failure = astar(g, v1, v2, path);  
      break;

    case(ALG_BFS):
      bfs(g, v1, v2, path);  
      break;

    case(ALG_DFS):
      dfs(g, v1, v2, path);  
      break;

    case(ALG_DIJKSTRA):
      path_failure = dijkstra(g, v1, v2, path);
      break;

    //This extended Dijkstra's algorithm allows a drone to go from one point to
    //another, stopping at an indefinite number of points along the way. 
    //TODO: Figure out how best to determine intermediate points.
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
      printf("MIDPOINTS: %3d %3d\n", v3.id, v4.id);
      dijkstraN(g, v1, v2, path, 2, v3, v4);
      break; 
  }

  print_path(path);

  if(path_failure)
  {
    interruptPath();
//    focused = 0;
  }
}


//Establishes a path to the given XY point using the given path algorithm, and
//sets the pathVector value so the drone can fly there.
void goToId(int alg, graph_t g, int id)
{
  static int *path = NULL;
  static int destId = -1;
  static int interX = -1;
  static int interY = -1;
  static int pathIndex = 0;

  //Allocate memory for the path variable, if that hasn't been done already.
  if(!path)
  {
    path = malloc(sizeof(int) * map.num_v);
    if(!path)
    {
      perror("ERROR: couldn't allocate memory\n");
      abort();
    }
  }

  //We're using -1 to represent a "null" id. If we're told to fly to an id of -1,
  //just update the destination id accordingly and return. We can use this "null" 
  //value to effectively interrupt or cancel the path in progress, enabling more
  //adaptability and dynamic path-finding.
  if(id == -1)
  {
    destId = id;
    return;
  }

  //If our target id has been changed, whether to -1 or some other valid vertex id,
  //update the destination id and plot our new course.
  if(id != destId)
  {
    destId = id;

    //If we're using the avoidant behavior, plot our new path using the "safe"
    //version of the map that removes all edges through "dangerous" points.
    if(avoidant)
    {
      pathToPointId(alg, safeMap, destId, path);
    }
    //Otherwise, just use the given map to plot our course.
    else
    {
      pathToPointId(alg, g, destId, path);
    }

    pathIndex = 0;
    interPointId = path[pathIndex];
    interX = g.vertices[getMPIndex(g, interPointId)].x;
    interY = g.vertices[getMPIndex(g, interPointId)].y;
    return;
  }

  if(wallBetween(selfX(), selfY(), interX, interY, 1, 1))
  {
    interruptPath();
    return;
  }  

 
  //If we're close to the next intermediate point, and if that point happens to be
  //the destination, then return.
  if(computeDistance(selfX(), interX, selfY(), interY) < POINT_PRECISION
     && interPointId == destId)
  {
    return;
  }

  //If we're within a short distance of the next intermediate point, but that point
  //is not the destination, just update the intermediate vertex.
  else if(computeDistance(selfX(), interX, selfY(), interY) < POINT_PRECISION)
  {
    interPointId = path[++pathIndex];
    interX = g.vertices[getMPIndex(g, interPointId)].x;
    interY = g.vertices[getMPIndex(g, interPointId)].y;
  }

  printf("%d\n", interPointId);

  //If we've made it this far, we must still have some distance to go toward our
  //next point in the path, so set the pathVector to point in that direction.
  pathVector = selfAngleToXY(interX, interY);
}


//Go to the map vertex with the given id.
void goToXY(int alg, graph_t g, int x, int y)
{
  goToId(alg, g, g.vertices[cpIndexXY(g, x, y)].id);
}


//If we currently don't have anywhere to go, pick a random point and go there.
void goToRandom(int alg, graph_t g)
{
  static int destX, destY;
  static vertex_t v;

  if((!destX && !destY) || computeDistance(selfX(), v.x, selfY(), v.y) < 100)
  {
    //Pick a random x and y coordinate.
    destX = rand() % mapWidth;
    destY = rand() % mapHeight;
    v = g.vertices[cpIndexXY(g, destX, destY)];
  }
 
  //Go to that x and y coordinate pair.
  goToId(alg, g, v.id);
}


//Terminate the path where the drone is currently flying by telling the drone
//to fly to (-1, -1), which goToXY() will recognize as a termination pattern.
void interruptPath()
{
  goToId(-1, map, -1);
}


/*****************************************************************************
 * Wall Avoidance
 * ***************************************************************************/

//Provides a mechanism for drones to spot walls ahead and steer away from them.
void wallAvoidance()
{
  //Static variables: to avoid magic numbers!
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
  
  //Having generated x- and y-component vectors, add these to our current position
  //to get the x- and y-coordinates of the point where we're now looking.
  newX = currX + delX;
  newY = currY + delY;

  //Using the new x- and y-coordinates generated above, check straight in front, 
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
  
  //Determine if we see a corner ahead, by checking up/down and left/right depending
  //on our current heading.
  currHeadingDegDiv90 = currHeadingDeg / 90;
  switch(currHeadingDegDiv90)
  {
    //If we have a current heading of 0 <= theta < 90, look right and up.
    case(0):
      closeToCorner = wallFeeler(cornerLookAhead, lookRight, dummyVal, dummyVal)
                      && wallFeeler(cornerLookAhead, lookUp, dummyVal, dummyVal);
      break;

    //If we have a current heading of 90 <= theta < 180, look up and left.
    case(1):
      closeToCorner = wallFeeler(cornerLookAhead, lookUp, dummyVal, dummyVal)
                      && wallFeeler(cornerLookAhead, lookLeft, dummyVal, dummyVal);
      break;

    //If we have a current heading of 180 <= theta < 270, look left and down.
    case(2):
      closeToCorner = wallFeeler(cornerLookAhead, lookLeft, dummyVal, dummyVal)
                      && wallFeeler(cornerLookAhead, lookDown, dummyVal, dummyVal);
      break;

    //If we have a current heading of 270 <= theta < 360, look down and right.
    case(3):
      closeToCorner = wallFeeler(cornerLookAhead, lookDown, dummyVal, dummyVal)
                      && wallFeeler(cornerLookAhead, lookRight, dummyVal, dummyVal);
      break;

    //Our current heading should be between 0 <= theta < 360, but if somehow that's
    //not the case, print an error statement, and indicate no wall avoidance.
    default:
      printf("ERROR: something's weird with the current heading\n");
      wallAvoidVector = -1;
      return;
  }

  //If turnLock is off, we are allowed to set a new turn angle.
  if(!turnLock)
  {
    if(closeToCorner)
    {
      //Turn some number of degrees to the left of the current heading.
      wallAvoidVector = modm(currHeadingDeg + turnLeft, MAX_DEG);

      //Set a turn lock of some number of frames, so that we don't adjust our turn
      //angle too frequently and just end up turning back and forth endlessly.
      turnLock = lockNum;

      //Go slow as long as we're near a corner by thrusting only twice every 5 frames,
      //so we don't crash into the corners as we turn.
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
      wallAvoidVector = modm(180 - currHeadingDeg, MAX_DEG);
      turnLock = lockNum;
    }

    //If we see a horizontal wall, mirror our heading on the x-axis, and set a turn
    //lock of some number of frames.
    else if(seeWallY)
    {
      wallAvoidVector = modm(-currHeadingDeg, MAX_DEG);
      turnLock = lockNum;
    }
    
    //If we see a wall that's directly in front of us or a little to the right, turn
    //left a bit, and set a turn lock.
    else if(seeWallAhead || seeWallR)
    {
      wallAvoidVector = modm(currHeadingDeg + turnLeft, MAX_DEG);
      turnLock = lockNum; 
    }
 
    //Similarly, if we see a wall that's just a little to the left of us, turn right
    //a little, and set a turn lock.
    else if(seeWallL)
    {
      wallAvoidVector = modm(currHeadingDeg + turnRight, MAX_DEG);
      turnLock = lockNum;
    }

    //If we see no walls at all, indicate this by returning a value of -1.
    else
    {
      wallAvoidVector = -1;
    }
  }
  
  //Finally, if there's a turn lock on, we aren't supposed to turn for some number of
  //frames still. So, just decrement the turn lock counter.
  else
  {
    turnLock--;
  }
}


//Check if we are near a designated corner point, and return a boolean value.
bool nearCornerPoint()
{
  int x, y;

  //Check first that we are currently following some direct path, and that we are
  //headed for a valid intermediate vertex.
  if(interPointId != -1)
  {
    x = map.vertices[getMPIndex(map, interPointId)].x;
    y = map.vertices[getMPIndex(map, interPointId)].y;

    //If the given intermediate point is a corner point, and if we are within
    //some specified distance of that corner, return true.
    if(found_in_array(corners_list, interPointId) 
       && computeDistance(selfX(), x, selfY(), y) < 2 * POINT_PRECISION)
    {
      return true;
    }
  }
 
  //Otherwise, we are not near a corner, so return false.
  return false;
}


/*****************************************************************************
 * No Enemy Flying (Controller)
 * ***************************************************************************/

void noEnemyFlying()
{
  static int destPoints[] = {94, 9};
  static int i = 0;
  int numVec = 0, totalVec = 0;

  wallAvoidance();

  //If the drone is focused, compute the shortest path from some point to another.
  //Otherwise, the drone will just fly aimlessly.
  if(focused)
  {
    //Oscillate between the two destination points, with ids 9 and 94.
    vertex_t v = map.vertices[getMPIndex(map, destPoints[i])];
    if(computeDistance(selfX(), v.x, selfY(), v.y) < POINT_PRECISION)
    {
      i = (i + 1) % 2;
    }
    goToRandom(ALG_DIJKSTRA, map);
    //goToId(ALG_DIJKSTRA, map, destPoints[i]);
    degToAim = pathVector;
  }

  //If there is a wall nearby, aim away from it. Wall avoidance takes precedence over
  //all the other vectors below.
  if(wallAvoidVector != -1)
  {
    degToAim = wallAvoidVector;
  }

  //Whatever direction we've decided to turn to, do so.
  turnToDeg(degToAim);

  //Now that we've turned, we can decide how much to thrust.
  if(mobile)
  {
    //If we're cautious and close to a corner, fly slow around the corner.
    if(cautious && nearCornerPoint())
    {
      //Slow down by thrusting only twice every 5 frames.
      if(frameCount % 5 < 2)
      {
        thrust(1);
      }
      else
      {
        thrust(0);
      }
    }
    else
    {
      thrust(1);
    }
  }
  //If we're especially anchored, just don't move.
  //TODO: edit this behavior to allow a little wiggle room. In guarding situations,
  //a drone could potentially hover back and forth over a small area or fly in circles
  //and be anchored despite not being perfectly still.
  else
  {
    thrust(0);
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
  
  //If any of the following three variables is not -1, meaning it contains a
  //meaningful value, update it.
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

  enemyHeadingToMe = getAngleBtwnPoints(pastMovement.x, selfX(), pastMovement.y, selfY());
  enemyHeading = pastMovement.deg;
  headingDiff = abs(enemyHeadingToMe - enemyHeading);
  
  //If the target enemy is within 30 degrees of our current heading, just beeline
  //toward the enemy.
  if(headingDiff < 30 || headingDiff > 330)
  {
    distInFront = 0;
  }
  //Similarly, if the enemy isn't moving very fast at all, go directly toward him.
  else if(pastMovement.spd < 3)
  {
    distInFront = 0;
  }

  else if(dist < 100)
  {
    distInFront = 0;
  }
 
  //Otherwise, aim some distance in front of the enemy, to try to beat him to
  //wherever he's going.
  else
  {
    //TODO: fix this distInFront equation: not ideal
    distInFront = MIN(dist, 500);
  }

  //Compute the new target location and turn.
  int newX = pastMovement.x + (distInFront * cos(degToRad(pastMovement.deg))); 
  int newY = pastMovement.y + (distInFront * sin(degToRad(pastMovement.deg)));

  aimInRad = atan2(newY - selfY(), newX - selfX());

  wallAvoidance();

  if(wallAvoidVector != -1)
  {
    degToAim = wallAvoidVector;
  }
  else
  {
    degToAim = radToDeg(aimInRad);
  }
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

  //Compute the exact opposite direction from the closest enemy.
  enemyX = getNearestEnemyX();
  enemyY = getNearestEnemyY();
  angleToEnemy = selfAngleToXY(enemyX, enemyY);
  otherWay = modm(angleToEnemy + 180, MAX_DEG);

  //Compute new x and y values.
  newX = selfX() + lookAhead * cos(degToRad(otherWay));
  newY = selfY() + lookAhead * sin(degToRad(otherWay));

  //Compute the wall avoidance vector.
  wallAvoidance();

  //If there's a wall nearby, prioritize avoiding the wall. Otherwise, aim to 
  //run away from the closest enemy.
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

//TODO: Expand upon this 'stealth' behavior.
void stealth()
{
  thrust(0);
}


/*****************************************************************************
 * Enemy Spotted (Controller) 
 * ***************************************************************************/

//Determine what to do if the drone sees an enemy.
void enemySpotted()
{
  //Halt whatever path we were flying on before.
  interruptPath();

  //If this drone really wants to stay alive, run away from the closest enemy.
  if(preserving)
  {
    runAway();
  }
  //If this drone wants to be stealthy instead, call the stealth() function.
  else if(stealthy)
  {
    stealth(); 
  }
  //If we aren't trying to self-preserve or be stealthy, go ahead and engage.
  else
  {
    int x = getNearestEnemyX();
    int y = getNearestEnemyY();
    int d = computeDistance(selfX(), x, selfY(), y);
    engaging(x, y, d);
  }
}


/*****************************************************************************
 * Message Handler (through chat feature) 
 * ***************************************************************************/

//Prints a list of all the points currently marked dangerous on the map, the
//points that an avoidant drone would want to stay away from.
void printDangerPoints()
{
  int i;
  for(i = 0; i < NUM_AVOIDPTS; i++)
  {
    if(dangerPoints[i].x != -1)
    {
      printf("%4d %4d %4d\n",
              dangerPoints[i].x,
              dangerPoints[i].y,
              dangerPoints[i].r);
    }
  }
}


//Handle whatever messages pop up through the chat feature.
void handleMsg()
{
  if(strcmp(currMsg, scanMsg(0)))
  {
    char buf[100];
    char *tok = NULL;

    currMsg = scanMsg(0);
    
    strcpy(buf, currMsg);
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
    else if(!strcmp(tok, "printdps"))
    {
      printDangerPoints();
    }
    else if(!strcmp(tok, "avoid"))
    {
      int x = atoi(strtok(NULL, " "));
      int y = atoi(strtok(NULL, " "));
      int r = atoi(strtok(NULL, " "));
      int i = -1;
      while(++i < NUM_AVOIDPTS)
      {
        if(dangerPoints[i].x == -1)
        {
          dangerPoints[i].x = x;
          dangerPoints[i].y = y;
          dangerPoints[i].r = r;
          break;
        }  
      }
      printDangerPoints();
      modMap();
      interruptPath();
    }
    else if(!strcmp(tok, "remavoid"))
    {
      int x = atoi(strtok(NULL, " "));
      int y = atoi(strtok(NULL, " "));
      int i = -1;
      while(++i < NUM_AVOIDPTS)
      {
        if(dangerPoints[i].x == x && dangerPoints[i].y == y)
        {
          dangerPoints[i].x = -1;
          dangerPoints[i].y = -1;
          dangerPoints[i].r = -1;
          break;
        }
      }
      printDangerPoints();
      modMap();
      interruptPath();
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
  static int fourSecondsInFrames = 56;
  static int enemySpottedFC = 0;
  char numMsg[4], talkMsg[10];
  char buf[100], *tok = NULL, *tm = NULL;

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
      //073118 only possible breaking spot for tomorrow
      !enemySpottedFC ? enemySpottedFC = fourSecondsInFrames : enemySpottedFC--;
      break;

    case(STATE_NOENEMY):
      noEnemyFlying();
      break;

    case(STATE_DEAD):
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
