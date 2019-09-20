//
// Created by makowskyb on 7/22/19.
//

#ifndef XPILOT_LE_BEEOBJECT_H
#define XPILOT_LE_BEEOBJECT_H

#include <stdbool.h>

static int honeyX;      //X coordinate of honey source
static int honeyY;      //Y coordinate of honey source
static int currState;
static int prev;
static int isDancing;
static int danceType;
static char* observed_dance;           //Pointer to the dance observed
static bool saw_dance;

void setHoneyX(int x);
void setHoneyY(int y);

int getHoneyX();
int getHoneyY();

int getCurrState();
void setCurrState(int state);

void setIsDancing(int answer);
int getIsDancing();

void setDanceType(int type);
int getDanceType();

void rememberObservedDanceMoves(char* dance);

bool getSaw_Dance();
void setSaw_Dance(bool seen);
#endif //XPILOT_LE_BEEOBJECT_H
