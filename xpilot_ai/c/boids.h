//
// Created by makowskyb on 6/20/19.
//

#ifndef XPILOT_LE_BOIDS_H
#define XPILOT_LE_BOIDS_H

//global variables
extern int cWeight;
extern int cRadius;

//(friend) alignment variables
extern int aWeight;
extern int aRadius;

//(friend) separation variables
extern int sWeight;
extern int sRadius;


//(enemy) separation variables
extern int eWeight;
extern int eRadius;

//field (angle) of vision
extern int fov;

//fuel levels
extern double fuel;

//String used fo debugging
extern char bugstring[50];


extern int frameCount;		//how many frames have elapsed

extern int degToAim;	    //what direction do we want to go

extern int turnLock;		//time not allowed to compute new wall avoidance

extern int wallVector;		//where to go to avoid crashing into a wall



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


#endif //XPILOT_LE_BOIDS_H