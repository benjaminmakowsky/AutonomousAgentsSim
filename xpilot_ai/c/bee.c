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


//INPROGRESS: Implement search - Benjamin Makowsky -----------------------
/*****************************************************************************
 * Searching (Controller)
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
  static bool fueling = false;
  static fuel_found = false;
  static int goal_frame = 0;
  static int original_distance;
  static int old_heading = 0;
  static int new_heading = 0;

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

    //Wait for ship to not move for 10 iterations
    if(counter < 50){
      if(current_x != selfX() && current_y != selfY()){
        counter = 0;
        current_x = selfX();
        current_y = selfY();
      } else{
        counter += 1;
      }
    } else {

      int* POICoordinates;
      POICoordinates = getPOICoordinates(x, y);
      goToCoordinates(POICoordinates[0],POICoordinates[1]);


      /*strcpy(bugstring, "goToCoordinate()");
      //INPROGRESS: Fly to point
      goToCoordinates(selfBaseX(),selfBaseY());*/
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
    //sprintf(temp_str, "Self %d Point %d", x, y);
    //sprintf(bugstring, "X %d Y %d", x, y);
    turnToDeg(new_heading);
  }else{
    //Travel to new point
    setPower(10);
  }


  //Stop at new point
  //Return current heading

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

  int xPOI = 9999999;
  int yPOI = 9999999;

  //Create array of POI's and get the number of elements in the array
  BaseStruct_t* bases = getBases("fuelpoints.csv");
  int length = bases[0].num_bases;


  //sprintf(bugstring, "X %d Y %d", x, y);
  //Traverse array to determine which location was closest to X, Y
  int i = 0;
  for(i; i < length; i++){
    int old_distance = computeDistance(x,xPOI,y,yPOI);
    int new_distance = computeDistance(x, bases[i].x, y, bases[i].y);
    sprintf(bugstring, "X %d Y %d", bases[i].x, bases[i].y);
    if(new_distance < old_distance) {
      xPOI = bases[i].x;
      yPOI = bases[i].y;
    }
  }

  static int coordinates[2];
  coordinates[0] = xPOI;
  coordinates[1] = yPOI;

  return coordinates;

}

