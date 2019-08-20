//
// Created by makowskyb on 7/2/19.
//

#include "beeGlobals.h"
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

#define EMPTY 200
#define FULL 700

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
        fileRead = !fileRead;
      }

      OPENLOG()
      fprintf(fp,"Saved POI Coordinates as (%d,%d)\n",getHoneyX(),getHoneyY());
      fprintf(fp,"Ending Search behavior\n");
      fprintf(fp,"------------------------------\n");

      fclose(fp);
      sprintf(bugstring, "Search Moving to %d and %d",getHoneyX(), getHoneyY());
      state = STATE_FORAGING;
      sendSelfState(state);
    }
  }
}

/*****************************************************************************
 * Foraging (Controller)- Benjamin Makowsky
 * ***************************************************************************/
void forage() {

  fuel = selfFuel();
  int destination_x = 0;
  int destination_y = 0;
  static bool initForage = true;
  static bool performed_dance = false;
  static bool depositing = true;
  static bool forage_state_changed = true;

  //Log status line showing what method just executed
  if (initForage) {
    OPENLOG()
    fprintf(fp, "\nBeginning Forage Behavior\n");
    fprintf(fp,"------------------------------\n");
    fprintf(fp,"Home Base:  (%d,%d)\n",selfBaseX(), selfBaseY());
    fprintf(fp,"Honey Spot: (%d,%d)\n", getHoneyX(), getHoneyY());
    fprintf(fp,"------------------------------\n");
    fclose(fp);
    initForage = false;
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
  if(depositing){
    destination_x = selfBaseX();
    destination_y = selfBaseY();
  } else {
    destination_x = getHoneyX();
    destination_y = getHoneyY();
  }



  static int fuelLVL = 0;
  //Step 2: Determine if near honey/hive
  if(!inVicinityOf(destination_x,destination_y)) {
    if(getPower() == 0){
      setPower(10);
    }
    refuel(0);
    sprintf(bugstring, "Forage: Moving to location (%d, %d) ",destination_x,destination_y);
    goToCoordinates(destination_x,destination_y);


  //Step 3; Gather or deposit fuel
  }else {
    static bool beingObserved = false;  //Used to determine if being observed
    static int waiting_counter = 0;     //Counter used to wait for specified frames
    int frameLimit = 14 * 10;           //14fps for 10 seconds
    setPower(0);                        //Come to stop until being observed or time limit has been reached

    //wait to fly off until someone has seen dance or time has been reached
    if(performed_dance == false && waiting_counter < frameLimit && depositing){

      //while not being observed increment counter
      if(!beingObserved){
        //do nothing for n seconds or until observed
        waiting_counter++;
        beingObserved = checkIfBeingObserved();
        sprintf(bugstring,"waiting: %.2f", (float)waiting_counter/(frameLimit) * 100);

      //If you are being observed perform dance
      }else {
        performed_dance = dance(STATE_SEARCHING);
      }

    }else {
      fuelLVL = (int) selfFuel();
      if (fuelLVL > EMPTY && depositing) {
        checkForFuel();
        strcpy(bugstring, "Depositing");
      } else if (fuelLVL < FULL && !depositing) {
        checkForFuel();
        strcpy(bugstring, "Gathering");

        //Step 4: Repeat Loop
      } else {
        refuel(0);
        strcpy(bugstring, "Moving");
        depositing = !depositing;       //Change internal forage state
        forage_state_changed = true;    //Set flag for Logging change
        waiting_counter = 0;            //Reset waiting limit for next iteration
      }
    }
  }
}

/*****************************************************************************
 * Foraging (Controller)- Benjamin Makowsky
 * ***************************************************************************/
void onlook(){
  static int dancing_ship = -1;


  //If not near the hive, go to it
  if(!inVicinityOf(selfBaseX(),selfBaseY())){

    //Make sure we are at the base when we are observing
    goToCoordinates(selfBaseX(),selfBaseY());

  //While at the hive observe dancing bee
  }else{

    //Once at base stop and wait
    setPower(0);

    //While not watching a dancing ship, check to see who may be dancing
    if(dancing_ship == -1){
      int field_of_view = 360;  //Looks for ships all around bee
      int range_of_view = 40;   //Distance for how far a bee can be seen

      //Check to see if any ships are nearby and if one is get its ID
      //TODO: Possibly change name to getNearbySHipID() or similar
      dancing_ship = seeIfDancing(field_of_view,range_of_view);
      sprintf(bugstring,"Observing Ship: %d",dancing_ship);
    }else{

      //Turn towards the bee we are observing
      int targetHeading = getHeadingBetween(selfX(),selfY(),getDancersX(dancing_ship),getDancersY(dancing_ship));
      if(selfHeadingDeg() < targetHeading-1 || selfHeadingDeg() > targetHeading + 1) {
        //sprintf(bugstring,"self: %d target: %d",selfHeadingDeg(),targetHeading);
        //sprintf(bugstring,"targetHeading: %d",targetHeading);
        turnToDeg(targetHeading);
      }else{
        //While looking at dancer observe dance
        int danceObserved = observeDance(dancing_ship);
        sprintf(bugstring, "Observed Dance: %d", danceObserved);
      }
    }
  }
}
