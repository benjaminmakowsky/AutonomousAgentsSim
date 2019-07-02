//
// Created by makowskyb on 6/20/19.
//

#ifndef XPILOT_LE_BOIDS_H
#define XPILOT_LE_BOIDS_H


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
extern char bugstring[50];  //String used fo debugging
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