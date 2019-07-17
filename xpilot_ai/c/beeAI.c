/// Author: Benjamin Makowsky
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <X11/keysym.h>
#include "beeAI.h"
#include "beeBoids.h"
#include "cAI.h"
#include "beeBoids.h"



BaseStruct_t *getBases(char *csv) {
  // Number of bases are the first line
  FILE *fp;
  char buf[1024];
  if (!csv) {
    printf("No points file provided");
    return NULL;
  }

  if ((fp = fopen(csv, "r")) == NULL) {
    printf("Failed to open file: %s\n", csv);
    return NULL;
  }

  //get the first line
  fgets(buf, sizeof(buf), fp);
  int numBases = atoi(buf);

  //skip the number of fuel depots line
  fgets(buf, sizeof(buf), fp);

  //Make an array of the bases
  int i;
  bases = malloc(numBases * sizeof(BaseStruct_t));
  for (i = 0; i < numBases; ++i) {
    fgets(buf, sizeof(buf), fp);
    char *token;
    token = strtok(buf, " ");
    BaseStruct_t newBase;

    newBase.team = atoi(token);
    token = strtok(NULL, " ");
    newBase.x = atoi(token);
    token = strtok(NULL, " ");
    newBase.y = atoi(token);
    newBase.num_bases = numBases;

    //add to array
    bases[i] = newBase;
  }
  if(fclose(fp) == -1){
    printf("Failed to close");
    return NULL;
  }

  return bases;
}

FuelStruct_t *getFuelDepots(char *csv) {
  // Number of fuel depots is the second line
  FILE *fp;
  char buf[1024];
  if (!csv) {
    printf("No points file provided");
    return NULL;
  }

  if ((fp = fopen(csv, "r")) == NULL) {
    printf("Failed to open file: %s\n", csv);
    return NULL;
  }

  // get number of bases
  fgets(buf, sizeof(buf), fp);
  int numBases = atoi(buf);

  //get the number of fuel depots
  fgets(buf, sizeof(buf), fp);
  int numFuels = atoi(buf);

  //skip the base lines
  int i = 0;
  while (i < numBases) {
    fgets(buf, sizeof(buf), fp);
    ++i;
  }

  //Make an array of the fuels
  fuels = malloc(numFuels * sizeof(FuelStruct_t));
  for (i = 0; i < numFuels; ++i) {
    fgets(buf, sizeof(buf), fp);
    char *token;
    token = strtok(buf, " ");
    FuelStruct_t newFuel;

    newFuel.x = atoi(token);
    token = strtok(NULL, " ");
    newFuel.y = atoi(token);
    newFuel.num_fuels = numFuels;

    //add to array
    fuels[i] = newFuel;

  }

  if(fclose(fp) == -1){
    printf("Could not close for Fuel struct reading");
    return NULL;
  }
  return fuels;

}

/*****************************************************************************
 * Move Bee To Coordinates Specified
 * ***************************************************************************/

int goToCoordinates(int x, int y){

  //Get Heading to new point
  int new_heading = getHeadingForCoordinates(x ,y);

  //Turn to new heading
  if(((int)selfHeadingDeg() <= (new_heading - 2)) || ((int)selfHeadingDeg() >= (new_heading + 2))) {
    turnToDeg(new_heading);
  }
  return new_heading;
}



/*****************************************************************************
 * Get the heading for the POI
 * ***************************************************************************/
int getHeadingForCoordinates(int x, int y){
  return (getAngleBtwnPoints(selfX(), x, selfY(), y));
}


/*****************************************************************************
 * Get the coordinates for the nearest Point Of Interest to X,Y
 * ***************************************************************************/
int* getPOICoordinates(int x ,int y){

  int xPOI = 99999;
  int yPOI = 99999;

  FILE *fp;
  fp = fopen(LogFile, "a");
  fprintf(fp,"getPOICoordinates(%d, %d)\n",x,y);
  //Create array of POI's and get the number of elements in the array


  //TODO: Set bases to a global array at beginning of program
  int length = bases[0].num_bases;
  fprintf(fp, "numBases read: %d\n",length);
  //Traverse array to determine which location was closest to X, Y
  int i = 0;
  for(i; i < length; i++){
    int old_distance = computeDistance(x,xPOI,y,yPOI);
    int new_distance = computeDistance(x, bases[i].x, y, bases[i].y);
    fprintf(fp, "From Bases \tIndex %d \tX: %d\tY: %d\n", i, bases[i].x, bases[i].y);
    if(new_distance < old_distance) {
      xPOI = bases[i].x;
      yPOI = bases[i].y;
    }
  }

  fprintf(fp,"Closest base is at (%d,%d)\n", xPOI,yPOI);

  //TODO: Set depots to a global array at beginning of program
  length = fuels->num_fuels;

  fprintf(fp, "\nnum_fuels read: %d\n",length);
  //Traverse array to determine which location was closest to X, Y
  i = 0;
  for(i; i < length; i++){
    int old_distance = abs(computeDistance(x,xPOI,y,yPOI));
    int new_distance = abs(computeDistance(x, fuels[i].x, y, fuels[i].y));
    fprintf(fp, "From Fuels \tIndex %d \tX: %d\tY: %d\n", i, fuels[i].x, fuels[i].y);
    if(new_distance < old_distance) {
      xPOI = fuels[i].x;
      yPOI = fuels[i].y;
    }
  }

  static int coordinates[2];

  coordinates[0] = xPOI;
  coordinates[1] = yPOI;
  fprintf(fp,"Closest POI is at (%d,%d)\n", xPOI,yPOI);
  fclose(fp);

  return coordinates;

}


bool inVicinityOf(int x,int y){
  int range = 60; //Had to increase range because of the walls around honey sources
  int lowerXRange = x - range/2;
  int upperXRange = x + range/2;
  int lowerYRange = y - range/2;
  int upperYRange = y + range/2;


  if(selfX() >= lowerXRange && selfX() <= upperXRange){
    if(selfY() >= lowerYRange && selfY() <= upperYRange){
      return true;
    }
  }else {
    //If not in the vicinty of the point slow down as you approach
    int distance = computeDistance(selfX(),x,selfY(),y);
    if(distance < 10) {
      setPower(10);
    }else if(distance < 20){
      setPower(20);
    }else{
      setPower(30);
    }
    return false;
  }
}

//avoidwall: check for wall, if wall present adjust heading
int avoidWalls(){
  getWallAvoidanceVector();  //Update ship direction vector for wall avoidance
  int newHeading = 0;
  if (wallVector != -1) {
    newHeading = selfHeadingDeg() + 90;
    turnToDeg(newHeading);
  }
  return newHeading;
}

bool comeToStop(int number_of_frames) {

  static int current_x = 0;
  static int current_y = 0;
  refuel(0);
  setPower(0);
  static int counter = 0;
  if (counter < number_of_frames) {
    sprintf(bugstring, "%d", counter);

    //Current coordinate positions are not the same then reset counter
    if (current_x != selfX() && current_y != selfY()) {
      counter = 0;
      current_x = selfX();
      current_y = selfY();
    } else { counter += 1; }
    return false;

  }else{ return true;}
}


void checkForFuel(){
  static bool fueling = false;
  int frames_passed = 3; //Minimum amount of frames that can be recognized is 3
  if (fueling == false) {
    fuel = selfFuel();
    refuel(1);
    fueling = true;
  }
  if ((frameCount % frames_passed == 0) && (fueling == true)) {
    refuel(0);
    fueling = false;
  }
}
