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
  static bool fuel_found = false;
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
        rememberPOICoords(xPOI,yPOI);
        FILE *fp;
        fp = fopen("Log.txt", "a");
        fclose(fp);
        fileRead = !fileRead;
      }


      FILE *fp;
      fp = fopen("Log.txt", "a");
      fprintf(fp,"fuelX: %d fuelY: %d\n", selfFuelX(),selfFuelY());
      fprintf(fp,"Ending Search behavior\n");
      fclose(fp);
      sprintf(bugstring, "Search Moving to %d and %d",selfFuelX(), selfFuelY());
      state = STATE_FORAGING;
    }
  }
}

//INPROGRESS: Get it to forage
/*****************************************************************************
 * Foraging (Controller)- Benjamin Makowsky
 * ***************************************************************************/
void forage() {

  fuel = selfFuel();
  int x = 0;
  int y = 0;
  static bool initForage = true;
  static bool depositing = true;
  static bool forage_state_changed = true;


  if (initForage) {
    FILE *fp;
    fp = fopen("Log.txt", "a");
    fprintf(fp, "\n\n\nBeginning Forage Behavior\n");
    fclose(fp);
    initForage = !initForage;
  }



  //Determine whetheror not you are heading to hive to deposit honey or
  //if you are headed to flower to pickup honey
  if(depositing){
    x = selfBaseX();
    y = selfBaseY();
  } else {
    x = selfFuelX();
    y = selfFuelY();
  }


  if(forage_state_changed){
    FILE *fp;
    fp = fopen("Log.txt", "a");
    if(depositing) {
      fprintf(fp, "Depositing honey at hive \n");
    }else{
      fprintf(fp, "Getting Honey from source (%d,%d)\n", x,y);
    }
    fclose(fp);
    forage_state_changed = false;
  }


  static int fuelLVL = 0;
  
  if(!inVicinityOf(x,y)) {
    refuel(0);
    sprintf(bugstring, "Forage: Moving to location (%d, %d) ",x,y);
    goToCoordinates(x,y);

  }else {
    setPower(0);
    fuelLVL = (int)selfFuel();
    if (fuelLVL > 500 && depositing ) {
      int frames_passed = 3; //Minimum amount of frames that can be recognized is 3
      if (fueling == false) {
        refuel(1);
        fueling = true;
      }
      if ((frameCount % frames_passed == 0) && (fueling == true)) {
        refuel(0);
        fueling = false;
      }
      strcpy(bugstring, "Depositing");
    } else if( fuelLVL < 700 && !depositing){
      int frames_passed = 3; //Minimum amount of frames that can be recognized is 3
      if (fueling == false) {
        refuel(1);
        fueling = true;
      }
      if ((frameCount % frames_passed == 0) && (fueling == true)) {
        refuel(0);
        fueling = false;
      }
      strcpy(bugstring, "Gathering");
    }else{
      refuel(0);
      strcpy(bugstring, "Moving");
      depositing = !depositing;
      forage_state_changed = true;
    }
  }
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

  fprintf(fp,"Closest base is at (%d,%d)\n", xPOI,yPOI);
  FuelStruct_t* depots = getFuelDepots("fuelpoints.csv");
  length = depots[0].num_fuels;

  fprintf(fp, "\nnum_fuels read: %d\n",length);
  //Traverse array to determine which location was closest to X, Y
  i = 0;
  for(i; i < length; i++){
    int old_distance = abs(computeDistance(x,xPOI,y,yPOI));
    int new_distance = abs(computeDistance(x, depots[i].x, y, depots[i].y));
    fprintf(fp, "From Fuels \tIndex %d \tX: %d\tY: %d\n", i, depots[i].x, depots[i].y);
    if(new_distance < old_distance) {
      xPOI = depots[i].x;
      yPOI = depots[i].y;
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
  int range = 30;
  int lowerXRange = x - range/2;
  int upperXRange = x + range/2;
  int lowerYRange = y - range/2;
  int upperYRange = y + range/2;


  if(selfX() >= lowerXRange && selfX() <= upperXRange){
    if(selfY() >= lowerYRange && selfY() <= upperYRange){
      return true;
    }
  }else {
    return false;
  }
}