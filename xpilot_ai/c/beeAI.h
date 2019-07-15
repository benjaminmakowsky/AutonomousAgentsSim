///Author: Benjamin Makowsky
#ifndef XPILOT_LE_BEEAI_H
#define XPILOT_LE_BEEAI_H

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

extern int start(int argc, char* argv[]); // Initialize AI interface and start XPilot -JRA

//Movement Methods
extern void turnToDeg(int deg); // Turns the ship to the inputed Degree -JRA
extern void setPower(double s); // Sets the speed the ship will thrust by, the minimum power level is 5.0 and the maximum power is 55.0 -JRA

//Environment Methods
extern void refuel(int flag);         // Used to gather honey
extern double selfFuel();             // Returns the ships current fuel level
extern BaseStruct_t* getBases();      // Creates array of hive coordinates
extern FuelStruct_t* getFuelDepots(); // Creates array of honey coordinatess

//Self methods
extern int selfX();                 // Returns the ship's X Position -JRA
extern int selfY();                 // Returns the ship's Y Position -JRA
extern int selfBaseX();             // Returns ship's base x coordinate
extern int selfBaseY();             // Returns ship's base y coordinate
extern int selfFuelX();             // Returns ships designated fuel source x coord - Ben
extern int selfFuelY();             // Returns ships designated fuel source y coord - Ben
extern void setSelfState(int state);//review(Deprecated?)
extern int selfState();             //review(Deprecated?)


#endif //XPILOT_LE_BEEAI_H
