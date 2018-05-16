//Justin Anderson - May 2012
//Compile: gcc TesterBot.c libcAI.so -o TesterBot
//Run: ./TesterBot
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

enum State{
  STATE_INIT,
  STATE_REFUELING,
  STATE_ENGAGING,
  STATE_IN_POSITION,
  STATE_HEADING_TO_POSITION,
  STATE_DEAD
};

struct pair{
  int first;
  int second;
};

// global variables
bool init = false;
bool inFuelingPosition = false;
bool engaging = false;
bool movingToGrid = false;
bool movingUp = false;
bool isLeader = false;
int enemyX, enemyY;
int degToAim;
double rallyx;
double rallyy;
struct pair prevRally;
int state = STATE_INIT;
//TODO: Make this dynamic
struct pair gridPointList[ 9 ];
pthread_mutex_t fuelLock;

// Some notes:
// 1.It appears that trying to execute thrust continually causes errors
// So do it only every 20 frames at most
// 2.selfX and selfY are incorrect before the drone fully spawns

int talk_frames = 0;
int angleToTurn;

//argv params
int idx;
int tot_idx;
double objx; 
double objy; 
int radius; 
int fuelx; 
int fuely; 
double pl_fuel;
int rubber;
int gridMovement = 0;
int numGrids = 0;

// chat lines
char* refuel_line = "I'm low, heading to refuel.";
char* rally_line = "Heading to rally-point.";
char* position_line = "I'm in position.";
char* engaging_line = "Engaging enemy.";

char* back_line = "Heading back.";
char* moveup_line = "Moving up.";

char custom_msg[50];
char prev_msg[50];

const char* printState(enum State s ){
  switch(s){
    case STATE_INIT : return "INIT";
    case STATE_REFUELING : return "REFUELING";
    case STATE_ENGAGING : return "ENGAGING";
    case STATE_IN_POSITION : return "IN_POSITION";
    case STATE_HEADING_TO_POSITION : return "HEADING_TO_POSITION";
    case STATE_DEAD : return "DEAD";
  }
}

// Simulates rate at which fuel is depleted in xpilot
void* lose_fuel(){
  while(1){
    if( !inFuelingPosition ){
      // We use less fuel when stationary
      // TODO: Implement an AI function for retrieving the current fuel from the server.
      pthread_mutex_lock(&fuelLock);
      if( ( state == STATE_IN_POSITION ) || ( state != STATE_ENGAGING && inFuelingPosition ) ){
        pl_fuel -= 2.5;
      }
      else{
        pl_fuel -= 3;
      }
      pthread_mutex_unlock(&fuelLock);
    }
    sleep(1);
  }
}

int distanceFormula( int x1, int x2, int y1, int y2 ){
  return sqrt( ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) );
}

void Initialize(){
  pthread_t tid;
  pthread_create( &tid, NULL, lose_fuel, NULL );

  if( !gridMovement ){
    //Calculate rallypoint
    printf( "idx:%d\n", idx );
    printf( "objx:%f\n", objx );
    printf( "objy:%f\n", objy );

    double ra = 360.0 / tot_idx * idx;
    rallyx = objx + radius * (double) cos( ra * PI / 180.0 ); 
    rallyy = objy + radius * (double) sin( ra * PI / 180.0 );
  }

  printf("rallypoint: %f,%f\n", rallyx, rallyy );
      
  talk( rally_line );
  init = true;
  state = STATE_HEADING_TO_POSITION;
}

void Refueling(){
  if( !inFuelingPosition ){
    talk( refuel_line );

    //Calculate fuel station degree
    angleToTurn = atan2( fuely - selfY(), fuelx - selfX()); 
    angleToTurn = radToDeg( angleToTurn ); // convert from rad to deg
        
    if( angleToTurn < 0 ){
      angleToTurn += 360;
    }

    turnToDeg( angleToTurn );
    
    //Thrust until we reach the fuel point +- 20 units
    if( ( selfX() + 20 >= fuelx && selfX() -20 <= fuelx  ) && 
        ( selfY() + 20 >= fuely && selfY() -20 <= fuely ) ){
      thrust(0);
      inFuelingPosition = true;
      refuel(1);
    }
    else{
      thrust(1);
    }
  }
  else{
    pl_fuel += 2;
    //TODO: Have refuel limit also be a parameter
    if( pl_fuel >= 1500 ){
      inFuelingPosition = false;
      refuel(0);
      state = STATE_HEADING_TO_POSITION; 
    }
  }
}

