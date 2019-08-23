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
static char danceMoves[4] = {none, none, none, none}; //Array to hold dance moves
int msgTypes[2] = {foundSource, foundEnemy};
int danceTree[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static int initialHeading = 0;
static int rightHeading = 0;
static int leftHeading = 0;


bool dance(int msgType) {
  static bool completed_first_dance = false;
  static bool completed_second_dance = false;
  static bool completed_third_dance = false;
  static bool isInitial = true;

  if(isInitial) {
    OPENLOG()

    completed_first_dance = false;
    completed_second_dance = false;
    completed_third_dance = false;
    initialHeading = (int) selfHeadingDeg();
    rightHeading = (initialHeading - 90 + 360) % 360; //+360 to account for going past -1 degrees
    leftHeading = (initialHeading + 90) % 360;
    fprintf(fp, "\nInitializing beeDance() with msgType: %d\n", msgType);
    fprintf(fp, "Init: %d, Left: %d, Right: %d\n",initialHeading,leftHeading,rightHeading);
    fprintf(fp,"------------------------------\n");
    isInitial = false;
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
        } else if (!completed_second_dance) {
          completed_second_dance = relayCoords(getHoneyX());
          //completed_second_dance = true;

          //Dance y coordinates
        } else if (!completed_third_dance) {
          completed_third_dance = true;
          //completed_third_dance = relayCoords(getHoneyY());
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
  if (completed_first_dance && completed_second_dance && completed_third_dance) {
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
    OPENLOG()
    finishedMove = false;
    danceDirection = (symbol == 0) ? left : right;
    isInitial = false;
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
  static bool isInitial = true;
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
    return finishedMove;
  }
}


/*********************
 * Helper Methods
 ********************/
char *buildDance(int coords) {

  int numInts = 0;
  int unitsPlace = 1;
  static char danceMoves[3 * 4];
  memset(danceMoves, 0, sizeof(danceMoves));
  OPENLOG()
  fprintf(fp, "\n\n\nBegin buildDance(%d)\n", coords);
  fprintf(fp, "--------------------\n");

  //Get the number of integers to relay in the coordinate
  //ie 100 is 3 ints 20 is 2 ints and 254672 is 6 ints
  do {
    numInts += 1; //increments the number of integers each time the unitsPlace exists in the coordinates
    unitsPlace *= 10;
  } while (coords / unitsPlace != 0);
  unitsPlace /= 10; //Divide by 10 because it is now too high from the do_while
  fprintf(fp, "numInts: (%d)\n", numInts);

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

  int sequenceLength = (int)(sizeof(sequence) / sizeof(sequence[0]));

  if(isInitial){
    OPENLOG()
    fprintf(fp,"\n\nBeginning performSequence()\n");
    fprintf(fp,"Performing %d moves\n", sequenceLength);
    fprintf(fp,"---------------------------\n");
    isInitial = false;
    fclose(fp);
  }

  if(needsReset){
    completedChar = false;
    needsReset = false;
    wait_count = 0;
  }

  //Iterate through all the dance moves
  static int i = 0;
  if(i < sequenceLength){
    char danceDir = sequence[i];

    completedChar = performMovementFor(danceDir);
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
    fprintf(fp,"completedSequence\n");
    fclose(fp);
  }
  return completedSequence;
}


bool performMovementFor(char dir){
  static bool finishedMove = false;
  static int wait_count = 0;
  static bool isInitial = true;
  OPENLOG()
  POWER_OFF

  if (isInitial) {
    finishedMove = false;
    wait_count= 0;
    isInitial = false;
    fprintf(fp, "\nBegin movement(%c)\n", dir);
  }

  //If you havent finished moving, do it again
  if (!finishedMove) {
    if (dir == left) {
      //fprintf(fp, "Turning left: %d Currently: %d\n", leftHeading, (int) selfHeadingDeg());
      turnToDeg(leftHeading);
      finishedMove = headingIsBetween((int)selfHeadingDeg(),leftHeading-2,leftHeading+2);
    } else if (dir == right){
      //fprintf(fp, "Turning right: %d Currently: %d\n", rightHeading, (int) selfHeadingDeg());
      turnToDeg(rightHeading);
      finishedMove = headingIsBetween((int)selfHeadingDeg(),rightHeading-2,rightHeading+2);
    }else{
      wait_count -= endOfWordSig;
      finishedMove = true;
    }
  }
  if (finishedMove) {

    //Wait 8 frames to signal end of Signal
    if(wait_count == 0){
      fprintf(fp, "END OF SYMBOL %c\n",dir);
      wait_count++;
    }else{
      if(!headingIsBetween(selfHeadingDeg(),initialHeading-2,initialHeading +2)){
        turnToDeg(initialHeading);
      }else{
        //reset for next msg
        //Signal end of word
        if(wait_count < endOfSymbolSig){
          wait_count++;
          fprintf(fp, "%d, ",wait_count);
        }else {
          isInitial = true;
          fclose(fp);
          return true;
        }
      }
    }
  }
  fclose(fp);
  return false;
}