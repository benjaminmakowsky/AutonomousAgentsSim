/// Author: Benjamin Makowsky
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <X11/keysym.h>
#include "beeAI.h"
#include "cAI.h"
#include "beeMain.h"
#include "beeObject.h"


// The global variables hives & honey_spots are problematic:
//    They are implicitly required to be set before calling getPOICoordinates. 
//    getPOICoordinates only works properly if getBases & getFuels have been called, and neither
//    the function name, header nor parameters clue the reader into this.
// We want to keep things modular so that we can scale-up and re-use as much functionality as 
// much possible. 
// BaseStruct_t* hives & FuelStruct_t* honey_spots should be passed as parameters to the getPOICoordinates,
// hinting to the reader that they need to construct these first before calling the function, and
// allowing the user to call the function with different arrays.
BaseStruct_t* hives;
FuelStruct_t* honey_spots;

BaseStruct_t *getBases(char *csv) {
  // Number of bases are the first line
  FILE *fp;
  char buf[1024];
  if (!csv) {
    printf("No points file provided");
    return NULL;
  }

  if ((fp = fopen(csv, "r")) == NULL) {
    printf("Failed to open file: %s\n", csv);
    return NULL;
  }

  //get the first line
  fgets(buf, sizeof(buf), fp);
  int numBases = atoi(buf);

  //skip the number of fuel depots line
  fgets(buf, sizeof(buf), fp);

  //Make an array of the bases
  int i;
  hives = malloc(numBases * sizeof(BaseStruct_t));
  for (i = 0; i < numBases; ++i) {
    fgets(buf, sizeof(buf), fp);
    char *token;
    token = strtok(buf, " ");
    BaseStruct_t newBase;

    newBase.team = atoi(token);
    token = strtok(NULL, " ");
    newBase.x = atoi(token);
    token = strtok(NULL, " ");
    newBase.y = atoi(token);
    newBase.num_bases = numBases;

    //add to array
    hives[i] = newBase;
  }
  if(fclose(fp) == -1){
    printf("Failed to close");
    return NULL;
  }

  return hives;
}

FuelStruct_t *getFuelDepots(char *csv) {
  // Number of fuel depots is the second line
  FILE *fp;
  char buf[1024];
  if (!csv) {
    printf("No points file provided");
    return NULL;
  }

  if ((fp = fopen(csv, "r")) == NULL) {
    printf("Failed to open file: %s\n", csv);
    return NULL;
  }

  // get number of bases
  fgets(buf, sizeof(buf), fp);
  int numBases = atoi(buf);

  //get the number of fuel depots
  fgets(buf, sizeof(buf), fp);
  int numFuels = atoi(buf);

  //skip the base lines
  int i = 0;
  while (i < numBases) {
    fgets(buf, sizeof(buf), fp);
    ++i;
  }

  //Make an array of the fuels
  honey_spots = malloc(numFuels * sizeof(FuelStruct_t));
  for (i = 0; i < numFuels; ++i) {
    fgets(buf, sizeof(buf), fp);
    char *token;
    token = strtok(buf, " ");
    FuelStruct_t newFuel;

    newFuel.x = atoi(token);
    token = strtok(NULL, " ");
    newFuel.y = atoi(token);
    newFuel.num_fuels = numFuels;

    //add to array
    honey_spots[i] = newFuel;

  }

  if(fclose(fp) == -1){
    printf("Could not close for Fuel struct reading");
    return NULL;
  }
  return honey_spots;

}

/*****************************************************************************
 * Move Bee To Coordinates Specified
 * ***************************************************************************/

int goToCoordinates(int x, int y){

  //Get Heading to new point
  int new_heading = getHeadingForCoordinates(x ,y);

  //Turn to new heading
  if(((int)selfHeadingDeg() <= (new_heading - 2)) || ((int)selfHeadingDeg() >= (new_heading + 2))) {
    turnToDeg(new_heading);
  }
  return new_heading;
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

  // use INT_MAX instead of 99999 here
  int xPOI = 99999;
  int yPOI = 99999;

  FILE *fp;
  fp = fopen(LogFile, "a");
  fprintf(fp,"getPOICoordinates(%d, %d)\n",x,y);

  // This global array you mention should be on the client-side (bee.c)
  //TODO: Set bases to a global array at beginning of program
  int length = hives->num_bases;
  fprintf(fp, "numBases read: %d\n",length);
  //Traverse array to determine which location was closest to X, Y
  int i = 0;
  for(i; i < length; i++){
    // you don't need to compute the old_distance through computeDistance in this for-loop.
    int old_distance = computeDistance(x,xPOI,y,yPOI);
    int new_distance = computeDistance(x, hives[i].x, y, hives[i].y);
    fprintf(fp, "From Bases \tIndex %d \tX: %d\tY: %d\n", i, hives[i].x, hives[i].y);
    if(new_distance < old_distance) {
      xPOI = hives[i].x;
      yPOI = hives[i].y;
    }
  }
  fprintf(fp,"Closest base is at (%d,%d)\n", xPOI,yPOI);

  //TODO: Set depots to a global array at beginning of program
  length = honey_spots[0].num_fuels;

  fprintf(fp, "\nnum_fuels read: %d\n",length);
  //Traverse array to determine which location was closest to X, Y
  i = 0;
  for(i; i < length; i++){
    // Again, don't need to re-compute old_distance like this
    int old_distance = abs(computeDistance(x,xPOI,y,yPOI));
    int new_distance = abs(computeDistance(x, honey_spots[i].x, y, honey_spots[i].y));
    fprintf(fp, "From Fuels \tIndex %d \tX: %d\tY: %d\n", i, honey_spots[i].x, honey_spots[i].y);
    if(new_distance < old_distance) {
      xPOI = honey_spots[i].x;
      yPOI = honey_spots[i].y;
    }
  }

  static int coordinates[2];


  coordinates[0] = xPOI;
  coordinates[1] = yPOI;
  fprintf(fp,"------------------------------\n");
  fclose(fp);

  return coordinates;

}


