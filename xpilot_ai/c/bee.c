//
// Created by makowskyb on 7/2/19.
//

#include "bee.h"
#include "beeAI.h"
#include "cAI.h"
#include "beeBoids.h"
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
      static bool fileRead = false;
      if(!fileRead) {
        memcpy(POICoordinates, getPOICoordinates(x, y), sizeof(getPOICoordinates(x, y)));
        rememberPOICoords(POICoordinates[0],POICoordinates[1]);
        fileRead = !fileRead;
      }

      FILE *fp;
      fp = fopen(LogFile, "a");
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
    fp = fopen(LogFile, "a");
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


  //If block placed here in order to debug and test that x and y are properly changed
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

