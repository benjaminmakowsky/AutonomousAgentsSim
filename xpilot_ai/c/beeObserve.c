//
// Created by makowskyb on 9/17/19.
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

char danceTree[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};


int observeDance(int ship_id){

  static bool observing_dance = false; //boolean used to flag if ship is dancing
  static int initialHeading = 0;       //Heading used to determine start of dance position
  static int targetHeading = 0;        //Heading used as the marker for a dance turn
  static int num_turns = 0;            //Number of turns made to determine dance
  int dance_observed = -1;             //Used to return the dance type
  static bool dancingCheck = true;     //Used to determine if bee is still dancing

  char LogFile[20] = "";
  sprintf(LogFile, "./logs/LOG%d.txt", selfID());
  FILE *fp;

  //Initialization code
  if(!observing_dance){
    initialHeading = getShipDir(ship_id);      //Record initial heading as start of dance orientation
    targetHeading = initialHeading + 180;         //Target Heading for when a dance move ends
    observing_dance = true;                       //Flag to exit initialization
    OPENLOG()
    fprintf(fp,"observeDance(ship_id: %d)\n",ship_id);
    fclose(fp);
  }else{
    char* dancePattern = NULL;
    if(dancingCheck)
    {
      dancingCheck = beeIsDancing(ship_id);
      dancePattern = observeDanceMoves(ship_id); //Gets the dance move performed by the dancer
    }

    //IF no longer dancing and dancePattern exists
    if(!dancingCheck && dancePattern != NULL){
      dance_observed = dancePattern[0];

      //INPROGRESS: Save coordinates to self bee
      //setHoneyX(interpretCoord('x',dancePattern));
      //setHoneyY(interpretCoord('y',dancePattern));
      //sethoneyx and y in beeobject
      //TODO: create dance type enumeration

    }
  }
  return dance_observed;
}

bool beeIsDancing(int ship_id){

  static bool isInitial = true;
  static int prevHeading = 0;
  static int num_frames_same_dir = 0;

  if(isInitial){
    prevHeading = getShipDir(ship_id);
    num_frames_same_dir = 0;
    isInitial = false;
  } else{

    //Compare current direction to previous direction
    int currentHeading = getShipDir(ship_id);
    if(currentHeading == prevHeading){
      num_frames_same_dir += 1;
    }else{
      num_frames_same_dir = 0;
      prevHeading = currentHeading;
    }

    int threshold = 14*3; //14 fps * 3 = 3 seconds
    if(num_frames_same_dir < threshold){
      return true;

      //Return false if you have been in the same direction for the threshold limit
    }else{
      isInitial = true;
      char LogFile[20] = "";
      sprintf(LogFile, "./logs/LOG%d.txt", selfID());
      FILE *fp;
      fp = fopen(LogFile, "a");
      fprintf(fp,"Bee finished Danced\n");
      fclose(fp);
      return false;
    }
  }
}

char* observeDanceMoves(int ship_id){

  char LogFile[20] = "";
  sprintf(LogFile, "./logs/LOG%d.txt", selfID());
  FILE *fp;
  fp = fopen(LogFile, "a");

  //Local Variables
  static bool is_initial_setup = true;
  static int initial_heading = 0;
  static int left_heading = 0;
  static int right_heading = 0;
  static int space_heading = 0;
  static char direction = 0;
  static char dance_moves[max_dance_moves];
  static int dance_index = 0;
  static bool directionSet = false;


  //Reset all static variables
  if(is_initial_setup){
    initial_heading = getShipDir(ship_id);      //Record initial heading as start of dance orientation
    right_heading = (initial_heading - 90 + 360) % 360;     //+360 to account for going past -1 degrees
    left_heading = (initial_heading + 90) % 360;
    space_heading = (initial_heading + 180) % 360;
    dance_index = 0;
    directionSet = false;
    direction = 0;
    fprintf(fp,"Observing Dance\nInitial Heading: %d\n",initial_heading);
    is_initial_setup = false;
    memset(dance_moves,'\0',max_dance_moves);
  }

  /**Get Dance Motions*/
  //Get current heading of ship being observed
  int observees_heading = getShipDir(ship_id);

  //Determine if observed bees heading is near their initial to signify end of symbol or start
  if(headingIsBetween(observees_heading, initial_heading-2, initial_heading+2)){
    //If a direction has not been set then do nothing because a valid move has not been observed
    if(!directionSet){
      //You have not began observing yet or recorded direction
      //NO ACTION NEEDED
    }else{
      //Otherwise you have already been observing and determine char
      fprintf(fp,"Storing Direction #%d: %c\n", dance_index+1,direction);
      dance_moves[dance_index] = direction; //Record move observed in array
      dance_index++;                        //Increment array index
      directionSet = false;                 //Resets if direction has been observed
      direction = 0;                        //Resets direction
    }
  }
  //If observee bee is not near intitial heading you are recording the max distance away
  //Determine if turning l/r/or word space or returning to initial
  if(headingIsBetween(observees_heading, left_heading-5, left_heading+5) && !directionSet){
    direction = left;
    directionSet = true;
  }
  //Record if at a right heading
  if(headingIsBetween(observees_heading, right_heading-5, right_heading+5) && !directionSet){
    direction = right;
    directionSet = true;
  }
  //Record if at heading indicating a space in the message
  if(headingIsBetween(observees_heading, space_heading-5, space_heading +5)){
    direction = endOfWord;
    directionSet = true;
  }

  fclose(fp);
  return dance_moves;
}

int interpretCoord(char coord, char* dance){

  char moves[3] = {'\0'};
  char number[3] = {'\0'};
  int number_index = 0;
  bool completed_coord = false;

  //Interpret Dance for x coordinate (starts at index 1)
  if(coord == 'x'){
    int dance_index = X_COORD_START;
    int move_index = 0;

    //Coord is considered completed when you have reached the end of word
    while(!completed_coord){
      moves[move_index] = dance[dance_index];
      dance_index++;
      move_index++;
      if(dance[dance_index] == endOfWord){
        number[number_index] = convertToInt(moves);
        memset(moves, '\0', 3);
        dance_index++;
        if(dance[dance_index] == endOfWord){
          completed_coord = true;
        }
      }
    }
  }else if(coord == 'y'){

  }else{
    return -1;
  }
  char * pEnd;
  return strtol (number,&pEnd,10);
}

char convertToInt(char* moves){
  int i = 0;
  int curr_tree_node = 0;
  while(i < 3 || moves[i] != '\0'){

    //Move down right
    if(moves[i] == left){
      curr_tree_node = (curr_tree_node + 1) * 2;
    }else{
      curr_tree_node = (curr_tree_node + 1) * 2 + 1;
    }
    //move down left
    i++;
  }
  return danceTree[curr_tree_node];
}