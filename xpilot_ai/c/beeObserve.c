//
// Created by makowskyb on 9/17/19.
//
#include "cAI.h"
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
    fp = fopen(LogFile, "a");
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
      setHoneyY(interpretCoord('x',dancePattern));
      setHoneyY(interpretCoord('y',dancePattern));
      //sethoneyx and y in beeobject
      //TODO: create dance type enumeration

    }
  }
  return dance_observed;
}

#include "beeObserve.h"
