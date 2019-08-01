//
// Created by makowskyb on 7/2/19.
//

#ifndef XPILOT_LE_BEE_H
#define XPILOT_LE_BEE_H

#include <stdbool.h>
/*
 *  Searches for a hive by constantly attempting to refuel and when
 *  it detects that fuel levels have increased it stops
 *  @Return: Void*/
void searching();

/*
 *  Forages back and forth from base to fuel source until source is empty
 *  @Return: Void*/
void forage();

/*
 *
 */
void onlook();


// Add comment explaining what this function does
int* getPOICoordinates(int x ,int y);

#endif //XPILOT_LE_BEE_H


