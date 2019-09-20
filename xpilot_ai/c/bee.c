//
// Created by makowskyb on 7/2/19.
//

#include "beeGlobals.h"
#include "bee.h"
#include "beeAI.h"
#include "beeDance.h"
#include "cAI.h"
#include "beeMain.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "beeObject.h"
#include "beeObserve.h"

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
    useFueler();

    //Step 3: Check if fuel levels changed
    fuel_found = checkforFuel("increasing", fuel);
  }

  //Step 4: Store location of found honey
  if (fuel_found) {
    x = selfX();
    y = selfY();

    static bool fileRead = false; //Boolean to set coordinates only once
    if(!fileRead) {
      int POICoordinates[2]; //Used to hold returned array from getPOICoordinates
      memcpy(POICoordinates, getPOICoordinates(x, y), sizeof(POICoordinates));
      setHoneyX(POICoordinates[0]);
      setHoneyY(POICoordinates[1]);
      fileRead = !fileRead;
    }

    OPENLOG()
    fprintf(fp,"Saved POI Coordinates as (%d,%d)\n",getHoneyX(),getHoneyY());
    fprintf(fp,"Ending Search behavior\n");
    fprintf(fp,"------------------------------\n");
    fclose(fp);

    state = STATE_FORAGING;
    sendSelfState(state);
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
    sprintf(bugstring, "Forage: Moving to location (%d, %d) ",destination_x,destination_y);
    turnToCoordinates(destination_x,destination_y);
    stopAtCoordinates(destination_x,destination_y);
    if(getPower() == 0){
      // Make this '5' a define
      setPower(5);
    }
    refuel(0);

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
        performed_dance = dance(foundSource);
      }

    }else {
      fuelLVL = (int) selfFuel();
      if (fuelLVL > EMPTY && depositing) {
        useFueler();
        strcpy(bugstring, "Depositing");
      } else if (fuelLVL < FULL && !depositing) {
        useFueler();
        strcpy(bugstring, "Gathering");

        //Step 4: Repeat Loop
      } else {
        refuel(0);
        depositing = !depositing;       //Change internal forage state
        forage_state_changed = true;    //Set flag for Logging change
	performed_dance = false;        //added DPM 20190830: reset flag so we dance again next return to hive
	beingObserved = false;          //added DPM 20190830: reset flag so we check before dancing on next return to hive
        waiting_counter = 0;            //Reset waiting limit for next iteration
      }
    }
  }
}

/*****************************************************************************
 * Onlooking (Controller)- Benjamin Makowsky
 * ***************************************************************************/
void onlook(){
  static int dancing_ship = -1;

  //If not near the hive, go to it
  if(!inVicinityOf(selfBaseX(),selfBaseY())){

    //Make sure we are at the base when we are observing
    turnToCoordinates(selfBaseX(),selfBaseY());
    stopAtCoordinates(selfBaseX(), selfBaseY());

  //While at the hive observe dancing bee
  }else{

    //Once at base stop and wait
    setPower(0);

    //While not watching a dancing ship, check to see who may be dancing
    if(dancing_ship == -1){
      int field_of_view = 360;  //Looks for ships all around bee
      int range_of_view = 60;   //Distance for how far a bee can be seen

      //Check to see if any ships are nearby and if one is get its ID
      dancing_ship = seeIfDancersWaiting(field_of_view,range_of_view);
      sprintf(bugstring,"Observing Ship: %d",dancing_ship);
    }else{

      //Turn towards the bee we are observing
      int targetHeading = getHeadingBetween(selfX(),selfY(),getDancersX(dancing_ship),getDancersY(dancing_ship));
      if(!headingIsBetween((int)selfHeadingDeg(),targetHeading-1,targetHeading+1)) {
        turnToDeg(targetHeading);
      }else{
        //While looking at dancer observe dance
        int danceObserved = observeDance(dancing_ship);
        sprintf(bugstring, "Observed Dance: %d", danceObserved);

        if(danceObserved == FOUND_HONEY){
          state = STATE_FORAGING;
          sendSelfState(state);
        }
      }
    }
  }
}