void HeadToPosition(){
  //Face rallypoint 
  angleToTurn = atan2( rallyy - selfY(), rallyx - selfX() );
  angleToTurn = radToDeg( angleToTurn );
  if( angleToTurn < 0 ){
    angleToTurn += 360;
  }
  printf( "selfX: %d\n", selfX() );
  printf( "selfY: %d\n", selfY() );

  printf( "ABOUT TO TURN TO %d\n", angleToTurn );
  turnToDeg( angleToTurn );

  printf( "selfHeading: %d\n", (int)selfHeadingDeg() );

  // Thrust until we reach rally-point +- 40 units
  // TODO: Set the +- units be a parameter?
  printf( "current position: %d,%d\n", selfX(), selfY() );
  printf("rallypoint: %f,%f\n", rallyx, rallyy );

  // BUG: Sometimes we move towards the wrong way
  
  int newDistance =  distanceFormula( selfX(), rallyx, selfY(), rallyy );
  printf( "Distance: %d\n", newDistance );
      
  if( newDistance < 40 ){
    thrust(0);
    talk( position_line );
    state = STATE_IN_POSITION;
    movingToGrid = false;
  }
  else{
    thrust(1);
  }
}

void Engaging( int closestEnemy){
  // Start moving towards enemy
  //degToAim = aimdir( closestEnemy );
  int enemyX = screenEnemyXId( closestEnemy );
  int enemyY = screenEnemyYId( closestEnemy );
  if( enemyX != -1 && enemyY != -1 ){
    if( !engaging ){
         talk( engaging_line );
          engaging = true;
    }

    if( gridMovement ){
      //Calculate closest neighbor grid to enemy
      int smallest = 9999999;
      int j = -1;
      int i;
      for( i = 0; i < sizeof( gridPointList ); ++i ){
        struct pair point = gridPointList[ i ];
        // logical xor
        int d = distanceFormula( point.first, enemyX, point.second, enemyY );
        if( d < smallest ){
          smallest = d;
          j = i;
        }
      }

      if( j != -1 ){
        rallyx = gridPointList[ j ].first;
        rallyy = gridPointList[ j ].second;
        printf( "j:%d\n", j );
        printf( "Best grid: %f,%f\n", rallyx, rallyy );
        movingToGrid = true;
      
        angleToTurn = atan2( rallyy - selfY(), rallyx - selfX() );
        angleToTurn = radToDeg( angleToTurn );
        if( angleToTurn < 0 ){
          angleToTurn += 360;
        }
            turnToDeg( angleToTurn );
            printf( "selfHeading: %f\n", selfHeadingDeg() );
      }
      return;
    }

    // Calculate direction based on angle
    degToAim = atan2( enemyY - selfY(), enemyX - selfX() );
    if( angleToTurn < 0 ){
      angleToTurn += 360;
    }

    // TODO: Paramaterize this random range
    // This line doesn't work as intended in practice, but it is hilarious
    //degToAim = radToDeg( degToAim + ( rand() % 6 )  );
    degToAim =  radToDeg( degToAim );

    printf( "TURNING TO: %d\n", degToAim );
    turnToDeg( degToAim );

    // TODO: Parameterize number of shots. Shooting on every frame isn't efficient. 
    // Notice that fireShot() has been commented out, since we don't need it for our
    // collision avoidance purposes.
    fireShot();
    //thrust( 1 ); //uncomment this to enable chasing

    // TODO: Paramaterize how far away we can move away from our rallypoint
    if( distanceFormula( selfX(), objx, selfY(), objy ) > rubber  ){
      thrust( 0 );
      return;
    }

    // TODO: Paramaterize how far away we can move away from our objective
    if( distanceFormula( selfX(), objx, selfY(), objy ) > 5000 || rubber != 999999 ){
      thrust( 0 ); //should be 0, set to 1 to enable chasing
      return;
    }

    // TODO: Paramaterize how close you want to get to the enemy
    if( enemyDistanceId( closestEnemy ) > 30 && isLeader ){
      if( !movingUp ){
        sprintf( custom_msg, "%d", (int)rallyy );
        talk( custom_msg );  
        movingUp = true;
        talk_frames = 2;
      }
      thrust(1);
    }
    else if( enemyDistanceId( closestEnemy ) > 30 ){
      thrust(1);
    }

  }
  else{
    if( engaging ){
      talk( back_line );
    }
    engaging = false;
    state = STATE_HEADING_TO_POSITION;
  }
}

