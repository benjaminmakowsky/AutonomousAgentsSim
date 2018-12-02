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
#define WAIT_FRAMES 100
#define WRONG_WAY_FRAMES 5
#define TURN_FRAMES 15
#define REPOSE_FRAMES 40

struct pair{
  int first;
  int second;
};

//TODO: Make this dynamic
struct pair gridPointList[ 9 ];

bool gotUnstuck = false;
bool init = false;
bool refueling = false;
bool inRallyPosition = false;
bool inFuelingPosition = false;
bool engaging = false;
bool goingWrongWay = false;
bool movingToGrid = false;
bool movingUp = false;
bool goingBack = false;
bool depth_def = true;
bool isLeader = false;
int enemyX, enemyY;
int rallyDeg;
int degToAim;
double rallyx;
double rallyy;
clock_t timer;
double time_since_switch;
struct pair prevRally;

// It appears that trying to execute thrust continually causes errors
// So do it only every 20 frames at most
int frames = 0;
int turnFrames = 0;
int talk_frames = 0;
int angleToTurn;

// Wait a bit before executing actions, stuff like 
// selfX and selfY are incorrect before the drone fully spawns
int waitFrames = WAIT_FRAMES;
int wrongWayFrames = 0;
int reposeFrames = 0;
double prevDistance = 0;

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

//TODO: Pass these values specific to map and ship, or figure out how to pass in a reference to
//      server's player_t struct for the bot.
//These values are hard-coded for now

/*int pl_fuel_sum = 200;
int pl_fuelpos_x = 140;
int pl_fuelpos_y = 160;
int pl_rallypos_x = 120;
int pl_rallypos_y = 140;
int pl_shots = 3; */

// Simulates rate at which fuel is depleted in xpilot
void* lose_fuel(){
  while(1){
    if( !inFuelingPosition ){
      // We use less fuel when stationary
      if( ( !engaging && inRallyPosition ) || (!engaging && inFuelingPosition ) ){
        pl_fuel -= 2.5;
      }
      else{
        pl_fuel -= 3;
      }
    }
    sleep(1);
  }
}

int distanceFormula( int x1, int x2, int y1, int y2 ){
  return sqrt( ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) );
}

void Initialize(){
    if( !init ){
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
      
      init = true;
      talk( rally_line );
    }

}

AI_loop( ) {
    frames++;
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
    if( !refueling ){
      refuel( 0 );
    }

    if( !init ){
      Initialize();
    }

    if( !selfAlive() ){
      exit(1);
      engaging = false;
      refueling = false;
      inRallyPosition = false;
      inFuelingPosition = false;
      return;
    }

    //printf( "current fuel: %f\n", pl_fuel );

    if( refueling ){
      if( !inFuelingPosition ){
        talk( refuel_line );

        //Calculate fuel station degree
        angleToTurn = atan2( selfY() - fuely, selfX() - fuelx ); 
        angleToTurn = radToDeg( angleToTurn ); // convert from rad to deg
        
        if( angleToTurn < 0 ){
          angleToTurn += 360;
        }

        turnToDeg( angleToTurn );
        /*if( !turnFrames ){
          turnFrames = 3;
          return;
        }
        else{ 
          turnFrames--;
        }
        */
        //Thrust until we reach the fuel point +- 20 units
        if( ( selfX() + 20 >= fuelx && selfX() -20 <= fuelx  ) && 
            ( selfY() + 20 >= fuely && selfY() -20 <= fuely ) ){
          thrust(0);
          inFuelingPosition = true;
          return;
        }
        else{
          thrust(1);
          return;
        }
      }

      refuel(1);
      pl_fuel += 2;
      //TODO: Have refuel limit also be a parameter
      if( pl_fuel >= 1500 ){
        refueling = false;
        inFuelingPosition = false;
        refuel(0);
      }
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
      inRallyPosition = false;
    }
    else if( !isLeader && !strcmp( scanMsg( 0 ), "Heading back. [Fixed1]" ) ){
      rallyx = prevRally.first;
      rallyy = prevRally.second;
      inRallyPosition = false;
    }

    if( ( !inRallyPosition && !engaging ) || movingToGrid || goingBack ){
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

      // BUG: Check if we are moving the wrong way all together
      int newDistance =  distanceFormula( selfX(), rallyx, selfY(), rallyy );
      printf( "Distance: %d\n", newDistance );
      
      if( newDistance < 40 ){
          thrust(0);
          talk( position_line );
          inRallyPosition = true;
          movingToGrid = false;
      }
      else{
         thrust(1);
      }
      //return;
    }

    //Now stay here and shoot at any enemy ships that come into sight
    int closestEnemy = closestEnemyShipId();
    if( closestEnemy != -1 ){ 
      printf( "closestEnemy: %d\n", closestEnemy );
      printf( "distance: %f\n", enemyDistanceId( closestEnemy ) );
    }
    //TODO: Paramaterize the distance limit here 
    if( closestEnemy != -1 && enemyDistanceId( closestEnemy ) < 3000.0 ){
      // Start moving towards enemy
      //degToAim = aimdir( closestEnemy );
      int enemyX = screenEnemyXId( closestEnemy );
      int enemyY = screenEnemyYId( closestEnemy );
      //if( degToAim != -1 || ( enemyX != -1 && enemyY != -1 ) ){
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
            //if( ( point.first == rallyx ) != ( point.second == rallyy ) ){
              int d = distanceFormula( point.first, enemyX, point.second, enemyY );
              if( d < smallest ){
                smallest = d;
                j = i;
              }
            //}
          }

          if( j != -1 ){
            rallyx = gridPointList[ j ].first;
            rallyy = gridPointList[ j ].second;
            printf( "j:%d\n", j );
            printf( "Best grid: %f,%f\n", rallyx, rallyy );
            inRallyPosition = false;
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
        // This line doens't work as intended in practice, but it is hilarious
        //degToAim = radToDeg( degToAim + ( rand() % 6 )  );
        degToAim =  radToDeg( degToAim );

        printf( "TURNING TO: %d\n", degToAim );
        inRallyPosition = false;
        turnToDeg( degToAim );
        fireShot();

        // TODO: Paramaterize how far away we can move away from our rallypoint
        if( distanceFormula( selfX(), objx, selfY(), objy ) > rubber  ){
          thrust( 0 );
          return;
        }


        // TODO: Paramaterize how far away we can move away from our objective
        if( distanceFormula( selfX(), objx, selfY(), objy ) > 5000 || rubber != 999999 ){
          thrust( 0 );
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

        // TODO: Parameterize number of shots to take
        //int i;
        //for( i = 0; i < 2; ++i ){
        //}
      }
      else{
        if( engaging ){
          talk( back_line );
        }
        engaging = false;
      }
    }
    else{
      if( engaging ){
        talk( back_line );
      }
      engaging = false;
    }

    //If low on fuel, refuel
    //TODO: Have refuel % threshold be a parameter
    if( pl_fuel < 50 ){
      refueling = true;
      inRallyPosition = false;
    }


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
    depth_def = true;
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

  //while(1);

  return start(argc, argv );
}
