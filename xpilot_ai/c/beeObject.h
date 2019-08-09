//
// Created by makowskyb on 7/22/19.
//

#ifndef XPILOT_LE_BEEOBJECT_H
#define XPILOT_LE_BEEOBJECT_H

static int honeyX;
static int honeyY;
static int currState;
static int prev;
static int isDancing;
static int danceType;

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
#endif //XPILOT_LE_BEEOBJECT_H
