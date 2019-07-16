//
// Created by makowskyb on 6/20/19.
//

#ifndef XPILOT_LE_BEEBOIDS_H
#define XPILOT_LE_BEEBOIDS_H

#include <stdbool.h>

//Machine State
enum State
{
    STATE_INIT,     //0
    STATE_FLYING,   //1
    STATE_DEAD,     //2
    STATE_SEARCHING,//3
    STATE_FORAGING  //4
};

extern enum State state;

//Public Variables
extern int cWeight;         //(friend) cohesion variables
extern int cRadius;         //(friend) cohesion variables
extern int aWeight;         //(friend) alignment variables
extern int aRadius;         //(friend) alignment variables
extern int sWeight;         //(friend) separation variables
extern int sRadius;         //(friend) separation variables
extern int eWeight;         //(enemy) separation variables
extern int eRadius;         //(enemy) separation variables
extern int fov;             //field (angle) of vision
extern double fuel;         //fuel levels
extern char bugstring[50];  //String used for debugging
extern int frameCount;		//how many frames have elapsed
extern int degToAim;	    //what direction do we want to go
extern int turnLock;		//time not allowed to compute new wall avoidance
extern int wallVector;		//where to go to avoid crashing into a wall
extern int ship_states[50][2];
//Local Globals
extern int idx;			        //what's our unique idx number
extern int pVector;		    //information on the most recent (past) heading
extern bool isLeader;		//whether this drone is a leader
extern bool init;		    //have we initialized yet
extern int teamNum;			    //what team do we belong to
extern int leaderMode;		    //whether we care just about leaders for flocking
extern int tot_idx;			    //how many drones are on our team
extern char LogFile[15];   //Name of the lo file to create

//function prototypes
void initialize();
void wallAvoidance();
void alignment();
void cohesion();
void separation();
void flocking();
void cWeightAdjustByDistance();
void sWeightAdjustByDistance();
void eWeightAdjustByDistance();
void handleMsgBuffer();


#endif //XPILOT_LE_BEEBOIDS_H