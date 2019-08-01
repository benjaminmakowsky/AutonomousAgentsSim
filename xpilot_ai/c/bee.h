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
 *  goToCoordinates(x,y)
 *  Moves the bee to a specified location on the map.
 *  @Param int x: The x coordinate of the location
 *  @Param int y: The y coordinate of the locaton
 *  @Return: int: current heading in degrees
 *  NOTE: Due to map constraints location will be + or - 5 */
int goToCoordinates(int x, int y);


/*
 *  getHeadingForCoordinates(x , y)
 *  Gets the heading for the the coordinates (x,y)
 *  @Param int x: The x coordinate of the location
 *  @Param int y: The y coordinate of the locaton
 *  @Return: int: heading of coordinates in degrees*/
int getHeadingForCoordinates(int x ,int y);

/*
 *  inVicinityOf(x, y)
 *  Determines whether or not the ship is close enough to a position to act upon
 *  @Param int x: The x coordinate of the location
 *  @Param int y: The y coordinate of the locaton
 *  @Return: bool - Boolean value if ships current position is close to x ,y*/
bool inVicinityOf(int x, int y);



// Add comment explaining what this function does
int* getPOICoordinates(int x ,int y);

#endif //XPILOT_LE_BEE_H


