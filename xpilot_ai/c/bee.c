//
// Created by makowskyb on 7/2/19.
//

#include "bee.h"
#include "beeAI.h"
#include "cAI.h"
#include "beeMain.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "beeObject.h"


//global variables
// What are these used for? Remove them if they aren't used. 
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
  static bool fuel_found = false;
  static int goal_frame = 0;
  static int old_heading = 0;
  static int new_heading = 0;
  static int xPOI = 0;
  static int yPOI = 0;


  //Step 1: Check for walls
  avoidWalls();


  //Step 2: Attempt to Attain Honey
  if (!fuel_found) {
    checkForFuel();

    //Step 3: Check if fuel levels changed
    double new_fuel_level = selfFuel();
    if (new_fuel_level - fuel > 0) {
      x = selfX();
      y = selfY();
      fuel_found = true;
    }
  }


  if (fuel_found) {
    if(comeToStop(30) == false){
      //Do nothing
    } else {

      int POICoordinates[2];
      static bool fileRead = false; //Boolean to set coordinates only once
      if(!fileRead) {
        memcpy(POICoordinates, getPOICoordinates(x, y), sizeof(getPOICoordinates(x, y)));
        setHoneyX(POICoordinates[0]);
        setHoneyY(POICoordinates[1]);
        // remove this commented line if it won't be used
        //rememberPOICoords(POICoordinates[0],POICoordinates[1]);
        fileRead = !fileRead;
      }

      FILE *fp;
      fp = fopen(LogFile, "a");
      fprintf(fp,"Saved POI Coordinates as (%d,%d)\n",getHoneyX(),getHoneyY());
      fprintf(fp,"Ending Search behavior\n");
      fprintf(fp,"------------------------------\n");

      fclose(fp);
      sprintf(bugstring, "Search Moving to %d and %d",getHoneyX(), getHoneyY());
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
  static bool performed_dance = false;
  static bool depositing = true;
  static bool forage_state_changed = true;

  //Log status line showing what method just executed
  if (initForage) {
    FILE *fp;
    fp = fopen(LogFile, "a");
    fprintf(fp, "\nBeginning Forage Behavior\n");
    fprintf(fp,"------------------------------\n");
    fprintf(fp,"Home Base:  (%d,%d)\n",selfBaseX(), selfBaseY());
    fprintf(fp,"Honey Spot: (%d,%d)\n", getHoneyX(), getHoneyY());
    fprintf(fp,"------------------------------\n");
    fclose(fp);
    // just set it to false, it's faster.
    initForage = !initForage;
  }



  /** Steps:
   *    1) Determine if heading to hive or honey
   *    2) Approach Destination and stop
   *    3) Gather or deposit honey
   *    4) Repeat
   */


  //Step 1:
  //Determine whether or not you are heading to hive to deposit honey or
  //if you are headed to flower to pickup honey
  // Use a more descriptive variable name than x,y, i.e objectiveX, objectiveY
  if(depositing){
    x = selfBaseX();
    y = selfBaseY();
  } else {
    x = getHoneyX();
    y = getHoneyY();
  }


  //If block placed here in order to debug and test that x and y are properly changed
  /***
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
  */


  static int fuelLVL = 0;

  //Step 2: Determine if near honey/hive
  if(!inVicinityOf(x,y)) {
    refuel(0);
    sprintf(bugstring, "Forage: Moving to location (%d, %d) ",x,y);
    goToCoordinates(x,y);

  //Step 3; Gather or deposit fuel
  }else {
    setPower(0);
    if(performed_dance == false){
      performed_dance = dance(STATE_SEARCHING);
    }else {
      fuelLVL = (int) selfFuel();
      // empty & full should be in #define 
      int empty = 500;
      int full = 700;
      if (fuelLVL > empty && depositing) {
        checkForFuel();
        strcpy(bugstring, "Depositing");
      } else if (fuelLVL < full && !depositing) {
        checkForFuel();
        strcpy(bugstring, "Gathering");

        //Step 4: Repeat Loop
      } else {
        refuel(0);
        strcpy(bugstring, "Moving");
        depositing = !depositing;       //Change internal forage state
        forage_state_changed = true;    //Set flag for Logging change
      }
    }
  }
}

