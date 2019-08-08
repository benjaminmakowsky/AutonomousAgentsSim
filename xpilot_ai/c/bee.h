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
 *  Waits at hive and observes incoming bee's dances
 *  @Return: void*/
void onlook();



#endif //XPILOT_LE_BEE_H


