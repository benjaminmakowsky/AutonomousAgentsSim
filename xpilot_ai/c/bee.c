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

      //Once ship has not moved for 10 iterations it can determined to be stopped
    } else {

      strcpy(bugstring, "goToCoordinate()");
      //INPROGRESS: Fly to point
      goToCoordinates(selfBaseX(),selfBaseY());

      /*//Turn To heading
      if ((int)selfHeadingDeg() != new_heading) {
        //turnToDeg(new_heading); //TODO: Added check for deg>360 in cAI.c
      } else{

      }*/


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
  if(((int)selfHeadingDeg() <= (new_heading - 2))|| ((int)selfHeadingDeg() >= (new_heading + 2))) {
    //sprintf(temp_str, "Self %d Point %d", x, y);
    sprintf(temp_str, "S %d P %d", (int)selfHeadingDeg(), new_heading);
    strcpy(bugstring, temp_str);
    turnToDeg(new_heading);
  }else{
    //Travel to new point
    setPower(20);
  }


  //Stop at new point
  //Return current heading

}


/*****************************************************************************
 * Get Heading of Specified Coordinates
 * ***************************************************************************/
int getHeadingForCoordinates(int x, int y){

  return (getAngleBtwnPoints(selfX(), x, selfY(), y)) % 360;

}

/*****************************************************************************
 * Pinpoint the center of a hive
 * ***************************************************************************/

int* pinpoint(int x, int y){

  /*
   *  Step 1: Resume moving until fuel stops increasing
   *  Step 2: Rotate 180 and move until fuel stops increasing
   *  Step 3: Find distance traveled and go to midpoint
   *  Step 4: Rotate 90 find edge rotate 180 find other edge find midpoint
   */

  setPower(5);
  //Step 1:
  /*
  fuel = selfFuel();
  setPower(1);
  do{
    int off_set = 10;
    if((fueling == false)){
      strcpy(bugstring, "Start Pin");
      refuel(1);
      fueling = true;
    }
    if((frameCount % 3 == 0) && (fueling == true)){
      strcpy(bugstring, "Stop Pin");
      refuel(0);
      fueling = false;
      double curr_fuel = selfFuel();
      if(curr_fuel - fuel == 0){      //If no change in fuel level then edge is found
        strcpy(bugstring, "Edge Found");
        setPower(0);
        turnToDeg(selfHeadingDeg() + 180);
      }
    }
  }while(pinpoint);
  */
}

