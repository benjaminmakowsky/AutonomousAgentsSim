//
// Created by makowskyb on 8/17/19.
//

#include "beeDance.h"
#include "beeObject.h"
#include "cAI.h"

//Local Variables
static char danceMoves[4] = {none,none,none,none}; //Array to hold dance moves
int msgType[2] = {foundSource, foundEnemy};
int danceTree[10] = {0,1,2,3,4,5,6,7,8,9};
static int initialHeading = 0;
static int rightHeading = 0;
static int leftHeading = 0;


bool dance(int msgType){
    static bool completed_first_dance = false;
    static bool completed_second_dance = false;
    static bool completed_third_dance = false;
    static bool isInitial = true;

    if(isInitial){
        completed_first_dance = false;
        completed_second_dance = false;
        completed_third_dance = false;
        initialHeading = (int)selfHeadingDeg();
        rightHeading = (initialHeading+90) % 360;
        leftHeading = (initialHeading-90) % 360;
    }

    //Make sure you are fully stopped before dancing
    if(selfSpeed() == 0) {

        switch (msgType) {
            case foundSource:
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
        setPower(0);
    }

    //Stop dancing once dance has finished
//    if (dance_is_completed) {
//        dance_is_completed = false;
//        return true;
//    } else {
//        return false;
//    }
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
        //determine which direction
        //move in that direction
        //Once in direction stop
    }
    if(finishedMove){
        //reset for next msg
        isInitial = true;
        return finishedMove;
    }

}



