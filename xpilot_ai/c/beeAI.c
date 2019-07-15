/// Author: Benjamin Makowsky
#include <stdio.h>
#include <stdlib.h>
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
  BaseStruct_t* bases = malloc(numBases * sizeof(BaseStruct_t));
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
    bases[i] = newBase;
  }
  if(fclose(fp) == -1){
    printf("Failed to close");
    return NULL;
  }

  return bases;
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
  FuelStruct_t* fuels = malloc(numFuels * sizeof(FuelStruct_t));
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
    fuels[i] = newFuel;

  }

  if(fclose(fp) == -1){
    printf("Could not close for Fuel struct reading");
    return NULL;
  }
  return fuels;

}

