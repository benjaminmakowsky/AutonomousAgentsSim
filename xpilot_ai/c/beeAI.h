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

typedef struct fuelStruct_t {
    int x;
    int y;
    int num_fuels;
} FuelStruct_t;


extern BaseStruct_t* hives;
extern FuelStruct_t* honey_spots;

/***********************
 * Function Declarations
 ***********************/

// The GNU standard states that comments describing the function should 
// be immediately before the function header 
// ( see https://www.gnu.org/prep/standards/html_node/Comments.html )
// Move the comment descriptions to immediately above the function.

extern BaseStruct_t* getBases();
/// Creates array of hive coordinates

extern FuelStruct_t* getFuelDepots();
/// Creates array of honey coordinatess

int goToCoordinates(int x, int y);
/// Moves user in the direction of a point constantly adjusting heading
/// \param x x coordinate of where to go
/// \param y y coordinate of where to go
/// \return The current heading of the user

int getHeadingForCoordinates(int x, int y);



int* getPOICoordinates(int x ,int y);
/// Determines the coordinates of the closest POI to the user
/// \param x The current x coordinate of the user
/// \param y The current y coordinate of the user
/// \return 1D array of size 2 where [0] = x and [1] = y

bool inVicinityOf(int x,int y);
/// Used to determine whether or not user is within a certain range of (x,y)
/// \param x The x coordinate of the POI coordinate
/// \param y The y coordinate of the POI coordinate
/// \return  Boolean of whether or not user is within range of (x,y)

int avoidWalls();
/// Used to determine the vector of the wall and turn to a specified degree
/// \return The new heading


bool comeToStop(int number_of_frames);
/// Brings bee to a stop by setting power to zero and wait until no movement
/// \param number_of_frames The number of frames to not be moving
/// \return boolean if stopped or not

void checkForFuel();
///Automatically cycles refueling state to determine if near honey


//void log(char* string);
/// Prints a string to the log file
/// \param String to be printed to file


bool dance(int prevState);
/// Performs a set of movements based on the state before dancing
/// \param prevState The state the bee was in before initiating dance
/// \return boolean if dance has been completed or not


bool honeyFoundDance();
/// Performs dance to relay that honey has been found
/// \return boolean if dance has been completed or not

bool beeDegIsBetween(int deg1, int deg2);
#endif //XPILOT_LE_BEEAI_H
