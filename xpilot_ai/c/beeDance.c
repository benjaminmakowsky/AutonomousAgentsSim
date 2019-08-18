//
// Created by makowskyb on 8/17/19.
//

#include "beeDance.h"
#include "beeObject.h"
#include "cAI.h"
#include "beeGlobals.h"

//Local Variables
static char danceMoves[4] = {none,none,none,none}; //Array to hold dance moves
int msgTypes[2] = {foundSource, foundEnemy};
int danceTree[10] = {0,1,2,3,4,5,6,7,8,9};
static int initialHeading = 0;
static int rightHeading = 0;
static int leftHeading = 0;


bool dance(int msgType){
    static bool completed_first_dance = false;
    static bool completed_second_dance = false;
    static bool completed_third_dance = false;
    static bool isInitial = true;
    OPENLOG()

    if(isInitial){
        fprintf(fp, "Intializing beeDance() with msgType: %d\n",msgType);
        completed_first_dance = false;
        completed_second_dance = false;
        completed_third_dance = false;
        initialHeading = (int)selfHeadingDeg();
        rightHeading = (initialHeading+90) % 360;
        leftHeading = (initialHeading-90) % 360;
        isInitial = false;
    }

    //Make sure you are fully stopped before dancing
    if(selfSpeed() == 0) {

        switch (msgType) {
            case foundSource:
                fprintf(fp,"Found Source Dance");
                if(!completed_first_dance){
                    completed_first_dance = relayMsg(msgType);
                }else if(!completed_second_dance){

                }else if(!completed_third_dance){

                }
                //dance_is_completed = honeyFoundDance();
                break;
            case foundEnemy:
                break;
        }
    }else{
        //IF you haven't stopped moving make sure the power is off
        setPower(0);
    }

    //Stop dancing once dance has finished
    if (completed_first_dance && completed_second_dance && completed_third_dance) {
        isInitial = true;
        return true;
    } else {
        return false;
    }
}


char* buildDance(int msgType){

}

//Helper Methods
int getParent(int child){
    return danceTree[child/2 -1];
}

bool relayMsg(int symbol){
    static bool finishedMove = false;
    static bool isInitial = true;

    if(isInitial){
        finishedMove = false;
        isInitial = false;

    }else if(!finishedMove){
        //determine which direction to dance
        char danceDirection = none;
        //If 0 turn left if 1 turn right
        danceDirection = (symbol == 0) ?  left : right;
        //move in that direction
        if(danceDirection == left){
            turnToDeg(leftHeading - 1);
        }else{
            turnToDeg(rightHeading + 1);
        }
        //Once in direction stop
        if(danceDirection == left){
            finishedMove = selfHeadingDeg() > leftHeading;
        }else{
            finishedMove = selfHeadingDeg() > rightHeading;
        }

    }
    if(finishedMove){
        //reset for next msg
        isInitial = true;
    }
    return finishedMove;

}



