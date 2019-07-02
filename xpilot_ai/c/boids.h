//
// Created by makowskyb on 6/20/19.
//


//global variables
int cWeight = 0;
int cRadius = 400;

//(friend) alignment variables
int aWeight = 0;
int aRadius = 400;

//(friend) separation variables
int sWeight = 0;
int sRadius = 200;


//(enemy) separation variables
int eWeight = 0;
int eRadius = 200;

//field (angle) of vision
int fov = 60;

//fuel levels
double fuel = 0;

//String used fo debugging
char bugstring[50] = "Init";

extern int frameCount;		//how many frames have elapsed
int frameCount = 0;

extern int degToAim;	    //what direction do we want to go
int degToAim = -1;

extern int turnLock;		//time not allowed to compute new wall avoidance
int turnLock = 0;

extern int wallVector;		//where to go to avoid crashing into a wall
int wallVector = -1;
