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




extern BaseStruct_t* getBases();      // Creates array of hive coordinates
extern FuelStruct_t* getFuelDepots(); // Creates array of honey coordinatess

//Self methods
extern int selfFuelX();             // Returns ships designated fuel source x coord - Ben
extern int selfFuelY();             // Returns ships designated fuel source y coord - Ben
extern void setSelfState(int state);//review(Deprecated?)
extern int selfState();             //review(Deprecated?)


#endif //XPILOT_LE_BEEAI_H
