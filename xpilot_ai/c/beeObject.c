//
// Created by makowskyb on 7/22/19.
//

#include "beeObject.h"
static int honeyX = 0;
static int honeyY = 0;
static int currState = 0;
static int prevState = 0;
static int isDancing = 0;

void setHoneyX(int x){
  honeyX = x;
}

void setHoneyY(int y){
  honeyY = y;
}

int getHoneyX(){
  return honeyX;
}

int getHoneyY(){
  return honeyY;
}

int getCurrState(){
  return currState;
}

void setCurrState(int state){
  prevState = getCurrState();
  currState = state;
}

void setIsDancing(int answer){
  isDancing = answer;
}
int getIsDancing(){
  return isDancing;
}