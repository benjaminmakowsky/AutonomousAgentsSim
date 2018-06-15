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
  STATE_ENGAGED,
  STATE_NOENEMY,
  STATE_DEAD
};
 

// global variables
bool init = false;
int enemyX, enemyY;
int degToAim;
int state = STATE_INIT;

//argv params
int idx;
int tot_idx;

int distanceFormula( int x1, int x2, int y1, int y2 ){
  return sqrt( ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) );
}

void Initialize(){
  int r;
  char output[50];
  srand(time(NULL));
  r = rand() % 360;
  //TODO: figure out how to initialize to a random direction
  //It's possible pure randomness is impossible, since every chaser will
  //be using the same compiled C code. An implementation that takes the idx
  //into account, or some other variable, might be necessary.
  turnToDeg(r / selfID());
  //turn(30);
  //sprintf(output, "%.2f", selfHeadingDeg());
  //talk("Setting initial heading.");
  //talk(output);
  init = true;
  state = STATE_NOENEMY;
}

void wallAvoidance(){
  double currHeading;
  int lookAhead = 50;
  int currX, currY, delX, delY, newX, newY;
  int foundWall;
  
  currHeading = selfHeadingRad();
  currX = selfX();
  currY = selfY();
  delX = (int)(lookAhead * cos(currHeading));
  delY = (int)(lookAhead * sin(currHeading));
  newX = currX + delX;
  newY = currY + delY;

  //this foundWall code using wallBetween does not work
  //foundWall = wallBetween(currX, currY, newX, newY, 1, 1);
  foundWall = wallFeelerRad(100, 0.0, 1, 1);
  printf("%d\n", foundWall);
   if(foundWall){
    turn(50);
  }
  else { turnToDeg(270); }

  if(newX > 3350 || newX < 150){
    if(currHeading >= 0 && currHeading <= 180){
      turnToDeg(180 - radToDeg(currHeading));
    }
    else if(currHeading > 180 && currHeading < 360){
      turnToDeg(540 - radToDeg(currHeading));
    }
  }
  if(newY > 3350 || newY < 150){
    if(currHeading >= 0 && currHeading <= 180){
      turnToDeg(360 - radToDeg(currHeading));
    }
    else if(currHeading > 180 && currHeading < 360){
      turnToDeg(0 - radToDeg(currHeading));
    }
  }
}

void Patrolling(){
  //No need for wall avoidance on a wall-less map 
  //wallAvoidance(); 
  thrust(1);  
}

void Engaging( int closestEnemy){
  // Start moving towards enemy
  int enemyX = screenEnemyXId(closestEnemy);
  int enemyY = screenEnemyYId(closestEnemy);
  if(enemyX != -1 && enemyY != -1){

    // Calculate direction based on angle
    degToAim = atan2(enemyY - selfY(), enemyX - selfX());
    degToAim = radToDeg(degToAim);

    printf("TURNING TO: %d\n", degToAim);
    turnToDeg(degToAim);

    thrust(1);
  }
}

AI_loop(){
    thrust(0);

    state = STATE_NOENEMY;

    //Shoot at any enemy ships that come into sight
    int closestEnemy = closestEnemyShipId();
    if(closestEnemy != -1){ 
      printf("closestEnemy: %d\n", closestEnemy);
      printf("distance: %f\n", enemyDistanceId(closestEnemy));
    }
    
    if(closestEnemy != -1 && enemyDistanceId(closestEnemy) < 1000.0){
      state = STATE_ENGAGED;
    }

    // Check if we are dead
    if(!selfAlive()){
      state = STATE_DEAD;
    }

    // If we haven't initialized yet
    if(!init){
      state = STATE_INIT;
    }

    switch(state){
      case(STATE_INIT):
        Initialize();
        break;

      case(STATE_ENGAGED):
        Engaging(closestEnemy);
        break;

      case(STATE_NOENEMY):
	Patrolling();
        break;

      case(STATE_DEAD):
        state = STATE_NOENEMY;
        break;
    }
}

int main(int argc, char *argv[]) {
  idx = strtol(argv[2], NULL, 10);
  tot_idx = strtol(argv[3], NULL, 10);

  printf("idx: %d\n", idx);
  printf("tot_idx: %d\n", tot_idx);

  return start(argc, argv);
}