AI_loop( ) {
    thrust( 0 );
    if( talk_frames ){
      if( talk_frames == 2 ){
        sprintf( custom_msg, "%d", (int)rallyx );
        talk( custom_msg );
      }
      else{
        talk( moveup_line );
      }
      talk_frames--;

    }
    if( state != STATE_REFUELING ){
      refuel( 0 );
    }
    else{
      Refueling();
      return;
    }

    if( !isLeader ){
      if( strcmp( scanMsg( 0 ), prev_msg ) != 0 ){
         strcpy( prev_msg, scanMsg( 0 ) );
         printf( "New message: %s\n", prev_msg );
      }
    }

    if( !isLeader &&  !strcmp( scanMsg( 0 ),  "Moving up. [Fixed1]" ) ){
      printf( "x:%s\n", scanMsg( 1 ) );
      printf( "y:%s\n", scanMsg( 2 ) );
      rallyx = strtol( scanMsg( 1 ), NULL, 10 );
      rallyy = strtol( scanMsg( 2 ), NULL, 10 );
      state = STATE_HEADING_TO_POSITION;
    }
    else if( !isLeader && !strcmp( scanMsg( 0 ), "Heading back. [Fixed1]" ) ){
      rallyx = prevRally.first;
      rallyy = prevRally.second;
      state = STATE_HEADING_TO_POSITION;
    }

    // Case where we have disingaged 
    if( ( state != STATE_IN_POSITION && state != STATE_ENGAGING ) || movingToGrid ){
      state = STATE_HEADING_TO_POSITION;
    }

    //Shoot at any enemy ships that come into sight
    int closestEnemy = closestEnemyShipId();
    if( closestEnemy != -1 ){ 
      printf( "closestEnemy: %d\n", closestEnemy );
      printf( "distance: %f\n", enemyDistanceId( closestEnemy ) );
    }
    //TODO: Paramaterize the distance limit here 
    if( closestEnemy != -1 && enemyDistanceId( closestEnemy ) < 3000.0 ){
      state = STATE_ENGAGING;
    }

    //If low on fuel, refuel
    //TODO: Have refuel % threshold be a parameter
    if( pl_fuel < 50 ){
      state = STATE_REFUELING;
    }

    // Check if we are dead
    if( !selfAlive() ){
      state = STATE_DEAD;
    }

    // If we haven't initialized yet
    if( !init ){
      state = STATE_INIT;
    }

    printf( "state: %s\n", printState( state ) );
    switch( state ){
      case( STATE_INIT ):
        Initialize();
        break;

      case( STATE_REFUELING ):
        Refueling();
        break;

      case( STATE_ENGAGING ):
        Engaging(closestEnemy);
        break;

      case( STATE_IN_POSITION ):
        break;

      case( STATE_DEAD ):
        state = STATE_HEADING_TO_POSITION;
        break;

      case( STATE_HEADING_TO_POSITION ):
        HeadToPosition();
        break;
    }

    printf( "current fuel: %f\n", pl_fuel );
}

int main(int argc, char *argv[]) {
  idx = strtol( argv[2], NULL, 10 );
  tot_idx = strtol( argv[3], NULL, 10 );
  objx = (double) strtol( argv[4], NULL, 10 );
  objy = (double) strtol( argv[5], NULL, 10 );
  radius = strtol( argv[6], NULL, 10 );
  fuelx = strtol( argv[7], NULL, 10 );
  fuely = strtol( argv[8], NULL, 10 );
  pl_fuel = (double) strtol( argv[9], NULL, 10 );
  rubber = strtol( argv[10], NULL, 10 );  

  gridMovement = strtol( argv[11], NULL, 10 );
  if( gridMovement == 2 ){
    gridMovement = 0;
    if( idx == 1 ){
      printf( "IS LEADER\n" );
      isLeader= true;
    }
  }
  printf( "Gridmovement: %d\n", gridMovement );
  if( gridMovement ){
    numGrids = strtol( argv[12], NULL, 10 );
    int i;
    for( i = 0; i < numGrids * 2; i += 2 ){
      struct pair newPoint;
      newPoint.first = strtol( argv[ 13 + i ], NULL, 10 );
      newPoint.second = strtol( argv[ 13 + i + 1 ], NULL, 10 );
      gridPointList[ i / 2 ] = newPoint;
      printf( "added: %d, %d\n", gridPointList[ i /2 ].first, gridPointList[ i/2].second );
    }
  }

  printf( "idx: %d\n", idx );
  printf( "tot_idx: %d\n", tot_idx );
  printf( "objx: %f\n", objx );
  printf( "objy: %f\n", objy );
  printf( "radius: %d\n", radius );
  printf( "fuelx: %d\n", fuelx );
  printf( "fuely: %d\n", fuely );
  printf( "pl_fuel: %f\n", pl_fuel );


  if( gridMovement ){
    rallyx = 1758;
    rallyy = 1740;
  }
  else{
    double ra = 360.0 / tot_idx * idx;
    rallyx = objx + radius * (double) cos( ra * PI / 180.0 ); 
    rallyy = objy + radius * (double) sin( ra * PI / 180.0 );
    printf("angle: %d\n", ra );
  }

  printf("rallypoint: %f,%f\n", rallyx, rallyy );
  prevRally.first = rallyx;
  prevRally.second = rallyy;

  return start(argc, argv );
}