bool inVicinityOf(int x,int y){
  // This '60' is a magic number. It should be a #define at the start of beeAI.h so that
  // it can easily be adjusted and re-used
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
    //If not in the vicinty of the point slow down as you approach
    int distance = computeDistance(selfX(),x,selfY(),y);
    // The numbers here should be put into #defines in beeAI.h
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

int avoidWalls(){
  getWallAvoidanceVector();  //Update ship direction vector for wall avoidance
  int newHeading = 0;
  if (wallVector != -1) {
    // Another number to put in a #define
    newHeading = selfHeadingDeg() + 90;
    turnToDeg(newHeading);
  }
  return newHeading;
}

bool comeToStop(int number_of_frames) {

  static int current_x = 0;
  static int current_y = 0;
  refuel(0);
  setPower(0);
  static int counter = 0;
  // Stylistic nitpick: I consider this if structure:
  // if{
  //
  // } else {
  //
  // }
  //
  // To be more readable than:
  //
  // if{ 
  //
  // } else {}
  if (counter < number_of_frames) {
    sprintf(bugstring, "%d", counter);

    //Current coordinate positions are not the same then reset counter
    if (current_x != selfX() && current_y != selfY()) {
      counter = 0;
      current_x = selfX();
      current_y = selfY();
    } else { counter += 1; }
    return false;

  }else{ return true;}
}


void checkForFuel(){
  static bool fueling = false;
  // Another case for adding a new #define
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
}

/*
void log(char[50] string){
  FILE *fp;
  fp = fopen(LogFile, "a");
  fprintf(fp,string);
  fprintf(fp,"------------------------------\n");

  fclose(fp);
}*/

bool dance(int prevState){
  static bool dance_is_completed = false;
  // Should change this switch to a simple 'if' if we only do
  // something when prevState == STATE_SEARCHING
  // Follow-up question: Why do we care about stae when we call dance?
  // Add a comment explaining why.
  switch(prevState)
  {
    case STATE_SEARCHING:
      dance_is_completed = honeyFoundDance();
      break;
    case STATE_FORAGING: 
      break;
  }

  //Stop dancing once dance has finished
  if(dance_is_completed){
    dance_is_completed = false;
    return true;
  }else{
    return false;
  }
}

bool honeyFoundDance(){

  static bool is_initial_setup = true;
  static int initial_heading = 0;
  static int number_of_spins = 0;
  static int target_degree = 0;
  // Another thing to add as a # define
  int desired_rotations = 2;

  // Add a comment explaining what is going on in this if statement.
  if(is_initial_setup){
    number_of_spins = 0;
    initial_heading = (int)selfHeadingDeg();
    target_degree = (initial_heading + 345) % 360;
    is_initial_setup = false;
  }

  //Perform Dance Motions
  turnToDeg((int)selfHeadingDeg() + 40);

  //Count number of rotations in order to determine when dance is finished
  if(beeDegIsBetween(target_degree, (target_degree + 10) % 360)) {
    number_of_spins+=1;
  }

  //If desired number of spins has been reached dance is finished
  if(number_of_spins >= desired_rotations ){
    is_initial_setup = true;
    return true;
  }
  return false;
}

bool beeDegIsBetween(int deg1, int deg2){

  /*******************************************************
   * Case 1: deg1 < deg2                                 *
   * Do not have to account for passing 360 ie (45 to 90)*
   *******************************************************/
   if(deg1 < deg2){
     return (deg1 <= (int)selfHeadingDeg() && (int)selfHeadingDeg() <= deg2);
   }
  /*******************************************************
  * Case 2: deg1 > deg2                                 *
  * Do have to account for passing 360 ie (355 to 5)    *
  *******************************************************/
   else if(deg1 > deg2){
     return((int)selfHeadingDeg() > deg1 || (int)selfHeadingDeg() < deg2);
   }
   else{
     return ((int)selfHeadingDeg() == deg1);
   }
}
