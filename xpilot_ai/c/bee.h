//
// Created by makowskyb on 7/2/19.
//

#ifndef XPILOT_LE_BEE_H
#define XPILOT_LE_BEE_H

/*
 *  Searches for a hive by constantly attempting to refuel and when
 *  it detects that fuel levels have increased it stops
 *  @Return: Void*/
void searching();   //Searches for a hive


/*  int* pinpoint()
 *  Pinpoints the exact location of a fuel source/hive
 *  @Returns: Array of size 2 with the x and y coordinates of hive */
int* pinpoint();


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




int* getPOICoordinates(int x ,int y);

#endif //XPILOT_LE_BEE_H


