//
// Created by makowskyb on 8/17/19.
//

#include "beeDance.h"
#include "beeObject.h"
#include "cAI.h"
#include "beeGlobals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


//Local Variables
// Consider making danceMoves a struct instead of an array, this would allow for more
// descriptive fields. Additionally, a struct is easier to expand upon.
//Global Variables used in multiple functions
static char danceMoves[4] = {none, none, none, none}; //Array to hold dance moves
// msgTypes would work better as an enum
int msgTypes[2] = {foundSource, foundEnemy};
// Make danceTree const
int danceTree[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static int initialHeading = 0;
static int rightHeading = 0;
static int leftHeading = 0;
static int rearHeading = 0;


bool dance(int msgType) {
  static bool completed_first_dance = false;
  static bool completed_xcoord_dance = false;
  static bool completed_ycoord_dance = false;
  static bool isInitial = true;

  if(isInitial) {
    //Reset directional headings for dance
    setDanceHeadings();

    //Reset Flags
    completed_first_dance = false;
    completed_xcoord_dance = false;
    completed_ycoord_dance = false;
    isInitial = false;

    OPENLOG()
    fprintf(fp, "\nInitializing beeDance() with msgType: %d\n", msgType);
    fprintf(fp, "Init: %d, Left: %d, Right: %d\n",initialHeading,leftHeading,rightHeading);
    fprintf(fp,"------------------------------\n");
    fclose(fp);
  }

  //Make sure you are fully stopped before dancing
  if (selfSpeed() == 0) {

    switch (msgType) {
      case foundSource:
        //Dance msgType
        if (!completed_first_dance) {
          completed_first_dance = relayMsg(msgType);

          //Dance x coordinates
        } else if (!completed_xcoord_dance) {
          completed_xcoord_dance = relayCoords(getHoneyX());
          //completed_second_dance = true;

          //Dance y coordinates
        } else if (!completed_ycoord_dance) {
          //completed_third_dance = true;
          completed_ycoord_dance = relayCoords(getHoneyY());
        }
        break;

      case foundEnemy:
        break;
    }
  } else {
    //IF you haven't stopped moving make sure the power is off
    setPower(0);
  }

  //Stop dancing once dance has finished
  if (completed_first_dance && completed_xcoord_dance && completed_ycoord_dance) {
    setNeedsToDance(false);
    isInitial = true;
    return true;
  } else {
    return false;
  }
}


bool relayMsg(int symbol) {
  static bool finishedMove = false;
  static bool isInitial = true;
  static char danceDirection = none;
  POWER_OFF

  //Function initialization; reset all values for future dances
  if (isInitial) {
    finishedMove = false;
    danceDirection = (symbol == 0) ? left : right;
    isInitial = false;
    OPENLOG()
    fprintf(fp, "Begin relayMsg(%d) in direction %c\n", symbol, danceDirection);
    fclose(fp);
  }

  //After initialization perform the movement
  finishedMove = performMovementFor(danceDirection);
  if (finishedMove) {isInitial = true;}
  return finishedMove;
}


bool relayCoords(int coords) {
  static bool finishedMove = false;
  static char danceDirection = none;
  static char *danceSequence = 0;


  if(danceSequence == 0) {
    danceSequence = buildDance(coords);
    OPENLOG()
    fprintf(fp, "\nDance built:\n");
    int i;
    for (i = 0; i < 8; i++) {fprintf(fp, "%c", danceSequence[i]);}
    fclose(fp);
  }else{

    finishedMove = performSequence(danceSequence);
    if(finishedMove){
      danceSequence = 0;
      finishedMove = false;
      return true;
    }
    return finishedMove;
  }
}


/*********************
 * Helper Methods
 ********************/
char* buildDance(int coords) {

  int numInts = 0;
  int unitsPlace = 1;
  static char danceMoves[max_num_moves];
  memset(danceMoves, 0, sizeof(danceMoves));
  OPENLOG()
  fprintf(fp, "\n\n\nBegin buildDance(%d)\n", coords);
  fprintf(fp, "--------------------\n");

  //Get the number of integers to relay in the coordinate
  //ie 100 is 3 ints 20 is 2 ints and 254672 is 6 ints
  // do-while loops are ugly, in my opinion. This can be converted to a for loop:
  // for( numInts = 0; coords / unitsPlace != 0; numIts++, unitsPlace *= 10 );
  do {
    numInts += 1; //increments the number of integers each time the unitsPlace exists in the coordinates
    unitsPlace *= 10;
  } while (coords / unitsPlace != 0);
  // Another reason to do a for-loop instead of a do-while
  unitsPlace /= 10; //Divide by 10 because it is now too high from the do_while
  fprintf(fp, "Number of digits: (%d)\n", numInts);

  //Create array to hold the units of the coordinate
  int units[numInts];
  int i;
  int tempUnits = coords;
  for (i = 0; i < numInts; i++) {
    units[i] = tempUnits / unitsPlace;
    fprintf(fp, "%d ", units[i]);
    tempUnits %= unitsPlace;           //ie 123 -> 23
    unitsPlace /= 10;                   //ie go from thousandths to hundredths
  }


  //Get the total number of dance moves in message
  int num_dance_moves = 0;
  for (i = 0; i < numInts; i++) {
    num_dance_moves += getDepthOfNumber(units[i]);
  }
  fprintf(fp, "\nnum_dance_moves: %d\n", num_dance_moves);

  //Create the array to hold all the dance moves
  //total_moves = num_dance_moves+num_ints to account for pauses after each int -1 because the end
  //doesnt need a pause
  int total_moves = num_dance_moves + numInts - 1;
  fprintf(fp, "total_moves: %d", total_moves);


  //Fill the danceMoves array with the moves
  int num_moves_added = 0;
  for (i = 0; i < numInts; i++) {

    //Get the dance moves for the specified number
    int currIdx = units[i];
    fprintf(fp, "\nDance moves for %d: ", units[i]);

    //Get path to idx
    int pathLength = getDepthOfNumber(currIdx);
    int pathToIdx[pathLength];
    int pathIdx = pathLength - 1;
    //Fill pathToIdx array
    for (; pathIdx >= 0; pathIdx--) {
      pathToIdx[pathIdx] = currIdx;
      currIdx = getParent(currIdx);
    }

    //Create dance for path to idx
    for (currIdx = 0; currIdx < pathLength; currIdx++) {
      if (pathToIdx[currIdx] % 2 == 0) {
        danceMoves[num_moves_added] = left;
      } else {
        danceMoves[num_moves_added] = right;
      }
      fprintf(fp, "%c ", danceMoves[num_moves_added]);
      num_moves_added++;
    }
    if (num_moves_added < total_moves) {
      danceMoves[num_moves_added] = endOfSequence;
      fprintf(fp, "%c ", danceMoves[num_moves_added]);
      num_moves_added++;
    }else{
      //Add in end of coordinate double spacing
      danceMoves[num_moves_added] = endOfSequence;
      fprintf(fp, "%c ", danceMoves[num_moves_added]);
      num_moves_added++;
      danceMoves[num_moves_added] = endOfSequence;
      fprintf(fp, "%c ", danceMoves[num_moves_added]);
    }
  }
  fclose(fp);
  return danceMoves;
}

int getParent(int child) {
  return danceTree[child / 2 - 1];
}

int getDepthOfNumber(int number) {
  int depth = 1;
  int currIndex = number;
  while (currIndex / 2 - 1 >= 0) {
    currIndex = currIndex / 2 - 1;
    depth += 1;
  }
  return depth;
}


bool performSequence(char* sequence){
  static bool completedSequence = false;
  static bool isInitial = true;
  static int wait_count = 0;
  static bool completedChar = false;
  static bool needsReset = false;
  static int i = 0;


  //Count the number of dance moves in the sequence
  int sequenceLength = 0;
  while(sequence[sequenceLength] != '\0'){
    sequenceLength++;
  }

  //IF first time or you have already completed 1 sequence
  if(isInitial || completedSequence){
    i = 0;
    completedSequence = false;
    OPENLOG()
    fprintf(fp,"\n\nBeginning performSequence()\n");
    fprintf(fp,"Performing %d moves\n", sequenceLength);
    fprintf(fp,"---------------------------\n");
    isInitial = false;
    fclose(fp);
  }

  //Resets the static variables for the y coordinate after peforming the x coordinate
  if(needsReset){
    completedChar = false;
    needsReset = false;
    wait_count = 0;
  }

  //Iterate through all the dance moves
  if(i < sequenceLength){
    completedChar = performMovementFor(sequence[i]);

    //Once completed wait and return to start position
    if(completedChar) {
      OPENLOG()
      i++;
      needsReset = true;
      fprintf(fp,"Finished sequence part %d\n", i);
      fclose(fp);
    }
  }else{
    OPENLOG()
    completedSequence = true;
    needsReset = true;
    fprintf(fp,"completedSequence\n");
    fclose(fp);
    i=0; //added by DPM 20190830 to dance next time we return to hive
  }
  return completedSequence;
}


bool performMovementFor(char dir){
  static bool finishedMove = false;
  static int wait_count = 0;
  static bool isInitial = true;
  POWER_OFF

  if (isInitial) {
    OPENLOG()
    finishedMove = false;
    wait_count= 0;
    isInitial = false;
    fprintf(fp, "\nBegin movement(%c)\n", dir);
    fclose(fp);
  }
  //If you havent finished moving, do it again
  if (!finishedMove) {
    finishedMove = turnToDanceDirection(dir);
  }

  //Once finished moving to designated dance direction return to intitial heading
  if (finishedMove) {
    isInitial = returnToInitialHeading(&wait_count);
  }

  //isInitial will be false unless you have returned to the initial heading
  return isInitial;
}

/// Set the 4 dance headings, left,right, initial, and rear
void setDanceHeadings(){
  initialHeading = (int) selfHeadingDeg();
  rightHeading = (initialHeading - 90 + 360) % 360; //+360 to account for going past -1 degrees
  leftHeading = (initialHeading + 90) % 360;
  rearHeading = (initialHeading + 180) % 360;
}

/// Turns bee to dance direction specified
/// \param dir Direction to turn
/// \return boolean if turn was completed
bool turnToDanceDirection(char dir){
  if (dir == left) {
    turnToDeg(leftHeading);
    return headingIsBetween((int)selfHeadingDeg(),leftHeading-2,leftHeading+2);

  } else if (dir == right){
    turnToDeg(rightHeading);
    return headingIsBetween((int)selfHeadingDeg(),rightHeading-2,rightHeading+2);

  }else if (dir == endOfSequence){
    turnToDeg(rearHeading);
    return headingIsBetween((int)selfHeadingDeg(),rearHeading-2,rearHeading+2);
  }
}

/// Returns bee to initial heading for dance
/// \param num_frames_to_wait number of frames needed to wait to observe
/// \return boolean if move was completed
bool returnToInitialHeading(int *num_frames_to_wait){
  //Wait frames to allow observation
  if((*num_frames_to_wait) < minimum_frames_to_observe){
    (*num_frames_to_wait)++;

  }else{
    if(!headingIsBetween(selfHeadingDeg(),initialHeading-2,initialHeading +2)){
      turnToDeg(initialHeading);
    }else{
      if((*num_frames_to_wait)  < minimum_frames_to_observe * 2){
        (*num_frames_to_wait)++;
      }else {return true;}
    }
  }
  return false;
}