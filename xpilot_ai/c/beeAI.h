///Author: Benjamin Makowsky
#ifndef XPILOT_LE_BEEAI_H
#define XPILOT_LE_BEEAI_H

#include <stdbool.h>

// Structs to hold the base coordinates &
// the fuel depot coordinates
typedef struct baseStruct_t {
    int team;
    int x;
    int y;
    int num_bases;
} BaseStruct_t;
//extern BaseStruct_t *hives; //global array for hives

typedef struct fuelStruct_t {
    int x;
    int y;
    int num_fuels;
} FuelStruct_t;


extern BaseStruct_t* bases;
extern FuelStruct_t* fuels;

extern BaseStruct_t* getBases();      // Creates array of hive coordinates
extern FuelStruct_t* getFuelDepots(); // Creates array of honey coordinatess

/// Moves user in the direction of a point constantly adjusting heading
/// \param x x coordinate of where to go
/// \param y y coordinate of where to go
/// \return The current heading of the user
int goToCoordinates(int x, int y);

int getHeadingForCoordinates(int x, int y);

/// Determines the coordinates of the closest POI to the user
/// \param x The current x coordinate of the user
/// \param y The current y coordinate of the user
/// \return 1D array of size 2 where [0] = x and [1] = y
int* getPOICoordinates(int x ,int y);

/// Used to determine whether or not user is within a certain range of (x,y)
/// \param x The x coordinate of the POI coordinate
/// \param y The y coordinate of the POI coordinate
/// \return  Boolean of whether or not user is within range of (x,y)
bool inVicinityOf(int x,int y);


#endif //XPILOT_LE_BEEAI_H
