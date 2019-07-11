//
// Created by makowskyb on 7/2/19.
//

#include "bee.h"
#include "cAI.h"
#include "boids.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>


//global variables
char temp_str[25];
bool fueling = false;


/*****************************************************************************
 * Searching (Controller)- Benjamin Makowsky
 * ***************************************************************************/
void searching() {
  /*
   * Process:
   *  1. Check for walls
   *  2. Attempt to attain honey
   *  3. Check new fuel levels vs past fuel levels
   *    3a. If new fuel is greater found a supply, if less found a hive
   *  4. Keep record of supply/hive location
   */

  //Used to store coordinates of location first detected by fuel pickup
  static int x = 0;
  static int y = 0;
  static int current_x = 0;
  static int current_y = 0;
  static fuel_found = false;
  static int goal_frame = 0;
  static int original_distance;
  static int old_heading = 0;
  static int new_heading = 0;
  static int xPOI = 0;
  static int yPOI = 0;

  /*
   * Step 1: Check for walls
   */
  wallAvoidance();  //Update vector to for wall avoidance
  if (wallVector != -1) {
    turnToDeg(selfHeadingDeg() + 100);
  }

  /*
   * Step 2: Attempt to Attain Honey
   */
  if (!fuel_found) {
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

    /*
     * Step 3: Check if fuel levels changed
     */
    double new_fuel_level = selfFuel();
    if (new_fuel_level - fuel > 0) {
      refuel(0);
      setPower(0);
      x = selfX();
      y = selfY();
      old_heading = (int)selfHeadingDeg();
      new_heading = ((int)selfHeadingDeg() + 180) % 360;
      fuel_found = true;
    }
  }


  if (fuel_found) {
    static int counter = 0;
    //Wait for ship to not move for 30 iterations
    if(counter < 30){
      sprintf(bugstring, "%d", counter);
      if(current_x != selfX() && current_y != selfY()){
        counter = 0;
        current_x = selfX();
        current_y = selfY();
      } else{
        counter += 1;
      }
    } else {

      int *POICoordinates;
      static bool fileRead = false;
      if(!fileRead) {
        POICoordinates = getPOICoordinates(x, y);
        xPOI = POICoordinates[0];
        yPOI = POICoordinates[1];
        fileRead = !fileRead;

        FILE *fp;
        fp = fopen("Log.txt", "a");
        fprintf(fp,"\nxPOI: %d yPOI: %d", xPOI,yPOI);
        fclose(fp);
      }

      /*
       * Working version goes to fuel coordinates
       */
      //strcpy(bugstring, "moving to coords");
      //goToCoordinates(xPOI,yPOI);

      //Used to debug current position vs heading
      cRadius = selfX();
      cWeight = selfY();
      //sprintf(bugstring, "Moving to %d and %d",x,y);


      rememberPOICoords(xPOI,yPOI);
      sprintf(bugstring, "Search Moving to %d and %d",selfFuelX(), selfFuelY());
      goToCoordinates(POICoordinates[0],POICoordinates[1]);
    }
  }
}

//INPROGRESS: Get it to forage
/*****************************************************************************
 * Foraging (Controller)- Benjamin Makowsky
 * ***************************************************************************/
void forage() {

  //Go to base
  static int x = 0;
  static int y = 0;

  /*if(x == 0){
    x = selfBaseX();
    y = selfBaseY();
  }*/

  //Coordinates for fuel depo stored in ship struct from searching function
  x = selfFuelX();
  y = selfFuelY();
  sprintf(bugstring, "Forage: Moving to %d and %d",x,y);

  goToCoordinates(x,y);

  //Drop off fuel
  if(inVicinityOf(x,y)) {
    setPower(0);

    if (selfFuel() > 0) {
        refuel(1);
        fueling = true;
    } else{
      refuel(0);

      //set coordinates of position to go to next
      x = selfFuelX();
      y = selfFuelY();
      setPower(10);
    }
  }

  //Get return to source


  //Get more fuel and repeat
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
  }else{
    if(state = STATE_SEARCHING) {
      state = STATE_FORAGING;
    }
    setPower(10);
  }
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
  fp = fopen("Log.txt", "w");

  fprintf(fp,"getPOICoordinates(%d, %d)\n",x,y);
  //Create array of POI's and get the number of elements in the array


  BaseStruct_t* bases = getBases("fuelpoints.csv");
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

  fprintf(fp,"Closest bases is at (%d,%d)\n", xPOI,yPOI);
  FuelStruct_t* depots = getFuelDepots("fuelpoints.csv");
  length = depots[0].num_fuels;

  fprintf(fp, "\nnum_fuels read: %d\n",length);
  //Traverse array to determine which location was closest to X, Y
  i = 0;
  for(i; i < length; i++){
    int old_distance = computeDistance(x,xPOI,y,yPOI);
    int new_distance = computeDistance(x, depots[i].x, y, depots[i].y);
    fprintf(fp, "From Fuels \tIndex %d \tX: %d\tY: %d\n", i, depots[i].x, depots[i].y);
    if(new_distance < old_distance) {
      xPOI = depots[i].x;
      yPOI = depots[i].y;
    }
  }

  static int coordinates[2];

  sprintf(bugstring, "X read: %d, y Read: %d", xPOI, yPOI);
  coordinates[0] = xPOI;
  coordinates[1] = yPOI;


  fprintf(fp, "X Read: %d Y Read: %d", xPOI, yPOI);
  fclose(fp);

  return coordinates;

}


bool inVicinityOf(int x,int y){
  int range = 30;
  int lowerRange = range/12;
  int upperRange = range + lowerRange;

  if(selfX() >= lowerRange && selfX() <= upperRange){
    if(selfY() >= lowerRange && selfY() <= upperRange){
      return true;
    }
  }
  return false;
}