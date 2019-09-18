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
#include "cAI.h"
#include "beeMain.h"
#include "beeObject.h"
#include <limits.h>
#include "beeGlobals.h"
#include "beeObserve.h"




BaseStruct_t* hives;
FuelStruct_t* honey_spots;

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
  hives = malloc(numBases * sizeof(BaseStruct_t));
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
    hives[i] = newBase;
  }
  if(fclose(fp) == -1){
    printf("Failed to close");
    return NULL;
  }

  return hives;
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
  honey_spots = malloc(numFuels * sizeof(FuelStruct_t));
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
    honey_spots[i] = newFuel;

  }

  if(fclose(fp) == -1){
    printf("Could not close for Fuel struct reading");
    return NULL;
  }
  return honey_spots;

}

/*****************************************************************************
 * Move Bee To Coordinates Specified
 * ***************************************************************************/

int turnToCoordinates(int x, int y){

  //Get Heading to new point
  int new_heading = getHeadingBetween(selfX(), selfY(), x, y);

  //Turn to new heading
  if( (int)selfHeadingDeg() != new_heading) {
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

  //Default values for location to be far
  int xPOI = 66101110;  //Using MAX_INT causes it to not work
  int yPOI = 66101110;  //Using MAX_INT causes it to not work

  OPENLOG()
  fprintf(fp,"getPOICoordinates(%d, %d)\n",x,y);


  int length = hives->num_bases;
  fprintf(fp, "numBases read: %d\n",length);
  //Traverse array to determine which location was closest to X, Y
  int i = 0;
  for(i; i < length; i++){
    //Distance from (x,y) to (hive_x,hive_y) if smaller than old distance than thats the closest point
    int old_distance = abs(computeDistance(x,xPOI,y,yPOI));
    int new_distance = abs(computeDistance(x, hives[i].x, y, hives[i].y));
    fprintf(fp, "From Bases \tIndex %d \tX: %d\tY: %d\n", i, hives[i].x, hives[i].y);
    if(new_distance < old_distance) {
      xPOI = hives[i].x;
      yPOI = hives[i].y;
    }
  }
  fprintf(fp,"Closest base is at (%d,%d)\n", xPOI,yPOI);

  //Determines how many fuel stations to traverse
  length = honey_spots[0].num_fuels;
  fprintf(fp, "\nnum_fuels read: %d\n",length);
  //Traverse array to determine which location was closest to X, Y
  i = 0;
  for(i; i < length; i++){
    int old_distance = abs(computeDistance(x,xPOI,y,yPOI));
    int new_distance = abs(computeDistance(x, honey_spots[i].x, y, honey_spots[i].y));
    fprintf(fp, "From Fuels \tIndex %d \tX: %d\tY: %d\n", i, honey_spots[i].x, honey_spots[i].y);
    if(new_distance < old_distance) {
      xPOI = honey_spots[i].x;
      yPOI = honey_spots[i].y;
    }
  }

  //In order to return an array it must be static
  static int coordinates[2];


  coordinates[0] = xPOI;
  coordinates[1] = yPOI;
  fprintf(fp,"------------------------------\n");
  fclose(fp);

  return coordinates;
}


bool inVicinityOf(int x,int y){
  int range = 30; //Had to increase range because of the walls around honey sources
  int distance = computeDistance(selfX(),x,selfY(),y);
  return distance < range;

}

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


void useFueler(){
  static bool fueling = false;
  if (fueling == false) {
    fuel = selfFuel();
    refuel(1);
    fueling = true;
  }
  if ((frameCount % MIN_FRAMES_PASSED == 0) && (fueling == true)) {
    refuel(0);
    fueling = false;
  }
}

/********************************************************
 * Used to check if fuel levels changed                 *
 * @param flag check whether increasing or decreasing   *
 * @param original_level original fuel level            *
 * @return bool whether the change happened             *
 ********************************************************/
bool checkforFuel(char* flag, double original_level){

  double current_fuel_level = selfFuel();
  if(strcmp(flag, "increasing") == 0){
    if (current_fuel_level - original_level > 0) {
      return true;
    } else {
      return false;
    }

  }else if(strcmp(flag, "decreasing") == 0){
    if (current_fuel_level - original_level < 0) {
      return true;
    } else {
      return false;
    }
  } else {return false;}
}

bool beeDegIsBetween(int deg1, int deg2){

  /*******************************************************
   * Case 1: deg1 < deg2                                 *
   * Do not have to account for passing 360 ie (45 to 90)*
   *******************************************************/
   if(deg1 < deg2){
     return (deg1 <= (int)selfHeadingDeg() && (int)selfHeadingDeg() <= deg2);
   }
  /*******************************************************
  * Case 2: deg1 > deg2                                 *
  * Do have to account for passing 360 ie (355 to 5)    *
  *******************************************************/
   else if(deg1 > deg2){
     return((int)selfHeadingDeg() > deg1 || (int)selfHeadingDeg() < deg2);
   }
   else{
     return ((int)selfHeadingDeg() == deg1);
   }
}


int interpretDance(int dance){
  if(dance == FOUND_HONEY){
    int danceChance = rand();
    if(danceChance % 1 == 0){
      state = STATE_FORAGING;
      setCurrState(STATE_FORAGING);
    }
  }else if(dance == FOUND_ENEMY_HIVE){
    //TODO: Complete this section
  }
}

void stopAtCoordinates(int x, int y){
  //If not in the vicinty of the point slow down as you approach
  int distance = computeDistance(selfX(),x,selfY(),y);
  int max_speed = 60;
  if(distance < 20) {
    setPower(max_speed/4);
  }else if(distance < 60){
    setPower(max_speed/2);
  }else{
    setPower(max_speed);
  }
}