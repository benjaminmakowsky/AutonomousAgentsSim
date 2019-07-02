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
extern int frameCount;		//how many frames have elapsed
extern int degToAim;		//what direction do we want to go
extern int turnLock;		//time not allowed to compute new wall avoidance
extern int wallVector;		//where to go to avoid crashing into a wall


extern double fuel;
extern int fov;			    //field (angle) of vision
extern char bugstring[50];


//INPROGRESS: Implement search - Benjamin Makowsky -----------------------
/*****************************************************************************
 * Searching (Controller)
 * ***************************************************************************/
void searching()
{
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
  static bool fueling = false;
  static fuel_found = false;
  static int goal_frame = 0;
  static int original_distance;

  /*
   * Step 1: Check for walls
   */
  wallAvoidance();  //Update vector to for wall avoidance
  if(wallVector != -1){
    turnToDeg(selfHeadingDeg() + 125);
    //printf("Found Wall");
  }

  /*
   * Step 2: Attempt to Attain Honey
   */
  if(!fuel_found) {
    int frames_passed = 3; //Minimum amount of frames that can be recognized is 3
    if (fueling == false) {
      fuel = selfFuel();
      strcpy(bugstring, "Start Refuel");
      refuel(1);
      fueling = true;
    }
    if ((frameCount % frames_passed == 0) && (fueling == true)) {
      strcpy(bugstring, "Stop Refuel");
      refuel(0);
      fueling = false;
    }

    /*
     * Step 3: Check if fuel levels changed
     */
    double new_fuel_level = selfFuel();
    if (new_fuel_level - fuel > 0) {
      setPower(0);
      x = selfX();
      y = selfY();
      degToAim = (int)selfHeadingDeg() + 180;
      fuel_found = true;
    }
  }


  //INPROGRESS: TODO:go to initial point and orient to closest base object
  if(fuel_found){

    //TODO: wait until ship stops (Could test coordinates until same n times in a row
    int num_seconds = 14 * 4;
    if(goal_frame == 0) {     //Checks if we have done this before
      goal_frame = frameCount + num_seconds;


    }else if(frameCount <= goal_frame) {
      char temp_str[25];
      sprintf(temp_str, "%.2f", (double) frameCount / goal_frame);
      strcpy(bugstring, temp_str);


    }else if(frameCount == goal_frame){

      degToAim = (int)atan(((float)(y-selfY())/(x-selfX())));
      char temp_str[25];
      sprintf(temp_str, "%d", degToAim);
      strcpy(bugstring, temp_str);

      //Must be >= because turning time is more than 1 frame
    }else{



      if((int)selfHeadingDeg() != degToAim) {
        turnToDeg(degToAim);

      }else if((int)selfHeadingDeg() == degToAim){
        strcpy(bugstring, "Turn Completed");
      }

      if(selfX() != x && selfY() != y){

        int d = goToPoint(x,y);
        char temp_str[25];
        sprintf(temp_str, "%d", d);
        //strcpy(bugstring, temp_str);
      }
    }


    //pinpoint(x, y);
  }

}

/*****************************************************************************
 * INPROGRESS:Pinpoint the center of a hive
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

