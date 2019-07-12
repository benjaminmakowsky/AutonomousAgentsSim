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
    # Whats the point of waiting for 30 frames? Add a comment explaining why if it is needed.
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

      # We ideally want to read the file during the initialization and only once, because:
      # 1. It takes a few second for an agent to fully spawn, can use this time to read the file.
      # 2. At some point the assigned honey depot will deplet, and the forager will need to go to 
      #    another honey location. With the current approach, that bee would have to re-read the 
      #    points file every time he would switch POI.
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


  # Bit of a nitpick, I would place this if-block right at the end of this
  # function. 
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

# This kind of utility function should go in cAI.cpp & cAI.h, since it would be useful
# to other bots
int goToCoordinates(int x, int y){

  //Get Heading to new point
  int new_heading = getHeadingForCoordinates(x ,y);


  //Turn to new heading
  if(((int)selfHeadingDeg() <= (new_heading - 2)) || ((int)selfHeadingDeg() >= (new_heading + 2))) {
    turnToDeg(new_heading);
  }else{
    //setPower(30);
  }
}


/*****************************************************************************
 * Get the heading for the POI
 * ***************************************************************************/
# This kind of utility function should go in cAI.cpp & cAI.h, since it would be useful
# to other bots
int getHeadingForCoordinates(int x, int y){

  return (getAngleBtwnPoints(selfX(), x, selfY(), y));

}


/*****************************************************************************
 * Get the coordinates for the nearest Point Of Interest to X,Y
 * ***************************************************************************/
# This kind of utility function should go in cAI.cpp & cAI.h, since it would be useful
# to other bots

int* getPOICoordinates(int x ,int y){

  int xPOI = 99999;
  int yPOI = 99999;

  FILE *fp;
  fp = fopen("Log.txt", "w");

  fprintf(fp,"getPOICoordinates(%d, %d)\n",x,y);
  //Create array of POI's and get the number of elements in the array


  BaseStruct_t* bases = getBases("fuelpoints.csv");
  # You *should* be able to get the number of base like this:
  # int length = sizeof(bases)/sizeof(bases[0])
  # If that doesn't work, I'd prefer to just have a single global extern int num_bases
  # in cAI.c / cAI.h to save in space
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
  # See previous comment, should work for this as well.
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

  # I would malloc this array just to be safe. 
  # Also, consider doing a memcpy to copy the values of x/yPOI to coordinates, to be safe
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
